/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef MY_HANDLER_H_

#define MY_HANDLER_H_

//#define LOG_NDEBUG 0

#ifndef LOG_TAG
#define LOG_TAG "MyHandler"
#endif

#include <utils/Log.h>
#include <cutils/properties.h> // for property_get

#include "APacketSource.h"
#include "ARTPConnection.h"
#include "ARTSPConnection.h"
#include "ASessionDescription.h"
#include "NetworkUtils.h"

#include <ctype.h>
#include <cutils/properties.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "HTTPBase.h"

#if LOG_NDEBUG
#define UNUSED_UNLESS_VERBOSE(x) (void)(x)
#else
#define UNUSED_UNLESS_VERBOSE(x)
#endif

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]  // NOLINT
#endif

// If no access units are received within 5 secs, assume that the rtp
// stream has ended and signal end of stream.
static int64_t kAccessUnitTimeoutUs = 10000000ll;

// If no access units arrive for the first 10 secs after starting the
// stream, assume none ever will and signal EOS or switch transports.
static int64_t kStartupTimeoutUs = 10000000ll;

static int64_t kDefaultKeepAliveTimeoutUs = 60000000ll;

static int64_t kPauseDelayUs = 3000000ll;

// The allowed maximum number of stale access units at the beginning of
// a new sequence.
static int32_t kMaxAllowedStaleAccessUnits = 20;

static int64_t kTearDownTimeoutUs = 3000000ll;

namespace android {

static bool GetAttribute(const char *s, const char *key, AString *value) {
    value->clear();

    size_t keyLen = strlen(key);

    for (;;) {
        while (isspace(*s)) {
            ++s;
        }

        const char *colonPos = strchr(s, ';');

        size_t len =
            (colonPos == NULL) ? strlen(s) : colonPos - s;

        if (len >= keyLen + 1 && s[keyLen] == '=' && !strncmp(s, key, keyLen)) {
            value->setTo(&s[keyLen + 1], len - keyLen - 1);
            return true;
        }

        if (colonPos == NULL) {
            return false;
        }

        s = colonPos + 1;
    }
}

struct MyHandler : public AHandler {
    enum {
        kWhatConnected                  = 'conn',
        kWhatDisconnected               = 'disc',
        kWhatSeekPaused                 = 'spau',
        kWhatSeekDone                   = 'sdon',

        kWhatAccessUnit                 = 'accU',
        kWhatEOS                        = 'eos!',
        kWhatSeekDiscontinuity          = 'seeD',
        kWhatNormalPlayTimeMapping      = 'nptM',
    };

    MyHandler(
            const char *url,
            const sp<AMessage> &notify,
            bool uidValid = false, uid_t uid = 0)
        : mNotify(notify),
          mUIDValid(uidValid),
          mUID(uid),
          mNetLooper(new ALooper),
          mConn(new ARTSPConnection(mUIDValid, mUID)),
          mRTPConn(new ARTPConnection),
          mOriginalSessionURL(url),
          mSessionURL(url),
          mSetupTracksSuccessful(false),
          mSeekPending(false),
          mFirstAccessUnit(true),
          mAllTracksHaveTime(false),
          mNTPAnchorUs(-1),
          mMediaAnchorUs(-1),
          mLastMediaTimeUs(0),
          mNumAccessUnitsReceived(0),
          mCheckPending(false),
          mCheckGeneration(0),
          mCheckTimeoutGeneration(0),
          mTryTCPInterleaving(property_get_bool("rtp.transport.TCP", false)),
          mTryFakeRTCP(false),
          mReceivedFirstRTCPPacket(false),
          mReceivedFirstRTPPacket(false),
          mSeekable(true),
          mKeepAliveTimeoutUs(kDefaultKeepAliveTimeoutUs),
          mKeepAliveGeneration(0),
          mPausing(false),
          mPauseGeneration(0),
          mPlayResponseParsed(false) {
        mNetLooper->setName("rtsp net");
        mNetLooper->start(false /* runOnCallingThread */,
                          false /* canCallJava */,
                          PRIORITY_HIGHEST);

        // Strip any authentication info from the session url, we don't
        // want to transmit user/pass in cleartext.
        AString host, path, user, pass;
        unsigned port;
        CHECK(ARTSPConnection::ParseURL(
                    mSessionURL.c_str(), &host, &port, &path, &user, &pass));

        if (user.size() > 0) {
            mSessionURL.clear();
            mSessionURL.append("rtsp://");
            mSessionURL.append(host);
            mSessionURL.append(":");
            mSessionURL.append(AStringPrintf("%u", port));
            mSessionURL.append(path);

            ALOGV("rewritten session url: '%s'", mSessionURL.c_str());
        }

        mSessionHost = host;
    }

    void connect() {
        looper()->registerHandler(mConn);
        (1 ? mNetLooper : looper())->registerHandler(mRTPConn);

        sp<AMessage> notify = new AMessage('biny', this);
        mConn->observeBinaryData(notify);

        sp<AMessage> reply = new AMessage('conn', this);
        mConn->connect(mOriginalSessionURL.c_str(), reply);
    }

    void loadSDP(const sp<ASessionDescription>& desc) {
        looper()->registerHandler(mConn);
        (1 ? mNetLooper : looper())->registerHandler(mRTPConn);

        sp<AMessage> notify = new AMessage('biny', this);
        mConn->observeBinaryData(notify);

        sp<AMessage> reply = new AMessage('sdpl', this);
        reply->setObject("description", desc);
        mConn->connect(mOriginalSessionURL.c_str(), reply);
    }

    AString getControlURL() {
        AString sessionLevelControlURL;
        if (mSessionDesc->findAttribute(
                0,
                "a=control",
                &sessionLevelControlURL)) {
            if (sessionLevelControlURL.compare("*") == 0) {
                return mBaseURL;
            } else {
                AString controlURL;
                CHECK(MakeURL(
                        mBaseURL.c_str(),
                        sessionLevelControlURL.c_str(),
                        &controlURL));
                return controlURL;
            }
        } else {
            return mSessionURL;
        }
    }

    void disconnect() {
        (new AMessage('abor', this))->post();
    }

    void seek(int64_t timeUs) {
        sp<AMessage> msg = new AMessage('seek', this);
        msg->setInt64("time", timeUs);
        mPauseGeneration++;
        msg->post();
    }

    void continueSeekAfterPause(int64_t timeUs) {
        sp<AMessage> msg = new AMessage('see1', this);
        msg->setInt64("time", timeUs);
        msg->post();
    }

    bool isSeekable() const {
        return mSeekable;
    }

    void pause() {
        sp<AMessage> msg = new AMessage('paus', this);
        mPauseGeneration++;
        msg->setInt32("pausecheck", mPauseGeneration);
        msg->post();
    }

    void resume() {
        sp<AMessage> msg = new AMessage('resu', this);
        mPauseGeneration++;
        msg->post();
    }

    static void addRR(const sp<ABuffer> &buf) {
        uint8_t *ptr = buf->data() + buf->size();
        ptr[0] = 0x80 | 0;
        ptr[1] = 201;  // RR
        ptr[2] = 0;
        ptr[3] = 1;
        ptr[4] = 0xde;  // SSRC
        ptr[5] = 0xad;
        ptr[6] = 0xbe;
        ptr[7] = 0xef;

        buf->setRange(0, buf->size() + 8);
    }

    static void addSDES(int s, const sp<ABuffer> &buffer) {
        struct sockaddr_in addr;
        socklen_t addrSize = sizeof(addr);
        if (getsockname(s, (sockaddr *)&addr, &addrSize) != 0) {
            inet_aton("0.0.0.0", &(addr.sin_addr));
        }

        uint8_t *data = buffer->data() + buffer->size();
        data[0] = 0x80 | 1;
        data[1] = 202;  // SDES
        data[4] = 0xde;  // SSRC
        data[5] = 0xad;
        data[6] = 0xbe;
        data[7] = 0xef;

        size_t offset = 8;

        data[offset++] = 1;  // CNAME

        AString cname = "stagefright@";
        cname.append(inet_ntoa(addr.sin_addr));
        data[offset++] = cname.size();

        memcpy(&data[offset], cname.c_str(), cname.size());
        offset += cname.size();

        data[offset++] = 6;  // TOOL

        AString tool = MakeUserAgent();

        data[offset++] = tool.size();

        memcpy(&data[offset], tool.c_str(), tool.size());
        offset += tool.size();

        data[offset++] = 0;

        if ((offset % 4) > 0) {
            size_t count = 4 - (offset % 4);
            switch (count) {
                case 3:
                    data[offset++] = 0;
                    FALLTHROUGH_INTENDED;
                case 2:
                    data[offset++] = 0;
                    FALLTHROUGH_INTENDED;
                case 1:
                    data[offset++] = 0;
            }
        }

        size_t numWords = (offset / 4) - 1;
        data[2] = numWords >> 8;
        data[3] = numWords & 0xff;

        buffer->setRange(buffer->offset(), buffer->size() + offset);
    }

    // In case we're behind NAT, fire off two UDP packets to the remote
    // rtp/rtcp ports to poke a hole into the firewall for future incoming
    // packets. We're going to send an RR/SDES RTCP packet to both of them.
    bool pokeAHole(int rtpSocket, int rtcpSocket, const AString &transport) {
        struct sockaddr_in addr;
        memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
        addr.sin_family = AF_INET;

        AString source;
        AString server_port;
        if (!GetAttribute(transport.c_str(),
                          "source",
                          &source)) {
            ALOGW("Missing 'source' field in Transport response. Using "
                 "RTSP endpoint address.");

            struct hostent *ent = gethostbyname(mSessionHost.c_str());
            if (ent == NULL) {
                ALOGE("Failed to look up address of session host");

                return false;
            }

            addr.sin_addr.s_addr = *(in_addr_t *)ent->h_addr;
        } else {
            addr.sin_addr.s_addr = inet_addr(source.c_str());
        }

        if (!GetAttribute(transport.c_str(),
                                 "server_port",
                                 &server_port)) {
            ALOGI("Missing 'server_port' field in Transport response.");
            return false;
        }

        int rtpPort, rtcpPort;
        if (sscanf(server_port.c_str(), "%d-%d", &rtpPort, &rtcpPort) != 2
                || rtpPort <= 0 || rtpPort > 65535
                || rtcpPort <=0 || rtcpPort > 65535
                || rtcpPort != rtpPort + 1) {
            ALOGE("Server picked invalid RTP/RTCP port pair %s,"
                 " RTP port must be even, RTCP port must be one higher.",
                 server_port.c_str());

            return false;
        }

        if (rtpPort & 1) {
            ALOGW("Server picked an odd RTP port, it should've picked an "
                 "even one, we'll let it pass for now, but this may break "
                 "in the future.");
        }

        if (addr.sin_addr.s_addr == INADDR_NONE) {
            return true;
        }

        if (IN_LOOPBACK(ntohl(addr.sin_addr.s_addr))) {
            // No firewalls to traverse on the loopback interface.
            return true;
        }

        // Make up an RR/SDES RTCP packet.
        sp<ABuffer> buf = new ABuffer(65536);
        buf->setRange(0, 0);
        addRR(buf);
        addSDES(rtpSocket, buf);

        addr.sin_port = htons(rtpPort);

        ssize_t n = sendto(
                rtpSocket, buf->data(), buf->size(), 0,
                (const sockaddr *)&addr, sizeof(addr));

        if (n < (ssize_t)buf->size()) {
            ALOGE("failed to poke a hole for RTP packets");
            return false;
        }

        addr.sin_port = htons(rtcpPort);

        n = sendto(
                rtcpSocket, buf->data(), buf->size(), 0,
                (const sockaddr *)&addr, sizeof(addr));

        if (n < (ssize_t)buf->size()) {
            ALOGE("failed to poke a hole for RTCP packets");
            return false;
        }

        ALOGV("successfully poked holes.");

        return true;
    }

    static bool isLiveStream(const sp<ASessionDescription> &desc) {
        AString attrLiveStream;
        if (desc->findAttribute(0, "a=LiveStream", &attrLiveStream)) {
            ssize_t semicolonPos = attrLiveStream.find(";", 2);

            const char* liveStreamValue;
            if (semicolonPos < 0) {
                liveStreamValue = attrLiveStream.c_str();
            } else {
                AString valString;
                valString.setTo(attrLiveStream,
                        semicolonPos + 1,
                        attrLiveStream.size() - semicolonPos - 1);
                liveStreamValue = valString.c_str();
            }

            uint32_t value = strtoul(liveStreamValue, NULL, 10);
            if (value == 1) {
                ALOGV("found live stream");
                return true;
            }
        } else {
            // It is a live stream if no duration is returned
            int64_t durationUs;
            if (!desc->getDurationUs(&durationUs)) {
                ALOGV("No duration found, assume live stream");
                return true;
            }
        }

        return false;
    }

    virtual void onMessageReceived(const sp<AMessage> &msg) {
        switch (msg->what()) {
            case 'conn':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("connection request completed with result %d (%s)",
                     result, strerror(-result));

                if (result == OK) {
                    AString request;
                    request = "DESCRIBE ";
                    request.append(mSessionURL);
                    request.append(" RTSP/1.0\r\n");
                    request.append("Accept: application/sdp\r\n");
                    request.append("\r\n");

                    sp<AMessage> reply = new AMessage('desc', this);
                    mConn->sendRequest(request.c_str(), reply);
                } else {
                    (new AMessage('disc', this))->post();
                }
                break;
            }

            case 'disc':
            {
                ++mKeepAliveGeneration;

                int32_t reconnect;
                if (msg->findInt32("reconnect", &reconnect) && reconnect) {
                    sp<AMessage> reply = new AMessage('conn', this);
                    mConn->connect(mOriginalSessionURL.c_str(), reply);
                } else {
                    (new AMessage('quit', this))->post();
                }
                break;
            }

            case 'desc':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("DESCRIBE completed with result %d (%s)",
                     result, strerror(-result));

                if (result == OK) {
                    sp<RefBase> obj;
                    CHECK(msg->findObject("response", &obj));
                    sp<ARTSPResponse> response =
                        static_cast<ARTSPResponse *>(obj.get());

                    if (response->mStatusCode == 301 || response->mStatusCode == 302) {
                        ssize_t i = response->mHeaders.indexOfKey("location");
                        CHECK_GE(i, 0);

                        mOriginalSessionURL = response->mHeaders.valueAt(i);
                        mSessionURL = mOriginalSessionURL;

                        // Strip any authentication info from the session url, we don't
                        // want to transmit user/pass in cleartext.
                        AString host, path, user, pass;
                        unsigned port;
                        if (ARTSPConnection::ParseURL(
                                    mSessionURL.c_str(), &host, &port, &path, &user, &pass)
                                && user.size() > 0) {
                            mSessionURL.clear();
                            mSessionURL.append("rtsp://");
                            mSessionURL.append(host);
                            mSessionURL.append(":");
                            mSessionURL.append(AStringPrintf("%u", port));
                            mSessionURL.append(path);

                            ALOGV("rewritten session url: '%s'", mSessionURL.c_str());
                        }

                        sp<AMessage> reply = new AMessage('conn', this);
                        mConn->connect(mOriginalSessionURL.c_str(), reply);
                        break;
                    }

                    if (response->mStatusCode != 200) {
                        result = UNKNOWN_ERROR;
                    } else if (response->mContent == NULL) {
                        result = ERROR_MALFORMED;
                        ALOGE("The response has no content.");
                    } else {
                        mSessionDesc = new ASessionDescription;

                        mSessionDesc->setTo(
                                response->mContent->data(),
                                response->mContent->size());

                        if (!mSessionDesc->isValid()) {
                            ALOGE("Failed to parse session description.");
                            result = ERROR_MALFORMED;
                        } else {
                            ssize_t i = response->mHeaders.indexOfKey("content-base");
                            if (i >= 0) {
                                mBaseURL = response->mHeaders.valueAt(i);
                            } else {
                                i = response->mHeaders.indexOfKey("content-location");
                                if (i >= 0) {
                                    mBaseURL = response->mHeaders.valueAt(i);
                                } else {
                                    mBaseURL = mSessionURL;
                                }
                            }

                            mSeekable = !isLiveStream(mSessionDesc);

                            if (!mBaseURL.startsWith("rtsp://")) {
                                // Some misbehaving servers specify a relative
                                // URL in one of the locations above, combine
                                // it with the absolute session URL to get
                                // something usable...

                                ALOGW("Server specified a non-absolute base URL"
                                     ", combining it with the session URL to "
                                     "get something usable...");

                                AString tmp;
                                CHECK(MakeURL(
                                            mSessionURL.c_str(),
                                            mBaseURL.c_str(),
                                            &tmp));

                                mBaseURL = tmp;
                            }

                            mControlURL = getControlURL();

                            if (mSessionDesc->countTracks() < 2) {
                                // There's no actual tracks in this session.
                                // The first "track" is merely session meta
                                // data.

                                ALOGW("Session doesn't contain any playable "
                                     "tracks. Aborting.");
                                result = ERROR_UNSUPPORTED;
                            } else {
                                setupTrack(1);
                            }
                        }
                    }
                }

                if (result != OK) {
                    sp<AMessage> reply = new AMessage('disc', this);
                    mConn->disconnect(reply);
                }
                break;
            }

            case 'sdpl':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("SDP connection request completed with result %d (%s)",
                     result, strerror(-result));

                if (result == OK) {
                    sp<RefBase> obj;
                    CHECK(msg->findObject("description", &obj));
                    mSessionDesc =
                        static_cast<ASessionDescription *>(obj.get());

                    if (!mSessionDesc->isValid()) {
                        ALOGE("Failed to parse session description.");
                        result = ERROR_MALFORMED;
                    } else {
                        mBaseURL = mSessionURL;

                        mSeekable = !isLiveStream(mSessionDesc);

                        mControlURL = getControlURL();

                        if (mSessionDesc->countTracks() < 2) {
                            // There's no actual tracks in this session.
                            // The first "track" is merely session meta
                            // data.

                            ALOGW("Session doesn't contain any playable "
                                 "tracks. Aborting.");
                            result = ERROR_UNSUPPORTED;
                        } else {
                            setupTrack(1);
                        }
                    }
                }

                if (result != OK) {
                    sp<AMessage> reply = new AMessage('disc', this);
                    mConn->disconnect(reply);
                }
                break;
            }

            case 'setu':
            {
                size_t index;
                CHECK(msg->findSize("index", &index));

                TrackInfo *track = NULL;
                size_t trackIndex;
                if (msg->findSize("track-index", &trackIndex)) {
                    track = &mTracks.editItemAt(trackIndex);
                }

                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("SETUP(%zu) completed with result %d (%s)",
                     index, result, strerror(-result));

                if (result == OK) {
                    CHECK(track != NULL);

                    sp<RefBase> obj;
                    CHECK(msg->findObject("response", &obj));
                    sp<ARTSPResponse> response =
                        static_cast<ARTSPResponse *>(obj.get());

                    if (response->mStatusCode != 200) {
                        result = UNKNOWN_ERROR;
                    } else {
                        ssize_t i = response->mHeaders.indexOfKey("session");
                        CHECK_GE(i, 0);

                        mSessionID = response->mHeaders.valueAt(i);

                        mKeepAliveTimeoutUs = kDefaultKeepAliveTimeoutUs;
                        AString timeoutStr;
                        if (GetAttribute(
                                    mSessionID.c_str(), "timeout", &timeoutStr)) {
                            char *end;
                            unsigned long timeoutSecs =
                                strtoul(timeoutStr.c_str(), &end, 10);

                            if (end == timeoutStr.c_str() || *end != '\0') {
                                ALOGW("server specified malformed timeout '%s'",
                                     timeoutStr.c_str());

                                mKeepAliveTimeoutUs = kDefaultKeepAliveTimeoutUs;
                            } else if (timeoutSecs < 15) {
                                ALOGW("server specified too short a timeout "
                                     "(%lu secs), using default.",
                                     timeoutSecs);

                                mKeepAliveTimeoutUs = kDefaultKeepAliveTimeoutUs;
                            } else {
                                mKeepAliveTimeoutUs = timeoutSecs * 1000000ll;

                                ALOGI("server specified timeout of %lu secs.",
                                     timeoutSecs);
                            }
                        }

                        i = mSessionID.find(";");
                        if (i >= 0) {
                            // Remove options, i.e. ";timeout=90"
                            mSessionID.erase(i, mSessionID.size() - i);
                        }

                        sp<AMessage> notify = new AMessage('accu', this);
                        notify->setSize("track-index", trackIndex);

                        i = response->mHeaders.indexOfKey("transport");
                        CHECK_GE(i, 0);

                        if (track->mRTPSocket != -1 && track->mRTCPSocket != -1) {
                            if (!track->mUsingInterleavedTCP) {
                                AString transport = response->mHeaders.valueAt(i);

                                // We are going to continue even if we were
                                // unable to poke a hole into the firewall...
                                pokeAHole(
                                        track->mRTPSocket,
                                        track->mRTCPSocket,
                                        transport);
                            }

                            mRTPConn->addStream(
                                    track->mRTPSocket, track->mRTCPSocket,
                                    mSessionDesc, index,
                                    notify, track->mUsingInterleavedTCP);

                            mSetupTracksSuccessful = true;
                        } else {
                            result = BAD_VALUE;
                        }
                    }
                }

                if (result != OK) {
                    if (track) {
                        if (!track->mUsingInterleavedTCP) {
                            // Clear the tag
                            if (mUIDValid) {
                                NetworkUtils::UnRegisterSocketUserTag(track->mRTPSocket);
                                NetworkUtils::UnRegisterSocketUserMark(track->mRTPSocket);
                                NetworkUtils::UnRegisterSocketUserTag(track->mRTCPSocket);
                                NetworkUtils::UnRegisterSocketUserMark(track->mRTCPSocket);
                            }

                            close(track->mRTPSocket);
                            close(track->mRTCPSocket);
                        }

                        mTracks.removeItemsAt(trackIndex);
                    }
                }

                ++index;
                if (result == OK && index < mSessionDesc->countTracks()) {
                    setupTrack(index);
                } else if (mSetupTracksSuccessful) {
                    ++mKeepAliveGeneration;
                    postKeepAlive();

                    AString request = "PLAY ";
                    request.append(mControlURL);
                    request.append(" RTSP/1.0\r\n");

                    request.append("Session: ");
                    request.append(mSessionID);
                    request.append("\r\n");

                    request.append("\r\n");

                    sp<AMessage> reply = new AMessage('play', this);
                    mConn->sendRequest(request.c_str(), reply);
                } else {
                    sp<AMessage> reply = new AMessage('disc', this);
                    mConn->disconnect(reply);
                }
                break;
            }

            case 'play':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("PLAY completed with result %d (%s)",
                     result, strerror(-result));

                if (result == OK) {
                    sp<RefBase> obj;
                    CHECK(msg->findObject("response", &obj));
                    sp<ARTSPResponse> response =
                        static_cast<ARTSPResponse *>(obj.get());

                    if (response->mStatusCode != 200) {
                        result = UNKNOWN_ERROR;
                    } else {
                        parsePlayResponse(response);
                        postTimeout();
                    }
                }

                if (result != OK) {
                    sp<AMessage> reply = new AMessage('disc', this);
                    mConn->disconnect(reply);
                }

                break;
            }

            case 'aliv':
            {
                int32_t generation;
                CHECK(msg->findInt32("generation", &generation));

                if (generation != mKeepAliveGeneration) {
                    // obsolete event.
                    break;
                }

                AString request;
                request.append("OPTIONS ");
                request.append(mSessionURL);
                request.append(" RTSP/1.0\r\n");
                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");
                request.append("\r\n");

                sp<AMessage> reply = new AMessage('opts', this);
                reply->setInt32("generation", mKeepAliveGeneration);
                mConn->sendRequest(request.c_str(), reply);
                break;
            }

            case 'opts':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("OPTIONS completed with result %d (%s)",
                     result, strerror(-result));

                int32_t generation;
                CHECK(msg->findInt32("generation", &generation));

                if (generation != mKeepAliveGeneration) {
                    // obsolete event.
                    break;
                }

                postKeepAlive();
                break;
            }

            case 'abor':
            {
                for (size_t i = 0; i < mTracks.size(); ++i) {
                    TrackInfo *info = &mTracks.editItemAt(i);

                    if (!mFirstAccessUnit) {
                        postQueueEOS(i, ERROR_END_OF_STREAM);
                    }

                    if (!info->mUsingInterleavedTCP) {
                        mRTPConn->removeStream(info->mRTPSocket, info->mRTCPSocket);

                        // Clear the tag
                        if (mUIDValid) {
                            NetworkUtils::UnRegisterSocketUserTag(info->mRTPSocket);
                            NetworkUtils::UnRegisterSocketUserMark(info->mRTPSocket);
                            NetworkUtils::UnRegisterSocketUserTag(info->mRTCPSocket);
                            NetworkUtils::UnRegisterSocketUserMark(info->mRTCPSocket);
                        }

                        close(info->mRTPSocket);
                        close(info->mRTCPSocket);
                    }
                }
                mTracks.clear();
                mSetupTracksSuccessful = false;
                mSeekPending = false;
                mFirstAccessUnit = true;
                mAllTracksHaveTime = false;
                mNTPAnchorUs = -1;
                mMediaAnchorUs = -1;
                mNumAccessUnitsReceived = 0;
                mReceivedFirstRTCPPacket = false;
                mReceivedFirstRTPPacket = false;
                mPausing = false;
                mSeekable = true;

                sp<AMessage> reply = new AMessage('tear', this);

                int32_t reconnect;
                if (msg->findInt32("reconnect", &reconnect) && reconnect) {
                    reply->setInt32("reconnect", true);
                }

                AString request;
                request = "TEARDOWN ";

                // XXX should use aggregate url from SDP here...
                request.append(mSessionURL);
                request.append(" RTSP/1.0\r\n");

                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");

                request.append("\r\n");

                mConn->sendRequest(request.c_str(), reply);

                // If the response of teardown hasn't been received in 3 seconds,
                // post 'tear' message to avoid ANR.
                if (!msg->findInt32("reconnect", &reconnect) || !reconnect) {
                    sp<AMessage> teardown = reply->dup();
                    teardown->setInt32("result", -ECONNABORTED);
                    teardown->post(kTearDownTimeoutUs);
                }
                break;
            }

            case 'tear':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("TEARDOWN completed with result %d (%s)",
                     result, strerror(-result));

                sp<AMessage> reply = new AMessage('disc', this);

                int32_t reconnect;
                if (msg->findInt32("reconnect", &reconnect) && reconnect) {
                    reply->setInt32("reconnect", true);
                }

                mConn->disconnect(reply);
                break;
            }

            case 'quit':
            {
                sp<AMessage> msg = mNotify->dup();
                msg->setInt32("what", kWhatDisconnected);
                msg->setInt32("result", UNKNOWN_ERROR);
                msg->post();
                break;
            }

            case 'chek':
            {
                int32_t generation;
                CHECK(msg->findInt32("generation", &generation));
                if (generation != mCheckGeneration) {
                    // This is an outdated message. Ignore.
                    break;
                }

                if (mNumAccessUnitsReceived == 0) {
#if 1
                    ALOGI("stream ended? aborting.");
                    (new AMessage('abor', this))->post();
                    break;
#else
                    ALOGI("haven't seen an AU in a looong time.");
#endif
                }

                mNumAccessUnitsReceived = 0;
                msg->post(kAccessUnitTimeoutUs);
                break;
            }

            case 'accu':
            {
                if (mSeekPending) {
                    ALOGV("Stale access unit.");
                    break;
                }

                int32_t timeUpdate;
                if (msg->findInt32("time-update", &timeUpdate) && timeUpdate) {
                    size_t trackIndex;
                    CHECK(msg->findSize("track-index", &trackIndex));

                    uint32_t rtpTime;
                    uint64_t ntpTime;
                    CHECK(msg->findInt32("rtp-time", (int32_t *)&rtpTime));
                    CHECK(msg->findInt64("ntp-time", (int64_t *)&ntpTime));

                    onTimeUpdate(trackIndex, rtpTime, ntpTime);
                    break;
                }

                int32_t first;
                if (msg->findInt32("first-rtcp", &first)) {
                    mReceivedFirstRTCPPacket = true;
                    break;
                }

                if (msg->findInt32("first-rtp", &first)) {
                    mReceivedFirstRTPPacket = true;
                    break;
                }

                ++mNumAccessUnitsReceived;
                postAccessUnitTimeoutCheck();

                size_t trackIndex;
                CHECK(msg->findSize("track-index", &trackIndex));

                if (trackIndex >= mTracks.size()) {
                    ALOGV("late packets ignored.");
                    break;
                }

                TrackInfo *track = &mTracks.editItemAt(trackIndex);

                int32_t eos;
                if (msg->findInt32("eos", &eos)) {
                    ALOGI("received BYE on track index %zu", trackIndex);
                    if (!mAllTracksHaveTime && dataReceivedOnAllChannels()) {
                        ALOGI("No time established => fake existing data");

                        track->mEOSReceived = true;
                        mTryFakeRTCP = true;
                        mReceivedFirstRTCPPacket = true;
                        fakeTimestamps();
                    } else {
                        postQueueEOS(trackIndex, ERROR_END_OF_STREAM);
                    }
                    return;
                }

                if (mSeekPending) {
                    ALOGV("we're seeking, dropping stale packet.");
                    break;
                }

                sp<ABuffer> accessUnit;
                CHECK(msg->findBuffer("access-unit", &accessUnit));
                onAccessUnitComplete(trackIndex, accessUnit);
                break;
            }

            case 'paus':
            {
                int32_t generation;
                CHECK(msg->findInt32("pausecheck", &generation));
                if (generation != mPauseGeneration) {
                    ALOGV("Ignoring outdated pause message.");
                    break;
                }

                if (!mSeekable) {
                    ALOGW("This is a live stream, ignoring pause request.");
                    break;
                }

                if (mPausing) {
                    ALOGV("This stream is already paused.");
                    break;
                }

                mCheckPending = true;
                ++mCheckGeneration;
                mPausing = true;

                AString request = "PAUSE ";
                request.append(mControlURL);
                request.append(" RTSP/1.0\r\n");

                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");

                request.append("\r\n");

                sp<AMessage> reply = new AMessage('pau2', this);
                mConn->sendRequest(request.c_str(), reply);
                break;
            }

            case 'pau2':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));
                mCheckTimeoutGeneration++;

                ALOGI("PAUSE completed with result %d (%s)",
                     result, strerror(-result));
                break;
            }

            case 'resu':
            {
                if (mPausing && mSeekPending) {
                    // If seeking, Play will be sent from see1 instead
                    break;
                }

                if (!mPausing) {
                    // Dont send PLAY if we have not paused
                    break;
                }
                AString request = "PLAY ";
                request.append(mControlURL);
                request.append(" RTSP/1.0\r\n");

                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");

                request.append("\r\n");

                sp<AMessage> reply = new AMessage('res2', this);
                mConn->sendRequest(request.c_str(), reply);
                break;
            }

            case 'res2':
            {
                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("PLAY (for resume) completed with result %d (%s)",
                     result, strerror(-result));

                mCheckPending = false;
                ++mCheckGeneration;
                postAccessUnitTimeoutCheck();

                if (result == OK) {
                    sp<RefBase> obj;
                    CHECK(msg->findObject("response", &obj));
                    sp<ARTSPResponse> response =
                        static_cast<ARTSPResponse *>(obj.get());

                    if (response->mStatusCode != 200) {
                        result = UNKNOWN_ERROR;
                    } else {
                        parsePlayResponse(response);

                        // Post new timeout in order to make sure to use
                        // fake timestamps if no new Sender Reports arrive
                        postTimeout();
                    }
                }

                if (result != OK) {
                    ALOGE("resume failed, aborting.");
                    (new AMessage('abor', this))->post();
                }

                mPausing = false;
                break;
            }

            case 'seek':
            {
                if (!mSeekable) {
                    ALOGW("This is a live stream, ignoring seek request.");

                    sp<AMessage> msg = mNotify->dup();
                    msg->setInt32("what", kWhatSeekDone);
                    msg->post();
                    break;
                }

                int64_t timeUs;
                CHECK(msg->findInt64("time", &timeUs));

                mSeekPending = true;

                // Disable the access unit timeout until we resumed
                // playback again.
                mCheckPending = true;
                ++mCheckGeneration;

                sp<AMessage> reply = new AMessage('see0', this);
                reply->setInt64("time", timeUs);

                if (mPausing) {
                    // PAUSE already sent
                    ALOGI("Pause already sent");
                    reply->post();
                    break;
                }
                AString request = "PAUSE ";
                request.append(mControlURL);
                request.append(" RTSP/1.0\r\n");

                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");

                request.append("\r\n");

                mConn->sendRequest(request.c_str(), reply);
                break;
            }

            case 'see0':
            {
                // Session is paused now.
                status_t err = OK;
                msg->findInt32("result", &err);

                int64_t timeUs;
                CHECK(msg->findInt64("time", &timeUs));

                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatSeekPaused);
                notify->setInt32("err", err);
                notify->setInt64("time", timeUs);
                notify->post();
                break;

            }

            case 'see1':
            {
                for (size_t i = 0; i < mTracks.size(); ++i) {
                    TrackInfo *info = &mTracks.editItemAt(i);

                    postQueueSeekDiscontinuity(i);
                    info->mEOSReceived = false;

                    info->mRTPAnchor = 0;
                    info->mNTPAnchorUs = -1;
                }

                mAllTracksHaveTime = false;
                mNTPAnchorUs = -1;

                // Start new timeoutgeneration to avoid getting timeout
                // before PLAY response arrive
                postTimeout();

                int64_t timeUs;
                CHECK(msg->findInt64("time", &timeUs));

                AString request = "PLAY ";
                request.append(mControlURL);
                request.append(" RTSP/1.0\r\n");

                request.append("Session: ");
                request.append(mSessionID);
                request.append("\r\n");

                request.append(
                        AStringPrintf(
                            "Range: npt=%lld-\r\n", timeUs / 1000000ll));

                request.append("\r\n");

                sp<AMessage> reply = new AMessage('see2', this);
                mConn->sendRequest(request.c_str(), reply);
                break;
            }

            case 'see2':
            {
                if (mTracks.size() == 0) {
                    // We have already hit abor, break
                    break;
                }

                int32_t result;
                CHECK(msg->findInt32("result", &result));

                ALOGI("PLAY (for seek) completed with result %d (%s)",
                     result, strerror(-result));

                mCheckPending = false;
                ++mCheckGeneration;
                postAccessUnitTimeoutCheck();

                if (result == OK) {
                    sp<RefBase> obj;
                    CHECK(msg->findObject("response", &obj));
                    sp<ARTSPResponse> response =
                        static_cast<ARTSPResponse *>(obj.get());

                    if (response->mStatusCode != 200) {
                        result = UNKNOWN_ERROR;
                    } else {
                        parsePlayResponse(response);

                        // Post new timeout in order to make sure to use
                        // fake timestamps if no new Sender Reports arrive
                        postTimeout();

                        ssize_t i = response->mHeaders.indexOfKey("rtp-info");
                        CHECK_GE(i, 0);

                        ALOGV("rtp-info: %s", response->mHeaders.valueAt(i).c_str());

                        ALOGI("seek completed.");
                    }
                }

                if (result != OK) {
                    ALOGE("seek failed, aborting.");
                    (new AMessage('abor', this))->post();
                }

                mPausing = false;
                mSeekPending = false;

                // Discard all stale access units.
                for (size_t i = 0; i < mTracks.size(); ++i) {
                    TrackInfo *track = &mTracks.editItemAt(i);
                    track->mPackets.clear();
                }

                sp<AMessage> msg = mNotify->dup();
                msg->setInt32("what", kWhatSeekDone);
                msg->post();
                break;
            }

            case 'biny':
            {
                sp<ABuffer> buffer;
                CHECK(msg->findBuffer("buffer", &buffer));

                int32_t index;
                CHECK(buffer->meta()->findInt32("index", &index));

                mRTPConn->injectPacket(index, buffer);
                break;
            }

            case 'tiou':
            {
                int32_t timeoutGenerationCheck;
                CHECK(msg->findInt32("tioucheck", &timeoutGenerationCheck));
                if (timeoutGenerationCheck != mCheckTimeoutGeneration) {
                    // This is an outdated message. Ignore.
                    // This typically happens if a lot of seeks are
                    // performed, since new timeout messages now are
                    // posted at seek as well.
                    break;
                }
                if (!mReceivedFirstRTCPPacket) {
                    if (dataReceivedOnAllChannels() && !mTryFakeRTCP) {
                        ALOGW("We received RTP packets but no RTCP packets, "
                             "using fake timestamps.");

                        mTryFakeRTCP = true;

                        mReceivedFirstRTCPPacket = true;

                        fakeTimestamps();
                    } else if (!mReceivedFirstRTPPacket && !mTryTCPInterleaving) {
                        ALOGW("Never received any data, switching transports.");

                        mTryTCPInterleaving = true;

                        sp<AMessage> msg = new AMessage('abor', this);
                        msg->setInt32("reconnect", true);
                        msg->post();
                    } else {
                        ALOGW("Never received any data, disconnecting.");
                        (new AMessage('abor', this))->post();
                    }
                } else {
                    if (!mAllTracksHaveTime) {
                        ALOGW("We received some RTCP packets, but time "
                              "could not be established on all tracks, now "
                              "using fake timestamps");

                        fakeTimestamps();
                    }
                }
                break;
            }

            default:
                TRESPASS();
                break;
        }
    }

    void postKeepAlive() {
        sp<AMessage> msg = new AMessage('aliv', this);
        msg->setInt32("generation", mKeepAliveGeneration);
        msg->post((mKeepAliveTimeoutUs * 9) / 10);
    }

    void cancelAccessUnitTimeoutCheck() {
        ALOGV("cancelAccessUnitTimeoutCheck");
        ++mCheckGeneration;
    }

    void postAccessUnitTimeoutCheck() {
        if (mCheckPending) {
            return;
        }

        mCheckPending = true;
        sp<AMessage> check = new AMessage('chek', this);
        check->setInt32("generation", mCheckGeneration);
        check->post(kAccessUnitTimeoutUs);
    }

    static void SplitString(
            const AString &s, const char *separator, List<AString> *items) {
        items->clear();
        size_t start = 0;
        while (start < s.size()) {
            ssize_t offset = s.find(separator, start);

            if (offset < 0) {
                items->push_back(AString(s, start, s.size() - start));
                break;
            }

            items->push_back(AString(s, start, offset - start));
            start = offset + strlen(separator);
        }
    }

    void parsePlayResponse(const sp<ARTSPResponse> &response) {
        mPlayResponseParsed = true;
        if (mTracks.size() == 0) {
            ALOGV("parsePlayResponse: late packets ignored.");
            return;
        }

        ssize_t i = response->mHeaders.indexOfKey("range");
        if (i < 0) {
            // Server doesn't even tell use what range it is going to
            // play, therefore we won't support seeking.
            return;
        }

        AString range = response->mHeaders.valueAt(i);
        ALOGV("Range: %s", range.c_str());

        AString val;
        CHECK(GetAttribute(range.c_str(), "npt", &val));

        float npt1, npt2;
        if (!ASessionDescription::parseNTPRange(val.c_str(), &npt1, &npt2)) {
            // This is a live stream and therefore not seekable.

            ALOGI("This is a live stream");
            return;
        }

        i = response->mHeaders.indexOfKey("rtp-info");
        CHECK_GE(i, 0);

        AString rtpInfo = response->mHeaders.valueAt(i);
        List<AString> streamInfos;
        SplitString(rtpInfo, ",", &streamInfos);

        int n = 1;
        for (List<AString>::iterator it = streamInfos.begin();
             it != streamInfos.end(); ++it) {
            (*it).trim();
            ALOGV("streamInfo[%d] = %s", n, (*it).c_str());

            CHECK(GetAttribute((*it).c_str(), "url", &val));

            size_t trackIndex = 0;
            while (trackIndex < mTracks.size()
                    && !(val == mTracks.editItemAt(trackIndex).mURL)) {
                ++trackIndex;
            }
            CHECK_LT(trackIndex, mTracks.size());

            CHECK(GetAttribute((*it).c_str(), "seq", &val));

            char *end;
            unsigned long seq = strtoul(val.c_str(), &end, 10);

            TrackInfo *info = &mTracks.editItemAt(trackIndex);
            info->mFirstSeqNumInSegment = seq;
            info->mNewSegment = true;
            info->mAllowedStaleAccessUnits = kMaxAllowedStaleAccessUnits;

            CHECK(GetAttribute((*it).c_str(), "rtptime", &val));

            uint32_t rtpTime = strtoul(val.c_str(), &end, 10);

            ALOGV("track #%d: rtpTime=%u <=> npt=%.2f", n, rtpTime, npt1);

            info->mNormalPlayTimeRTP = rtpTime;
            info->mNormalPlayTimeUs = (int64_t)(npt1 * 1E6);

            if (!mFirstAccessUnit) {
                postNormalPlayTimeMapping(
                        trackIndex,
                        info->mNormalPlayTimeRTP, info->mNormalPlayTimeUs);
            }

            ++n;
        }
    }

    sp<MetaData> getTrackFormat(size_t index, int32_t *timeScale) {
        CHECK_GE(index, 0u);
        CHECK_LT(index, mTracks.size());

        const TrackInfo &info = mTracks.itemAt(index);

        *timeScale = info.mTimeScale;

        return info.mPacketSource->getFormat();
    }

    size_t countTracks() const {
        return mTracks.size();
    }

private:
    struct TrackInfo {
        AString mURL;
        int mRTPSocket;
        int mRTCPSocket;
        bool mUsingInterleavedTCP;
        uint32_t mFirstSeqNumInSegment;
        bool mNewSegment;
        int32_t mAllowedStaleAccessUnits;

        uint32_t mRTPAnchor;
        int64_t mNTPAnchorUs;
        int32_t mTimeScale;
        bool mEOSReceived;

        uint32_t mNormalPlayTimeRTP;
        int64_t mNormalPlayTimeUs;

        sp<APacketSource> mPacketSource;

        // Stores packets temporarily while no notion of time
        // has been established yet.
        List<sp<ABuffer> > mPackets;
    };

    sp<AMessage> mNotify;
    bool mUIDValid;
    uid_t mUID;
    sp<ALooper> mNetLooper;
    sp<ARTSPConnection> mConn;
    sp<ARTPConnection> mRTPConn;
    sp<ASessionDescription> mSessionDesc;
    AString mOriginalSessionURL;  // This one still has user:pass@
    AString mSessionURL;
    AString mSessionHost;
    AString mBaseURL;
    AString mControlURL;
    AString mSessionID;
    bool mSetupTracksSuccessful;
    bool mSeekPending;
    bool mFirstAccessUnit;

    bool mAllTracksHaveTime;
    int64_t mNTPAnchorUs;
    int64_t mMediaAnchorUs;
    int64_t mLastMediaTimeUs;

    int64_t mNumAccessUnitsReceived;
    bool mCheckPending;
    int32_t mCheckGeneration;
    int32_t mCheckTimeoutGeneration;
    bool mTryTCPInterleaving;
    bool mTryFakeRTCP;
    bool mReceivedFirstRTCPPacket;
    bool mReceivedFirstRTPPacket;
    bool mSeekable;
    int64_t mKeepAliveTimeoutUs;
    int32_t mKeepAliveGeneration;
    bool mPausing;
    int32_t mPauseGeneration;

    Vector<TrackInfo> mTracks;

    bool mPlayResponseParsed;

    void setupTrack(size_t index) {
        sp<APacketSource> source =
            new APacketSource(mSessionDesc, index);

        if (source->initCheck() != OK) {
            ALOGW("Unsupported format. Ignoring track #%zu.", index);

            sp<AMessage> reply = new AMessage('setu', this);
            reply->setSize("index", index);
            reply->setInt32("result", ERROR_UNSUPPORTED);
            reply->post();
            return;
        }

        AString url;
        CHECK(mSessionDesc->findAttribute(index, "a=control", &url));

        AString trackURL;
        CHECK(MakeURL(mBaseURL.c_str(), url.c_str(), &trackURL));

        mTracks.push(TrackInfo());
        TrackInfo *info = &mTracks.editItemAt(mTracks.size() - 1);
        info->mURL = trackURL;
        info->mPacketSource = source;
        info->mUsingInterleavedTCP = false;
        info->mFirstSeqNumInSegment = 0;
        info->mNewSegment = true;
        info->mAllowedStaleAccessUnits = kMaxAllowedStaleAccessUnits;
        info->mRTPSocket = -1;
        info->mRTCPSocket = -1;
        info->mRTPAnchor = 0;
        info->mNTPAnchorUs = -1;
        info->mNormalPlayTimeRTP = 0;
        info->mNormalPlayTimeUs = 0ll;

        unsigned long PT;
        AString formatDesc;
        AString formatParams;
        mSessionDesc->getFormatType(index, &PT, &formatDesc, &formatParams);

        int32_t timescale;
        int32_t numChannels;
        ASessionDescription::ParseFormatDesc(
                formatDesc.c_str(), &timescale, &numChannels);

        info->mTimeScale = timescale;
        info->mEOSReceived = false;

        ALOGV("track #%zu URL=%s", mTracks.size(), trackURL.c_str());

        AString request = "SETUP ";
        request.append(trackURL);
        request.append(" RTSP/1.0\r\n");

        if (mTryTCPInterleaving) {
            size_t interleaveIndex = 2 * (mTracks.size() - 1);
            info->mUsingInterleavedTCP = true;
            info->mRTPSocket = interleaveIndex;
            info->mRTCPSocket = interleaveIndex + 1;

            request.append("Transport: RTP/AVP/TCP;interleaved=");
            request.append(interleaveIndex);
            request.append("-");
            request.append(interleaveIndex + 1);
        } else {
            unsigned rtpPort;
            ARTPConnection::MakePortPair(
                    &info->mRTPSocket, &info->mRTCPSocket, &rtpPort);

            if (mUIDValid) {
                NetworkUtils::RegisterSocketUserTag(info->mRTPSocket, mUID,
                        (uint32_t)*(uint32_t*) "RTP_");
                NetworkUtils::RegisterSocketUserTag(info->mRTCPSocket, mUID,
                        (uint32_t)*(uint32_t*) "RTP_");
                NetworkUtils::RegisterSocketUserMark(info->mRTPSocket, mUID);
                NetworkUtils::RegisterSocketUserMark(info->mRTCPSocket, mUID);
            }

            request.append("Transport: RTP/AVP/UDP;unicast;client_port=");
            request.append(rtpPort);
            request.append("-");
            request.append(rtpPort + 1);
        }

        request.append("\r\n");

        if (index > 1) {
            request.append("Session: ");
            request.append(mSessionID);
            request.append("\r\n");
        }

        request.append("\r\n");

        sp<AMessage> reply = new AMessage('setu', this);
        reply->setSize("index", index);
        reply->setSize("track-index", mTracks.size() - 1);
        mConn->sendRequest(request.c_str(), reply);
    }

    static bool MakeURL(const char *baseURL, const char *url, AString *out) {
        out->clear();

        if (strncasecmp("rtsp://", baseURL, 7)) {
            // Base URL must be absolute
            return false;
        }

        if (!strncasecmp("rtsp://", url, 7)) {
            // "url" is already an absolute URL, ignore base URL.
            out->setTo(url);
            return true;
        }

        size_t n = strlen(baseURL);
        out->setTo(baseURL);
        if (baseURL[n - 1] != '/') {
            out->append("/");
        }
        out->append(url);

        return true;
    }

    void fakeTimestamps() {
        mNTPAnchorUs = -1ll;
        for (size_t i = 0; i < mTracks.size(); ++i) {
            onTimeUpdate(i, 0, 0ll);
        }
    }

    bool dataReceivedOnAllChannels() {
        TrackInfo *track;
        for (size_t i = 0; i < mTracks.size(); ++i) {
            track = &mTracks.editItemAt(i);
            if (track->mPackets.empty()) {
                return false;
            }
        }
        return true;
    }

    void handleFirstAccessUnit() {
        if (mFirstAccessUnit) {
            sp<AMessage> msg = mNotify->dup();
            msg->setInt32("what", kWhatConnected);
            msg->post();

            if (mSeekable) {
                for (size_t i = 0; i < mTracks.size(); ++i) {
                    TrackInfo *info = &mTracks.editItemAt(i);

                    postNormalPlayTimeMapping(
                            i,
                            info->mNormalPlayTimeRTP, info->mNormalPlayTimeUs);
                }
            }

            mFirstAccessUnit = false;
        }
    }

    void onTimeUpdate(int32_t trackIndex, uint32_t rtpTime, uint64_t ntpTime) {
        ALOGV("onTimeUpdate track %d, rtpTime = 0x%08x, ntpTime = %#016llx",
             trackIndex, rtpTime, (long long)ntpTime);

        int64_t ntpTimeUs = (int64_t)(ntpTime * 1E6 / (1ll << 32));

        TrackInfo *track = &mTracks.editItemAt(trackIndex);

        track->mRTPAnchor = rtpTime;
        track->mNTPAnchorUs = ntpTimeUs;

        if (mNTPAnchorUs < 0) {
            mNTPAnchorUs = ntpTimeUs;
            mMediaAnchorUs = mLastMediaTimeUs;
        }

        if (!mAllTracksHaveTime) {
            bool allTracksHaveTime = (mTracks.size() > 0);
            for (size_t i = 0; i < mTracks.size(); ++i) {
                TrackInfo *track = &mTracks.editItemAt(i);
                if (track->mNTPAnchorUs < 0) {
                    allTracksHaveTime = false;
                    break;
                }
            }
            if (allTracksHaveTime) {
                mAllTracksHaveTime = true;
                ALOGI("Time now established for all tracks.");
            }
        }
        if (mAllTracksHaveTime && dataReceivedOnAllChannels()) {
            handleFirstAccessUnit();

            // Time is now established, lets start timestamping immediately
            for (size_t i = 0; i < mTracks.size(); ++i) {
                if (OK != processAccessUnitQueue(i)) {
                    return;
                }
            }
            for (size_t i = 0; i < mTracks.size(); ++i) {
                TrackInfo *trackInfo = &mTracks.editItemAt(i);
                if (trackInfo->mEOSReceived) {
                    postQueueEOS(i, ERROR_END_OF_STREAM);
                    trackInfo->mEOSReceived = false;
                }
            }
        }
    }

    status_t processAccessUnitQueue(int32_t trackIndex) {
        TrackInfo *track = &mTracks.editItemAt(trackIndex);
        while (!track->mPackets.empty()) {
            sp<ABuffer> accessUnit = *track->mPackets.begin();
            track->mPackets.erase(track->mPackets.begin());

            uint32_t seqNum = (uint32_t)accessUnit->int32Data();
            if (track->mNewSegment) {
                // The sequence number from RTP packet has only 16 bits and is extended
                // by ARTPSource. Only the low 16 bits of seq in RTP-Info of reply of
                // RTSP "PLAY" command should be used to detect the first RTP packet
                // after seeking.
                if (mSeekable) {
                    if (track->mAllowedStaleAccessUnits > 0) {
                        uint32_t seqNum16 = seqNum & 0xffff;
                        uint32_t firstSeqNumInSegment16 = track->mFirstSeqNumInSegment & 0xffff;
                        if (seqNum16 > firstSeqNumInSegment16 + kMaxAllowedStaleAccessUnits
                                || seqNum16 < firstSeqNumInSegment16) {
                            // Not the first rtp packet of the stream after seeking, discarding.
                            track->mAllowedStaleAccessUnits--;
                            ALOGV("discarding stale access unit (0x%x : 0x%x)",
                                 seqNum, track->mFirstSeqNumInSegment);
                            continue;
                        }
                        ALOGW_IF(seqNum16 != firstSeqNumInSegment16,
                                "Missing the first packet(%u), now take packet(%u) as first one",
                                track->mFirstSeqNumInSegment, seqNum);
                    } else { // track->mAllowedStaleAccessUnits <= 0
                        mNumAccessUnitsReceived = 0;
                        ALOGW_IF(track->mAllowedStaleAccessUnits == 0,
                             "Still no first rtp packet after %d stale ones",
                             kMaxAllowedStaleAccessUnits);
                        track->mAllowedStaleAccessUnits = -1;
                        return UNKNOWN_ERROR;
                    }
                }

                // Now found the first rtp packet of the stream after seeking.
                track->mFirstSeqNumInSegment = seqNum;
                track->mNewSegment = false;
            }

            if (seqNum < track->mFirstSeqNumInSegment) {
                ALOGV("dropping stale access-unit (%d < %d)",
                     seqNum, track->mFirstSeqNumInSegment);
                continue;
            }

            if (addMediaTimestamp(trackIndex, track, accessUnit)) {
                postQueueAccessUnit(trackIndex, accessUnit);
            }
        }
        return OK;
    }

    void onAccessUnitComplete(
            int32_t trackIndex, const sp<ABuffer> &accessUnit) {
        TrackInfo *track = &mTracks.editItemAt(trackIndex);
        track->mPackets.push_back(accessUnit);

        uint32_t seqNum = (uint32_t)accessUnit->int32Data();
        ALOGV("onAccessUnitComplete track %d storing accessunit %u", trackIndex, seqNum);

        if(!mPlayResponseParsed){
            ALOGV("play response is not parsed");
            return;
        }

        handleFirstAccessUnit();

        if (!mAllTracksHaveTime) {
            ALOGV("storing accessUnit, no time established yet");
            return;
        }

        if (OK != processAccessUnitQueue(trackIndex)) {
            return;
        }

        if (track->mEOSReceived) {
            postQueueEOS(trackIndex, ERROR_END_OF_STREAM);
            track->mEOSReceived = false;
        }
    }

    bool addMediaTimestamp(
            int32_t trackIndex, const TrackInfo *track,
            const sp<ABuffer> &accessUnit) {
        UNUSED_UNLESS_VERBOSE(trackIndex);

        uint32_t rtpTime;
        CHECK(accessUnit->meta()->findInt32(
                    "rtp-time", (int32_t *)&rtpTime));

        int64_t relRtpTimeUs =
            (((int64_t)rtpTime - (int64_t)track->mRTPAnchor) * 1000000ll)
                / track->mTimeScale;

        int64_t ntpTimeUs = track->mNTPAnchorUs + relRtpTimeUs;

        int64_t mediaTimeUs = mMediaAnchorUs + ntpTimeUs - mNTPAnchorUs;

        if (mediaTimeUs > mLastMediaTimeUs) {
            mLastMediaTimeUs = mediaTimeUs;
        }

        if (mediaTimeUs < 0 && !mSeekable) {
            ALOGV("dropping early accessUnit.");
            return false;
        }

        ALOGV("track %d rtpTime=%u mediaTimeUs = %lld us (%.2f secs)",
             trackIndex, rtpTime, (long long)mediaTimeUs, mediaTimeUs / 1E6);

        accessUnit->meta()->setInt64("timeUs", mediaTimeUs);

        return true;
    }

    void postQueueAccessUnit(
            size_t trackIndex, const sp<ABuffer> &accessUnit) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatAccessUnit);
        msg->setSize("trackIndex", trackIndex);
        msg->setBuffer("accessUnit", accessUnit);
        msg->post();
    }

    void postQueueEOS(size_t trackIndex, status_t finalResult) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatEOS);
        msg->setSize("trackIndex", trackIndex);
        msg->setInt32("finalResult", finalResult);
        msg->post();
    }

    void postQueueSeekDiscontinuity(size_t trackIndex) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatSeekDiscontinuity);
        msg->setSize("trackIndex", trackIndex);
        msg->post();
    }

    void postNormalPlayTimeMapping(
            size_t trackIndex, uint32_t rtpTime, int64_t nptUs) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatNormalPlayTimeMapping);
        msg->setSize("trackIndex", trackIndex);
        msg->setInt32("rtpTime", rtpTime);
        msg->setInt64("nptUs", nptUs);
        msg->post();
    }

    void postTimeout() {
        sp<AMessage> timeout = new AMessage('tiou', this);
        mCheckTimeoutGeneration++;
        timeout->setInt32("tioucheck", mCheckTimeoutGeneration);

        int64_t startupTimeoutUs;
        startupTimeoutUs = property_get_int64("media.rtsp.timeout-us", kStartupTimeoutUs);
        timeout->post(startupTimeoutUs);
    }

    DISALLOW_EVIL_CONSTRUCTORS(MyHandler);
};

}  // namespace android

#endif  // MY_HANDLER_H_
