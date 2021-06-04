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
#define LOG_TAG "codec2_hidl_hal_video_enc_test"

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

class GraphicBuffer : public C2Buffer {
public:
  explicit GraphicBuffer(const std::shared_ptr<C2GraphicBlock> &block)
      : C2Buffer({block->share(C2Rect(block->width(), block->height()),
                               ::C2Fence())}) {}
};

static ComponentTestEnvironment* gEnv = nullptr;

namespace {

class Codec2VideoEncHidlTest : public ::testing::VtsHalHidlTargetTestBase {
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
        ALOGV("Codec2VideoEncHidlTest SetUp");
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
        CHECK_EQ(store->fetchAllocator(C2AllocatorStore::DEFAULT_GRAPHIC,
                                       &mGraphicAllocator),
                 C2_OK);
        mGraphicPool = std::make_shared<C2PooledBlockPool>(mGraphicAllocator,
                                                           mBlockPoolId++);
        ASSERT_NE(mGraphicPool, nullptr);

        mCompName = unknown_comp;
        struct StringToName {
            const char* Name;
            standardComp CompName;
        };

        const StringToName kStringToName[] = {
            {"h263", h263}, {"avc", avc}, {"mpeg4", mpeg4},
            {"hevc", hevc}, {"vp8", vp8}, {"vp9", vp9},
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
        mCsd = false;
        mFramesReceived = 0;
        mFailedWorkReceived = 0;
        mTimestampUs = 0u;
        mTimestampDevTest = false;
        if (mCompName == unknown_comp) mDisableTest = true;
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

    bool setupConfigParam(int32_t nWidth, int32_t nHeight);

    // callback function to process onWorkDone received by Listener
    void handleWorkDone(std::list<std::unique_ptr<C2Work>>& workItems) {
        for (std::unique_ptr<C2Work>& work : workItems) {
            if (!work->worklets.empty()) {
                // For encoder components current timestamp always exceeds
                // previous timestamp
                typedef std::unique_lock<std::mutex> ULock;
                if (!mTimestampUslist.empty()) {
                    EXPECT_GE((work->worklets.front()
                                   ->output.ordinal.timestamp.peeku()),
                              mTimestampUs);
                    mTimestampUs = work->worklets.front()
                                       ->output.ordinal.timestamp.peeku();
                    // Currently this lock is redundant as no mTimestampUslist is only initialized
                    // before queuing any work to component. Once AdaptiveTest is added similar to
                    // the one in video decoders, this is needed.
                    ULock l(mQueueLock);

                    if (mTimestampDevTest) {
                        bool tsHit = false;
                        std::list<uint64_t>::iterator it =
                            mTimestampUslist.begin();
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
                                std::cout
                                    << "[   INFO   ] Received non-zero "
                                       "output / TimeStamp not recognized \n";
                            }
                        }
                    }
                }

                if (work->result != C2_OK) mFailedWorkReceived++;
                workDone(mComponent, work, mFlushedIndices, mQueueLock,
                         mQueueCondition, mWorkQueue, mEos, mCsd,
                         mFramesReceived);
            }
        }
    }

    enum standardComp {
        h263,
        avc,
        mpeg4,
        hevc,
        vp8,
        vp9,
        unknown_comp,
    };

    bool mEos;
    bool mCsd;
    bool mDisableTest;
    bool mConfig;
    bool mTimestampDevTest;
    standardComp mCompName;
    uint32_t mFramesReceived;
    uint32_t mFailedWorkReceived;
    uint64_t mTimestampUs;

    std::list<uint64_t> mTimestampUslist;
    std::list<uint64_t> mFlushedIndices;

    C2BlockPool::local_id_t mBlockPoolId;
    std::shared_ptr<C2BlockPool> mGraphicPool;
    std::shared_ptr<C2Allocator> mGraphicAllocator;

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
    Codec2VideoEncHidlTest::standardComp compName, bool& disableTest) {
    // Validate its a C2 Component
    if (component->getName().find("c2") == std::string::npos) {
        ALOGE("Not a c2 component");
        disableTest = true;
        return;
    }

    // Validate its not an encoder and the component to be tested is video
    if (component->getName().find("decoder") != std::string::npos) {
        ALOGE("Expected Encoder, given Decoder");
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
    if (compName == Codec2VideoEncHidlTest::unknown_comp) {
        ALOGE("Component InValid");
        disableTest = true;
        return;
    }
    ALOGV("Component Valid");
}

// Set Default config param.
bool Codec2VideoEncHidlTest::setupConfigParam(int32_t nWidth, int32_t nHeight) {
    std::vector<std::unique_ptr<C2SettingResult>> failures;
    C2StreamPictureSizeInfo::input inputSize(0u, nWidth, nHeight);
    std::vector<C2Param*> configParam{&inputSize};
    c2_status_t status =
        mComponent->config(configParam, C2_DONT_BLOCK, &failures);
    if (status == C2_OK && failures.size() == 0u) return true;
    return false;
}

// LookUpTable of clips for component testing
void GetURLForComponent(char* URL) {
    strcat(URL, "bbb_352x288_420p_30fps_32frames.yuv");
}

void encodeNFrames(const std::shared_ptr<android::Codec2Client::Component>& component,
                   std::mutex &queueLock, std::condition_variable& queueCondition,
                   std::list<std::unique_ptr<C2Work>>& workQueue,
                   std::list<uint64_t>& flushedIndices,
                   std::shared_ptr<C2BlockPool>& graphicPool,
                   std::ifstream& eleStream, bool& disableTest,
                   uint32_t frameID, uint32_t nFrames, uint32_t nWidth,
                   int32_t nHeight, bool flushed = false, bool signalEOS = true) {
    typedef std::unique_lock<std::mutex> ULock;

    uint32_t maxRetry = 0;
    int bytesCount = nWidth * nHeight * 3 >> 1;
    int32_t timestampIncr = ENCODER_TIMESTAMP_INCREMENT;
    uint64_t timestamp = 0;
    c2_status_t err = C2_OK;
    while (1) {
        if (nFrames == 0) break;
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
        if (signalEOS && (nFrames == 1))
            flags |= C2FrameData::FLAG_END_OF_STREAM;
        if (flushed) {
            flags |= SYNC_FRAME;
            flushed = false;
        }

        work->input.flags = (C2FrameData::flags_t)flags;
        work->input.ordinal.timestamp = timestamp;
        work->input.ordinal.frameIndex = frameID;
        {
            ULock l(queueLock);
            flushedIndices.emplace_back(frameID);
        }
        char* data = (char*)malloc(bytesCount);
        ASSERT_NE(data, nullptr);
        memset(data, 0, bytesCount);
        if (eleStream.is_open()) {
            eleStream.read(data, bytesCount);
            ASSERT_EQ(eleStream.gcount(), bytesCount);
        }
        std::shared_ptr<C2GraphicBlock> block;
        err = graphicPool->fetchGraphicBlock(
                nWidth, nHeight, HAL_PIXEL_FORMAT_YV12,
                {C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE}, &block);
        if (err != C2_OK) {
            fprintf(stderr, "fetchGraphicBlock failed : %d\n", err);
            disableTest = true;
            break;
        }

        ASSERT_TRUE(block);
        // Graphic View
        C2GraphicView view = block->map().get();
        if (view.error() != C2_OK) {
            fprintf(stderr, "C2GraphicBlock::map() failed : %d", view.error());
            disableTest = true;
            break;
        }

        uint8_t* pY = view.data()[C2PlanarLayout::PLANE_Y];
        uint8_t* pU = view.data()[C2PlanarLayout::PLANE_U];
        uint8_t* pV = view.data()[C2PlanarLayout::PLANE_V];

        memcpy(pY, data, nWidth * nHeight);
        memcpy(pU, data + nWidth * nHeight, (nWidth * nHeight >> 2));
        memcpy(pV, data + (nWidth * nHeight * 5 >> 2), nWidth * nHeight >> 2);

        work->input.buffers.clear();
        work->input.buffers.emplace_back(new GraphicBuffer(block));
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);
        free(data);

        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        // DO THE ENCODING
        ASSERT_EQ(component->queue(&items), C2_OK);
        ALOGV("Frame #%d size = %d queued", frameID, bytesCount);
        nFrames--;
        timestamp += timestampIncr;
        frameID++;
        maxRetry = 0;
    }
}

TEST_F(Codec2VideoEncHidlTest, validateCompName) {
    if (mDisableTest) return;
    ALOGV("Checks if the given component is a valid video component");
    validateComponent(mComponent, mCompName, mDisableTest);
    ASSERT_EQ(mDisableTest, false);
}

class Codec2VideoEncEncodeTest : public Codec2VideoEncHidlTest,
                                 public ::testing::WithParamInterface<bool> {
};

TEST_P(Codec2VideoEncEncodeTest, EncodeTest) {
    description("Encodes input file");
    if (mDisableTest) return;

    char mURL[512];
    int32_t nWidth = ENC_DEFAULT_FRAME_WIDTH;
    int32_t nHeight = ENC_DEFAULT_FRAME_HEIGHT;
    bool signalEOS = GetParam();

    strcpy(mURL, gEnv->getRes().c_str());
    GetURLForComponent(mURL);

    std::ifstream eleStream;
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true) << mURL << " file not found";
    ALOGV("mURL : %s", mURL);

    mTimestampUs = 0;
    mTimestampDevTest = true;
    mFlushedIndices.clear();
    mTimestampUslist.clear();
    uint32_t inputFrames = ENC_NUM_FRAMES;
    uint32_t timestamp = 0;
    // Add input timestamp to timestampUslist
    while (inputFrames) {
        if (mTimestampDevTest) mTimestampUslist.push_back(timestamp);
        timestamp += ENCODER_TIMESTAMP_INCREMENT;
        inputFrames--;
    }
    if (!setupConfigParam(nWidth, nHeight)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      0, ENC_NUM_FRAMES, nWidth, nHeight, false, signalEOS));
    // mDisableTest will be set if buffer was not fetched properly.
    // This may happen when resolution is not proper but config suceeded
    // In this cases, we skip encoding the input stream
    if (mDisableTest) {
        std::cout << "[   WARN   ] Test Disabled \n";
        ASSERT_EQ(mComponent->stop(), C2_OK);
        return;
    }

    // If EOS is not sent, sending empty input with EOS flag
    inputFrames = ENC_NUM_FRAMES;
    if (!signalEOS) {
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue, 1));
        ASSERT_NO_FATAL_FAILURE(
            testInputBuffer(mComponent, mQueueLock, mWorkQueue,
                            C2FrameData::FLAG_END_OF_STREAM, false));
        inputFrames += 1;
    }

    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    ALOGD("Waiting for input consumption");
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));

    eleStream.close();
    if (mFramesReceived != inputFrames) {
        ALOGE("Input buffer count and Output buffer count mismatch");
        ALOGE("framesReceived : %d inputFrames : %d", mFramesReceived,
              inputFrames);
        ASSERT_TRUE(false);
    }

    if (!mCsd && (mCompName != vp8 && mCompName != vp9)) {
        ASSERT_TRUE(false) << "CSD Buffer not received";
    }

    if (mCsd && (mCompName == vp8 || mCompName == vp9)) {
        ASSERT_TRUE(false) << "CSD Buffer not expected";
    }

    if (mTimestampDevTest) EXPECT_EQ(mTimestampUslist.empty(), true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}

// EncodeTest with EOS / No EOS
INSTANTIATE_TEST_CASE_P(EncodeTestwithEOS, Codec2VideoEncEncodeTest,
                        ::testing::Values(true, false));

TEST_F(Codec2VideoEncHidlTest, EOSTest) {
    description("Test empty input buffer with EOS flag");
    if (mDisableTest) return;
    ASSERT_EQ(mComponent->start(), C2_OK);

    typedef std::unique_lock<std::mutex> ULock;
    std::unique_ptr<C2Work> work;
    {
        ULock l(mQueueLock);
        if (!mWorkQueue.empty()) {
            work.swap(mWorkQueue.front());
            mWorkQueue.pop_front();
        } else {
            ALOGE("mWorkQueue Empty is not expected at the start of the test");
            ASSERT_TRUE(false);
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
    uint32_t queueSize;
    {
        ULock l(mQueueLock);
        queueSize = mWorkQueue.size();
        if (queueSize < MAX_INPUT_BUFFERS) {
            mQueueCondition.wait_for(l, TIME_OUT);
        }
    }
    ASSERT_EQ(mEos, true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}

TEST_F(Codec2VideoEncHidlTest, FlushTest) {
    description("Test Request for flush");
    if (mDisableTest) return;

    typedef std::unique_lock<std::mutex> ULock;
    char mURL[512];
    int32_t nWidth = ENC_DEFAULT_FRAME_WIDTH;
    int32_t nHeight = ENC_DEFAULT_FRAME_HEIGHT;
    strcpy(mURL, gEnv->getRes().c_str());
    GetURLForComponent(mURL);

    if (!setupConfigParam(nWidth, nHeight)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);

    // Setting default configuration
    mFlushedIndices.clear();
    std::ifstream eleStream;
    uint32_t numFramesFlushed = 10;
    uint32_t numFrames = ENC_NUM_FRAMES;
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    ALOGV("mURL : %s", mURL);
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      0, numFramesFlushed, nWidth, nHeight));
    // mDisableTest will be set if buffer was not fetched properly.
    // This may happen when resolution is not proper but config suceeded
    // In this cases, we skip encoding the input stream
    if (mDisableTest) {
        std::cout << "[   WARN   ] Test Disabled \n";
        ASSERT_EQ(mComponent->stop(), C2_OK);
        return;
    }

    std::list<std::unique_ptr<C2Work>> flushedWork;
    c2_status_t err =
        mComponent->flush(C2Component::FLUSH_COMPONENT, &flushedWork);
    ASSERT_EQ(err, C2_OK);
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue,
            (size_t)MAX_INPUT_BUFFERS - flushedWork.size()));
    uint64_t frameIndex;
    {
        //Update mFlushedIndices based on the index received from flush()
        ULock l(mQueueLock);
        for (std::unique_ptr<C2Work>& work : flushedWork) {
            ASSERT_NE(work, nullptr);
            frameIndex = work->input.ordinal.frameIndex.peeku();
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
    mFlushedIndices.clear();
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      numFramesFlushed, numFrames - numFramesFlushed,
                      nWidth, nHeight, true));
    eleStream.close();
    // mDisableTest will be set if buffer was not fetched properly.
    // This may happen when resolution is not proper but config suceeded
    // In this cases, we skip encoding the input stream
    if (mDisableTest) {
        std::cout << "[   WARN   ] Test Disabled \n";
        ASSERT_EQ(mComponent->stop(), C2_OK);
        return;
    }

    err = mComponent->flush(C2Component::FLUSH_COMPONENT, &flushedWork);
    ASSERT_EQ(err, C2_OK);
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue,
            (size_t)MAX_INPUT_BUFFERS - flushedWork.size()));
    {
        //Update mFlushedIndices based on the index received from flush()
        ULock l(mQueueLock);
        for (std::unique_ptr<C2Work>& work : flushedWork) {
            ASSERT_NE(work, nullptr);
            frameIndex = work->input.ordinal.frameIndex.peeku();
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

TEST_F(Codec2VideoEncHidlTest, InvalidBufferTest) {
    description("Tests feeding larger/smaller input buffer");
    if (mDisableTest) return;

    std::ifstream eleStream;
    int32_t nWidth = ENC_DEFAULT_FRAME_WIDTH / 2;
    int32_t nHeight = ENC_DEFAULT_FRAME_HEIGHT / 2;

    if (!setupConfigParam(nWidth, nHeight)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);

    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      0, 1, nWidth, nHeight, false, false));

    // Feed larger input buffer.
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      1, 1, nWidth*2, nHeight*2, false, false));

    // Feed smaller input buffer.
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      2, 1, nWidth/2, nHeight/2, false, true));

    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    ALOGD("Waiting for input consumption");
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));

    if (mFramesReceived != 3) {
        std::cout << "[   WARN   ] Component didn't receive all buffers back \n";
        ALOGW("framesReceived : %d inputFrames : 3", mFramesReceived);
    }

    if (mFailedWorkReceived == 0) {
        std::cout << "[   WARN   ] Expected failed frame count mismatch \n";
        ALOGW("failedFramesReceived : %d", mFailedWorkReceived);
    }

    ASSERT_EQ(mComponent->stop(), C2_OK);
}

class Codec2VideoEncResolutionTest : public Codec2VideoEncHidlTest,
        public ::testing::WithParamInterface<std::pair<int32_t, int32_t> > {
};

TEST_P(Codec2VideoEncResolutionTest, ResolutionTest) {
    description("Tests encoding at different resolutions");
    if (mDisableTest) return;

    std::ifstream eleStream;
    int32_t nWidth = GetParam().first;
    int32_t nHeight = GetParam().second;
    ALOGD("Trying encode for width %d height %d", nWidth, nHeight);
    mEos = false;

    if (!setupConfigParam(nWidth, nHeight)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);

    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mGraphicPool, eleStream, mDisableTest,
                      0, MAX_INPUT_BUFFERS, nWidth, nHeight, false, true));

    // mDisableTest will be set if buffer was not fetched properly.
    // This may happen when resolution is not proper but config suceeded
    // In this cases, we skip encoding the input stream
    if (mDisableTest) {
        std::cout << "[   WARN   ] Test Disabled \n";
        ASSERT_EQ(mComponent->stop(), C2_OK);
        return;
    }

    ALOGD("Waiting for input consumption");
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));

    ASSERT_EQ(mEos, true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
    ASSERT_EQ(mComponent->reset(), C2_OK);
}

INSTANTIATE_TEST_CASE_P(NonStdSizes, Codec2VideoEncResolutionTest, ::testing::Values(
    std::make_pair(52, 18),
    std::make_pair(365, 365),
    std::make_pair(484, 362),
    std::make_pair(244, 488)));

}  // anonymous namespace

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
