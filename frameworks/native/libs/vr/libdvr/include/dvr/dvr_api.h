#ifndef ANDROID_DVR_API_H_
#define ANDROID_DVR_API_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cstdio>

#include <dvr/dvr_display_types.h>
#include <dvr/dvr_hardware_composer_types.h>
#include <dvr/dvr_pose.h>
#include <dvr/dvr_tracking_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define ALIGNED_DVR_STRUCT(x) __attribute__((packed, aligned(x)))
#else
#define ALIGNED_DVR_STRUCT(x)
#endif

typedef struct ANativeWindow ANativeWindow;

typedef struct DvrPoseAsync DvrPoseAsync;

typedef uint64_t DvrSurfaceUpdateFlags;
typedef struct DvrDisplayManager DvrDisplayManager;
typedef struct DvrSurfaceState DvrSurfaceState;
typedef struct DvrPoseClient DvrPoseClient;
typedef struct DvrPoseDataCaptureRequest DvrPoseDataCaptureRequest;
typedef struct DvrVSyncClient DvrVSyncClient;
typedef struct DvrVirtualTouchpad DvrVirtualTouchpad;

typedef struct DvrBuffer DvrBuffer;
typedef struct DvrWriteBuffer DvrWriteBuffer;
typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct AHardwareBuffer AHardwareBuffer;

typedef struct DvrReadBufferQueue DvrReadBufferQueue;
typedef struct DvrWriteBufferQueue DvrWriteBufferQueue;
typedef struct DvrNativeBufferMetadata DvrNativeBufferMetadata;

typedef struct DvrSurface DvrSurface;
typedef uint64_t DvrSurfaceAttributeType;
typedef int32_t DvrSurfaceAttributeKey;
typedef int32_t DvrGlobalBufferKey;

typedef struct DvrSurfaceAttributeValue DvrSurfaceAttributeValue;
typedef struct DvrSurfaceAttribute DvrSurfaceAttribute;

typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrTrackingCamera DvrTrackingCamera;
typedef struct DvrTrackingFeatureExtractor DvrTrackingFeatureExtractor;
typedef struct DvrTrackingSensors DvrTrackingSensors;
typedef struct DvrWriteBufferQueue DvrWriteBufferQueue;

// Note: To avoid breaking others during active development, only modify this
// struct by appending elements to the end.
// If you do feel we should to re-arrange or remove elements, please make a
// note of it, and wait until we're about to finalize for an API release to do
// so.
typedef struct DvrNativeDisplayMetrics {
  uint32_t display_width;
  uint32_t display_height;
  uint32_t display_x_dpi;
  uint32_t display_y_dpi;
  uint32_t vsync_period_ns;
} DvrNativeDisplayMetrics;

// native_handle contains the fds for the underlying ION allocations inside
// the gralloc buffer. This is needed temporarily while GPU vendors work on
// better support for AHardwareBuffer via glBindSharedBuffer APIs. See
// b/37207909. For now we can declare the native_handle struct where it is
// used for GPU late latching. See cutils/native_handle.h for the struct layout.
struct native_handle;

// Device metrics data type enums.
enum {
  // Request the device lens metrics protobuf. This matches cardboard protos.
  DVR_CONFIGURATION_DATA_LENS_METRICS = 0,
  // Request the device metrics protobuf.
  DVR_CONFIGURATION_DATA_DEVICE_METRICS = 1,
  // Request the per device configuration data file.
  DVR_CONFIGURATION_DATA_DEVICE_CONFIG = 2,
};

// dvr_display_manager.h
typedef int (*DvrDisplayManagerCreatePtr)(DvrDisplayManager** client_out);
typedef void (*DvrDisplayManagerDestroyPtr)(DvrDisplayManager* client);
typedef int (*DvrDisplayManagerGetEventFdPtr)(DvrDisplayManager* client);
typedef int (*DvrDisplayManagerTranslateEpollEventMaskPtr)(
    DvrDisplayManager* client, int in_events, int* out_events);
typedef int (*DvrDisplayManagerGetSurfaceStatePtr)(
    DvrDisplayManager* client, DvrSurfaceState* surface_state);
typedef int (*DvrDisplayManagerGetReadBufferQueuePtr)(
    DvrDisplayManager* client, int surface_id, int queue_id,
    DvrReadBufferQueue** queue_out);
typedef int (*DvrConfigurationDataGetPtr)(int config_type, uint8_t** data,
                                          size_t* data_size);
typedef void (*DvrConfigurationDataDestroyPtr)(uint8_t* data);
typedef int (*DvrSurfaceStateCreatePtr)(DvrSurfaceState** surface_state);
typedef void (*DvrSurfaceStateDestroyPtr)(DvrSurfaceState* surface_state);
typedef int (*DvrSurfaceStateGetSurfaceCountPtr)(DvrSurfaceState* surface_state,
                                                 size_t* count_out);
typedef int (*DvrSurfaceStateGetUpdateFlagsPtr)(
    DvrSurfaceState* surface_state, size_t surface_index,
    DvrSurfaceUpdateFlags* flags_out);
typedef int (*DvrSurfaceStateGetSurfaceIdPtr)(DvrSurfaceState* surface_state,
                                              size_t surface_index,
                                              int* surface_id_out);
typedef int (*DvrSurfaceStateGetProcessIdPtr)(DvrSurfaceState* surface_state,
                                              size_t surface_index,
                                              int* process_id_out);
typedef int (*DvrSurfaceStateGetQueueCountPtr)(DvrSurfaceState* surface_state,
                                               size_t surface_index,
                                               size_t* count_out);
typedef ssize_t (*DvrSurfaceStateGetQueueIdsPtr)(DvrSurfaceState* surface_state,
                                                 size_t surface_index,
                                                 int* queue_ids,
                                                 size_t max_count);
typedef int (*DvrSurfaceStateGetZOrderPtr)(DvrSurfaceState* surface_state,
                                           size_t surface_index,
                                           int* z_order_out);
typedef int (*DvrSurfaceStateGetVisiblePtr)(DvrSurfaceState* surface_state,
                                            size_t surface_index,
                                            bool* visible_out);
typedef int (*DvrSurfaceStateGetAttributeCountPtr)(
    DvrSurfaceState* surface_state, size_t surface_index, size_t* count_out);
typedef ssize_t (*DvrSurfaceStateGetAttributesPtr)(
    DvrSurfaceState* surface_state, size_t surface_index,
    DvrSurfaceAttribute* attributes, size_t max_attribute_count);

// dvr_buffer.h
typedef void (*DvrWriteBufferCreateEmptyPtr)(DvrWriteBuffer** write_buffer_out);
typedef void (*DvrWriteBufferDestroyPtr)(DvrWriteBuffer* write_buffer);
typedef int (*DvrWriteBufferIsValidPtr)(DvrWriteBuffer* write_buffer);
typedef int (*DvrWriteBufferClearPtr)(DvrWriteBuffer* write_buffer);
typedef int (*DvrWriteBufferGetIdPtr)(DvrWriteBuffer* write_buffer);
typedef int (*DvrWriteBufferGetAHardwareBufferPtr)(
    DvrWriteBuffer* write_buffer, AHardwareBuffer** hardware_buffer);
typedef int (*DvrWriteBufferPostPtr)(DvrWriteBuffer* write_buffer,
                                     int ready_fence_fd, const void* meta,
                                     size_t meta_size_bytes);
typedef int (*DvrWriteBufferGainPtr)(DvrWriteBuffer* write_buffer,
                                     int* release_fence_fd);
typedef int (*DvrWriteBufferGainAsyncPtr)(DvrWriteBuffer* write_buffer);
typedef const struct native_handle* (*DvrWriteBufferGetNativeHandlePtr)(
    DvrWriteBuffer* write_buffer);

typedef void (*DvrReadBufferCreateEmptyPtr)(DvrReadBuffer** read_buffer_out);
typedef void (*DvrReadBufferDestroyPtr)(DvrReadBuffer* read_buffer);
typedef int (*DvrReadBufferIsValidPtr)(DvrReadBuffer* read_buffer);
typedef int (*DvrReadBufferClearPtr)(DvrReadBuffer* read_buffer);
typedef int (*DvrReadBufferGetIdPtr)(DvrReadBuffer* read_buffer);
typedef int (*DvrReadBufferGetAHardwareBufferPtr)(
    DvrReadBuffer* read_buffer, AHardwareBuffer** hardware_buffer);
typedef int (*DvrReadBufferAcquirePtr)(DvrReadBuffer* read_buffer,
                                       int* ready_fence_fd, void* meta,
                                       size_t meta_size_bytes);
typedef int (*DvrReadBufferReleasePtr)(DvrReadBuffer* read_buffer,
                                       int release_fence_fd);
typedef int (*DvrReadBufferReleaseAsyncPtr)(DvrReadBuffer* read_buffer);
typedef const struct native_handle* (*DvrReadBufferGetNativeHandlePtr)(
    DvrReadBuffer* read_buffer);

typedef void (*DvrBufferDestroyPtr)(DvrBuffer* buffer);
typedef int (*DvrBufferGetAHardwareBufferPtr)(
    DvrBuffer* buffer, AHardwareBuffer** hardware_buffer);
typedef int (*DvrBufferGlobalLayoutVersionGetPtr)();
typedef const struct native_handle* (*DvrBufferGetNativeHandlePtr)(
    DvrBuffer* buffer);

// dvr_buffer_queue.h
typedef int (*DvrWriteBufferQueueCreatePtr)(uint32_t width, uint32_t height,
                                            uint32_t format,
                                            uint32_t layer_count,
                                            uint64_t usage, size_t capacity,
                                            size_t metadata_size,
                                            DvrWriteBufferQueue** queue_out);
typedef void (*DvrWriteBufferQueueDestroyPtr)(DvrWriteBufferQueue* write_queue);
typedef ssize_t (*DvrWriteBufferQueueGetCapacityPtr)(
    DvrWriteBufferQueue* write_queue);
typedef int (*DvrWriteBufferQueueGetIdPtr)(DvrWriteBufferQueue* write_queue);
typedef int (*DvrWriteBufferQueueGetExternalSurfacePtr)(
    DvrWriteBufferQueue* write_queue, ANativeWindow** out_window);
typedef int (*DvrWriteBufferQueueGetANativeWindowPtr)(
    DvrWriteBufferQueue* write_queue, ANativeWindow** out_window);
typedef int (*DvrWriteBufferQueueCreateReadQueuePtr)(
    DvrWriteBufferQueue* write_queue, DvrReadBufferQueue** out_read_queue);
typedef int (*DvrWriteBufferQueueDequeuePtr)(DvrWriteBufferQueue* write_queue,
                                             int timeout,
                                             DvrWriteBuffer* out_buffer,
                                             int* out_fence_fd);
typedef int (*DvrWriteBufferQueueGainBufferPtr)(
    DvrWriteBufferQueue* write_queue, int timeout,
    DvrWriteBuffer** out_write_buffer, DvrNativeBufferMetadata* out_meta,
    int* out_fence_fd);
typedef int (*DvrWriteBufferQueuePostBufferPtr)(
    DvrWriteBufferQueue* write_queue, DvrWriteBuffer* write_buffer,
    const DvrNativeBufferMetadata* meta, int ready_fence_fd);
typedef int (*DvrWriteBufferQueueResizeBufferPtr)(
    DvrWriteBufferQueue* write_queue, uint32_t width, uint32_t height);
typedef void (*DvrReadBufferQueueDestroyPtr)(DvrReadBufferQueue* read_queue);
typedef ssize_t (*DvrReadBufferQueueGetCapacityPtr)(
    DvrReadBufferQueue* read_queue);
typedef int (*DvrReadBufferQueueGetIdPtr)(DvrReadBufferQueue* read_queue);
typedef int (*DvrReadBufferQueueGetEventFdPtr)(DvrReadBufferQueue* read_queue);
typedef int (*DvrReadBufferQueueCreateReadQueuePtr)(
    DvrReadBufferQueue* read_queue, DvrReadBufferQueue** out_read_queue);
typedef int (*DvrReadBufferQueueDequeuePtr)(DvrReadBufferQueue* read_queue,
                                            int timeout,
                                            DvrReadBuffer* out_buffer,
                                            int* out_fence_fd, void* out_meta,
                                            size_t meta_size_bytes);
typedef int (*DvrReadBufferQueueAcquireBufferPtr)(
    DvrReadBufferQueue* read_queue, int timeout,
    DvrReadBuffer** out_read_buffer, DvrNativeBufferMetadata* out_meta,
    int* out_fence_fd);
typedef int (*DvrReadBufferQueueReleaseBufferPtr)(
    DvrReadBufferQueue* read_queue, DvrReadBuffer* read_buffer,
    const DvrNativeBufferMetadata* meta, int release_fence_fd);
typedef void (*DvrReadBufferQueueBufferAvailableCallback)(void* context);
typedef int (*DvrReadBufferQueueSetBufferAvailableCallbackPtr)(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferAvailableCallback callback, void* context);
typedef void (*DvrReadBufferQueueBufferRemovedCallback)(DvrReadBuffer* buffer,
                                                        void* context);
typedef int (*DvrReadBufferQueueSetBufferRemovedCallbackPtr)(
    DvrReadBufferQueue* read_queue,
    DvrReadBufferQueueBufferRemovedCallback callback, void* context);
typedef int (*DvrReadBufferQueueHandleEventsPtr)(
    DvrReadBufferQueue* read_queue);

// dvr_surface.h
typedef int (*DvrSetupGlobalBufferPtr)(DvrGlobalBufferKey key, size_t size,
                                       uint64_t usage, DvrBuffer** buffer_out);
typedef int (*DvrDeleteGlobalBufferPtr)(DvrGlobalBufferKey key);
typedef int (*DvrGetGlobalBufferPtr)(DvrGlobalBufferKey key,
                                     DvrBuffer** out_buffer);
typedef int (*DvrSurfaceCreatePtr)(const DvrSurfaceAttribute* attributes,
                                   size_t attribute_count,
                                   DvrSurface** surface_out);
typedef void (*DvrSurfaceDestroyPtr)(DvrSurface* surface);
typedef int (*DvrSurfaceGetIdPtr)(DvrSurface* surface);
typedef int (*DvrSurfaceSetAttributesPtr)(DvrSurface* surface,
                                          const DvrSurfaceAttribute* attributes,
                                          size_t attribute_count);
typedef int (*DvrSurfaceCreateWriteBufferQueuePtr)(
    DvrSurface* surface, uint32_t width, uint32_t height, uint32_t format,
    uint32_t layer_count, uint64_t usage, size_t capacity, size_t metadata_size,
    DvrWriteBufferQueue** queue_out);
typedef int (*DvrGetNativeDisplayMetricsPtr)(size_t sizeof_metrics,
                                             DvrNativeDisplayMetrics* metrics);

// dvr_vsync.h
typedef int (*DvrVSyncClientCreatePtr)(DvrVSyncClient** client_out);
typedef void (*DvrVSyncClientDestroyPtr)(DvrVSyncClient* client);
typedef int (*DvrVSyncClientGetSchedInfoPtr)(DvrVSyncClient* client,
                                             int64_t* vsync_period_ns,
                                             int64_t* next_timestamp_ns,
                                             uint32_t* next_vsync_count);

// libs/vr/libvrsensor/include/dvr/pose_client.h
typedef DvrPoseClient* (*DvrPoseClientCreatePtr)();
typedef void (*DvrPoseClientDestroyPtr)(DvrPoseClient* client);
typedef int (*DvrPoseClientGetPtr)(DvrPoseClient* client, uint32_t vsync_count,
                                   DvrPoseAsync* out_pose);
typedef uint32_t (*DvrPoseClientGetVsyncCountPtr)(DvrPoseClient* client);
typedef int (*DvrPoseClientGetControllerPtr)(DvrPoseClient* client,
                                             int32_t controller_id,
                                             uint32_t vsync_count,
                                             DvrPoseAsync* out_pose);
typedef int (*DvrPoseClientSensorsEnablePtr)(DvrPoseClient* client,
                                             bool enabled);
typedef int (*DvrPoseClientDataCapturePtr)(DvrPoseClient* client,
    const DvrPoseDataCaptureRequest* request);
typedef int (*DvrPoseClientDataReaderDestroyPtr)(DvrPoseClient* client,
                                                 uint64_t data_type);

// dvr_pose.h
typedef int (*DvrPoseClientGetDataReaderPtr)(DvrPoseClient* client,
                                             uint64_t data_type,
                                             DvrReadBufferQueue** read_queue);

// services/vr/virtual_touchpad/include/dvr/virtual_touchpad_client.h

// Touchpad IDs for *Touch*() and *ButtonState*() calls.
enum {
  DVR_VIRTUAL_TOUCHPAD_PRIMARY = 0,
  DVR_VIRTUAL_TOUCHPAD_VIRTUAL = 1,
};
typedef DvrVirtualTouchpad* (*DvrVirtualTouchpadCreatePtr)();
typedef void (*DvrVirtualTouchpadDestroyPtr)(DvrVirtualTouchpad* client);
typedef int (*DvrVirtualTouchpadAttachPtr)(DvrVirtualTouchpad* client);
typedef int (*DvrVirtualTouchpadDetachPtr)(DvrVirtualTouchpad* client);
typedef int (*DvrVirtualTouchpadTouchPtr)(DvrVirtualTouchpad* client,
                                          int touchpad, float x, float y,
                                          float pressure);
typedef int (*DvrVirtualTouchpadButtonStatePtr)(DvrVirtualTouchpad* client,
                                                int touchpad, int buttons);
typedef int (*DvrVirtualTouchpadScrollPtr)(DvrVirtualTouchpad* client,
                                           int touchpad, float x, float y);

// dvr_hardware_composer_client.h
typedef struct DvrHwcClient DvrHwcClient;
typedef struct DvrHwcFrame DvrHwcFrame;
typedef int (*DvrHwcOnFrameCallback)(void* client_state, DvrHwcFrame* frame);
typedef DvrHwcClient* (*DvrHwcClientCreatePtr)(DvrHwcOnFrameCallback callback,
                                               void* client_state);
typedef void (*DvrHwcClientDestroyPtr)(DvrHwcClient* client);
typedef void (*DvrHwcFrameDestroyPtr)(DvrHwcFrame* frame);
typedef DvrHwcDisplay (*DvrHwcFrameGetDisplayIdPtr)(DvrHwcFrame* frame);
typedef int32_t (*DvrHwcFrameGetDisplayWidthPtr)(DvrHwcFrame* frame);
typedef int32_t (*DvrHwcFrameGetDisplayHeightPtr)(DvrHwcFrame* frame);
typedef bool (*DvrHwcFrameGetDisplayRemovedPtr)(DvrHwcFrame* frame);
typedef size_t (*DvrHwcFrameGetLayerCountPtr)(DvrHwcFrame* frame);
typedef DvrHwcLayer (*DvrHwcFrameGetLayerIdPtr)(DvrHwcFrame* frame,
                                                size_t layer_index);
typedef uint32_t (*DvrHwcFrameGetActiveConfigPtr)(DvrHwcFrame* frame);
typedef uint32_t (*DvrHwcFrameGetColorModePtr)(DvrHwcFrame* frame);
typedef void (*DvrHwcFrameGetColorTransformPtr)(DvrHwcFrame* frame,
                                                float* out_matrix,
                                                int32_t* out_hint);
typedef uint32_t (*DvrHwcFrameGetPowerModePtr)(DvrHwcFrame* frame);
typedef uint32_t (*DvrHwcFrameGetVsyncEnabledPtr)(DvrHwcFrame* frame);
typedef AHardwareBuffer* (*DvrHwcFrameGetLayerBufferPtr)(DvrHwcFrame* frame,
                                                         size_t layer_index);
typedef int (*DvrHwcFrameGetLayerFencePtr)(DvrHwcFrame* frame,
                                           size_t layer_index);
typedef DvrHwcRecti (*DvrHwcFrameGetLayerDisplayFramePtr)(DvrHwcFrame* frame,
                                                          size_t layer_index);
typedef DvrHwcRectf (*DvrHwcFrameGetLayerCropPtr)(DvrHwcFrame* frame,
                                                  size_t layer_index);
typedef DvrHwcBlendMode (*DvrHwcFrameGetLayerBlendModePtr)(DvrHwcFrame* frame,
                                                           size_t layer_index);
typedef float (*DvrHwcFrameGetLayerAlphaPtr)(DvrHwcFrame* frame,
                                             size_t layer_index);
typedef uint32_t (*DvrHwcFrameGetLayerTypePtr)(DvrHwcFrame* frame,
                                               size_t layer_index);
typedef uint32_t (*DvrHwcFrameGetLayerApplicationIdPtr)(DvrHwcFrame* frame,
                                                        size_t layer_index);
typedef uint32_t (*DvrHwcFrameGetLayerZOrderPtr)(DvrHwcFrame* frame,
                                                 size_t layer_index);

typedef void (*DvrHwcFrameGetLayerCursorPtr)(DvrHwcFrame* frame,
                                             size_t layer_index, int32_t* out_x,
                                             int32_t* out_y);

typedef uint32_t (*DvrHwcFrameGetLayerTransformPtr)(DvrHwcFrame* frame,
                                                    size_t layer_index);

typedef uint32_t (*DvrHwcFrameGetLayerDataspacePtr)(DvrHwcFrame* frame,
                                                    size_t layer_index);

typedef uint32_t (*DvrHwcFrameGetLayerColorPtr)(DvrHwcFrame* frame,
                                                size_t layer_index);

typedef uint32_t (*DvrHwcFrameGetLayerNumVisibleRegionsPtr)(DvrHwcFrame* frame,
                                                            size_t layer_index);
typedef DvrHwcRecti (*DvrHwcFrameGetLayerVisibleRegionPtr)(DvrHwcFrame* frame,
                                                           size_t layer_index,
                                                           size_t index);

typedef uint32_t (*DvrHwcFrameGetLayerNumDamagedRegionsPtr)(DvrHwcFrame* frame,
                                                            size_t layer_index);
typedef DvrHwcRecti (*DvrHwcFrameGetLayerDamagedRegionPtr)(DvrHwcFrame* frame,
                                                           size_t layer_index,
                                                           size_t index);

// dvr_performance.h
typedef int (*DvrPerformanceSetSchedulerPolicyPtr)(
    pid_t task_id, const char* scheduler_policy);

// dvr_tracking.h
typedef int (*DvrTrackingCameraCreatePtr)(DvrTrackingCamera** out_camera);
typedef void (*DvrTrackingCameraDestroyPtr)(DvrTrackingCamera* camera);
typedef int (*DvrTrackingCameraStartPtr)(DvrTrackingCamera* camera,
                                         DvrWriteBufferQueue* write_queue);
typedef int (*DvrTrackingCameraStopPtr)(DvrTrackingCamera* camera);

typedef int (*DvrTrackingFeatureExtractorCreatePtr)(
    DvrTrackingFeatureExtractor** out_extractor);
typedef void (*DvrTrackingFeatureExtractorDestroyPtr)(
    DvrTrackingFeatureExtractor* extractor);
typedef void (*DvrTrackingFeatureCallback)(void* context,
                                           const DvrTrackingFeatures* event);
typedef int (*DvrTrackingFeatureExtractorStartPtr)(
    DvrTrackingFeatureExtractor* extractor,
    DvrTrackingFeatureCallback callback, void* context);
typedef int (*DvrTrackingFeatureExtractorStopPtr)(
    DvrTrackingFeatureExtractor* extractor);
typedef int (*DvrTrackingFeatureExtractorProcessBufferPtr)(
    DvrTrackingFeatureExtractor* extractor, DvrReadBuffer* buffer,
    const DvrTrackingBufferMetadata* metadata, bool* out_skipped);

typedef void (*DvrTrackingSensorEventCallback)(void* context,
                                               DvrTrackingSensorEvent* event);
typedef int (*DvrTrackingSensorsCreatePtr)(DvrTrackingSensors** out_sensors,
                                           const char* mode);
typedef void (*DvrTrackingSensorsDestroyPtr)(DvrTrackingSensors* sensors);
typedef int (*DvrTrackingSensorsStartPtr)(
    DvrTrackingSensors* sensors, DvrTrackingSensorEventCallback callback,
    void* context);
typedef int (*DvrTrackingSensorsStopPtr)(DvrTrackingSensors* sensors);

// The buffer metadata that an Android Surface (a.k.a. ANativeWindow)
// will populate. A DvrWriteBufferQueue must be created with this metadata iff
// ANativeWindow access is needed. Please do not remove, modify, or reorder
// existing data members. If new fields need to be added, please take extra care
// to make sure that new data field is padded properly the size of the struct
// stays same.
// TODO(b/118893702): move the definition to libnativewindow or libui
struct ALIGNED_DVR_STRUCT(8) DvrNativeBufferMetadata {
#ifdef __cplusplus
  DvrNativeBufferMetadata()
      : timestamp(0),
        is_auto_timestamp(0),
        dataspace(0),
        crop_left(0),
        crop_top(0),
        crop_right(0),
        crop_bottom(0),
        scaling_mode(0),
        transform(0),
        index(0),
        user_metadata_size(0),
        user_metadata_ptr(0),
        release_fence_mask(0),
        reserved{0} {}
#endif
  // Timestamp of the frame.
  int64_t timestamp;

  // Whether the buffer is using auto timestamp.
  int32_t is_auto_timestamp;

  // Must be one of the HAL_DATASPACE_XXX value defined in system/graphics.h
  int32_t dataspace;

  // Crop extracted from an ACrop or android::Crop object.
  int32_t crop_left;
  int32_t crop_top;
  int32_t crop_right;
  int32_t crop_bottom;

  // Must be one of the NATIVE_WINDOW_SCALING_MODE_XXX value defined in
  // system/window.h.
  int32_t scaling_mode;

  // Must be one of the ANATIVEWINDOW_TRANSFORM_XXX value defined in
  // android/native_window.h
  int32_t transform;

  // The index of the frame.
  int64_t index;

  // Size of additional metadata requested by user.
  uint64_t user_metadata_size;

  // Raw memory address of the additional user defined metadata. Only valid when
  // user_metadata_size is non-zero.
  uint64_t user_metadata_ptr;

  // Only applicable for metadata retrieved from GainAsync. This indicates which
  // consumer has pending fence that producer should epoll on.
  uint32_t release_fence_mask;

  // Reserved bytes for so that the struct is forward compatible and padding to
  // 104 bytes so the size is a multiple of 8.
  int32_t reserved[9];
};

#ifdef __cplusplus
// Warning: DvrNativeBufferMetadata is part of the DVR API and changing its size
// will cause compatiblity issues between different DVR API releases.
static_assert(sizeof(DvrNativeBufferMetadata) == 104,
              "Unexpected size for DvrNativeBufferMetadata");
#endif

struct DvrApi_v1 {
// Defines an API entry for V1 (no version suffix).
#define DVR_V1_API_ENTRY(name) Dvr##name##Ptr name
#define DVR_V1_API_ENTRY_DEPRECATED(name) Dvr##name##Ptr name

#include "dvr_api_entries.h"

// Undefine macro definitions to play nice with Google3 style rules.
#undef DVR_V1_API_ENTRY
#undef DVR_V1_API_ENTRY_DEPRECATED
};

int dvrGetApi(void* api, size_t struct_size, int version);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_DVR_API_H_
