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

#ifndef CODEC2_HIDL_V1_0_UTILS_TYPES_H
#define CODEC2_HIDL_V1_0_UTILS_TYPES_H

#include <bufferpool/ClientManager.h>
#include <android/hardware/media/bufferpool/2.0/IClientManager.h>
#include <android/hardware/media/bufferpool/2.0/types.h>
#include <android/hardware/media/c2/1.0/IComponentStore.h>
#include <android/hardware/media/c2/1.0/types.h>
#include <android/hidl/safe_union/1.0/types.h>

#include <C2Component.h>
#include <C2Param.h>
#include <C2ParamDef.h>
#include <C2Work.h>
#include <util/C2Debug-base.h>

#include <chrono>

using namespace std::chrono_literals;

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::hidl_bitfield;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::status_t;
using ::android::sp;
using ::android::hardware::media::bufferpool::V2_0::implementation::
        ConnectionId;

// Types of metadata for Blocks.
struct C2Hidl_Range {
    uint32_t offset;
    uint32_t length; // Do not use "size" because the name collides with C2Info::size().
};
typedef C2GlobalParam<C2Info, C2Hidl_Range, 0> C2Hidl_RangeInfo;

struct C2Hidl_Rect {
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
};
typedef C2GlobalParam<C2Info, C2Hidl_Rect, 1> C2Hidl_RectInfo;

// Make asString() and operator<< work with Status as well as c2_status_t.
C2_DECLARE_AS_STRING_AND_DEFINE_STREAM_OUT(Status);

/**
 * All objcpy() functions will return a boolean value indicating whether the
 * conversion succeeds or not.
 */

// C2SettingResult -> SettingResult
bool objcpy(
        SettingResult* d,
        const C2SettingResult& s);

// SettingResult -> std::unique_ptr<C2SettingResult>
bool objcpy(
        std::unique_ptr<C2SettingResult>* d,
        const SettingResult& s);

// C2ParamDescriptor -> ParamDescriptor
bool objcpy(
        ParamDescriptor* d,
        const C2ParamDescriptor& s);

// ParamDescriptor -> std::shared_ptr<C2ParamDescriptor>
bool objcpy(
        std::shared_ptr<C2ParamDescriptor>* d,
        const ParamDescriptor& s);

// C2FieldSupportedValuesQuery -> FieldSupportedValuesQuery
bool objcpy(
        FieldSupportedValuesQuery* d,
        const C2FieldSupportedValuesQuery& s);

// FieldSupportedValuesQuery -> C2FieldSupportedValuesQuery
bool objcpy(
        C2FieldSupportedValuesQuery* d,
        const FieldSupportedValuesQuery& s);

// C2FieldSupportedValuesQuery -> FieldSupportedValuesQueryResult
bool objcpy(
        FieldSupportedValuesQueryResult* d,
        const C2FieldSupportedValuesQuery& s);

// FieldSupportedValuesQuery, FieldSupportedValuesQueryResult -> C2FieldSupportedValuesQuery
bool objcpy(
        C2FieldSupportedValuesQuery* d,
        const FieldSupportedValuesQuery& sq,
        const FieldSupportedValuesQueryResult& sr);

// C2Component::Traits -> ComponentTraits
bool objcpy(
        IComponentStore::ComponentTraits* d,
        const C2Component::Traits& s);

// ComponentTraits -> C2Component::Traits
bool objcpy(
        C2Component::Traits* d,
        const IComponentStore::ComponentTraits& s);

// C2StructDescriptor -> StructDescriptor
bool objcpy(
        StructDescriptor* d,
        const C2StructDescriptor& s);

// StructDescriptor -> C2StructDescriptor
bool objcpy(
        std::unique_ptr<C2StructDescriptor>* d,
        const StructDescriptor& s);

// Abstract class to be used in
// objcpy(std::list<std::unique_ptr<C2Work>> -> WorkBundle).
struct BufferPoolSender {
    typedef ::android::hardware::media::bufferpool::V2_0::
            ResultStatus ResultStatus;
    typedef ::android::hardware::media::bufferpool::V2_0::
            BufferStatusMessage BufferStatusMessage;
    typedef ::android::hardware::media::bufferpool::
            BufferPoolData BufferPoolData;

    /**
     * Send bpData and return BufferStatusMessage that can be supplied to
     * IClientManager::receive() in the receiving process.
     *
     * This function will be called from within the function
     * objcpy(std::list<std::unique_ptr<C2Work>> -> WorkBundle).
     *
     * \param[in] bpData BufferPoolData identifying the buffer to send.
     * \param[out] bpMessage BufferStatusMessage of the transaction. Information
     *    inside \p bpMessage should be passed to the receiving process by some
     *    other means so it can call receive() properly.
     * \return ResultStatus value that determines the success of the operation.
     *    (See the possible values of ResultStatus in
     *    hardware/interfaces/media/bufferpool/2.0/types.hal.)
     */
    virtual ResultStatus send(
            const std::shared_ptr<BufferPoolData>& bpData,
            BufferStatusMessage* bpMessage) = 0;

    virtual ~BufferPoolSender() = default;
};

// Default implementation of BufferPoolSender.
//
// To use DefaultBufferPoolSender, the IClientManager instance of the receiving
// process must be set before send() can operate. DefaultBufferPoolSender will
// hold a strong reference to the IClientManager instance and use it to call
// IClientManager::registerSender() to establish the bufferpool connection when
// send() is called.
struct DefaultBufferPoolSender : BufferPoolSender {
    typedef ::android::hardware::media::bufferpool::V2_0::implementation::
            ClientManager ClientManager;
    typedef ::android::hardware::media::bufferpool::V2_0::
            IClientManager IClientManager;

    // Set the IClientManager instance of the receiving process and the refresh
    // interval for the connection. The default interval is 4.5 seconds, which
    // is slightly shorter than the amount of time the bufferpool will keep an
    // inactive connection for.
    DefaultBufferPoolSender(
            const sp<IClientManager>& receiverManager = nullptr,
            std::chrono::steady_clock::duration refreshInterval = 4500ms);

    // Set the IClientManager instance of the receiving process and the refresh
    // interval for the connection. The default interval is 4.5 seconds, which
    // is slightly shorter than the amount of time the bufferpool will keep an
    // inactive connection for.
    void setReceiver(
            const sp<IClientManager>& receiverManager,
            std::chrono::steady_clock::duration refreshInterval = 4500ms);

    // Implementation of BufferPoolSender::send(). send() will establish a
    // bufferpool connection if needed, then send the bufferpool data over to
    // the receiving process.
    virtual ResultStatus send(
            const std::shared_ptr<BufferPoolData>& bpData,
            BufferStatusMessage* bpMessage) override;

private:
    std::mutex mMutex;
    sp<ClientManager> mSenderManager;
    sp<IClientManager> mReceiverManager;
    int64_t mReceiverConnectionId;
    int64_t mSourceConnectionId;
    std::chrono::steady_clock::time_point mLastSent;
    std::chrono::steady_clock::duration mRefreshInterval;
};

// std::list<std::unique_ptr<C2Work>> -> WorkBundle
// Note: If bufferpool will be used, bpSender must not be null.
bool objcpy(
        WorkBundle* d,
        const std::list<std::unique_ptr<C2Work>>& s,
        BufferPoolSender* bpSender = nullptr);

// WorkBundle -> std::list<std::unique_ptr<C2Work>>
bool objcpy(
        std::list<std::unique_ptr<C2Work>>* d,
        const WorkBundle& s);

/**
 * Parses a params blob and returns C2Param pointers to its params. The pointers
 * point to locations inside the underlying buffer of \p blob. If \p blob is
 * destroyed, the pointers become invalid.
 *
 * \param[out] params target vector of C2Param pointers
 * \param[in] blob parameter blob to parse
 * \retval true if the full blob was parsed
 * \retval false otherwise
 */
bool parseParamsBlob(
        std::vector<C2Param*> *params,
        const hidl_vec<uint8_t> &blob);

/**
 * Concatenates a list of C2Params into a params blob.
 *
 * \param[out] blob target blob
 * \param[in] params parameters to concatenate
 * \retval true if the blob was successfully created
 * \retval false if the blob was not successful (this only happens if the
 *         parameters were not const)
 */
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<C2Param*> &params);
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::unique_ptr<C2Param>> &params);
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::shared_ptr<const C2Info>> &params);
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::unique_ptr<C2Tuning>> &params);

/**
 * Parses a params blob and create a vector of C2Params whose members are copies
 * of the params in the blob.
 *
 * \param[out] params the resulting vector
 * \param[in] blob parameter blob to parse
 * \retval true if the full blob was parsed and params was constructed
 * \retval false otherwise
 */
bool copyParamsFromBlob(
        std::vector<std::unique_ptr<C2Param>>* params,
        Params blob);
bool copyParamsFromBlob(
        std::vector<std::unique_ptr<C2Tuning>>* params,
        Params blob);

/**
 * Parses a params blob and applies updates to params.
 *
 * \param[in,out] params params to be updated
 * \param[in] blob parameter blob containing updates
 * \retval true if the full blob was parsed and params was updated
 * \retval false otherwise
 */
bool updateParamsFromBlob(
        const std::vector<C2Param*>& params,
        const Params& blob);

/**
 * Converts a BufferPool status value to c2_status_t.
 * \param BufferPool status
 * \return Corresponding c2_status_t
 */
c2_status_t toC2Status(::android::hardware::media::bufferpool::V2_0::
        ResultStatus rs);

// BufferQueue-Based Block Operations
// ==================================

// Call before transferring block to other processes.
//
// The given block is ready to transfer to other processes. This will guarantee
// the given block data is not mutated by bufferqueue migration.
bool beginTransferBufferQueueBlock(const C2ConstGraphicBlock& block);

// Call beginTransferBufferQueueBlock() on blocks in the given workList.
// processInput determines whether input blocks are yielded. processOutput
// works similarly on output blocks. (The default value of processInput is
// false while the default value of processOutput is true. This implies that in
// most cases, only output buffers contain bufferqueue-based blocks.)
void beginTransferBufferQueueBlocks(
        const std::list<std::unique_ptr<C2Work>>& workList,
        bool processInput = false,
        bool processOutput = true);

// Call after transferring block is finished and make sure that
// beginTransferBufferQueueBlock() is called before.
//
// The transfer of given block is finished. If transfer is successful the given
// block is not owned by process anymore. Since transfer is finished the given
// block data is OK to mutate by bufferqueue migration after this call.
bool endTransferBufferQueueBlock(const C2ConstGraphicBlock& block,
                                 bool transfer);

// Call endTransferBufferQueueBlock() on blocks in the given workList.
// processInput determines whether input blocks are yielded. processOutput
// works similarly on output blocks. (The default value of processInput is
// false while the default value of processOutput is true. This implies that in
// most cases, only output buffers contain bufferqueue-based blocks.)
void endTransferBufferQueueBlocks(
        const std::list<std::unique_ptr<C2Work>>& workList,
        bool transfer,
        bool processInput = false,
        bool processOutput = true);

// The given block is ready to be rendered. the given block is not owned by
// process anymore. If migration is in progress, this returns false in order
// not to render.
bool displayBufferQueueBlock(const C2ConstGraphicBlock& block);

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_TYPES_H
