/*
 * Copyright 2018 The Android Open Source Project
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
#define LOG_TAG "Codec2-InputSurfaceConnection"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/InputSurfaceConnection.h>
#include <codec2/hidl/1.0/InputSurfaceConnection.h>

#include <memory>
#include <list>
#include <mutex>
#include <atomic>

#include <hidl/HidlSupport.h>
#include <media/stagefright/bqhelper/ComponentWrapper.h>
#include <system/graphics.h>
#include <ui/GraphicBuffer.h>
#include <utils/Errors.h>

#include <C2.h>
#include <C2AllocatorGralloc.h>
#include <C2BlockInternal.h>
#include <C2Buffer.h>
#include <C2Component.h>
#include <C2Config.h>
#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <C2Work.h>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

constexpr int32_t kBufferCount = 16;

using namespace ::android;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;

namespace /* unnamed */ {

class Buffer2D : public C2Buffer {
public:
    explicit Buffer2D(C2ConstGraphicBlock block) : C2Buffer({ block }) {
    }
};

} // unnamed namespace

// Derived class of ComponentWrapper for use with
// GraphicBufferSource::configure().
//
struct InputSurfaceConnection::Impl : public ComponentWrapper {

    Impl(const sp<GraphicBufferSource>& source,
         const std::shared_ptr<C2Component>& localComp)
          : mSource{source}, mLocalComp{localComp}, mSink{}, mFrameIndex{0} {
        std::shared_ptr<C2ComponentInterface> intf = localComp->intf();
        mSinkName = intf ? intf->getName() : "";
    }

    Impl(const sp<GraphicBufferSource>& source,
         const sp<IInputSink>& sink)
          : mSource{source}, mLocalComp{}, mSink{sink}, mFrameIndex{0} {
        Return<sp<IConfigurable>> transResult = sink->getConfigurable();
        if (!transResult.isOk()) {
            LOG(ERROR) << "Remote sink is dead.";
            return;
        }
        mSinkConfigurable =
                static_cast<sp<IConfigurable>>(transResult);
        if (!mSinkConfigurable) {
            LOG(ERROR) << "Remote sink is not configurable.";
            mSinkName = "";
            return;
        }

        hidl_string name;
        Return<void> transStatus = mSinkConfigurable->getName(
                [&name](const hidl_string& n) {
                    name = n;
                });
        if (!transStatus.isOk()) {
            LOG(ERROR) << "Remote sink's configurable is dead.";
            mSinkName = "";
            return;
        }
        mSinkName = name.c_str();
    }

    virtual ~Impl() {
        mSource->stop();
        mSource->release();
    }

    bool init() {
        if (mSource == nullptr) {
            return false;
        }
        status_t err = mSource->initCheck();
        if (err != OK) {
            LOG(WARNING) << "Impl::init -- GraphicBufferSource init failed: "
                         << "status = " << err << ".";
            return false;
        }

        // TODO: read settings properly from the interface
        C2StreamPictureSizeInfo::input inputSize;
        C2StreamUsageTuning::input usage;
        c2_status_t c2Status = queryFromSink({ &inputSize, &usage },
                                         {},
                                         C2_MAY_BLOCK,
                                         nullptr);
        if (c2Status != C2_OK) {
            LOG(WARNING) << "Impl::init -- cannot query information from "
                            "the component interface: "
                         << "status = " << asString(c2Status) << ".";
            return false;
        }

        // TODO: proper color aspect & dataspace
        android_dataspace dataSpace = HAL_DATASPACE_BT709;

        // TODO: use the usage read from intf
        // uint32_t grallocUsage =
        //         C2AndroidMemoryUsage(C2MemoryUsage(usage.value)).
        //         asGrallocUsage();

        uint32_t grallocUsage =
                mSinkName.compare(0, 11, "c2.android.") == 0 ?
                GRALLOC_USAGE_SW_READ_OFTEN :
                GRALLOC_USAGE_HW_VIDEO_ENCODER;

        err = mSource->configure(
                this, dataSpace, kBufferCount,
                inputSize.width, inputSize.height,
                grallocUsage);
        if (err != OK) {
            LOG(WARNING) << "Impl::init -- GBS configure failed: "
                         << "status = " << err << ".";
            return false;
        }
        for (int32_t i = 0; i < kBufferCount; ++i) {
            if (!mSource->onInputBufferAdded(i).isOk()) {
                LOG(WARNING) << "Impl::init: failed to populate GBS slots.";
                return false;
            }
        }
        if (!mSource->start().isOk()) {
            LOG(WARNING) << "Impl::init -- GBS failed to start.";
            return false;
        }
        mAllocatorMutex.lock();
        c2_status_t c2err = GetCodec2PlatformAllocatorStore()->fetchAllocator(
                C2AllocatorStore::PLATFORM_START + 1,  // GRALLOC
                &mAllocator);
        mAllocatorMutex.unlock();
        if (c2err != OK) {
            LOG(WARNING) << "Impl::init -- failed to fetch gralloc allocator: "
                         << "status = " << asString(c2err) << ".";
            return false;
        }
        return true;
    }

    // From ComponentWrapper
    virtual status_t submitBuffer(
            int32_t bufferId,
            const sp<GraphicBuffer>& buffer,
            int64_t timestamp,
            int fenceFd) override {
        LOG(VERBOSE) << "Impl::submitBuffer -- bufferId = " << bufferId << ".";
        // TODO: Use fd to construct fence
        (void)fenceFd;

        std::shared_ptr<C2GraphicAllocation> alloc;
        C2Handle* handle = WrapNativeCodec2GrallocHandle(
                buffer->handle,
                buffer->width, buffer->height,
                buffer->format, buffer->usage, buffer->stride);
        mAllocatorMutex.lock();
        c2_status_t err = mAllocator->priorGraphicAllocation(handle, &alloc);
        mAllocatorMutex.unlock();
        if (err != OK) {
            return UNKNOWN_ERROR;
        }
        std::shared_ptr<C2GraphicBlock> block =
                _C2BlockFactory::CreateGraphicBlock(alloc);

        std::unique_ptr<C2Work> work(new C2Work);
        work->input.flags = (C2FrameData::flags_t)0;
        work->input.ordinal.timestamp = timestamp;
        work->input.ordinal.frameIndex = mFrameIndex.fetch_add(
                1, std::memory_order_relaxed);
        work->input.buffers.clear();
        std::shared_ptr<C2Buffer> c2Buffer(
                // TODO: fence
                new Buffer2D(block->share(
                        C2Rect(block->width(), block->height()), ::C2Fence())),
                [bufferId, source = mSource](C2Buffer* ptr) {
                    delete ptr;
                    if (source != nullptr) {
                        // TODO: fence
                        (void)source->onInputBufferEmptied(bufferId, -1);
                    }
                });
        work->input.buffers.push_back(c2Buffer);
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);
        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        err = queueToSink(&items);
        return (err == C2_OK) ? OK : UNKNOWN_ERROR;
    }

    virtual status_t submitEos(int32_t bufferId) override {
        LOG(VERBOSE) << "Impl::submitEos -- bufferId = " << bufferId << ".";
        (void)bufferId;

        std::unique_ptr<C2Work> work(new C2Work);
        work->input.flags = (C2FrameData::flags_t)0;
        work->input.ordinal.frameIndex = mFrameIndex.fetch_add(
                1, std::memory_order_relaxed);
        work->input.buffers.clear();
        work->worklets.clear();
        work->worklets.emplace_back(new C2Worklet);
        std::list<std::unique_ptr<C2Work>> items;
        items.push_back(std::move(work));

        c2_status_t err = queueToSink(&items);
        return (err == C2_OK) ? OK : UNKNOWN_ERROR;
    }

    virtual void dispatchDataSpaceChanged(
            int32_t dataSpace, int32_t aspects, int32_t pixelFormat) override {
        // TODO
        (void)dataSpace;
        (void)aspects;
        (void)pixelFormat;
    }

    // Configurable interface for InputSurfaceConnection::Impl.
    //
    // This class is declared as an inner class so that it will have access to
    // all Impl's members.
    struct ConfigurableIntf : public ConfigurableC2Intf {
        sp<Impl> mConnection;
        ConfigurableIntf(const sp<Impl>& connection)
              : ConfigurableC2Intf{"input-surface-connection", 0},
                mConnection{connection} {}
        virtual c2_status_t config(
                const std::vector<C2Param*> &params,
                c2_blocking_t mayBlock,
                std::vector<std::unique_ptr<C2SettingResult>> *const failures
                ) override;
        virtual c2_status_t query(
                const std::vector<C2Param::Index> &indices,
                c2_blocking_t mayBlock,
                std::vector<std::unique_ptr<C2Param>> *const params) const override;
        virtual c2_status_t querySupportedParams(
                std::vector<std::shared_ptr<C2ParamDescriptor>> *const params
                ) const override;
        virtual c2_status_t querySupportedValues(
                std::vector<C2FieldSupportedValuesQuery> &fields,
                c2_blocking_t mayBlock) const override;
    };

private:
    c2_status_t queryFromSink(
            const std::vector<C2Param*> &stackParams,
            const std::vector<C2Param::Index> &heapParamIndices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const heapParams) {
        if (mLocalComp) {
            std::shared_ptr<C2ComponentInterface> intf = mLocalComp->intf();
            if (intf) {
                return intf->query_vb(stackParams,
                                      heapParamIndices,
                                      mayBlock,
                                      heapParams);
            } else {
                LOG(ERROR) << "queryFromSink -- "
                           << "component does not have an interface.";
                return C2_BAD_STATE;
            }
        }

        CHECK(mSink) << "-- queryFromSink "
                     << "-- connection has no sink.";
        CHECK(mSinkConfigurable) << "-- queryFromSink "
                                 << "-- sink has no configurable.";

        hidl_vec<ParamIndex> indices(
                stackParams.size() + heapParamIndices.size());
        size_t numIndices = 0;
        for (C2Param* const& stackParam : stackParams) {
            if (!stackParam) {
                LOG(DEBUG) << "queryFromSink -- null stack param encountered.";
                continue;
            }
            indices[numIndices++] = static_cast<ParamIndex>(stackParam->index());
        }
        size_t numStackIndices = numIndices;
        for (const C2Param::Index& index : heapParamIndices) {
            indices[numIndices++] =
                    static_cast<ParamIndex>(static_cast<uint32_t>(index));
        }
        indices.resize(numIndices);
        if (heapParams) {
            heapParams->reserve(heapParams->size() + numIndices);
        }
        c2_status_t status;
        Return<void> transStatus = mSinkConfigurable->query(
                indices,
                mayBlock == C2_MAY_BLOCK,
                [&status, &numStackIndices, &stackParams, heapParams](
                        Status s, const Params& p) {
                    status = static_cast<c2_status_t>(s);
                    if (status != C2_OK && status != C2_BAD_INDEX) {
                        LOG(DEBUG) << "queryFromSink -- call failed: "
                                   << "status = " << asString(status) << ".";
                        return;
                    }
                    std::vector<C2Param*> paramPointers;
                    if (!parseParamsBlob(&paramPointers, p)) {
                        LOG(DEBUG) << "queryFromSink -- error while "
                                   << "parsing params.";
                        status = C2_CORRUPTED;
                        return;
                    }
                    size_t i = 0;
                    for (auto it = paramPointers.begin();
                            it != paramPointers.end(); ) {
                        C2Param* paramPointer = *it;
                        if (numStackIndices > 0) {
                            --numStackIndices;
                            if (!paramPointer) {
                                LOG(DEBUG) << "queryFromSink -- "
                                              "null stack param.";
                                ++it;
                                continue;
                            }
                            for (; i < stackParams.size() &&
                                    !stackParams[i]; ) {
                                ++i;
                            }
                            CHECK(i < stackParams.size());
                            if (stackParams[i]->index() !=
                                    paramPointer->index()) {
                                LOG(DEBUG) << "queryFromSink -- "
                                              "param skipped (index = "
                                           << stackParams[i]->index() << ").";
                                stackParams[i++]->invalidate();
                                continue;
                            }
                            if (!stackParams[i++]->updateFrom(*paramPointer)) {
                                LOG(DEBUG) << "queryFromSink -- "
                                              "param update failed (index = "
                                           << paramPointer->index() << ").";
                            }
                        } else {
                            if (!paramPointer) {
                                LOG(DEBUG) << "queryFromSink -- "
                                              "null heap param.";
                                ++it;
                                continue;
                            }
                            if (!heapParams) {
                                LOG(WARNING) << "queryFromSink -- "
                                                "too many stack params.";
                                break;
                            }
                            heapParams->emplace_back(C2Param::Copy(*paramPointer));
                        }
                        ++it;
                    }
                });
        if (!transStatus.isOk()) {
            LOG(ERROR) << "queryFromSink -- transaction failed.";
            return C2_CORRUPTED;
        }
        return status;
    }

    c2_status_t queueToSink(std::list<std::unique_ptr<C2Work>>* const items) {
        if (mLocalComp) {
            return mLocalComp->queue_nb(items);
        }

        CHECK(mSink) << "-- queueToSink "
                     << "-- connection has no sink.";

        WorkBundle workBundle;
        if (!objcpy(&workBundle, *items, nullptr)) {
            LOG(ERROR) << "queueToSink -- bad input.";
            return C2_CORRUPTED;
        }
        Return<Status> transStatus = mSink->queue(workBundle);
        if (!transStatus.isOk()) {
            LOG(ERROR) << "queueToSink -- transaction failed.";
            return C2_CORRUPTED;
        }
        c2_status_t status =
                static_cast<c2_status_t>(static_cast<Status>(transStatus));
        if (status != C2_OK) {
            LOG(DEBUG) << "queueToSink -- call failed: "
                         << asString(status);
        }
        return status;
    }

    sp<GraphicBufferSource> mSource;
    std::shared_ptr<C2Component> mLocalComp;
    sp<IInputSink> mSink;
    sp<IConfigurable> mSinkConfigurable;
    std::string mSinkName;

    // Needed for ComponentWrapper implementation
    std::mutex mAllocatorMutex;
    std::shared_ptr<C2Allocator> mAllocator;
    std::atomic_uint64_t mFrameIndex;

};

InputSurfaceConnection::InputSurfaceConnection(
        const sp<GraphicBufferSource>& source,
        const std::shared_ptr<C2Component>& comp,
        const sp<ComponentStore>& store)
      : mImpl{new Impl(source, comp)},
        mConfigurable{new CachedConfigurable(
            std::make_unique<Impl::ConfigurableIntf>(mImpl))} {
    mConfigurable->init(store.get());
}

InputSurfaceConnection::InputSurfaceConnection(
        const sp<GraphicBufferSource>& source,
        const sp<IInputSink>& sink,
        const sp<ComponentStore>& store)
      : mImpl{new Impl(source, sink)},
        mConfigurable{new CachedConfigurable(
            std::make_unique<Impl::ConfigurableIntf>(mImpl))} {
    mConfigurable->init(store.get());
}

Return<Status> InputSurfaceConnection::disconnect() {
    std::lock_guard<std::mutex> lock(mImplMutex);
    mImpl = nullptr;
    return Status::OK;
}

InputSurfaceConnection::~InputSurfaceConnection() {
    mImpl = nullptr;
}

bool InputSurfaceConnection::init() {
    std::lock_guard<std::mutex> lock(mImplMutex);
    return mImpl->init();
}

Return<sp<IConfigurable>> InputSurfaceConnection::getConfigurable() {
    return mConfigurable;
}

// Configurable interface for InputSurfaceConnection::Impl
c2_status_t InputSurfaceConnection::Impl::ConfigurableIntf::config(
        const std::vector<C2Param*> &params,
        c2_blocking_t mayBlock,
        std::vector<std::unique_ptr<C2SettingResult>> *const failures) {
    // TODO: implement
    (void)params;
    (void)mayBlock;
    (void)failures;
    return C2_OK;
}

c2_status_t InputSurfaceConnection::Impl::ConfigurableIntf::query(
        const std::vector<C2Param::Index> &indices,
        c2_blocking_t mayBlock,
        std::vector<std::unique_ptr<C2Param>> *const params) const {
    // TODO: implement
    (void)indices;
    (void)mayBlock;
    (void)params;
    return C2_OK;
}

c2_status_t InputSurfaceConnection::Impl::ConfigurableIntf::querySupportedParams(
        std::vector<std::shared_ptr<C2ParamDescriptor>> *const params) const {
    // TODO: implement
    (void)params;
    return C2_OK;
}

c2_status_t InputSurfaceConnection::Impl::ConfigurableIntf::querySupportedValues(
        std::vector<C2FieldSupportedValuesQuery> &fields,
        c2_blocking_t mayBlock) const {
    // TODO: implement
    (void)fields;
    (void)mayBlock;
    return C2_OK;
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

