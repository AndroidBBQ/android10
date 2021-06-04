/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <inttypes.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <thread>

//#define LOG_NDEBUG 0
#define LOG_TAG "codec2"
#include <log/log.h>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <media/DataSource.h>
#include <media/ICrypto.h>
#include <media/IMediaHTTPService.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

#include <gui/GLConsumer.h>
#include <gui/IProducerListener.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <C2AllocatorGralloc.h>
#include <C2Buffer.h>
#include <C2BufferPriv.h>
#include <C2Component.h>
#include <C2Config.h>
#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <C2Work.h>

using namespace android;
using namespace std::chrono_literals;

namespace {

class LinearBuffer : public C2Buffer {
public:
    explicit LinearBuffer(const std::shared_ptr<C2LinearBlock> &block)
        : C2Buffer({ block->share(block->offset(), block->size(), ::C2Fence()) }) {}
};

class Listener;

class SimplePlayer {
public:
    SimplePlayer();
    ~SimplePlayer();

    void onWorkDone(std::weak_ptr<C2Component> component,
                    std::list<std::unique_ptr<C2Work>> workItems);
    void onTripped(std::weak_ptr<C2Component> component,
                   std::vector<std::shared_ptr<C2SettingResult>> settingResult);
    void onError(std::weak_ptr<C2Component> component, uint32_t errorCode);

    void play(const sp<IMediaSource> &source);

private:
    typedef std::unique_lock<std::mutex> ULock;

    std::shared_ptr<Listener> mListener;
    std::shared_ptr<C2Component> mComponent;

    sp<IProducerListener> mProducerListener;

    std::atomic_int mLinearPoolId;

    std::shared_ptr<C2Allocator> mAllocIon;
    std::shared_ptr<C2BlockPool> mLinearPool;

    std::mutex mQueueLock;
    std::condition_variable mQueueCondition;
    std::list<std::unique_ptr<C2Work>> mWorkQueue;

    std::mutex mProcessedLock;
    std::condition_variable mProcessedCondition;
    std::list<std::unique_ptr<C2Work>> mProcessedWork;

    sp<Surface> mSurface;
    sp<SurfaceComposerClient> mComposerClient;
    sp<SurfaceControl> mControl;
};

class Listener : public C2Component::Listener {
public:
    explicit Listener(SimplePlayer *thiz) : mThis(thiz) {}
    virtual ~Listener() = default;

    virtual void onWorkDone_nb(std::weak_ptr<C2Component> component,
                            std::list<std::unique_ptr<C2Work>> workItems) override {
        mThis->onWorkDone(component, std::move(workItems));
    }

    virtual void onTripped_nb(std::weak_ptr<C2Component> component,
                           std::vector<std::shared_ptr<C2SettingResult>> settingResult) override {
        mThis->onTripped(component, settingResult);
    }

    virtual void onError_nb(std::weak_ptr<C2Component> component,
                         uint32_t errorCode) override {
        mThis->onError(component, errorCode);
    }

private:
    SimplePlayer * const mThis;
};


SimplePlayer::SimplePlayer()
    : mListener(new Listener(this)),
      mProducerListener(new DummyProducerListener),
      mLinearPoolId(C2BlockPool::PLATFORM_START),
      mComposerClient(new SurfaceComposerClient) {
    CHECK_EQ(mComposerClient->initCheck(), (status_t)OK);

    std::shared_ptr<C2AllocatorStore> store = GetCodec2PlatformAllocatorStore();
    CHECK_EQ(store->fetchAllocator(C2AllocatorStore::DEFAULT_LINEAR, &mAllocIon), C2_OK);
    mLinearPool = std::make_shared<C2PooledBlockPool>(mAllocIon, mLinearPoolId++);

    mControl = mComposerClient->createSurface(
            String8("A Surface"),
            1280,
            800,
            HAL_PIXEL_FORMAT_YV12);
            //PIXEL_FORMAT_RGB_565);

    CHECK(mControl != nullptr);
    CHECK(mControl->isValid());

    SurfaceComposerClient::Transaction{}
            .setLayer(mControl, INT_MAX)
            .show(mControl)
            .apply();

    mSurface = mControl->getSurface();
    CHECK(mSurface != nullptr);
    mSurface->connect(NATIVE_WINDOW_API_CPU, mProducerListener);
}

SimplePlayer::~SimplePlayer() {
    mComposerClient->dispose();
}

void SimplePlayer::onWorkDone(
        std::weak_ptr<C2Component> component, std::list<std::unique_ptr<C2Work>> workItems) {
    ALOGV("SimplePlayer::onWorkDone");
    (void) component;
    ULock l(mProcessedLock);
    for (auto & item : workItems) {
        mProcessedWork.push_back(std::move(item));
    }
    mProcessedCondition.notify_all();
}

void SimplePlayer::onTripped(
        std::weak_ptr<C2Component> component,
        std::vector<std::shared_ptr<C2SettingResult>> settingResult) {
    (void) component;
    (void) settingResult;
    // TODO
}

void SimplePlayer::onError(std::weak_ptr<C2Component> component, uint32_t errorCode) {
    (void) component;
    (void) errorCode;
    // TODO
}

void SimplePlayer::play(const sp<IMediaSource> &source) {
    ALOGV("SimplePlayer::play");
    sp<AMessage> format;
    (void) convertMetaDataToMessage(source->getFormat(), &format);

    sp<ABuffer> csd0, csd1;
    format->findBuffer("csd-0", &csd0);
    format->findBuffer("csd-1", &csd1);

    status_t err = source->start();

    if (err != OK) {
        fprintf(stderr, "source returned error %d (0x%08x)\n", err, err);
        return;
    }

    std::shared_ptr<C2ComponentStore> store = GetCodec2PlatformComponentStore();
    std::shared_ptr<C2Component> component;
    (void)store->createComponent("c2.android.avc.decoder", &component);

    (void)component->setListener_vb(mListener, C2_DONT_BLOCK);
    std::unique_ptr<C2PortBlockPoolsTuning::output> pools =
        C2PortBlockPoolsTuning::output::AllocUnique({ (uint64_t)C2BlockPool::BASIC_GRAPHIC });
    std::vector<std::unique_ptr<C2SettingResult>> result;
    (void)component->intf()->config_vb({pools.get()}, C2_DONT_BLOCK, &result);
    component->start();

    for (int i = 0; i < 8; ++i) {
        mWorkQueue.emplace_back(new C2Work);
    }

    std::atomic_bool running(true);
    std::thread surfaceThread([this, &running]() {
        const sp<IGraphicBufferProducer> &igbp = mSurface->getIGraphicBufferProducer();
        while (running) {
            std::unique_ptr<C2Work> work;
            {
                ULock l(mProcessedLock);
                if (mProcessedWork.empty()) {
                    mProcessedCondition.wait_for(l, 100ms);
                    if (mProcessedWork.empty()) {
                        continue;
                    }
                }
                work.swap(mProcessedWork.front());
                mProcessedWork.pop_front();
            }
            int slot;
            sp<Fence> fence;
            ALOGV("Render: Frame #%lld", work->worklets.front()->output.ordinal.frameIndex.peekll());
            const std::shared_ptr<C2Buffer> &output = work->worklets.front()->output.buffers[0];
            if (output) {
                const C2ConstGraphicBlock block = output->data().graphicBlocks().front();
                native_handle_t *grallocHandle = UnwrapNativeCodec2GrallocHandle(block.handle());
                sp<GraphicBuffer> buffer(new GraphicBuffer(
                        grallocHandle,
                        GraphicBuffer::CLONE_HANDLE,
                        block.width(),
                        block.height(),
                        HAL_PIXEL_FORMAT_YV12,
                        1,
                        (uint64_t)GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
                        block.width()));
                native_handle_delete(grallocHandle);

                status_t err = igbp->attachBuffer(&slot, buffer);

                IGraphicBufferProducer::QueueBufferInput qbi(
                        (work->worklets.front()->output.ordinal.timestamp * 1000ll).peekll(),
                        false,
                        HAL_DATASPACE_UNKNOWN,
                        Rect(block.width(), block.height()),
                        NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW,
                        0,
                        Fence::NO_FENCE,
                        0);
                IGraphicBufferProducer::QueueBufferOutput qbo;
                err = igbp->queueBuffer(slot, qbi, &qbo);
            }

            work->input.buffers.clear();
            work->worklets.clear();

            ULock l(mQueueLock);
            mWorkQueue.push_back(std::move(work));
            mQueueCondition.notify_all();
        }
        ALOGV("render loop finished");
    });

    long numFrames = 0;
    mLinearPool.reset(new C2PooledBlockPool(mAllocIon, mLinearPoolId++));

    for (;;) {
        size_t size = 0u;
        void *data = nullptr;
        int64_t timestamp = 0u;
        MediaBufferBase *buffer = nullptr;
        sp<ABuffer> csd;
        if (csd0 != nullptr) {
            csd = csd0;
            csd0 = nullptr;
        } else if (csd1 != nullptr) {
            csd = csd1;
            csd1 = nullptr;
        } else {
            status_t err = source->read(&buffer);
            if (err != OK) {
                CHECK(buffer == nullptr);

                if (err == INFO_FORMAT_CHANGED) {
                    continue;
                }

                break;
            }
            MetaDataBase &meta = buffer->meta_data();
            CHECK(meta.findInt64(kKeyTime, &timestamp));

            size = buffer->size();
            data = buffer->data();
        }

        if (csd != nullptr) {
            size = csd->size();
            data = csd->data();
        }

        // Prepare C2Work

        std::unique_ptr<C2Work> work;
        while (!work) {
            ULock l(mQueueLock);
            if (!mWorkQueue.empty()) {
                work.swap(mWorkQueue.front());
                mWorkQueue.pop_front();
            } else {
                mQueueCondition.wait_for(l, 100ms);
            }
        }
        work->input.flags = (C2FrameData::flags_t)0;
        work->input.ordinal.timestamp = timestamp;
        work->input.ordinal.frameIndex = numFrames;

        std::shared_ptr<C2LinearBlock> block;
        mLinearPool->fetchLinearBlock(
                size,
                { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE },
                &block);
        C2WriteView view = block->map().get();
        if (view.error() != C2_OK) {
            fprintf(stderr, "C2LinearBlock::map() failed : %d", view.error());
            break;
        }
        memcpy(view.base(), data, size);

        work->input.buffers.clear();
        work->input.buffers.emplace_back(new LinearBuffer(block));
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);

        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        ALOGV("Frame #%ld size = %zu", numFrames, size);
        // DO THE DECODING
        component->queue_nb(&items);

        if (buffer) {
            buffer->release();
            buffer = nullptr;
        }

        ++numFrames;
    }
    ALOGV("main loop finished");
    source->stop();
    running.store(false);
    surfaceThread.join();

    component->release();
    printf("\n");
}

}  // namespace

static void usage(const char *me) {
    fprintf(stderr, "usage: %s [options] [input_filename]\n", me);
    fprintf(stderr, "       -h(elp)\n");
}

int main(int argc, char **argv) {
    android::ProcessState::self()->startThreadPool();

    int res;
    while ((res = getopt(argc, argv, "h")) >= 0) {
        switch (res) {
            case 'h':
            default:
            {
                usage(argv[0]);
                exit(1);
                break;
            }
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        fprintf(stderr, "No input file specified\n");
        return 1;
    }

    status_t err = OK;
    SimplePlayer player;

    for (int k = 0; k < argc && err == OK; ++k) {
        const char *filename = argv[k];

        sp<DataSource> dataSource =
            DataSourceFactory::CreateFromURI(nullptr /* httpService */, filename);

        if (strncasecmp(filename, "sine:", 5) && dataSource == nullptr) {
            fprintf(stderr, "Unable to create data source.\n");
            return 1;
        }

        Vector<sp<IMediaSource> > mediaSources;
        sp<IMediaSource> mediaSource;

        sp<IMediaExtractor> extractor = MediaExtractorFactory::Create(dataSource);

        if (extractor == nullptr) {
            fprintf(stderr, "could not create extractor.\n");
            return -1;
        }

        sp<MetaData> meta = extractor->getMetaData();

        if (meta != nullptr) {
            const char *mime;
            if (!meta->findCString(kKeyMIMEType, &mime)) {
                fprintf(stderr, "extractor did not provide MIME type.\n");
                return -1;
            }
        }

        size_t numTracks = extractor->countTracks();

        size_t i;
        for (i = 0; i < numTracks; ++i) {
            meta = extractor->getTrackMetaData(i, 0);

            if (meta == nullptr) {
                break;
            }
            const char *mime;
            meta->findCString(kKeyMIMEType, &mime);

            // TODO: allowing AVC only for the time being
            if (!strncasecmp(mime, "video/avc", 9)) {
                break;
            }

            meta = nullptr;
        }

        if (meta == nullptr) {
            fprintf(stderr, "No AVC track found.\n");
            return -1;
        }

        mediaSource = extractor->getTrack(i);
        if (mediaSource == nullptr) {
            fprintf(stderr, "skip NULL track %zu, total tracks %zu.\n", i, numTracks);
            return -1;
        }

        player.play(mediaSource);
    }

    return 0;
}
