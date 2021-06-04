/*
 * Copyright (C) 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "codec2_hidl_hal_video_dec_test"

#include <android-base/logging.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <fstream>

#include <codec2/hidl/client.h>
#include <C2AllocatorIon.h>
#include <C2Config.h>
#include <C2Debug.h>
#include <C2Buffer.h>
#include <C2BufferPriv.h>

using android::C2AllocatorIon;

#include <VtsHalHidlTargetTestBase.h>
#include "media_c2_video_hidl_test_common.h"
#include "media_c2_hidl_test_common.h"

struct FrameInfo {
    int bytesCount;
    uint32_t flags;
    int64_t timestamp;
};

class LinearBuffer : public C2Buffer {
   public:
    explicit LinearBuffer(const std::shared_ptr<C2LinearBlock>& block)
        : C2Buffer(
              {block->share(block->offset(), block->size(), ::C2Fence())}) {}

    explicit LinearBuffer(const std::shared_ptr<C2LinearBlock>& block, size_t size)
        : C2Buffer(
              {block->share(block->offset(), size, ::C2Fence())}) {}
};

static ComponentTestEnvironment* gEnv = nullptr;

namespace {

class Codec2VideoDecHidlTest : public ::testing::VtsHalHidlTargetTestBase {
   private:
    typedef ::testing::VtsHalHidlTargetTestBase Super;

   public:
    ::std::string getTestCaseInfo() const override {
        return ::std::string() +
                "Component: " + gEnv->getComponent().c_str() + " | " +
                "Instance: " + gEnv->getInstance().c_str() + " | " +
                "Res: " + gEnv->getRes().c_str();
    }

    // google.codec2 Video test setup
    virtual void SetUp() override {
        Super::SetUp();
        mDisableTest = false;
        ALOGV("Codec2VideoDecHidlTest SetUp");
        mClient = android::Codec2Client::CreateFromService(
            gEnv->getInstance().c_str());
        ASSERT_NE(mClient, nullptr);
        mListener.reset(new CodecListener(
            [this](std::list<std::unique_ptr<C2Work>>& workItems) {
                handleWorkDone(workItems);
            }));
        ASSERT_NE(mListener, nullptr);
        for (int i = 0; i < MAX_INPUT_BUFFERS; ++i) {
            mWorkQueue.emplace_back(new C2Work);
        }
        mClient->createComponent(gEnv->getComponent().c_str(), mListener,
                                 &mComponent);
        ASSERT_NE(mComponent, nullptr);

        std::shared_ptr<C2AllocatorStore> store =
            android::GetCodec2PlatformAllocatorStore();
        CHECK_EQ(store->fetchAllocator(C2AllocatorStore::DEFAULT_LINEAR,
                                       &mLinearAllocator),
                 C2_OK);
        mLinearPool = std::make_shared<C2PooledBlockPool>(mLinearAllocator,
                                                          mBlockPoolId++);
        ASSERT_NE(mLinearPool, nullptr);

        mCompName = unknown_comp;
        struct StringToName {
            const char* Name;
            standardComp CompName;
        };

        const StringToName kStringToName[] = {
            {"h263", h263}, {"avc", avc}, {"mpeg2", mpeg2}, {"mpeg4", mpeg4},
            {"hevc", hevc}, {"vp8", vp8}, {"vp9", vp9}, {"av1", av1},
        };

        const size_t kNumStringToName =
            sizeof(kStringToName) / sizeof(kStringToName[0]);

        // Find the component type
        std::string comp = std::string(gEnv->getComponent());
        for (size_t i = 0; i < kNumStringToName; ++i) {
            if (strcasestr(comp.c_str(), kStringToName[i].Name)) {
                mCompName = kStringToName[i].CompName;
                break;
            }
        }
        mEos = false;
        mFramesReceived = 0;
        mTimestampUs = 0u;
        mTimestampDevTest = false;
        if (mCompName == unknown_comp) mDisableTest = true;

        C2SecureModeTuning secureModeTuning{};
        mComponent->query({ &secureModeTuning }, {}, C2_MAY_BLOCK, nullptr);
        if (secureModeTuning.value == C2Config::SM_READ_PROTECTED) {
            mDisableTest = true;
        }

        if (mDisableTest) std::cout << "[   WARN   ] Test Disabled \n";
    }

    virtual void TearDown() override {
        if (mComponent != nullptr) {
            if (::testing::Test::HasFatalFailure()) return;
            mComponent->release();
            mComponent = nullptr;
        }
        Super::TearDown();
    }

    // callback function to process onWorkDone received by Listener
    void handleWorkDone(std::list<std::unique_ptr<C2Work>>& workItems) {
        for (std::unique_ptr<C2Work>& work : workItems) {
            if (!work->worklets.empty()) {
                // For decoder components current timestamp always exceeds
                // previous timestamp
                typedef std::unique_lock<std::mutex> ULock;
                bool codecConfig = ((work->worklets.front()->output.flags &
                                     C2FrameData::FLAG_CODEC_CONFIG) != 0);
                if (!codecConfig &&
                    !work->worklets.front()->output.buffers.empty()) {
                    EXPECT_GE(
                        (work->worklets.front()->output.ordinal.timestamp.peeku()),
                        mTimestampUs);
                    mTimestampUs =
                        work->worklets.front()->output.ordinal.timestamp.peeku();

                    ULock l(mQueueLock);
                    if (mTimestampDevTest) {
                        bool tsHit = false;
                        std::list<uint64_t>::iterator it = mTimestampUslist.begin();
                        while (it != mTimestampUslist.end()) {
                            if (*it == mTimestampUs) {
                                mTimestampUslist.erase(it);
                                tsHit = true;
                                break;
                            }
                            it++;
                        }
                        if (tsHit == false) {
                            if (mTimestampUslist.empty() == false) {
                                EXPECT_EQ(tsHit, true)
                                    << "TimeStamp not recognized";
                            } else {
                                std::cout << "[   INFO   ] Received non-zero "
                                             "output / TimeStamp not recognized \n";
                            }
                        }
                    }
                }
                bool mCsd;
                workDone(mComponent, work, mFlushedIndices, mQueueLock,
                         mQueueCondition, mWorkQueue, mEos, mCsd,
                         mFramesReceived);
                (void)mCsd;
            }
        }
    }

    enum standardComp {
        h263,
        avc,
        mpeg2,
        mpeg4,
        hevc,
        vp8,
        vp9,
        av1,
        unknown_comp,
    };

    bool mEos;
    bool mDisableTest;
    bool mTimestampDevTest;
    uint64_t mTimestampUs;
    std::list<uint64_t> mTimestampUslist;
    std::list<uint64_t> mFlushedIndices;
    standardComp mCompName;
    uint32_t mFramesReceived;
    C2BlockPool::local_id_t mBlockPoolId;
    std::shared_ptr<C2BlockPool> mLinearPool;
    std::shared_ptr<C2Allocator> mLinearAllocator;

    std::mutex mQueueLock;
    std::condition_variable mQueueCondition;
    std::list<std::unique_ptr<C2Work>> mWorkQueue;

    std::shared_ptr<android::Codec2Client> mClient;
    std::shared_ptr<android::Codec2Client::Listener> mListener;
    std::shared_ptr<android::Codec2Client::Component> mComponent;

   protected:
    static void description(const std::string& description) {
        RecordProperty("description", description);
    }
};

void validateComponent(
    const std::shared_ptr<android::Codec2Client::Component>& component,
    Codec2VideoDecHidlTest::standardComp compName, bool& disableTest) {
    // Validate its a C2 Component
    if (component->getName().find("c2") == std::string::npos) {
        ALOGE("Not a c2 component");
        disableTest = true;
        return;
    }

    // Validate its not an encoder and the component to be tested is video
    if (component->getName().find("encoder") != std::string::npos) {
        ALOGE("Expected Decoder, given Encoder");
        disableTest = true;
        return;
    }
    std::vector<std::unique_ptr<C2Param>> queried;
    c2_status_t c2err =
        component->query({}, {C2PortMediaTypeSetting::input::PARAM_TYPE},
                         C2_DONT_BLOCK, &queried);
    if (c2err != C2_OK && queried.size() == 0) {
        ALOGE("Query media type failed => %d", c2err);
    } else {
        std::string inputDomain =
            ((C2StreamMediaTypeSetting::input*)queried[0].get())->m.value;
        if (inputDomain.find("video/") == std::string::npos) {
            ALOGE("Expected Video Component");
            disableTest = true;
            return;
        }
    }

    // Validates component name
    if (compName == Codec2VideoDecHidlTest::unknown_comp) {
        ALOGE("Component InValid");
        disableTest = true;
        return;
    }
    ALOGV("Component Valid");
}

// number of elementary streams per component
#define STREAM_COUNT 2
// LookUpTable of clips and metadata for component testing
void GetURLForComponent(Codec2VideoDecHidlTest::standardComp comp, char* mURL,
                        char* info, size_t streamIndex = 1) {
    struct CompToURL {
        Codec2VideoDecHidlTest::standardComp comp;
        const char mURL[STREAM_COUNT][512];
        const char info[STREAM_COUNT][512];
    };
    ASSERT_TRUE(streamIndex < STREAM_COUNT);

    static const CompToURL kCompToURL[] = {
        {Codec2VideoDecHidlTest::standardComp::avc,
         {"bbb_avc_176x144_300kbps_60fps.h264",
          "bbb_avc_640x360_768kbps_30fps.h264"},
         {"bbb_avc_176x144_300kbps_60fps.info",
          "bbb_avc_640x360_768kbps_30fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::hevc,
         {"bbb_hevc_176x144_176kbps_60fps.hevc",
          "bbb_hevc_640x360_1600kbps_30fps.hevc"},
         {"bbb_hevc_176x144_176kbps_60fps.info",
          "bbb_hevc_640x360_1600kbps_30fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::mpeg2,
         {"bbb_mpeg2_176x144_105kbps_25fps.m2v",
          "bbb_mpeg2_352x288_1mbps_60fps.m2v"},
         {"bbb_mpeg2_176x144_105kbps_25fps.info",
          "bbb_mpeg2_352x288_1mbps_60fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::h263,
         {"", "bbb_h263_352x288_300kbps_12fps.h263"},
         {"", "bbb_h263_352x288_300kbps_12fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::mpeg4,
         {"", "bbb_mpeg4_352x288_512kbps_30fps.m4v"},
         {"", "bbb_mpeg4_352x288_512kbps_30fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::vp8,
         {"bbb_vp8_176x144_240kbps_60fps.vp8",
          "bbb_vp8_640x360_2mbps_30fps.vp8"},
         {"bbb_vp8_176x144_240kbps_60fps.info",
          "bbb_vp8_640x360_2mbps_30fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::vp9,
         {"bbb_vp9_176x144_285kbps_60fps.vp9",
          "bbb_vp9_640x360_1600kbps_30fps.vp9"},
         {"bbb_vp9_176x144_285kbps_60fps.info",
          "bbb_vp9_640x360_1600kbps_30fps.info"}},
        {Codec2VideoDecHidlTest::standardComp::av1,
         {"bbb_av1_640_360.av1",
          "bbb_av1_176_144.av1"},
         {"bbb_av1_640_360.info",
          "bbb_av1_176_144.info"}},
    };

    for (size_t i = 0; i < sizeof(kCompToURL) / sizeof(kCompToURL[0]); ++i) {
        if (kCompToURL[i].comp == comp) {
            strcat(mURL, kCompToURL[i].mURL[streamIndex]);
            strcat(info, kCompToURL[i].info[streamIndex]);
            return;
        }
    }
}

void decodeNFrames(const std::shared_ptr<android::Codec2Client::Component>& component,
                   std::mutex &queueLock, std::condition_variable& queueCondition,
                   std::list<std::unique_ptr<C2Work>>& workQueue,
                   std::list<uint64_t>& flushedIndices,
                   std::shared_ptr<C2BlockPool>& linearPool,
                   std::ifstream& eleStream,
                   android::Vector<FrameInfo>* Info,
                   int offset, int range, bool signalEOS = true) {
    typedef std::unique_lock<std::mutex> ULock;
    int frameID = offset;
    int maxRetry = 0;
    while (1) {
        if (frameID == (int)Info->size() || frameID == (offset + range)) break;
        uint32_t flags = 0;
        std::unique_ptr<C2Work> work;
        // Prepare C2Work
        while (!work && (maxRetry < MAX_RETRY)) {
            ULock l(queueLock);
            if (!workQueue.empty()) {
                work.swap(workQueue.front());
                workQueue.pop_front();
            } else {
                queueCondition.wait_for(l, TIME_OUT);
                maxRetry++;
            }
        }
        if (!work && (maxRetry >= MAX_RETRY)) {
            ASSERT_TRUE(false) << "Wait for generating C2Work exceeded timeout";
        }
        int64_t timestamp = (*Info)[frameID].timestamp;
        if ((*Info)[frameID].flags) flags = (1 << ((*Info)[frameID].flags - 1));
        if (signalEOS && ((frameID == (int)Info->size() - 1) ||
                          (frameID == (offset + range - 1))))
            flags |= C2FrameData::FLAG_END_OF_STREAM;

        work->input.flags = (C2FrameData::flags_t)flags;
        work->input.ordinal.timestamp = timestamp;
        work->input.ordinal.frameIndex = frameID;
        {
            ULock l(queueLock);
            flushedIndices.emplace_back(frameID);
        }

        int size = (*Info)[frameID].bytesCount;
        char* data = (char*)malloc(size);
        ASSERT_NE(data, nullptr);

        eleStream.read(data, size);
        ASSERT_EQ(eleStream.gcount(), size);

        work->input.buffers.clear();
        auto alignedSize = ALIGN(size, PAGE_SIZE);
        if (size) {
            std::shared_ptr<C2LinearBlock> block;
            ASSERT_EQ(C2_OK,
                    linearPool->fetchLinearBlock(
                        alignedSize, {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE},
                        &block));
            ASSERT_TRUE(block);

            // Write View
            C2WriteView view = block->map().get();
            if (view.error() != C2_OK) {
                fprintf(stderr, "C2LinearBlock::map() failed : %d", view.error());
                break;
            }
            ASSERT_EQ((size_t)alignedSize, view.capacity());
            ASSERT_EQ(0u, view.offset());
            ASSERT_EQ((size_t)alignedSize, view.size());

            memcpy(view.base(), data, size);

            work->input.buffers.emplace_back(new LinearBuffer(block, size));
            free(data);
        }
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);
        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        // DO THE DECODING
        ASSERT_EQ(component->queue(&items), C2_OK);
        ALOGV("Frame #%d size = %d queued", frameID, size);
        frameID++;
        maxRetry = 0;
    }
}

TEST_F(Codec2VideoDecHidlTest, validateCompName) {
    if (mDisableTest) return;
    ALOGV("Checks if the given component is a valid video component");
    validateComponent(mComponent, mCompName, mDisableTest);
    ASSERT_EQ(mDisableTest, false);
}

class Codec2VideoDecDecodeTest
    : public Codec2VideoDecHidlTest,
      public ::testing::WithParamInterface<std::pair<int32_t, bool>> {
};

// Bitstream Test
TEST_P(Codec2VideoDecDecodeTest, DecodeTest) {
    description("Decodes input file");
    if (mDisableTest) return;

    uint32_t streamIndex = GetParam().first;
    bool signalEOS = GetParam().second;
    char mURL[512], info[512];
    std::ifstream eleStream, eleInfo;
    strcpy(mURL, gEnv->getRes().c_str());
    strcpy(info, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL, info, streamIndex);

    eleInfo.open(info);
    ASSERT_EQ(eleInfo.is_open(), true) << mURL << " - file not found";
    android::Vector<FrameInfo> Info;
    int bytesCount = 0;
    uint32_t flags = 0;
    uint32_t timestamp = 0;
    mTimestampDevTest = true;
    mFlushedIndices.clear();
    mTimestampUslist.clear();
    while (1) {
        if (!(eleInfo >> bytesCount)) break;
        eleInfo >> flags;
        eleInfo >> timestamp;
        bool codecConfig = flags ?
            ((1 << (flags - 1)) & C2FrameData::FLAG_CODEC_CONFIG) != 0 : 0;
        if (mTimestampDevTest && !codecConfig)
            mTimestampUslist.push_back(timestamp);
        Info.push_back({bytesCount, flags, timestamp});
    }
    eleInfo.close();

    ASSERT_EQ(mComponent->start(), C2_OK);
    // Reset total no of frames received
    mFramesReceived = 0;
    mTimestampUs = 0;
    ALOGV("mURL : %s", mURL);
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    ASSERT_NO_FATAL_FAILURE(decodeNFrames(
        mComponent, mQueueLock, mQueueCondition, mWorkQueue, mFlushedIndices,
        mLinearPool, eleStream, &Info, 0, (int)Info.size(), signalEOS));

    // If EOS is not sent, sending empty input with EOS flag
    size_t infoSize = Info.size();
    if (!signalEOS) {
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue, 1));
        ASSERT_NO_FATAL_FAILURE(
            testInputBuffer(mComponent, mQueueLock, mWorkQueue,
                            C2FrameData::FLAG_END_OF_STREAM, false));
        infoSize += 1;
    }
    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    if (!mEos) {
        ALOGV("Waiting for input consumption");
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));
    }

    eleStream.close();
    if (mFramesReceived != infoSize) {
        ALOGE("Input buffer count and Output buffer count mismatch");
        ALOGV("framesReceived : %d inputFrames : %zu", mFramesReceived,
              infoSize);
        ASSERT_TRUE(false);
    }

    if (mTimestampDevTest) EXPECT_EQ(mTimestampUslist.empty(), true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}
// DecodeTest with StreamIndex and EOS / No EOS
INSTANTIATE_TEST_CASE_P(StreamIndexAndEOS, Codec2VideoDecDecodeTest,
                        ::testing::Values(std::make_pair(0, false),
                                          std::make_pair(0, true),
                                          std::make_pair(1, false),
                                          std::make_pair(1, true)));

// Adaptive Test
TEST_F(Codec2VideoDecHidlTest, AdaptiveDecodeTest) {
    description("Adaptive Decode Test");
    if (mDisableTest) return;
    if (!(mCompName == avc || mCompName == hevc || mCompName == vp8 ||
          mCompName == vp9 || mCompName == mpeg2))
        return;

    typedef std::unique_lock<std::mutex> ULock;
    ASSERT_EQ(mComponent->start(), C2_OK);

    mTimestampDevTest = true;
    uint32_t timestampOffset = 0;
    uint32_t offset = 0;
    android::Vector<FrameInfo> Info;
    for (uint32_t i = 0; i < STREAM_COUNT * 2; i++) {
        char mURL[512], info[512];
        std::ifstream eleStream, eleInfo;

        strcpy(mURL, gEnv->getRes().c_str());
        strcpy(info, gEnv->getRes().c_str());
        GetURLForComponent(mCompName, mURL, info, i % STREAM_COUNT);

        eleInfo.open(info);
        ASSERT_EQ(eleInfo.is_open(), true) << mURL << " - file not found";
        int bytesCount = 0;
        uint32_t flags = 0;
        uint32_t timestamp = 0;
        uint32_t timestampMax = 0;
        while (1) {
            if (!(eleInfo >> bytesCount)) break;
            eleInfo >> flags;
            eleInfo >> timestamp;
            timestamp += timestampOffset;
            Info.push_back({bytesCount, flags, timestamp});
            bool codecConfig = flags ?
                ((1 << (flags - 1)) & C2FrameData::FLAG_CODEC_CONFIG) != 0 : 0;

            {
                ULock l(mQueueLock);
                if (mTimestampDevTest && !codecConfig)
                    mTimestampUslist.push_back(timestamp);
            }
            if (timestampMax < timestamp) timestampMax = timestamp;
        }
        timestampOffset = timestampMax;
        eleInfo.close();

        // Reset Total frames before second decode loop
        // mFramesReceived = 0;
        ALOGV("mURL : %s", mURL);
        eleStream.open(mURL, std::ifstream::binary);
        ASSERT_EQ(eleStream.is_open(), true);
        ASSERT_NO_FATAL_FAILURE(
            decodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                          mFlushedIndices, mLinearPool, eleStream, &Info,
                          offset, (int)(Info.size() - offset), false));

        eleStream.close();
        offset = (int)Info.size();
    }

    // Send EOS
    // TODO Add function to send EOS work item
    int maxRetry = 0;
    std::unique_ptr<C2Work> work;
    while (!work && (maxRetry < MAX_RETRY)) {
        ULock l(mQueueLock);
        if (!mWorkQueue.empty()) {
            work.swap(mWorkQueue.front());
            mWorkQueue.pop_front();
        } else {
            mQueueCondition.wait_for(l, TIME_OUT);
            maxRetry++;
        }
    }
    ASSERT_NE(work, nullptr);
    work->input.flags = (C2FrameData::flags_t)C2FrameData::FLAG_END_OF_STREAM;
    work->input.ordinal.timestamp = 0;
    work->input.ordinal.frameIndex = 0;
    work->input.buffers.clear();
    work->worklets.clear();
    work->worklets.emplace_back(new C2Worklet);

    std::list<std::unique_ptr<C2Work>> items;
    items.push_back(std::move(work));
    ASSERT_EQ(mComponent->queue(&items), C2_OK);

    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    ALOGV("Waiting for input consumption");
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));

    if (mFramesReceived != ((Info.size()) + 1)) {
        ALOGE("Input buffer count and Output buffer count mismatch");
        ALOGV("framesReceived : %d inputFrames : %zu", mFramesReceived,
              Info.size() + 1);
        ASSERT_TRUE(false);
    }

    if (mTimestampDevTest) EXPECT_EQ(mTimestampUslist.empty(), true);
}

// thumbnail test
TEST_F(Codec2VideoDecHidlTest, ThumbnailTest) {
    description("Test Request for thumbnail");
    if (mDisableTest) return;

    char mURL[512], info[512];
    std::ifstream eleStream, eleInfo;

    strcpy(mURL, gEnv->getRes().c_str());
    strcpy(info, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL, info);

    eleInfo.open(info);
    ASSERT_EQ(eleInfo.is_open(), true);
    android::Vector<FrameInfo> Info;
    int bytesCount = 0;
    uint32_t flags = 0;
    uint32_t timestamp = 0;
    while (1) {
        if (!(eleInfo >> bytesCount)) break;
        eleInfo >> flags;
        eleInfo >> timestamp;
        Info.push_back({bytesCount, flags, timestamp});
    }
    eleInfo.close();

    for (size_t i = 0; i < MAX_ITERATIONS; i++) {
        ASSERT_EQ(mComponent->start(), C2_OK);

        // request EOS for thumbnail
        // signal EOS flag with last frame
        size_t j = -1;
        do {
            j++;
            flags = 0;
            if (Info[j].flags) flags = 1u << (Info[j].flags - 1);

        } while (!(flags & SYNC_FRAME));
        eleStream.open(mURL, std::ifstream::binary);
        ASSERT_EQ(eleStream.is_open(), true);
        ASSERT_NO_FATAL_FAILURE(decodeNFrames(
            mComponent, mQueueLock, mQueueCondition, mWorkQueue,
            mFlushedIndices, mLinearPool, eleStream, &Info, 0, j + 1));
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));
        eleStream.close();
        EXPECT_GE(mFramesReceived, 1U);
        ASSERT_EQ(mEos, true);
        ASSERT_EQ(mComponent->stop(), C2_OK);
    }
    ASSERT_EQ(mComponent->release(), C2_OK);
}

TEST_F(Codec2VideoDecHidlTest, EOSTest) {
    description("Test empty input buffer with EOS flag");
    if (mDisableTest) return;
    typedef std::unique_lock<std::mutex> ULock;
    ASSERT_EQ(mComponent->start(), C2_OK);
    std::unique_ptr<C2Work> work;
    // Prepare C2Work
    {
        ULock l(mQueueLock);
        if (!mWorkQueue.empty()) {
            work.swap(mWorkQueue.front());
            mWorkQueue.pop_front();
        } else {
            ASSERT_TRUE(false) << "mWorkQueue Empty at the start of test";
        }
    }
    ASSERT_NE(work, nullptr);

    work->input.flags = (C2FrameData::flags_t)C2FrameData::FLAG_END_OF_STREAM;
    work->input.ordinal.timestamp = 0;
    work->input.ordinal.frameIndex = 0;
    work->input.buffers.clear();
    work->worklets.clear();
    work->worklets.emplace_back(new C2Worklet);

    std::list<std::unique_ptr<C2Work>> items;
    items.push_back(std::move(work));
    ASSERT_EQ(mComponent->queue(&items), C2_OK);

    {
        ULock l(mQueueLock);
        if (mWorkQueue.size() != MAX_INPUT_BUFFERS) {
            mQueueCondition.wait_for(l, TIME_OUT);
        }
    }
    ASSERT_EQ(mEos, true);
    ASSERT_EQ(mWorkQueue.size(), (size_t)MAX_INPUT_BUFFERS);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}

TEST_F(Codec2VideoDecHidlTest, FlushTest) {
    description("Tests Flush calls");
    if (mDisableTest) return;
    typedef std::unique_lock<std::mutex> ULock;
    ASSERT_EQ(mComponent->start(), C2_OK);
    char mURL[512], info[512];
    std::ifstream eleStream, eleInfo;

    strcpy(mURL, gEnv->getRes().c_str());
    strcpy(info, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL, info);

    eleInfo.open(info);
    ASSERT_EQ(eleInfo.is_open(), true);
    android::Vector<FrameInfo> Info;
    int bytesCount = 0;
    uint32_t flags = 0;
    uint32_t timestamp = 0;
    mFlushedIndices.clear();
    while (1) {
        if (!(eleInfo >> bytesCount)) break;
        eleInfo >> flags;
        eleInfo >> timestamp;
        Info.push_back({bytesCount, flags, timestamp});
    }
    eleInfo.close();

    ALOGV("mURL : %s", mURL);
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    // Decode 128 frames and flush. here 128 is chosen to ensure there is a key
    // frame after this so that the below section can be covered for all
    // components
    uint32_t numFramesFlushed = 128;
    ASSERT_NO_FATAL_FAILURE(decodeNFrames(
        mComponent, mQueueLock, mQueueCondition, mWorkQueue, mFlushedIndices,
        mLinearPool, eleStream, &Info, 0, numFramesFlushed, false));
    // flush
    std::list<std::unique_ptr<C2Work>> flushedWork;
    c2_status_t err =
        mComponent->flush(C2Component::FLUSH_COMPONENT, &flushedWork);
    ASSERT_EQ(err, C2_OK);
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue,
                               (size_t)MAX_INPUT_BUFFERS - flushedWork.size()));

    {
        // Update mFlushedIndices based on the index received from flush()
        ULock l(mQueueLock);
        for (std::unique_ptr<C2Work>& work : flushedWork) {
            ASSERT_NE(work, nullptr);
            auto frameIndexIt =
                std::find(mFlushedIndices.begin(), mFlushedIndices.end(),
                          work->input.ordinal.frameIndex.peeku());
            if (!mFlushedIndices.empty() &&
                (frameIndexIt != mFlushedIndices.end())) {
                mFlushedIndices.erase(frameIndexIt);
                work->input.buffers.clear();
                work->worklets.clear();
                mWorkQueue.push_back(std::move(work));
            }
        }
    }
    // Seek to next key frame and start decoding till the end
    mFlushedIndices.clear();
    int index = numFramesFlushed;
    bool keyFrame = false;
    flags = 0;
    while (index < (int)Info.size()) {
        if (Info[index].flags) flags = 1u << (Info[index].flags - 1);
        if ((flags & SYNC_FRAME) == SYNC_FRAME) {
            keyFrame = true;
            break;
        }
        flags = 0;
        eleStream.ignore(Info[index].bytesCount);
        index++;
    }
    if (keyFrame) {
        ASSERT_NO_FATAL_FAILURE(
            decodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                          mFlushedIndices, mLinearPool, eleStream, &Info, index,
                          (int)Info.size() - index));
    }
    eleStream.close();
    err = mComponent->flush(C2Component::FLUSH_COMPONENT, &flushedWork);
    ASSERT_EQ(err, C2_OK);
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue,
                               (size_t)MAX_INPUT_BUFFERS - flushedWork.size()));
    {
        // Update mFlushedIndices based on the index received from flush()
        ULock l(mQueueLock);
        for (std::unique_ptr<C2Work>& work : flushedWork) {
            ASSERT_NE(work, nullptr);
            uint64_t frameIndex = work->input.ordinal.frameIndex.peeku();
            std::list<uint64_t>::iterator frameIndexIt = std::find(
                mFlushedIndices.begin(), mFlushedIndices.end(), frameIndex);
            if (!mFlushedIndices.empty() &&
                (frameIndexIt != mFlushedIndices.end())) {
                mFlushedIndices.erase(frameIndexIt);
                work->input.buffers.clear();
                work->worklets.clear();
                mWorkQueue.push_back(std::move(work));
            }
        }
    }
    ASSERT_EQ(mFlushedIndices.empty(), true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}

TEST_F(Codec2VideoDecHidlTest, DecodeTestEmptyBuffersInserted) {
    description("Decode with multiple empty input frames");
    if (mDisableTest) return;

    char mURL[512], info[512];
    std::ifstream eleStream, eleInfo;

    strcpy(mURL, gEnv->getRes().c_str());
    strcpy(info, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL, info);

    eleInfo.open(info);
    ASSERT_EQ(eleInfo.is_open(), true) << mURL << " - file not found";
    android::Vector<FrameInfo> Info;
    int bytesCount = 0;
    uint32_t frameId = 0;
    uint32_t flags = 0;
    uint32_t timestamp = 0;
    bool codecConfig = false;
    // This test introduces empty CSD after every 20th frame
    // and empty input frames at an interval of 5 frames.
    while (1) {
        if (!(frameId % 5)) {
            if (!(frameId % 20)) flags = 32;
            else flags = 0;
            bytesCount = 0;
        } else {
            if (!(eleInfo >> bytesCount)) break;
            eleInfo >> flags;
            eleInfo >> timestamp;
            codecConfig = flags ?
                ((1 << (flags - 1)) & C2FrameData::FLAG_CODEC_CONFIG) != 0 : 0;
        }
        Info.push_back({bytesCount, flags, timestamp});
        frameId++;
    }
    eleInfo.close();

    ASSERT_EQ(mComponent->start(), C2_OK);
    ALOGV("mURL : %s", mURL);
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    ASSERT_NO_FATAL_FAILURE(decodeNFrames(
        mComponent, mQueueLock, mQueueCondition, mWorkQueue, mFlushedIndices,
        mLinearPool, eleStream, &Info, 0, (int)Info.size()));

    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    if (!mEos) {
        ALOGV("Waiting for input consumption");
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));
    }

    eleStream.close();
    if (mFramesReceived != Info.size()) {
        ALOGE("Input buffer count and Output buffer count mismatch");
        ALOGV("framesReceived : %d inputFrames : %zu", mFramesReceived,
              Info.size());
        ASSERT_TRUE(false);
    }
}

}  // anonymous namespace

// TODO : Video specific configuration Test
int main(int argc, char** argv) {
    gEnv = new ComponentTestEnvironment();
    ::testing::AddGlobalTestEnvironment(gEnv);
    ::testing::InitGoogleTest(&argc, argv);
    gEnv->init(&argc, argv);
    int status = gEnv->initFromOptions(argc, argv);
    if (status == 0) {
        int status = RUN_ALL_TESTS();
        LOG(INFO) << "C2 Test result = " << status;
    }
    return status;
}
