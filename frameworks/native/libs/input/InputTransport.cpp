//
// Copyright 2010 The Android Open Source Project
//
// Provides a shared memory transport for input events.
//
#define LOG_TAG "InputTransport"

//#define LOG_NDEBUG 0

// Log debug messages about channel messages (send message, receive message)
#define DEBUG_CHANNEL_MESSAGES 0

// Log debug messages whenever InputChannel objects are created/destroyed
#define DEBUG_CHANNEL_LIFECYCLE 0

// Log debug messages about transport actions
#define DEBUG_TRANSPORT_ACTIONS 0

// Log debug messages about touch event resampling
#define DEBUG_RESAMPLING 0

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <android-base/stringprintf.h>
#include <binder/Parcel.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <utils/Trace.h>

#include <input/InputTransport.h>

using android::base::StringPrintf;

namespace android {

// Socket buffer size.  The default is typically about 128KB, which is much larger than
// we really need.  So we make it smaller.  It just needs to be big enough to hold
// a few dozen large multi-finger motion events in the case where an application gets
// behind processing touches.
static const size_t SOCKET_BUFFER_SIZE = 32 * 1024;

// Nanoseconds per milliseconds.
static const nsecs_t NANOS_PER_MS = 1000000;

// Latency added during resampling.  A few milliseconds doesn't hurt much but
// reduces the impact of mispredicted touch positions.
static const nsecs_t RESAMPLE_LATENCY = 5 * NANOS_PER_MS;

// Minimum time difference between consecutive samples before attempting to resample.
static const nsecs_t RESAMPLE_MIN_DELTA = 2 * NANOS_PER_MS;

// Maximum time difference between consecutive samples before attempting to resample
// by extrapolation.
static const nsecs_t RESAMPLE_MAX_DELTA = 20 * NANOS_PER_MS;

// Maximum time to predict forward from the last known state, to avoid predicting too
// far into the future.  This time is further bounded by 50% of the last time delta.
static const nsecs_t RESAMPLE_MAX_PREDICTION = 8 * NANOS_PER_MS;

/**
 * System property for enabling / disabling touch resampling.
 * Resampling extrapolates / interpolates the reported touch event coordinates to better
 * align them to the VSYNC signal, thus resulting in smoother scrolling performance.
 * Resampling is not needed (and should be disabled) on hardware that already
 * has touch events triggered by VSYNC.
 * Set to "1" to enable resampling (default).
 * Set to "0" to disable resampling.
 * Resampling is enabled by default.
 */
static const char* PROPERTY_RESAMPLING_ENABLED = "ro.input.resampling";

template<typename T>
inline static T min(const T& a, const T& b) {
    return a < b ? a : b;
}

inline static float lerp(float a, float b, float alpha) {
    return a + alpha * (b - a);
}

inline static bool isPointerEvent(int32_t source) {
    return (source & AINPUT_SOURCE_CLASS_POINTER) == AINPUT_SOURCE_CLASS_POINTER;
}

// --- InputMessage ---

bool InputMessage::isValid(size_t actualSize) const {
    if (size() == actualSize) {
        switch (header.type) {
        case TYPE_KEY:
            return true;
        case TYPE_MOTION:
            return body.motion.pointerCount > 0
                    && body.motion.pointerCount <= MAX_POINTERS;
        case TYPE_FINISHED:
            return true;
        }
    }
    return false;
}

size_t InputMessage::size() const {
    switch (header.type) {
    case TYPE_KEY:
        return sizeof(Header) + body.key.size();
    case TYPE_MOTION:
        return sizeof(Header) + body.motion.size();
    case TYPE_FINISHED:
        return sizeof(Header) + body.finished.size();
    }
    return sizeof(Header);
}

/**
 * There could be non-zero bytes in-between InputMessage fields. Force-initialize the entire
 * memory to zero, then only copy the valid bytes on a per-field basis.
 */
void InputMessage::getSanitizedCopy(InputMessage* msg) const {
    memset(msg, 0, sizeof(*msg));

    // Write the header
    msg->header.type = header.type;

    // Write the body
    switch(header.type) {
        case InputMessage::TYPE_KEY: {
            // uint32_t seq
            msg->body.key.seq = body.key.seq;
            // nsecs_t eventTime
            msg->body.key.eventTime = body.key.eventTime;
            // int32_t deviceId
            msg->body.key.deviceId = body.key.deviceId;
            // int32_t source
            msg->body.key.source = body.key.source;
            // int32_t displayId
            msg->body.key.displayId = body.key.displayId;
            // int32_t action
            msg->body.key.action = body.key.action;
            // int32_t flags
            msg->body.key.flags = body.key.flags;
            // int32_t keyCode
            msg->body.key.keyCode = body.key.keyCode;
            // int32_t scanCode
            msg->body.key.scanCode = body.key.scanCode;
            // int32_t metaState
            msg->body.key.metaState = body.key.metaState;
            // int32_t repeatCount
            msg->body.key.repeatCount = body.key.repeatCount;
            // nsecs_t downTime
            msg->body.key.downTime = body.key.downTime;
            break;
        }
        case InputMessage::TYPE_MOTION: {
            // uint32_t seq
            msg->body.motion.seq = body.motion.seq;
            // nsecs_t eventTime
            msg->body.motion.eventTime = body.motion.eventTime;
            // int32_t deviceId
            msg->body.motion.deviceId = body.motion.deviceId;
            // int32_t source
            msg->body.motion.source = body.motion.source;
            // int32_t displayId
            msg->body.motion.displayId = body.motion.displayId;
            // int32_t action
            msg->body.motion.action = body.motion.action;
            // int32_t actionButton
            msg->body.motion.actionButton = body.motion.actionButton;
            // int32_t flags
            msg->body.motion.flags = body.motion.flags;
            // int32_t metaState
            msg->body.motion.metaState = body.motion.metaState;
            // int32_t buttonState
            msg->body.motion.buttonState = body.motion.buttonState;
            // MotionClassification classification
            msg->body.motion.classification = body.motion.classification;
            // int32_t edgeFlags
            msg->body.motion.edgeFlags = body.motion.edgeFlags;
            // nsecs_t downTime
            msg->body.motion.downTime = body.motion.downTime;
            // float xOffset
            msg->body.motion.xOffset = body.motion.xOffset;
            // float yOffset
            msg->body.motion.yOffset = body.motion.yOffset;
            // float xPrecision
            msg->body.motion.xPrecision = body.motion.xPrecision;
            // float yPrecision
            msg->body.motion.yPrecision = body.motion.yPrecision;
            // uint32_t pointerCount
            msg->body.motion.pointerCount = body.motion.pointerCount;
            //struct Pointer pointers[MAX_POINTERS]
            for (size_t i = 0; i < body.motion.pointerCount; i++) {
                // PointerProperties properties
                msg->body.motion.pointers[i].properties.id = body.motion.pointers[i].properties.id;
                msg->body.motion.pointers[i].properties.toolType =
                        body.motion.pointers[i].properties.toolType,
                // PointerCoords coords
                msg->body.motion.pointers[i].coords.bits = body.motion.pointers[i].coords.bits;
                const uint32_t count = BitSet64::count(body.motion.pointers[i].coords.bits);
                memcpy(&msg->body.motion.pointers[i].coords.values[0],
                        &body.motion.pointers[i].coords.values[0],
                        count * (sizeof(body.motion.pointers[i].coords.values[0])));
            }
            break;
        }
        case InputMessage::TYPE_FINISHED: {
            msg->body.finished.seq = body.finished.seq;
            msg->body.finished.handled = body.finished.handled;
            break;
        }
        default: {
            LOG_FATAL("Unexpected message type %i", header.type);
            break;
        }
    }
}

// --- InputChannel ---

InputChannel::InputChannel(const std::string& name, int fd) :
        mName(name) {
#if DEBUG_CHANNEL_LIFECYCLE
    ALOGD("Input channel constructed: name='%s', fd=%d",
            mName.c_str(), fd);
#endif

    setFd(fd);
}

InputChannel::~InputChannel() {
#if DEBUG_CHANNEL_LIFECYCLE
    ALOGD("Input channel destroyed: name='%s', fd=%d",
            mName.c_str(), mFd);
#endif

    ::close(mFd);
}

void InputChannel::setFd(int fd) {
    if (mFd > 0) {
        ::close(mFd);
    }
    mFd = fd;
    if (mFd > 0) {
        int result = fcntl(mFd, F_SETFL, O_NONBLOCK);
        LOG_ALWAYS_FATAL_IF(result != 0, "channel '%s' ~ Could not make socket "
            "non-blocking.  errno=%d", mName.c_str(), errno);
    }
}

status_t InputChannel::openInputChannelPair(const std::string& name,
        sp<InputChannel>& outServerChannel, sp<InputChannel>& outClientChannel) {
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sockets)) {
        status_t result = -errno;
        ALOGE("channel '%s' ~ Could not create socket pair.  errno=%d",
                name.c_str(), errno);
        outServerChannel.clear();
        outClientChannel.clear();
        return result;
    }

    int bufferSize = SOCKET_BUFFER_SIZE;
    setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[0], SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[1], SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(sockets[1], SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));

    std::string serverChannelName = name;
    serverChannelName += " (server)";
    outServerChannel = new InputChannel(serverChannelName, sockets[0]);

    std::string clientChannelName = name;
    clientChannelName += " (client)";
    outClientChannel = new InputChannel(clientChannelName, sockets[1]);
    return OK;
}

status_t InputChannel::sendMessage(const InputMessage* msg) {
    const size_t msgLength = msg->size();
    InputMessage cleanMsg;
    msg->getSanitizedCopy(&cleanMsg);
    ssize_t nWrite;
    do {
        nWrite = ::send(mFd, &cleanMsg, msgLength, MSG_DONTWAIT | MSG_NOSIGNAL);
    } while (nWrite == -1 && errno == EINTR);

    if (nWrite < 0) {
        int error = errno;
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ error sending message of type %d, errno=%d", mName.c_str(),
                msg->header.type, error);
#endif
        if (error == EAGAIN || error == EWOULDBLOCK) {
            return WOULD_BLOCK;
        }
        if (error == EPIPE || error == ENOTCONN || error == ECONNREFUSED || error == ECONNRESET) {
            return DEAD_OBJECT;
        }
        return -error;
    }

    if (size_t(nWrite) != msgLength) {
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ error sending message type %d, send was incomplete",
                mName.c_str(), msg->header.type);
#endif
        return DEAD_OBJECT;
    }

#if DEBUG_CHANNEL_MESSAGES
    ALOGD("channel '%s' ~ sent message of type %d", mName.c_str(), msg->header.type);
#endif
    return OK;
}

status_t InputChannel::receiveMessage(InputMessage* msg) {
    ssize_t nRead;
    do {
        nRead = ::recv(mFd, msg, sizeof(InputMessage), MSG_DONTWAIT);
    } while (nRead == -1 && errno == EINTR);

    if (nRead < 0) {
        int error = errno;
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ receive message failed, errno=%d", mName.c_str(), errno);
#endif
        if (error == EAGAIN || error == EWOULDBLOCK) {
            return WOULD_BLOCK;
        }
        if (error == EPIPE || error == ENOTCONN || error == ECONNREFUSED) {
            return DEAD_OBJECT;
        }
        return -error;
    }

    if (nRead == 0) { // check for EOF
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ receive message failed because peer was closed", mName.c_str());
#endif
        return DEAD_OBJECT;
    }

    if (!msg->isValid(nRead)) {
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ received invalid message", mName.c_str());
#endif
        return BAD_VALUE;
    }

#if DEBUG_CHANNEL_MESSAGES
    ALOGD("channel '%s' ~ received message of type %d", mName.c_str(), msg->header.type);
#endif
    return OK;
}

sp<InputChannel> InputChannel::dup() const {
    int fd = ::dup(getFd());
    return fd >= 0 ? new InputChannel(getName(), fd) : nullptr;
}


status_t InputChannel::write(Parcel& out) const {
    status_t s = out.writeString8(String8(getName().c_str()));

    if (s != OK) {
        return s;
    }
    s = out.writeStrongBinder(mToken);
    if (s != OK) {
        return s;
    }

    s = out.writeDupFileDescriptor(getFd());

    return s;
}

status_t InputChannel::read(const Parcel& from) {
    mName = from.readString8();
    mToken = from.readStrongBinder();

    int rawFd = from.readFileDescriptor();
    setFd(::dup(rawFd));

    if (mFd < 0) {
        return BAD_VALUE;
    }

    return OK;
}

sp<IBinder> InputChannel::getToken() const {
    return mToken;
}

void InputChannel::setToken(const sp<IBinder>& token) {
    if (mToken != nullptr) {
        ALOGE("Assigning InputChannel (%s) a second handle?", mName.c_str());
    }
    mToken = token;
}

// --- InputPublisher ---

InputPublisher::InputPublisher(const sp<InputChannel>& channel) :
        mChannel(channel) {
}

InputPublisher::~InputPublisher() {
}

status_t InputPublisher::publishKeyEvent(
        uint32_t seq,
        int32_t deviceId,
        int32_t source,
        int32_t displayId,
        int32_t action,
        int32_t flags,
        int32_t keyCode,
        int32_t scanCode,
        int32_t metaState,
        int32_t repeatCount,
        nsecs_t downTime,
        nsecs_t eventTime) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf("publishKeyEvent(inputChannel=%s, keyCode=%" PRId32 ")",
                mChannel->getName().c_str(), keyCode);
        ATRACE_NAME(message.c_str());
    }
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' publisher ~ publishKeyEvent: seq=%u, deviceId=%d, source=0x%x, "
            "action=0x%x, flags=0x%x, keyCode=%d, scanCode=%d, metaState=0x%x, repeatCount=%d,"
            "downTime=%" PRId64 ", eventTime=%" PRId64,
            mChannel->getName().c_str(), seq,
            deviceId, source, action, flags, keyCode, scanCode, metaState, repeatCount,
            downTime, eventTime);
#endif

    if (!seq) {
        ALOGE("Attempted to publish a key event with sequence number 0.");
        return BAD_VALUE;
    }

    InputMessage msg;
    msg.header.type = InputMessage::TYPE_KEY;
    msg.body.key.seq = seq;
    msg.body.key.deviceId = deviceId;
    msg.body.key.source = source;
    msg.body.key.displayId = displayId;
    msg.body.key.action = action;
    msg.body.key.flags = flags;
    msg.body.key.keyCode = keyCode;
    msg.body.key.scanCode = scanCode;
    msg.body.key.metaState = metaState;
    msg.body.key.repeatCount = repeatCount;
    msg.body.key.downTime = downTime;
    msg.body.key.eventTime = eventTime;
    return mChannel->sendMessage(&msg);
}

status_t InputPublisher::publishMotionEvent(
        uint32_t seq,
        int32_t deviceId,
        int32_t source,
        int32_t displayId,
        int32_t action,
        int32_t actionButton,
        int32_t flags,
        int32_t edgeFlags,
        int32_t metaState,
        int32_t buttonState,
        MotionClassification classification,
        float xOffset,
        float yOffset,
        float xPrecision,
        float yPrecision,
        nsecs_t downTime,
        nsecs_t eventTime,
        uint32_t pointerCount,
        const PointerProperties* pointerProperties,
        const PointerCoords* pointerCoords) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf(
                "publishMotionEvent(inputChannel=%s, action=%" PRId32 ")",
                mChannel->getName().c_str(), action);
        ATRACE_NAME(message.c_str());
    }
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' publisher ~ publishMotionEvent: seq=%u, deviceId=%d, source=0x%x, "
            "displayId=%" PRId32 ", "
            "action=0x%x, actionButton=0x%08x, flags=0x%x, edgeFlags=0x%x, "
            "metaState=0x%x, buttonState=0x%x, classification=%s, xOffset=%f, yOffset=%f, "
            "xPrecision=%f, yPrecision=%f, downTime=%" PRId64 ", eventTime=%" PRId64 ", "
            "pointerCount=%" PRIu32,
            mChannel->getName().c_str(), seq,
            deviceId, source, displayId, action, actionButton, flags, edgeFlags, metaState,
            buttonState, motionClassificationToString(classification),
            xOffset, yOffset, xPrecision, yPrecision, downTime, eventTime, pointerCount);
#endif

    if (!seq) {
        ALOGE("Attempted to publish a motion event with sequence number 0.");
        return BAD_VALUE;
    }

    if (pointerCount > MAX_POINTERS || pointerCount < 1) {
        ALOGE("channel '%s' publisher ~ Invalid number of pointers provided: %" PRIu32 ".",
                mChannel->getName().c_str(), pointerCount);
        return BAD_VALUE;
    }

    InputMessage msg;
    msg.header.type = InputMessage::TYPE_MOTION;
    msg.body.motion.seq = seq;
    msg.body.motion.deviceId = deviceId;
    msg.body.motion.source = source;
    msg.body.motion.displayId = displayId;
    msg.body.motion.action = action;
    msg.body.motion.actionButton = actionButton;
    msg.body.motion.flags = flags;
    msg.body.motion.edgeFlags = edgeFlags;
    msg.body.motion.metaState = metaState;
    msg.body.motion.buttonState = buttonState;
    msg.body.motion.classification = classification;
    msg.body.motion.xOffset = xOffset;
    msg.body.motion.yOffset = yOffset;
    msg.body.motion.xPrecision = xPrecision;
    msg.body.motion.yPrecision = yPrecision;
    msg.body.motion.downTime = downTime;
    msg.body.motion.eventTime = eventTime;
    msg.body.motion.pointerCount = pointerCount;
    for (uint32_t i = 0; i < pointerCount; i++) {
        msg.body.motion.pointers[i].properties.copyFrom(pointerProperties[i]);
        msg.body.motion.pointers[i].coords.copyFrom(pointerCoords[i]);
    }
    return mChannel->sendMessage(&msg);
}

status_t InputPublisher::receiveFinishedSignal(uint32_t* outSeq, bool* outHandled) {
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' publisher ~ receiveFinishedSignal",
            mChannel->getName().c_str());
#endif

    InputMessage msg;
    status_t result = mChannel->receiveMessage(&msg);
    if (result) {
        *outSeq = 0;
        *outHandled = false;
        return result;
    }
    if (msg.header.type != InputMessage::TYPE_FINISHED) {
        ALOGE("channel '%s' publisher ~ Received unexpected message of type %d from consumer",
                mChannel->getName().c_str(), msg.header.type);
        return UNKNOWN_ERROR;
    }
    *outSeq = msg.body.finished.seq;
    *outHandled = msg.body.finished.handled;
    return OK;
}

// --- InputConsumer ---

InputConsumer::InputConsumer(const sp<InputChannel>& channel) :
        mResampleTouch(isTouchResamplingEnabled()),
        mChannel(channel), mMsgDeferred(false) {
}

InputConsumer::~InputConsumer() {
}

bool InputConsumer::isTouchResamplingEnabled() {
    return property_get_bool(PROPERTY_RESAMPLING_ENABLED, true);
}

status_t InputConsumer::consume(InputEventFactoryInterface* factory,
        bool consumeBatches, nsecs_t frameTime, uint32_t* outSeq, InputEvent** outEvent) {
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' consumer ~ consume: consumeBatches=%s, frameTime=%" PRId64,
            mChannel->getName().c_str(), consumeBatches ? "true" : "false", frameTime);
#endif

    *outSeq = 0;
    *outEvent = nullptr;

    // Fetch the next input message.
    // Loop until an event can be returned or no additional events are received.
    while (!*outEvent) {
        if (mMsgDeferred) {
            // mMsg contains a valid input message from the previous call to consume
            // that has not yet been processed.
            mMsgDeferred = false;
        } else {
            // Receive a fresh message.
            status_t result = mChannel->receiveMessage(&mMsg);
            if (result) {
                // Consume the next batched event unless batches are being held for later.
                if (consumeBatches || result != WOULD_BLOCK) {
                    result = consumeBatch(factory, frameTime, outSeq, outEvent);
                    if (*outEvent) {
#if DEBUG_TRANSPORT_ACTIONS
                        ALOGD("channel '%s' consumer ~ consumed batch event, seq=%u",
                                mChannel->getName().c_str(), *outSeq);
#endif
                        break;
                    }
                }
                return result;
            }
        }

        switch (mMsg.header.type) {
        case InputMessage::TYPE_KEY: {
            KeyEvent* keyEvent = factory->createKeyEvent();
            if (!keyEvent) return NO_MEMORY;

            initializeKeyEvent(keyEvent, &mMsg);
            *outSeq = mMsg.body.key.seq;
            *outEvent = keyEvent;
#if DEBUG_TRANSPORT_ACTIONS
            ALOGD("channel '%s' consumer ~ consumed key event, seq=%u",
                    mChannel->getName().c_str(), *outSeq);
#endif
            break;
        }

        case InputMessage::TYPE_MOTION: {
            ssize_t batchIndex = findBatch(mMsg.body.motion.deviceId, mMsg.body.motion.source);
            if (batchIndex >= 0) {
                Batch& batch = mBatches.editItemAt(batchIndex);
                if (canAddSample(batch, &mMsg)) {
                    batch.samples.push(mMsg);
#if DEBUG_TRANSPORT_ACTIONS
                    ALOGD("channel '%s' consumer ~ appended to batch event",
                            mChannel->getName().c_str());
#endif
                    break;
                } else if (isPointerEvent(mMsg.body.motion.source) &&
                        mMsg.body.motion.action == AMOTION_EVENT_ACTION_CANCEL) {
                    // No need to process events that we are going to cancel anyways
                    const size_t count = batch.samples.size();
                    for (size_t i = 0; i < count; i++) {
                        const InputMessage& msg = batch.samples.itemAt(i);
                        sendFinishedSignal(msg.body.motion.seq, false);
                    }
                    batch.samples.removeItemsAt(0, count);
                    mBatches.removeAt(batchIndex);
                } else {
                    // We cannot append to the batch in progress, so we need to consume
                    // the previous batch right now and defer the new message until later.
                    mMsgDeferred = true;
                    status_t result = consumeSamples(factory,
                            batch, batch.samples.size(), outSeq, outEvent);
                    mBatches.removeAt(batchIndex);
                    if (result) {
                        return result;
                    }
#if DEBUG_TRANSPORT_ACTIONS
                    ALOGD("channel '%s' consumer ~ consumed batch event and "
                            "deferred current event, seq=%u",
                            mChannel->getName().c_str(), *outSeq);
#endif
                    break;
                }
            }

            // Start a new batch if needed.
            if (mMsg.body.motion.action == AMOTION_EVENT_ACTION_MOVE
                    || mMsg.body.motion.action == AMOTION_EVENT_ACTION_HOVER_MOVE) {
                mBatches.push();
                Batch& batch = mBatches.editTop();
                batch.samples.push(mMsg);
#if DEBUG_TRANSPORT_ACTIONS
                ALOGD("channel '%s' consumer ~ started batch event",
                        mChannel->getName().c_str());
#endif
                break;
            }

            MotionEvent* motionEvent = factory->createMotionEvent();
            if (! motionEvent) return NO_MEMORY;

            updateTouchState(mMsg);
            initializeMotionEvent(motionEvent, &mMsg);
            *outSeq = mMsg.body.motion.seq;
            *outEvent = motionEvent;

#if DEBUG_TRANSPORT_ACTIONS
            ALOGD("channel '%s' consumer ~ consumed motion event, seq=%u",
                    mChannel->getName().c_str(), *outSeq);
#endif
            break;
        }

        default:
            ALOGE("channel '%s' consumer ~ Received unexpected message of type %d",
                    mChannel->getName().c_str(), mMsg.header.type);
            return UNKNOWN_ERROR;
        }
    }
    return OK;
}

status_t InputConsumer::consumeBatch(InputEventFactoryInterface* factory,
        nsecs_t frameTime, uint32_t* outSeq, InputEvent** outEvent) {
    status_t result;
    for (size_t i = mBatches.size(); i > 0; ) {
        i--;
        Batch& batch = mBatches.editItemAt(i);
        if (frameTime < 0) {
            result = consumeSamples(factory, batch, batch.samples.size(), outSeq, outEvent);
            mBatches.removeAt(i);
            return result;
        }

        nsecs_t sampleTime = frameTime;
        if (mResampleTouch) {
            sampleTime -= RESAMPLE_LATENCY;
        }
        ssize_t split = findSampleNoLaterThan(batch, sampleTime);
        if (split < 0) {
            continue;
        }

        result = consumeSamples(factory, batch, split + 1, outSeq, outEvent);
        const InputMessage* next;
        if (batch.samples.isEmpty()) {
            mBatches.removeAt(i);
            next = nullptr;
        } else {
            next = &batch.samples.itemAt(0);
        }
        if (!result && mResampleTouch) {
            resampleTouchState(sampleTime, static_cast<MotionEvent*>(*outEvent), next);
        }
        return result;
    }

    return WOULD_BLOCK;
}

status_t InputConsumer::consumeSamples(InputEventFactoryInterface* factory,
        Batch& batch, size_t count, uint32_t* outSeq, InputEvent** outEvent) {
    MotionEvent* motionEvent = factory->createMotionEvent();
    if (! motionEvent) return NO_MEMORY;

    uint32_t chain = 0;
    for (size_t i = 0; i < count; i++) {
        InputMessage& msg = batch.samples.editItemAt(i);
        updateTouchState(msg);
        if (i) {
            SeqChain seqChain;
            seqChain.seq = msg.body.motion.seq;
            seqChain.chain = chain;
            mSeqChains.push(seqChain);
            addSample(motionEvent, &msg);
        } else {
            initializeMotionEvent(motionEvent, &msg);
        }
        chain = msg.body.motion.seq;
    }
    batch.samples.removeItemsAt(0, count);

    *outSeq = chain;
    *outEvent = motionEvent;
    return OK;
}

void InputConsumer::updateTouchState(InputMessage& msg) {
    if (!mResampleTouch || !isPointerEvent(msg.body.motion.source)) {
        return;
    }

    int32_t deviceId = msg.body.motion.deviceId;
    int32_t source = msg.body.motion.source;

    // Update the touch state history to incorporate the new input message.
    // If the message is in the past relative to the most recently produced resampled
    // touch, then use the resampled time and coordinates instead.
    switch (msg.body.motion.action & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_DOWN: {
        ssize_t index = findTouchState(deviceId, source);
        if (index < 0) {
            mTouchStates.push();
            index = mTouchStates.size() - 1;
        }
        TouchState& touchState = mTouchStates.editItemAt(index);
        touchState.initialize(deviceId, source);
        touchState.addHistory(msg);
        break;
    }

    case AMOTION_EVENT_ACTION_MOVE: {
        ssize_t index = findTouchState(deviceId, source);
        if (index >= 0) {
            TouchState& touchState = mTouchStates.editItemAt(index);
            touchState.addHistory(msg);
            rewriteMessage(touchState, msg);
        }
        break;
    }

    case AMOTION_EVENT_ACTION_POINTER_DOWN: {
        ssize_t index = findTouchState(deviceId, source);
        if (index >= 0) {
            TouchState& touchState = mTouchStates.editItemAt(index);
            touchState.lastResample.idBits.clearBit(msg.body.motion.getActionId());
            rewriteMessage(touchState, msg);
        }
        break;
    }

    case AMOTION_EVENT_ACTION_POINTER_UP: {
        ssize_t index = findTouchState(deviceId, source);
        if (index >= 0) {
            TouchState& touchState = mTouchStates.editItemAt(index);
            rewriteMessage(touchState, msg);
            touchState.lastResample.idBits.clearBit(msg.body.motion.getActionId());
        }
        break;
    }

    case AMOTION_EVENT_ACTION_SCROLL: {
        ssize_t index = findTouchState(deviceId, source);
        if (index >= 0) {
            TouchState& touchState = mTouchStates.editItemAt(index);
            rewriteMessage(touchState, msg);
        }
        break;
    }

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL: {
        ssize_t index = findTouchState(deviceId, source);
        if (index >= 0) {
            TouchState& touchState = mTouchStates.editItemAt(index);
            rewriteMessage(touchState, msg);
            mTouchStates.removeAt(index);
        }
        break;
    }
    }
}

/**
 * Replace the coordinates in msg with the coordinates in lastResample, if necessary.
 *
 * If lastResample is no longer valid for a specific pointer (i.e. the lastResample time
 * is in the past relative to msg and the past two events do not contain identical coordinates),
 * then invalidate the lastResample data for that pointer.
 * If the two past events have identical coordinates, then lastResample data for that pointer will
 * remain valid, and will be used to replace these coordinates. Thus, if a certain coordinate x0 is
 * resampled to the new value x1, then x1 will always be used to replace x0 until some new value
 * not equal to x0 is received.
 */
void InputConsumer::rewriteMessage(TouchState& state, InputMessage& msg) {
    nsecs_t eventTime = msg.body.motion.eventTime;
    for (uint32_t i = 0; i < msg.body.motion.pointerCount; i++) {
        uint32_t id = msg.body.motion.pointers[i].properties.id;
        if (state.lastResample.idBits.hasBit(id)) {
            if (eventTime < state.lastResample.eventTime ||
                    state.recentCoordinatesAreIdentical(id)) {
                PointerCoords& msgCoords = msg.body.motion.pointers[i].coords;
                const PointerCoords& resampleCoords = state.lastResample.getPointerById(id);
#if DEBUG_RESAMPLING
                ALOGD("[%d] - rewrite (%0.3f, %0.3f), old (%0.3f, %0.3f)", id,
                        resampleCoords.getX(), resampleCoords.getY(),
                        msgCoords.getX(), msgCoords.getY());
#endif
                msgCoords.setAxisValue(AMOTION_EVENT_AXIS_X, resampleCoords.getX());
                msgCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, resampleCoords.getY());
            } else {
                state.lastResample.idBits.clearBit(id);
            }
        }
    }
}

void InputConsumer::resampleTouchState(nsecs_t sampleTime, MotionEvent* event,
    const InputMessage* next) {
    if (!mResampleTouch
            || !(isPointerEvent(event->getSource()))
            || event->getAction() != AMOTION_EVENT_ACTION_MOVE) {
        return;
    }

    ssize_t index = findTouchState(event->getDeviceId(), event->getSource());
    if (index < 0) {
#if DEBUG_RESAMPLING
        ALOGD("Not resampled, no touch state for device.");
#endif
        return;
    }

    TouchState& touchState = mTouchStates.editItemAt(index);
    if (touchState.historySize < 1) {
#if DEBUG_RESAMPLING
        ALOGD("Not resampled, no history for device.");
#endif
        return;
    }

    // Ensure that the current sample has all of the pointers that need to be reported.
    const History* current = touchState.getHistory(0);
    size_t pointerCount = event->getPointerCount();
    for (size_t i = 0; i < pointerCount; i++) {
        uint32_t id = event->getPointerId(i);
        if (!current->idBits.hasBit(id)) {
#if DEBUG_RESAMPLING
            ALOGD("Not resampled, missing id %d", id);
#endif
            return;
        }
    }

    // Find the data to use for resampling.
    const History* other;
    History future;
    float alpha;
    if (next) {
        // Interpolate between current sample and future sample.
        // So current->eventTime <= sampleTime <= future.eventTime.
        future.initializeFrom(*next);
        other = &future;
        nsecs_t delta = future.eventTime - current->eventTime;
        if (delta < RESAMPLE_MIN_DELTA) {
#if DEBUG_RESAMPLING
            ALOGD("Not resampled, delta time is too small: %" PRId64 " ns.", delta);
#endif
            return;
        }
        alpha = float(sampleTime - current->eventTime) / delta;
    } else if (touchState.historySize >= 2) {
        // Extrapolate future sample using current sample and past sample.
        // So other->eventTime <= current->eventTime <= sampleTime.
        other = touchState.getHistory(1);
        nsecs_t delta = current->eventTime - other->eventTime;
        if (delta < RESAMPLE_MIN_DELTA) {
#if DEBUG_RESAMPLING
            ALOGD("Not resampled, delta time is too small: %" PRId64 " ns.", delta);
#endif
            return;
        } else if (delta > RESAMPLE_MAX_DELTA) {
#if DEBUG_RESAMPLING
            ALOGD("Not resampled, delta time is too large: %" PRId64 " ns.", delta);
#endif
            return;
        }
        nsecs_t maxPredict = current->eventTime + min(delta / 2, RESAMPLE_MAX_PREDICTION);
        if (sampleTime > maxPredict) {
#if DEBUG_RESAMPLING
            ALOGD("Sample time is too far in the future, adjusting prediction "
                    "from %" PRId64 " to %" PRId64 " ns.",
                    sampleTime - current->eventTime, maxPredict - current->eventTime);
#endif
            sampleTime = maxPredict;
        }
        alpha = float(current->eventTime - sampleTime) / delta;
    } else {
#if DEBUG_RESAMPLING
        ALOGD("Not resampled, insufficient data.");
#endif
        return;
    }

    // Resample touch coordinates.
    History oldLastResample;
    oldLastResample.initializeFrom(touchState.lastResample);
    touchState.lastResample.eventTime = sampleTime;
    touchState.lastResample.idBits.clear();
    for (size_t i = 0; i < pointerCount; i++) {
        uint32_t id = event->getPointerId(i);
        touchState.lastResample.idToIndex[id] = i;
        touchState.lastResample.idBits.markBit(id);
        if (oldLastResample.hasPointerId(id) && touchState.recentCoordinatesAreIdentical(id)) {
            // We maintain the previously resampled value for this pointer (stored in
            // oldLastResample) when the coordinates for this pointer haven't changed since then.
            // This way we don't introduce artificial jitter when pointers haven't actually moved.

            // We know here that the coordinates for the pointer haven't changed because we
            // would've cleared the resampled bit in rewriteMessage if they had. We can't modify
            // lastResample in place becasue the mapping from pointer ID to index may have changed.
            touchState.lastResample.pointers[i].copyFrom(oldLastResample.getPointerById(id));
            continue;
        }

        PointerCoords& resampledCoords = touchState.lastResample.pointers[i];
        const PointerCoords& currentCoords = current->getPointerById(id);
        resampledCoords.copyFrom(currentCoords);
        if (other->idBits.hasBit(id)
                && shouldResampleTool(event->getToolType(i))) {
            const PointerCoords& otherCoords = other->getPointerById(id);
            resampledCoords.setAxisValue(AMOTION_EVENT_AXIS_X,
                    lerp(currentCoords.getX(), otherCoords.getX(), alpha));
            resampledCoords.setAxisValue(AMOTION_EVENT_AXIS_Y,
                    lerp(currentCoords.getY(), otherCoords.getY(), alpha));
#if DEBUG_RESAMPLING
            ALOGD("[%d] - out (%0.3f, %0.3f), cur (%0.3f, %0.3f), "
                    "other (%0.3f, %0.3f), alpha %0.3f",
                    id, resampledCoords.getX(), resampledCoords.getY(),
                    currentCoords.getX(), currentCoords.getY(),
                    otherCoords.getX(), otherCoords.getY(),
                    alpha);
#endif
        } else {
#if DEBUG_RESAMPLING
            ALOGD("[%d] - out (%0.3f, %0.3f), cur (%0.3f, %0.3f)",
                    id, resampledCoords.getX(), resampledCoords.getY(),
                    currentCoords.getX(), currentCoords.getY());
#endif
        }
    }

    event->addSample(sampleTime, touchState.lastResample.pointers);
}

bool InputConsumer::shouldResampleTool(int32_t toolType) {
    return toolType == AMOTION_EVENT_TOOL_TYPE_FINGER
            || toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN;
}

status_t InputConsumer::sendFinishedSignal(uint32_t seq, bool handled) {
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' consumer ~ sendFinishedSignal: seq=%u, handled=%s",
            mChannel->getName().c_str(), seq, handled ? "true" : "false");
#endif

    if (!seq) {
        ALOGE("Attempted to send a finished signal with sequence number 0.");
        return BAD_VALUE;
    }

    // Send finished signals for the batch sequence chain first.
    size_t seqChainCount = mSeqChains.size();
    if (seqChainCount) {
        uint32_t currentSeq = seq;
        uint32_t chainSeqs[seqChainCount];
        size_t chainIndex = 0;
        for (size_t i = seqChainCount; i > 0; ) {
             i--;
             const SeqChain& seqChain = mSeqChains.itemAt(i);
             if (seqChain.seq == currentSeq) {
                 currentSeq = seqChain.chain;
                 chainSeqs[chainIndex++] = currentSeq;
                 mSeqChains.removeAt(i);
             }
        }
        status_t status = OK;
        while (!status && chainIndex > 0) {
            chainIndex--;
            status = sendUnchainedFinishedSignal(chainSeqs[chainIndex], handled);
        }
        if (status) {
            // An error occurred so at least one signal was not sent, reconstruct the chain.
            for (;;) {
                SeqChain seqChain;
                seqChain.seq = chainIndex != 0 ? chainSeqs[chainIndex - 1] : seq;
                seqChain.chain = chainSeqs[chainIndex];
                mSeqChains.push(seqChain);
                if (!chainIndex) break;
                chainIndex--;
            }
            return status;
        }
    }

    // Send finished signal for the last message in the batch.
    return sendUnchainedFinishedSignal(seq, handled);
}

status_t InputConsumer::sendUnchainedFinishedSignal(uint32_t seq, bool handled) {
    InputMessage msg;
    msg.header.type = InputMessage::TYPE_FINISHED;
    msg.body.finished.seq = seq;
    msg.body.finished.handled = handled;
    return mChannel->sendMessage(&msg);
}

bool InputConsumer::hasDeferredEvent() const {
    return mMsgDeferred;
}

bool InputConsumer::hasPendingBatch() const {
    return !mBatches.isEmpty();
}

ssize_t InputConsumer::findBatch(int32_t deviceId, int32_t source) const {
    for (size_t i = 0; i < mBatches.size(); i++) {
        const Batch& batch = mBatches.itemAt(i);
        const InputMessage& head = batch.samples.itemAt(0);
        if (head.body.motion.deviceId == deviceId && head.body.motion.source == source) {
            return i;
        }
    }
    return -1;
}

ssize_t InputConsumer::findTouchState(int32_t deviceId, int32_t source) const {
    for (size_t i = 0; i < mTouchStates.size(); i++) {
        const TouchState& touchState = mTouchStates.itemAt(i);
        if (touchState.deviceId == deviceId && touchState.source == source) {
            return i;
        }
    }
    return -1;
}

void InputConsumer::initializeKeyEvent(KeyEvent* event, const InputMessage* msg) {
    event->initialize(
            msg->body.key.deviceId,
            msg->body.key.source,
            msg->body.key.displayId,
            msg->body.key.action,
            msg->body.key.flags,
            msg->body.key.keyCode,
            msg->body.key.scanCode,
            msg->body.key.metaState,
            msg->body.key.repeatCount,
            msg->body.key.downTime,
            msg->body.key.eventTime);
}

void InputConsumer::initializeMotionEvent(MotionEvent* event, const InputMessage* msg) {
    uint32_t pointerCount = msg->body.motion.pointerCount;
    PointerProperties pointerProperties[pointerCount];
    PointerCoords pointerCoords[pointerCount];
    for (uint32_t i = 0; i < pointerCount; i++) {
        pointerProperties[i].copyFrom(msg->body.motion.pointers[i].properties);
        pointerCoords[i].copyFrom(msg->body.motion.pointers[i].coords);
    }

    event->initialize(
            msg->body.motion.deviceId,
            msg->body.motion.source,
            msg->body.motion.displayId,
            msg->body.motion.action,
            msg->body.motion.actionButton,
            msg->body.motion.flags,
            msg->body.motion.edgeFlags,
            msg->body.motion.metaState,
            msg->body.motion.buttonState,
            msg->body.motion.classification,
            msg->body.motion.xOffset,
            msg->body.motion.yOffset,
            msg->body.motion.xPrecision,
            msg->body.motion.yPrecision,
            msg->body.motion.downTime,
            msg->body.motion.eventTime,
            pointerCount,
            pointerProperties,
            pointerCoords);
}

void InputConsumer::addSample(MotionEvent* event, const InputMessage* msg) {
    uint32_t pointerCount = msg->body.motion.pointerCount;
    PointerCoords pointerCoords[pointerCount];
    for (uint32_t i = 0; i < pointerCount; i++) {
        pointerCoords[i].copyFrom(msg->body.motion.pointers[i].coords);
    }

    event->setMetaState(event->getMetaState() | msg->body.motion.metaState);
    event->addSample(msg->body.motion.eventTime, pointerCoords);
}

bool InputConsumer::canAddSample(const Batch& batch, const InputMessage *msg) {
    const InputMessage& head = batch.samples.itemAt(0);
    uint32_t pointerCount = msg->body.motion.pointerCount;
    if (head.body.motion.pointerCount != pointerCount
            || head.body.motion.action != msg->body.motion.action) {
        return false;
    }
    for (size_t i = 0; i < pointerCount; i++) {
        if (head.body.motion.pointers[i].properties
                != msg->body.motion.pointers[i].properties) {
            return false;
        }
    }
    return true;
}

ssize_t InputConsumer::findSampleNoLaterThan(const Batch& batch, nsecs_t time) {
    size_t numSamples = batch.samples.size();
    size_t index = 0;
    while (index < numSamples
            && batch.samples.itemAt(index).body.motion.eventTime <= time) {
        index += 1;
    }
    return ssize_t(index) - 1;
}

} // namespace android
