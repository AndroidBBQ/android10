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

// #define LOG_NDEBUG 0
#define LOG_TAG "codec2_hidl_hal_audio_enc_test"

#include <android-base/logging.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>

#include <codec2/hidl/client.h>
#include <C2AllocatorIon.h>
#include <C2Config.h>
#include <C2Debug.h>
#include <C2Buffer.h>
#include <C2BufferPriv.h>

using android::C2AllocatorIon;

#include <VtsHalHidlTargetTestBase.h>
#include "media_c2_audio_hidl_test_common.h"
#include "media_c2_hidl_test_common.h"

class LinearBuffer : public C2Buffer {
   public:
    explicit LinearBuffer(const std::shared_ptr<C2LinearBlock>& block)
        : C2Buffer(
              {block->share(block->offset(), block->size(), ::C2Fence())}) {}
};

static ComponentTestEnvironment* gEnv = nullptr;

namespace {

class Codec2AudioEncHidlTest : public ::testing::VtsHalHidlTargetTestBase {
   private:
    typedef ::testing::VtsHalHidlTargetTestBase Super;

   public:
    ::std::string getTestCaseInfo() const override {
        return ::std::string() +
                "Component: " + gEnv->getComponent().c_str() + " | " +
                "Instance: " + gEnv->getInstance().c_str() + " | " +
                "Res: " + gEnv->getRes().c_str();
    }

    // google.codec2 Audio test setup
    virtual void SetUp() override {
        Super::SetUp();
        mDisableTest = false;
        ALOGV("Codec2AudioEncHidlTest SetUp");
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
            {"aac", aac},
            {"flac", flac},
            {"opus", opus},
            {"amrnb", amrnb},
            {"amrwb", amrwb},
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
        if (mCompName == unknown_comp) mDisableTest = true;
        if (mDisableTest) std::cout << "[   WARN   ] Test Disabled \n";
        getInputMaxBufSize();
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
                workDone(mComponent, work, mFlushedIndices, mQueueLock,
                         mQueueCondition, mWorkQueue, mEos, mCsd,
                         mFramesReceived);
            }
        }
    }
    enum standardComp {
        aac,
        flac,
        opus,
        amrnb,
        amrwb,
        unknown_comp,
    };

    bool mEos;
    bool mCsd;
    bool mDisableTest;
    standardComp mCompName;
    uint32_t mFramesReceived;
    int32_t mInputMaxBufSize;
    std::list<uint64_t> mFlushedIndices;

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

    // In encoder components, fetch the size of input buffer allocated
    void getInputMaxBufSize() {
        int32_t bitStreamInfo[1] = {0};
        std::vector<std::unique_ptr<C2Param>> inParams;
        c2_status_t status = mComponent->query(
            {}, {C2StreamMaxBufferSizeInfo::input::PARAM_TYPE}, C2_DONT_BLOCK,
            &inParams);
        if (status != C2_OK && inParams.size() == 0) {
            ALOGE("Query MaxBufferSizeInfo failed => %d", status);
            ASSERT_TRUE(false);
        } else {
            size_t offset = sizeof(C2Param);
            for (size_t i = 0; i < inParams.size(); ++i) {
                C2Param* param = inParams[i].get();
                bitStreamInfo[i] = *(int32_t*)((uint8_t*)param + offset);
            }
        }
        mInputMaxBufSize = bitStreamInfo[0];
    }

};

void validateComponent(
    const std::shared_ptr<android::Codec2Client::Component>& component,
    Codec2AudioEncHidlTest::standardComp compName, bool& disableTest) {
    // Validate its a C2 Component
    if (component->getName().find("c2") == std::string::npos) {
        ALOGE("Not a c2 component");
        disableTest = true;
        return;
    }

    // Validate its not an encoder and the component to be tested is audio
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
        if (inputDomain.find("audio/") == std::string::npos) {
            ALOGE("Expected Audio Component");
            disableTest = true;
            return;
        }
    }

    // Validates component name
    if (compName == Codec2AudioEncHidlTest::unknown_comp) {
        ALOGE("Component InValid");
        disableTest = true;
        return;
    }
    ALOGV("Component Valid");
}

// Set Default config param.
bool setupConfigParam(
    const std::shared_ptr<android::Codec2Client::Component>& component,
    int32_t nChannels, int32_t nSampleRate) {
    std::vector<std::unique_ptr<C2SettingResult>> failures;
    C2StreamSampleRateInfo::input sampleRateInfo(0u, nSampleRate);
    C2StreamChannelCountInfo::input channelCountInfo(0u, nChannels);

    std::vector<C2Param*> configParam{&sampleRateInfo, &channelCountInfo};
    c2_status_t status =
        component->config(configParam, C2_DONT_BLOCK, &failures);
    if (status == C2_OK && failures.size() == 0u) return true;
    return false;
}

// LookUpTable of clips and metadata for component testing
void GetURLForComponent(Codec2AudioEncHidlTest::standardComp comp, char* mURL) {
    struct CompToURL {
        Codec2AudioEncHidlTest::standardComp comp;
        const char* mURL;
    };
    static const CompToURL kCompToURL[] = {
        {Codec2AudioEncHidlTest::standardComp::aac,
         "bbb_raw_2ch_48khz_s16le.raw"},
        {Codec2AudioEncHidlTest::standardComp::amrnb,
         "bbb_raw_1ch_8khz_s16le.raw"},
        {Codec2AudioEncHidlTest::standardComp::amrwb,
         "bbb_raw_1ch_16khz_s16le.raw"},
        {Codec2AudioEncHidlTest::standardComp::flac,
         "bbb_raw_2ch_48khz_s16le.raw"},
        {Codec2AudioEncHidlTest::standardComp::opus,
         "bbb_raw_2ch_48khz_s16le.raw"},
    };

    for (size_t i = 0; i < sizeof(kCompToURL) / sizeof(kCompToURL[0]); ++i) {
        if (kCompToURL[i].comp == comp) {
            strcat(mURL, kCompToURL[i].mURL);
            return;
        }
    }
}

void encodeNFrames(const std::shared_ptr<android::Codec2Client::Component>& component,
                   std::mutex &queueLock, std::condition_variable& queueCondition,
                   std::list<std::unique_ptr<C2Work>>& workQueue,
                   std::list<uint64_t>& flushedIndices,
                   std::shared_ptr<C2BlockPool>& linearPool,
                   std::ifstream& eleStream, uint32_t nFrames,
                   int32_t samplesPerFrame, int32_t nChannels,
                   int32_t nSampleRate, bool flushed = false,
                   bool signalEOS = true) {
    typedef std::unique_lock<std::mutex> ULock;

    uint32_t frameID = 0;
    uint32_t maxRetry = 0;
    int bytesCount = samplesPerFrame * nChannels * 2;
    int32_t timestampIncr =
        (int)(((float)samplesPerFrame / nSampleRate) * 1000000);
    uint64_t timestamp = 0;
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
        eleStream.read(data, bytesCount);
        ASSERT_EQ(eleStream.gcount(), bytesCount);
        std::shared_ptr<C2LinearBlock> block;
        ASSERT_EQ(C2_OK, linearPool->fetchLinearBlock(
                             bytesCount, {C2MemoryUsage::CPU_READ,
                                          C2MemoryUsage::CPU_WRITE},
                             &block));
        ASSERT_TRUE(block);
        // Write View
        C2WriteView view = block->map().get();
        if (view.error() != C2_OK) {
            fprintf(stderr, "C2LinearBlock::map() failed : %d", view.error());
            break;
        }
        ASSERT_EQ((size_t)bytesCount, view.capacity());
        ASSERT_EQ(0u, view.offset());
        ASSERT_EQ((size_t)bytesCount, view.size());

        memcpy(view.base(), data, bytesCount);
        work->input.buffers.clear();
        work->input.buffers.emplace_back(new LinearBuffer(block));
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);
        free(data);

        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        // DO THE DECODING
        ASSERT_EQ(component->queue(&items), C2_OK);
        ALOGV("Frame #%d size = %d queued", frameID, bytesCount);
        nFrames--;
        timestamp += timestampIncr;
        frameID++;
        maxRetry = 0;
    }
}

TEST_F(Codec2AudioEncHidlTest, validateCompName) {
    if (mDisableTest) return;
    ALOGV("Checks if the given component is a valid audio component");
    validateComponent(mComponent, mCompName, mDisableTest);
    ASSERT_EQ(mDisableTest, false);
}

class Codec2AudioEncEncodeTest
    : public Codec2AudioEncHidlTest,
      public ::testing::WithParamInterface<std::pair<bool, int32_t>> {
};

TEST_P(Codec2AudioEncEncodeTest, EncodeTest) {
    ALOGV("EncodeTest");
    if (mDisableTest) return;
    char mURL[512];
    strcpy(mURL, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL);
    bool signalEOS = GetParam().first;
    // Ratio w.r.t to mInputMaxBufSize
    int32_t inputMaxBufRatio = GetParam().second;

    // Setting default sampleRate
    int32_t nChannels = 2;
    int32_t nSampleRate = 44100;
    switch (mCompName) {
        case aac:
            nChannels = 2;
            nSampleRate = 48000;
            break;
        case flac:
            nChannels = 2;
            nSampleRate = 48000;
            break;
        case opus:
            nChannels = 2;
            nSampleRate = 48000;
            break;
        case amrnb:
            nChannels = 1;
            nSampleRate = 8000;
            break;
        case amrwb:
            nChannels = 1;
            nSampleRate = 16000;
            break;
        default:
            ASSERT_TRUE(false);
    }
    int32_t samplesPerFrame =
        ((mInputMaxBufSize / inputMaxBufRatio) / (nChannels * 2));
    ALOGV("signalEOS %d mInputMaxBufSize %d samplesPerFrame %d", signalEOS,
          mInputMaxBufSize, samplesPerFrame);

    if (!setupConfigParam(mComponent, nChannels, nSampleRate)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);
    std::ifstream eleStream;
    uint32_t numFrames = 16;
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    ALOGV("mURL : %s", mURL);
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mLinearPool, eleStream, numFrames,
                      samplesPerFrame, nChannels, nSampleRate, false,
                      signalEOS));

    // If EOS is not sent, sending empty input with EOS flag
    if (!signalEOS) {
        ASSERT_NO_FATAL_FAILURE(
            waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue, 1));
        ASSERT_NO_FATAL_FAILURE(
            testInputBuffer(mComponent, mQueueLock, mWorkQueue,
                            C2FrameData::FLAG_END_OF_STREAM, false));
        numFrames += 1;
    }

    // blocking call to ensures application to Wait till all the inputs are
    // consumed
    ASSERT_NO_FATAL_FAILURE(
        waitOnInputConsumption(mQueueLock, mQueueCondition, mWorkQueue));
    eleStream.close();
    if (mFramesReceived != numFrames) {
        ALOGE("Input buffer count and Output buffer count mismatch");
        ALOGE("framesReceived : %d inputFrames : %u", mFramesReceived, numFrames);
        ASSERT_TRUE(false);
    }
    if ((mCompName == flac || mCompName == opus || mCompName == aac)) {
        if (!mCsd) {
            ALOGE("CSD buffer missing");
            ASSERT_TRUE(false);
        }
    }
    ASSERT_EQ(mEos, true);
    ASSERT_EQ(mComponent->stop(), C2_OK);
}

// EncodeTest with EOS / No EOS and inputMaxBufRatio
// inputMaxBufRatio is ratio w.r.t. to mInputMaxBufSize
INSTANTIATE_TEST_CASE_P(EncodeTest, Codec2AudioEncEncodeTest,
                        ::testing::Values(std::make_pair(false, 1),
                                          std::make_pair(false, 2),
                                          std::make_pair(true, 1),
                                          std::make_pair(true, 2)));


TEST_F(Codec2AudioEncHidlTest, EOSTest) {
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

TEST_F(Codec2AudioEncHidlTest, FlushTest) {
    description("Test Request for flush");
    if (mDisableTest) return;

    typedef std::unique_lock<std::mutex> ULock;
    char mURL[512];
    strcpy(mURL, gEnv->getRes().c_str());
    GetURLForComponent(mCompName, mURL);

    // Setting default configuration
    mFlushedIndices.clear();
    int32_t nChannels = 2;
    int32_t nSampleRate = 44100;
    int32_t samplesPerFrame = 1024;
    switch (mCompName) {
        case aac:
            nChannels = 2;
            nSampleRate = 48000;
            samplesPerFrame = 1024;
            break;
        case flac:
            nChannels = 2;
            nSampleRate = 48000;
            samplesPerFrame = 1152;
            break;
        case opus:
            nChannels = 2;
            nSampleRate = 48000;
            samplesPerFrame = 960;
            break;
        case amrnb:
            nChannels = 1;
            nSampleRate = 8000;
            samplesPerFrame = 160;
            break;
        case amrwb:
            nChannels = 1;
            nSampleRate = 16000;
            samplesPerFrame = 160;
            break;
        default:
            ASSERT_TRUE(false);
    }

    if (!setupConfigParam(mComponent, nChannels, nSampleRate)) {
        std::cout << "[   WARN   ] Test Skipped \n";
        return;
    }
    ASSERT_EQ(mComponent->start(), C2_OK);

    std::ifstream eleStream;
    uint32_t numFramesFlushed = 30;
    uint32_t numFrames = 128;
    eleStream.open(mURL, std::ifstream::binary);
    ASSERT_EQ(eleStream.is_open(), true);
    ALOGV("mURL : %s", mURL);
    ASSERT_NO_FATAL_FAILURE(
        encodeNFrames(mComponent, mQueueLock, mQueueCondition, mWorkQueue,
                      mFlushedIndices, mLinearPool, eleStream, numFramesFlushed,
                      samplesPerFrame, nChannels, nSampleRate));
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
            std::list<uint64_t>::iterator frameIndexIt =
                std::find(mFlushedIndices.begin(), mFlushedIndices.end(),
                          frameIndex);
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
                      mFlushedIndices, mLinearPool, eleStream,
                      numFrames - numFramesFlushed, samplesPerFrame,
                      nChannels, nSampleRate, true));
    eleStream.close();
    err =
        mComponent->flush(C2Component::FLUSH_COMPONENT, &flushedWork);
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
            std::list<uint64_t>::iterator frameIndexIt =
                std::find(mFlushedIndices.begin(), mFlushedIndices.end(),
                          frameIndex);
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
