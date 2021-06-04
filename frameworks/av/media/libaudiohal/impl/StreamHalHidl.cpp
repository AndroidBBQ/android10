/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "StreamHalHidl"
//#define LOG_NDEBUG 0

#include PATH(android/hardware/audio/FILE_VERSION/IStreamOutCallback.h)
#include <hwbinder/IPCThreadState.h>
#include <media/AudioParameter.h>
#include <mediautils/SchedulingPolicyService.h>
#include <utils/Log.h>

#include "DeviceHalHidl.h"
#include "EffectHalHidl.h"
#include "StreamHalHidl.h"
#include "VersionUtils.h"

using ::android::hardware::MQDescriptorSync;
using ::android::hardware::Return;
using ::android::hardware::Void;

namespace android {
namespace CPP_VERSION {

using EffectHalHidl = ::android::effect::CPP_VERSION::EffectHalHidl;
using ReadCommand = ::android::hardware::audio::CPP_VERSION::IStreamIn::ReadCommand;

using namespace ::android::hardware::audio::common::CPP_VERSION;
using namespace ::android::hardware::audio::CPP_VERSION;

StreamHalHidl::StreamHalHidl(IStream *stream)
        : ConversionHelperHidl("Stream"),
          mStream(stream),
          mHalThreadPriority(HAL_THREAD_PRIORITY_DEFAULT),
          mCachedBufferSize(0){

    // Instrument audio signal power logging.
    // Note: This assumes channel mask, format, and sample rate do not change after creation.
    if (mStream != nullptr /* && mStreamPowerLog.isUserDebugOrEngBuild() */) {
        // Obtain audio properties (see StreamHalHidl::getAudioProperties() below).
        Return<void> ret = mStream->getAudioProperties(
                [&](auto sr, auto m, auto f) {
                mStreamPowerLog.init(sr,
                        static_cast<audio_channel_mask_t>(m),
                        static_cast<audio_format_t>(f));
            });
    }
}

StreamHalHidl::~StreamHalHidl() {
    mStream = nullptr;
}

status_t StreamHalHidl::getSampleRate(uint32_t *rate) {
    if (!mStream) return NO_INIT;
    return processReturn("getSampleRate", mStream->getSampleRate(), rate);
}

status_t StreamHalHidl::getBufferSize(size_t *size) {
    if (!mStream) return NO_INIT;
    status_t status = processReturn("getBufferSize", mStream->getBufferSize(), size);
    if (status == OK) {
        mCachedBufferSize = *size;
    }
    return status;
}

status_t StreamHalHidl::getChannelMask(audio_channel_mask_t *mask) {
    if (!mStream) return NO_INIT;
    return processReturn("getChannelMask", mStream->getChannelMask(), mask);
}

status_t StreamHalHidl::getFormat(audio_format_t *format) {
    if (!mStream) return NO_INIT;
    return processReturn("getFormat", mStream->getFormat(), format);
}

status_t StreamHalHidl::getAudioProperties(
        uint32_t *sampleRate, audio_channel_mask_t *mask, audio_format_t *format) {
    if (!mStream) return NO_INIT;
    Return<void> ret = mStream->getAudioProperties(
            [&](uint32_t sr, auto m, auto f) {
                *sampleRate = sr;
                *mask = static_cast<audio_channel_mask_t>(m);
                *format = static_cast<audio_format_t>(f);
            });
    return processReturn("getAudioProperties", ret);
}

status_t StreamHalHidl::setParameters(const String8& kvPairs) {
    if (!mStream) return NO_INIT;
    hidl_vec<ParameterValue> hidlParams;
    status_t status = parametersFromHal(kvPairs, &hidlParams);
    if (status != OK) return status;
    return processReturn("setParameters",
                         utils::setParameters(mStream, {} /* context */, hidlParams));
}

status_t StreamHalHidl::getParameters(const String8& keys, String8 *values) {
    values->clear();
    if (!mStream) return NO_INIT;
    hidl_vec<hidl_string> hidlKeys;
    status_t status = keysFromHal(keys, &hidlKeys);
    if (status != OK) return status;
    Result retval;
    Return<void> ret = utils::getParameters(
            mStream,
            {} /* context */,
            hidlKeys,
            [&](Result r, const hidl_vec<ParameterValue>& parameters) {
                retval = r;
                if (retval == Result::OK) {
                    parametersToHal(parameters, values);
                }
            });
    return processReturn("getParameters", ret, retval);
}

status_t StreamHalHidl::addEffect(sp<EffectHalInterface> effect) {
    if (!mStream) return NO_INIT;
    return processReturn("addEffect", mStream->addEffect(
                    static_cast<EffectHalHidl*>(effect.get())->effectId()));
}

status_t StreamHalHidl::removeEffect(sp<EffectHalInterface> effect) {
    if (!mStream) return NO_INIT;
    return processReturn("removeEffect", mStream->removeEffect(
                    static_cast<EffectHalHidl*>(effect.get())->effectId()));
}

status_t StreamHalHidl::standby() {
    if (!mStream) return NO_INIT;
    return processReturn("standby", mStream->standby());
}

status_t StreamHalHidl::dump(int fd) {
    if (!mStream) return NO_INIT;
    native_handle_t* hidlHandle = native_handle_create(1, 0);
    hidlHandle->data[0] = fd;
    Return<void> ret = mStream->debug(hidlHandle, {} /* options */);
    native_handle_delete(hidlHandle);
    mStreamPowerLog.dump(fd);
    return processReturn("dump", ret);
}

status_t StreamHalHidl::start() {
    if (!mStream) return NO_INIT;
    return processReturn("start", mStream->start());
}

status_t StreamHalHidl::stop() {
    if (!mStream) return NO_INIT;
    return processReturn("stop", mStream->stop());
}

status_t StreamHalHidl::createMmapBuffer(int32_t minSizeFrames,
                                  struct audio_mmap_buffer_info *info) {
    Result retval;
    Return<void> ret = mStream->createMmapBuffer(
            minSizeFrames,
            [&](Result r, const MmapBufferInfo& hidlInfo) {
                retval = r;
                if (retval == Result::OK) {
                    const native_handle *handle = hidlInfo.sharedMemory.handle();
                    if (handle->numFds > 0) {
                        info->shared_memory_fd = handle->data[0];
#if MAJOR_VERSION >= 4
                        info->flags = audio_mmap_buffer_flag(hidlInfo.flags);
#endif
                        info->buffer_size_frames = hidlInfo.bufferSizeFrames;
                        // Negative buffer size frame was a hack in O and P to
                        // indicate that the buffer is shareable to applications
                        if (info->buffer_size_frames < 0) {
                            info->buffer_size_frames *= -1;
                            info->flags = audio_mmap_buffer_flag(
                                    info->flags | AUDIO_MMAP_APPLICATION_SHAREABLE);
                        }
                        info->burst_size_frames = hidlInfo.burstSizeFrames;
                        // info->shared_memory_address is not needed in HIDL context
                        info->shared_memory_address = NULL;
                    } else {
                        retval = Result::NOT_INITIALIZED;
                    }
                }
            });
    return processReturn("createMmapBuffer", ret, retval);
}

status_t StreamHalHidl::getMmapPosition(struct audio_mmap_position *position) {
    Result retval;
    Return<void> ret = mStream->getMmapPosition(
            [&](Result r, const MmapPosition& hidlPosition) {
                retval = r;
                if (retval == Result::OK) {
                    position->time_nanoseconds = hidlPosition.timeNanoseconds;
                    position->position_frames = hidlPosition.positionFrames;
                }
            });
    return processReturn("getMmapPosition", ret, retval);
}

status_t StreamHalHidl::setHalThreadPriority(int priority) {
    mHalThreadPriority = priority;
    return OK;
}

status_t StreamHalHidl::getCachedBufferSize(size_t *size) {
    if (mCachedBufferSize != 0) {
        *size = mCachedBufferSize;
        return OK;
    }
    return getBufferSize(size);
}

bool StreamHalHidl::requestHalThreadPriority(pid_t threadPid, pid_t threadId) {
    if (mHalThreadPriority == HAL_THREAD_PRIORITY_DEFAULT) {
        return true;
    }
    int err = requestPriority(
            threadPid, threadId,
            mHalThreadPriority, false /*isForApp*/, true /*asynchronous*/);
    ALOGE_IF(err, "failed to set priority %d for pid %d tid %d; error %d",
            mHalThreadPriority, threadPid, threadId, err);
    // Audio will still work, but latency will be higher and sometimes unacceptable.
    return err == 0;
}

namespace {

/* Notes on callback ownership.

This is how (Hw)Binder ownership model looks like. The server implementation
is owned by Binder framework (via sp<>). Proxies are owned by clients.
When the last proxy disappears, Binder framework releases the server impl.

Thus, it is not needed to keep any references to StreamOutCallback (this is
the server impl) -- it will live as long as HAL server holds a strong ref to
IStreamOutCallback proxy. We clear that reference by calling 'clearCallback'
from the destructor of StreamOutHalHidl.

The callback only keeps a weak reference to the stream. The stream is owned
by AudioFlinger.

*/

struct StreamOutCallback : public IStreamOutCallback {
    StreamOutCallback(const wp<StreamOutHalHidl>& stream) : mStream(stream) {}

    // IStreamOutCallback implementation
    Return<void> onWriteReady()  override {
        sp<StreamOutHalHidl> stream = mStream.promote();
        if (stream != 0) {
            stream->onWriteReady();
        }
        return Void();
    }

    Return<void> onDrainReady()  override {
        sp<StreamOutHalHidl> stream = mStream.promote();
        if (stream != 0) {
            stream->onDrainReady();
        }
        return Void();
    }

    Return<void> onError()  override {
        sp<StreamOutHalHidl> stream = mStream.promote();
        if (stream != 0) {
            stream->onError();
        }
        return Void();
    }

  private:
    wp<StreamOutHalHidl> mStream;
};

}  // namespace

StreamOutHalHidl::StreamOutHalHidl(const sp<IStreamOut>& stream)
        : StreamHalHidl(stream.get()), mStream(stream), mWriterClient(0), mEfGroup(nullptr) {
}

StreamOutHalHidl::~StreamOutHalHidl() {
    if (mStream != 0) {
        if (mCallback.unsafe_get()) {
            processReturn("clearCallback", mStream->clearCallback());
        }
        processReturn("close", mStream->close());
        mStream.clear();
    }
    mCallback.clear();
    hardware::IPCThreadState::self()->flushCommands();
    if (mEfGroup) {
        EventFlag::deleteEventFlag(&mEfGroup);
    }
}

status_t StreamOutHalHidl::getFrameSize(size_t *size) {
    if (mStream == 0) return NO_INIT;
    return processReturn("getFrameSize", mStream->getFrameSize(), size);
}

status_t StreamOutHalHidl::getLatency(uint32_t *latency) {
    if (mStream == 0) return NO_INIT;
    if (mWriterClient == gettid() && mCommandMQ) {
        return callWriterThread(
                WriteCommand::GET_LATENCY, "getLatency", nullptr, 0,
                [&](const WriteStatus& writeStatus) {
                    *latency = writeStatus.reply.latencyMs;
                });
    } else {
        return processReturn("getLatency", mStream->getLatency(), latency);
    }
}

status_t StreamOutHalHidl::setVolume(float left, float right) {
    if (mStream == 0) return NO_INIT;
    return processReturn("setVolume", mStream->setVolume(left, right));
}

#if MAJOR_VERSION == 2
status_t StreamOutHalHidl::selectPresentation(int presentationId, int programId) {
    if (mStream == 0) return NO_INIT;
    std::vector<ParameterValue> parameters;
    String8 halParameters;
    parameters.push_back({AudioParameter::keyPresentationId, std::to_string(presentationId)});
    parameters.push_back({AudioParameter::keyProgramId, std::to_string(programId)});
    parametersToHal(hidl_vec<ParameterValue>(parameters), &halParameters);
    return setParameters(halParameters);
}
#elif MAJOR_VERSION >= 4
status_t StreamOutHalHidl::selectPresentation(int presentationId, int programId) {
    if (mStream == 0) return NO_INIT;
    return processReturn("selectPresentation",
            mStream->selectPresentation(presentationId, programId));
}
#endif

status_t StreamOutHalHidl::write(const void *buffer, size_t bytes, size_t *written) {
    if (mStream == 0) return NO_INIT;
    *written = 0;

    if (bytes == 0 && !mDataMQ) {
        // Can't determine the size for the MQ buffer. Wait for a non-empty write request.
        ALOGW_IF(mCallback.unsafe_get(), "First call to async write with 0 bytes");
        return OK;
    }

    status_t status;
    if (!mDataMQ) {
        // In case if playback starts close to the end of a compressed track, the bytes
        // that need to be written is less than the actual buffer size. Need to use
        // full buffer size for the MQ since otherwise after seeking back to the middle
        // data will be truncated.
        size_t bufferSize;
        if ((status = getCachedBufferSize(&bufferSize)) != OK) {
            return status;
        }
        if (bytes > bufferSize) bufferSize = bytes;
        if ((status = prepareForWriting(bufferSize)) != OK) {
            return status;
        }
    }

    status = callWriterThread(
            WriteCommand::WRITE, "write", static_cast<const uint8_t*>(buffer), bytes,
            [&] (const WriteStatus& writeStatus) {
                *written = writeStatus.reply.written;
                // Diagnostics of the cause of b/35813113.
                ALOGE_IF(*written > bytes,
                        "hal reports more bytes written than asked for: %lld > %lld",
                        (long long)*written, (long long)bytes);
            });
    mStreamPowerLog.log(buffer, *written);
    return status;
}

status_t StreamOutHalHidl::callWriterThread(
        WriteCommand cmd, const char* cmdName,
        const uint8_t* data, size_t dataSize, StreamOutHalHidl::WriterCallback callback) {
    if (!mCommandMQ->write(&cmd)) {
        ALOGE("command message queue write failed for \"%s\"", cmdName);
        return -EAGAIN;
    }
    if (data != nullptr) {
        size_t availableToWrite = mDataMQ->availableToWrite();
        if (dataSize > availableToWrite) {
            ALOGW("truncating write data from %lld to %lld due to insufficient data queue space",
                    (long long)dataSize, (long long)availableToWrite);
            dataSize = availableToWrite;
        }
        if (!mDataMQ->write(data, dataSize)) {
            ALOGE("data message queue write failed for \"%s\"", cmdName);
        }
    }
    mEfGroup->wake(static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY));

    // TODO: Remove manual event flag handling once blocking MQ is implemented. b/33815422
    uint32_t efState = 0;
retry:
    status_t ret = mEfGroup->wait(static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL), &efState);
    if (efState & static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL)) {
        WriteStatus writeStatus;
        writeStatus.retval = Result::NOT_INITIALIZED;
        if (!mStatusMQ->read(&writeStatus)) {
            ALOGE("status message read failed for \"%s\"", cmdName);
        }
        if (writeStatus.retval == Result::OK) {
            ret = OK;
            callback(writeStatus);
        } else {
            ret = processReturn(cmdName, writeStatus.retval);
        }
        return ret;
    }
    if (ret == -EAGAIN || ret == -EINTR) {
        // Spurious wakeup. This normally retries no more than once.
        goto retry;
    }
    return ret;
}

status_t StreamOutHalHidl::prepareForWriting(size_t bufferSize) {
    std::unique_ptr<CommandMQ> tempCommandMQ;
    std::unique_ptr<DataMQ> tempDataMQ;
    std::unique_ptr<StatusMQ> tempStatusMQ;
    Result retval;
    pid_t halThreadPid, halThreadTid;
    Return<void> ret = mStream->prepareForWriting(
            1, bufferSize,
            [&](Result r,
                    const CommandMQ::Descriptor& commandMQ,
                    const DataMQ::Descriptor& dataMQ,
                    const StatusMQ::Descriptor& statusMQ,
                    const ThreadInfo& halThreadInfo) {
                retval = r;
                if (retval == Result::OK) {
                    tempCommandMQ.reset(new CommandMQ(commandMQ));
                    tempDataMQ.reset(new DataMQ(dataMQ));
                    tempStatusMQ.reset(new StatusMQ(statusMQ));
                    if (tempDataMQ->isValid() && tempDataMQ->getEventFlagWord()) {
                        EventFlag::createEventFlag(tempDataMQ->getEventFlagWord(), &mEfGroup);
                    }
                    halThreadPid = halThreadInfo.pid;
                    halThreadTid = halThreadInfo.tid;
                }
            });
    if (!ret.isOk() || retval != Result::OK) {
        return processReturn("prepareForWriting", ret, retval);
    }
    if (!tempCommandMQ || !tempCommandMQ->isValid() ||
            !tempDataMQ || !tempDataMQ->isValid() ||
            !tempStatusMQ || !tempStatusMQ->isValid() ||
            !mEfGroup) {
        ALOGE_IF(!tempCommandMQ, "Failed to obtain command message queue for writing");
        ALOGE_IF(tempCommandMQ && !tempCommandMQ->isValid(),
                "Command message queue for writing is invalid");
        ALOGE_IF(!tempDataMQ, "Failed to obtain data message queue for writing");
        ALOGE_IF(tempDataMQ && !tempDataMQ->isValid(), "Data message queue for writing is invalid");
        ALOGE_IF(!tempStatusMQ, "Failed to obtain status message queue for writing");
        ALOGE_IF(tempStatusMQ && !tempStatusMQ->isValid(),
                "Status message queue for writing is invalid");
        ALOGE_IF(!mEfGroup, "Event flag creation for writing failed");
        return NO_INIT;
    }
    requestHalThreadPriority(halThreadPid, halThreadTid);

    mCommandMQ = std::move(tempCommandMQ);
    mDataMQ = std::move(tempDataMQ);
    mStatusMQ = std::move(tempStatusMQ);
    mWriterClient = gettid();
    return OK;
}

status_t StreamOutHalHidl::getRenderPosition(uint32_t *dspFrames) {
    if (mStream == 0) return NO_INIT;
    Result retval;
    Return<void> ret = mStream->getRenderPosition(
            [&](Result r, uint32_t d) {
                retval = r;
                if (retval == Result::OK) {
                    *dspFrames = d;
                }
            });
    return processReturn("getRenderPosition", ret, retval);
}

status_t StreamOutHalHidl::getNextWriteTimestamp(int64_t *timestamp) {
    if (mStream == 0) return NO_INIT;
    Result retval;
    Return<void> ret = mStream->getNextWriteTimestamp(
            [&](Result r, int64_t t) {
                retval = r;
                if (retval == Result::OK) {
                    *timestamp = t;
                }
            });
    return processReturn("getRenderPosition", ret, retval);
}

status_t StreamOutHalHidl::setCallback(wp<StreamOutHalInterfaceCallback> callback) {
    if (mStream == 0) return NO_INIT;
    status_t status = processReturn(
            "setCallback", mStream->setCallback(new StreamOutCallback(this)));
    if (status == OK) {
        mCallback = callback;
    }
    return status;
}

status_t StreamOutHalHidl::supportsPauseAndResume(bool *supportsPause, bool *supportsResume) {
    if (mStream == 0) return NO_INIT;
    Return<void> ret = mStream->supportsPauseAndResume(
            [&](bool p, bool r) {
                *supportsPause = p;
                *supportsResume = r;
            });
    return processReturn("supportsPauseAndResume", ret);
}

status_t StreamOutHalHidl::pause() {
    if (mStream == 0) return NO_INIT;
    return processReturn("pause", mStream->pause());
}

status_t StreamOutHalHidl::resume() {
    if (mStream == 0) return NO_INIT;
    return processReturn("pause", mStream->resume());
}

status_t StreamOutHalHidl::supportsDrain(bool *supportsDrain) {
    if (mStream == 0) return NO_INIT;
    return processReturn("supportsDrain", mStream->supportsDrain(), supportsDrain);
}

status_t StreamOutHalHidl::drain(bool earlyNotify) {
    if (mStream == 0) return NO_INIT;
    return processReturn(
            "drain", mStream->drain(earlyNotify ? AudioDrain::EARLY_NOTIFY : AudioDrain::ALL));
}

status_t StreamOutHalHidl::flush() {
    if (mStream == 0) return NO_INIT;
    return processReturn("pause", mStream->flush());
}

status_t StreamOutHalHidl::getPresentationPosition(uint64_t *frames, struct timespec *timestamp) {
    if (mStream == 0) return NO_INIT;
    if (mWriterClient == gettid() && mCommandMQ) {
        return callWriterThread(
                WriteCommand::GET_PRESENTATION_POSITION, "getPresentationPosition", nullptr, 0,
                [&](const WriteStatus& writeStatus) {
                    *frames = writeStatus.reply.presentationPosition.frames;
                    timestamp->tv_sec = writeStatus.reply.presentationPosition.timeStamp.tvSec;
                    timestamp->tv_nsec = writeStatus.reply.presentationPosition.timeStamp.tvNSec;
                });
    } else {
        Result retval;
        Return<void> ret = mStream->getPresentationPosition(
                [&](Result r, uint64_t hidlFrames, const TimeSpec& hidlTimeStamp) {
                    retval = r;
                    if (retval == Result::OK) {
                        *frames = hidlFrames;
                        timestamp->tv_sec = hidlTimeStamp.tvSec;
                        timestamp->tv_nsec = hidlTimeStamp.tvNSec;
                    }
                });
        return processReturn("getPresentationPosition", ret, retval);
    }
}

#if MAJOR_VERSION == 2
status_t StreamOutHalHidl::updateSourceMetadata(
        const StreamOutHalInterface::SourceMetadata& /* sourceMetadata */) {
    // Audio HAL V2.0 does not support propagating source metadata
    return INVALID_OPERATION;
}
#elif MAJOR_VERSION >= 4
/** Transform a standard collection to an HIDL vector. */
template <class Values, class ElementConverter>
static auto transformToHidlVec(const Values& values, ElementConverter converter) {
    hidl_vec<decltype(converter(*values.begin()))> result{values.size()};
    using namespace std;
    transform(begin(values), end(values), begin(result), converter);
    return result;
}

status_t StreamOutHalHidl::updateSourceMetadata(
        const StreamOutHalInterface::SourceMetadata& sourceMetadata) {
    CPP_VERSION::SourceMetadata halMetadata = {
        .tracks = transformToHidlVec(sourceMetadata.tracks,
              [](const playback_track_metadata& metadata) -> PlaybackTrackMetadata {
                  return {
                    .usage=static_cast<AudioUsage>(metadata.usage),
                    .contentType=static_cast<AudioContentType>(metadata.content_type),
                    .gain=metadata.gain,
                  };
              })};
    return processReturn("updateSourceMetadata", mStream->updateSourceMetadata(halMetadata));
}
#endif

void StreamOutHalHidl::onWriteReady() {
    sp<StreamOutHalInterfaceCallback> callback = mCallback.promote();
    if (callback == 0) return;
    ALOGV("asyncCallback onWriteReady");
    callback->onWriteReady();
}

void StreamOutHalHidl::onDrainReady() {
    sp<StreamOutHalInterfaceCallback> callback = mCallback.promote();
    if (callback == 0) return;
    ALOGV("asyncCallback onDrainReady");
    callback->onDrainReady();
}

void StreamOutHalHidl::onError() {
    sp<StreamOutHalInterfaceCallback> callback = mCallback.promote();
    if (callback == 0) return;
    ALOGV("asyncCallback onError");
    callback->onError();
}


StreamInHalHidl::StreamInHalHidl(const sp<IStreamIn>& stream)
        : StreamHalHidl(stream.get()), mStream(stream), mReaderClient(0), mEfGroup(nullptr) {
}

StreamInHalHidl::~StreamInHalHidl() {
    if (mStream != 0) {
        processReturn("close", mStream->close());
        mStream.clear();
        hardware::IPCThreadState::self()->flushCommands();
    }
    if (mEfGroup) {
        EventFlag::deleteEventFlag(&mEfGroup);
    }
}

status_t StreamInHalHidl::getFrameSize(size_t *size) {
    if (mStream == 0) return NO_INIT;
    return processReturn("getFrameSize", mStream->getFrameSize(), size);
}

status_t StreamInHalHidl::setGain(float gain) {
    if (mStream == 0) return NO_INIT;
    return processReturn("setGain", mStream->setGain(gain));
}

status_t StreamInHalHidl::read(void *buffer, size_t bytes, size_t *read) {
    if (mStream == 0) return NO_INIT;
    *read = 0;

    if (bytes == 0 && !mDataMQ) {
        // Can't determine the size for the MQ buffer. Wait for a non-empty read request.
        return OK;
    }

    status_t status;
    if (!mDataMQ && (status = prepareForReading(bytes)) != OK) {
        return status;
    }

    ReadParameters params;
    params.command = ReadCommand::READ;
    params.params.read = bytes;
    status = callReaderThread(params, "read",
            [&](const ReadStatus& readStatus) {
                const size_t availToRead = mDataMQ->availableToRead();
                if (!mDataMQ->read(static_cast<uint8_t*>(buffer), std::min(bytes, availToRead))) {
                    ALOGE("data message queue read failed for \"read\"");
                }
                ALOGW_IF(availToRead != readStatus.reply.read,
                        "HAL read report inconsistent: mq = %d, status = %d",
                        (int32_t)availToRead, (int32_t)readStatus.reply.read);
                *read = readStatus.reply.read;
            });
    mStreamPowerLog.log(buffer, *read);
    return status;
}

status_t StreamInHalHidl::callReaderThread(
        const ReadParameters& params, const char* cmdName,
        StreamInHalHidl::ReaderCallback callback) {
    if (!mCommandMQ->write(&params)) {
        ALOGW("command message queue write failed");
        return -EAGAIN;
    }
    mEfGroup->wake(static_cast<uint32_t>(MessageQueueFlagBits::NOT_FULL));

    // TODO: Remove manual event flag handling once blocking MQ is implemented. b/33815422
    uint32_t efState = 0;
retry:
    status_t ret = mEfGroup->wait(static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY), &efState);
    if (efState & static_cast<uint32_t>(MessageQueueFlagBits::NOT_EMPTY)) {
        ReadStatus readStatus;
        readStatus.retval = Result::NOT_INITIALIZED;
        if (!mStatusMQ->read(&readStatus)) {
            ALOGE("status message read failed for \"%s\"", cmdName);
        }
         if (readStatus.retval == Result::OK) {
            ret = OK;
            callback(readStatus);
        } else {
            ret = processReturn(cmdName, readStatus.retval);
        }
        return ret;
    }
    if (ret == -EAGAIN || ret == -EINTR) {
        // Spurious wakeup. This normally retries no more than once.
        goto retry;
    }
    return ret;
}

status_t StreamInHalHidl::prepareForReading(size_t bufferSize) {
    std::unique_ptr<CommandMQ> tempCommandMQ;
    std::unique_ptr<DataMQ> tempDataMQ;
    std::unique_ptr<StatusMQ> tempStatusMQ;
    Result retval;
    pid_t halThreadPid, halThreadTid;
    Return<void> ret = mStream->prepareForReading(
            1, bufferSize,
            [&](Result r,
                    const CommandMQ::Descriptor& commandMQ,
                    const DataMQ::Descriptor& dataMQ,
                    const StatusMQ::Descriptor& statusMQ,
                    const ThreadInfo& halThreadInfo) {
                retval = r;
                if (retval == Result::OK) {
                    tempCommandMQ.reset(new CommandMQ(commandMQ));
                    tempDataMQ.reset(new DataMQ(dataMQ));
                    tempStatusMQ.reset(new StatusMQ(statusMQ));
                    if (tempDataMQ->isValid() && tempDataMQ->getEventFlagWord()) {
                        EventFlag::createEventFlag(tempDataMQ->getEventFlagWord(), &mEfGroup);
                    }
                    halThreadPid = halThreadInfo.pid;
                    halThreadTid = halThreadInfo.tid;
                }
            });
    if (!ret.isOk() || retval != Result::OK) {
        return processReturn("prepareForReading", ret, retval);
    }
    if (!tempCommandMQ || !tempCommandMQ->isValid() ||
            !tempDataMQ || !tempDataMQ->isValid() ||
            !tempStatusMQ || !tempStatusMQ->isValid() ||
            !mEfGroup) {
        ALOGE_IF(!tempCommandMQ, "Failed to obtain command message queue for writing");
        ALOGE_IF(tempCommandMQ && !tempCommandMQ->isValid(),
                "Command message queue for writing is invalid");
        ALOGE_IF(!tempDataMQ, "Failed to obtain data message queue for reading");
        ALOGE_IF(tempDataMQ && !tempDataMQ->isValid(), "Data message queue for reading is invalid");
        ALOGE_IF(!tempStatusMQ, "Failed to obtain status message queue for reading");
        ALOGE_IF(tempStatusMQ && !tempStatusMQ->isValid(),
                "Status message queue for reading is invalid");
        ALOGE_IF(!mEfGroup, "Event flag creation for reading failed");
        return NO_INIT;
    }
    requestHalThreadPriority(halThreadPid, halThreadTid);

    mCommandMQ = std::move(tempCommandMQ);
    mDataMQ = std::move(tempDataMQ);
    mStatusMQ = std::move(tempStatusMQ);
    mReaderClient = gettid();
    return OK;
}

status_t StreamInHalHidl::getInputFramesLost(uint32_t *framesLost) {
    if (mStream == 0) return NO_INIT;
    return processReturn("getInputFramesLost", mStream->getInputFramesLost(), framesLost);
}

status_t StreamInHalHidl::getCapturePosition(int64_t *frames, int64_t *time) {
    if (mStream == 0) return NO_INIT;
    if (mReaderClient == gettid() && mCommandMQ) {
        ReadParameters params;
        params.command = ReadCommand::GET_CAPTURE_POSITION;
        return callReaderThread(params, "getCapturePosition",
                [&](const ReadStatus& readStatus) {
                    *frames = readStatus.reply.capturePosition.frames;
                    *time = readStatus.reply.capturePosition.time;
                });
    } else {
        Result retval;
        Return<void> ret = mStream->getCapturePosition(
                [&](Result r, uint64_t hidlFrames, uint64_t hidlTime) {
                    retval = r;
                    if (retval == Result::OK) {
                        *frames = hidlFrames;
                        *time = hidlTime;
                    }
                });
        return processReturn("getCapturePosition", ret, retval);
    }
}

#if MAJOR_VERSION == 2
status_t StreamInHalHidl::getActiveMicrophones(
        std::vector<media::MicrophoneInfo> *microphones __unused) {
    if (mStream == 0) return NO_INIT;
    return INVALID_OPERATION;
}

status_t StreamInHalHidl::updateSinkMetadata(
        const StreamInHalInterface::SinkMetadata& /* sinkMetadata */) {
    // Audio HAL V2.0 does not support propagating sink metadata
    return INVALID_OPERATION;
}

#elif MAJOR_VERSION >= 4
status_t StreamInHalHidl::getActiveMicrophones(
        std::vector<media::MicrophoneInfo> *microphonesInfo) {
    if (!mStream) return NO_INIT;
    Result retval;
    Return<void> ret = mStream->getActiveMicrophones(
            [&](Result r, hidl_vec<MicrophoneInfo> micArrayHal) {
        retval = r;
        for (size_t k = 0; k < micArrayHal.size(); k++) {
            audio_microphone_characteristic_t dst;
            // convert
            microphoneInfoToHal(micArrayHal[k], &dst);
            media::MicrophoneInfo microphone = media::MicrophoneInfo(dst);
            microphonesInfo->push_back(microphone);
        }
    });
    return processReturn("getActiveMicrophones", ret, retval);
}

status_t StreamInHalHidl::updateSinkMetadata(const
        StreamInHalInterface::SinkMetadata& sinkMetadata) {
    CPP_VERSION::SinkMetadata halMetadata = {
        .tracks = transformToHidlVec(sinkMetadata.tracks,
              [](const record_track_metadata& metadata) -> RecordTrackMetadata {
                  return {
                    .source=static_cast<AudioSource>(metadata.source),
                    .gain=metadata.gain,
                  };
              })};
    return processReturn("updateSinkMetadata", mStream->updateSinkMetadata(halMetadata));
}
#endif

#if MAJOR_VERSION < 5
status_t StreamInHalHidl::setPreferredMicrophoneDirection(
            audio_microphone_direction_t direction __unused) {
    if (mStream == 0) return NO_INIT;
    return INVALID_OPERATION;
}

status_t StreamInHalHidl::setPreferredMicrophoneFieldDimension(float zoom __unused) {
    if (mStream == 0) return NO_INIT;
    return INVALID_OPERATION;
}
#else
status_t StreamInHalHidl::setPreferredMicrophoneDirection(audio_microphone_direction_t direction) {
    if (!mStream) return NO_INIT;
    return processReturn("setPreferredMicrophoneDirection",
        mStream->setMicrophoneDirection(static_cast<MicrophoneDirection>(direction)));
}

status_t StreamInHalHidl::setPreferredMicrophoneFieldDimension(float zoom) {
    if (!mStream) return NO_INIT;
    return processReturn("setPreferredMicrophoneFieldDimension",
                mStream->setMicrophoneFieldDimension(zoom));
}
#endif

} // namespace CPP_VERSION
} // namespace android
