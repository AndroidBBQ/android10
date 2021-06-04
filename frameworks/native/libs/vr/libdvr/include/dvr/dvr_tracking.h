#ifndef ANDROID_DVR_TRACKING_H_
#define ANDROID_DVR_TRACKING_H_

#include <stdint.h>
#include <sys/cdefs.h>

#include <dvr/dvr_tracking_types.h>

__BEGIN_DECLS

typedef struct DvrReadBuffer DvrReadBuffer;
typedef struct DvrTrackingCamera DvrTrackingCamera;
typedef struct DvrTrackingFeatureExtractor DvrTrackingFeatureExtractor;
typedef struct DvrTrackingSensors DvrTrackingSensors;
typedef struct DvrWriteBufferQueue DvrWriteBufferQueue;

// The callback for DvrTrackingFeatureExtractor that will deliver the feature
// events. This callback is passed to dvrTrackingFeatureExtractorStart.
typedef void (*DvrTrackingFeatureCallback)(void* context,
                                           const DvrTrackingFeatures* event);

// The callback for DvrTrackingSensors session that will deliver the events.
// This callback is passed to dvrTrackingSensorsStart.
typedef void (*DvrTrackingSensorEventCallback)(void* context,
                                               DvrTrackingSensorEvent* event);

// Creates a DvrTrackingCamera session.
//
// On creation, the session is not in operating mode. Client code must call
// dvrTrackingCameraStart to bootstrap the underlying camera stack.
//
// There is no plan to expose camera configuration through this API. All camera
// parameters are determined by the system optimized for better tracking
// results. See b/78662281 for detailed deprecation plan of this API and the
// Stage 2 of VR tracking data source refactoring.
//
// @param out_camera The pointer of a DvrTrackingCamera will be filled here if
//     the method call succeeds.
// @return Zero on success, or negative error code.
int dvrTrackingCameraCreate(DvrTrackingCamera** out_camera);

// Destroys a DvrTrackingCamera handle.
//
// @param camera The DvrTrackingCamera of interest.
void dvrTrackingCameraDestroy(DvrTrackingCamera* camera);

// Starts the DvrTrackingCamera.
//
// On successful return, all DvrReadBufferQueue's associated with the given
// write_queue will start to receive buffers from the camera stack. Note that
// clients of this API should not assume the buffer dimension, format, and/or
// usage of the outcoming buffers, as they are governed by the underlying camera
// logic. Also note that it's the client's responsibility to consume buffers
// from DvrReadBufferQueue on time and return them back to the producer;
// otherwise the camera stack might be blocked.
//
// @param camera The DvrTrackingCamera of interest.
// @param write_queue A DvrWriteBufferQueue that the camera stack can use to
//     populate the buffer into. The queue must be empty and the camera stack
//     will request buffer allocation with proper buffer dimension, format, and
//     usage. Note that the write queue must be created with user_metadata_size
//     set to sizeof(DvrTrackingBufferMetadata). On success, the write_queue
//     handle will become invalid and the ownership of the queue handle will be
//     transferred into the camera; otherwise, the write_queue handle will keep
//     untouched and the caller still has the ownership.
// @return Zero on success, or negative error code.
int dvrTrackingCameraStart(DvrTrackingCamera* camera,
                           DvrWriteBufferQueue* write_queue);

// Stops the DvrTrackingCamera.
//
// On successful return, the DvrWriteBufferQueue set during
// dvrTrackingCameraStart will stop getting new buffers from the camera stack.
//
// @param camera The DvrTrackingCamera of interest.
// @return Zero on success, or negative error code.
int dvrTrackingCameraStop(DvrTrackingCamera* camera);

// Creates a DvrTrackingSensors session.
//
// This will initialize but not start device sensors (gyro / accel). Upon
// successfull creation, the clients can call dvrTrackingSensorsStart to start
// receiving sensor events.
//
// @param out_sensors The pointer of a DvrTrackingSensors will be filled here if
//     the method call succeeds.
// @param mode The sensor mode.
//        mode="ndk": Use the Android NDK.
//        mode="direct": Use direct mode sensors (lower latency).
// @return Zero on success, or negative error code.
int dvrTrackingSensorsCreate(DvrTrackingSensors** out_sensors,
                             const char* mode);

// Destroys a DvrTrackingSensors session.
//
// @param sensors The DvrTrackingSensors struct to destroy.
void dvrTrackingSensorsDestroy(DvrTrackingSensors* sensors);

// Starts the tracking sensor session.
//
// This will start the device sensors and start pumping the feature and sensor
// events as they arrive.
//
// @param client A tracking client created by dvrTrackingSensorsCreate.
// @param context A client supplied pointer that will be passed to the callback.
// @param callback A callback that will receive the sensor events on an
// arbitrary thread.
// @return Zero on success, or negative error code.
int dvrTrackingSensorsStart(DvrTrackingSensors* sensors,
                            DvrTrackingSensorEventCallback callback,
                            void* context);

// Stops a DvrTrackingSensors session.
//
// This will stop the device sensors. dvrTrackingSensorsStart can be called to
// restart them again.
//
// @param client A tracking client created by dvrTrackingClientCreate.
// @return Zero on success, or negative error code.
int dvrTrackingSensorsStop(DvrTrackingSensors* sensors);

// Creates a tracking feature extractor.
//
// This will initialize but not start the feature extraction session. Upon
// successful creation, the client can call dvrTrackingFeatureExtractorStart to
// start receiving features.
//
// @param out_extractor The pointer of a DvrTrackingFeatureExtractor will be
//     filled here if the method call succeeds.
int dvrTrackingFeatureExtractorCreate(
    DvrTrackingFeatureExtractor** out_extractor);

// Destroys a tracking feature extractor.
//
// @param extractor The DvrTrackingFeatureExtractor to destroy.
void dvrTrackingFeatureExtractorDestroy(DvrTrackingFeatureExtractor* extractor);

// Starts the tracking feature extractor.
//
// This will start the extractor and start pumping the output feature events to
// the registered callback. Note that this method will create one or more
// threads to handle feature processing.
//
// @param extractor The DvrTrackingFeatureExtractor to destroy.
int dvrTrackingFeatureExtractorStart(DvrTrackingFeatureExtractor* extractor,
                                     DvrTrackingFeatureCallback callback,
                                     void* context);

// Stops the tracking feature extractor.
//
// This will stop the extractor session and clean up all internal resourcse
// related to this extractor. On succssful return, all internal therad started
// by dvrTrackingFeatureExtractorStart should be stopped.
//
// @param extractor The DvrTrackingFeatureExtractor to destroy.
int dvrTrackingFeatureExtractorStop(DvrTrackingFeatureExtractor* extractor);

// Processes one buffer to extract features from.
//
// The buffer will be sent over to DSP for feature extraction. Once the process
// is done, the processing thread will invoke DvrTrackingFeatureCallback with
// newly extracted features. Note that not all buffers will be processed, as the
// underlying DSP can only process buffers at a certain framerate. If a buffer
// needs to be skipped, out_skipped filed will be set to true. Also note that
// for successfully processed stereo buffer, two callbacks (one for each eye)
// will be fired.
//
// @param extractor The DvrTrackingFeatureExtractor to destroy.
// @param buffer The buffer to extract features from. Note that the buffer must
//     be in acquired state for the buffer to be processed. Also note that the
//     buffer will be released back to its producer on successful return of the
//     method.
// @param metadata The metadata associated with the buffer. Should be populated
//     by DvrTrackingCamera session as user defined metadata.
// @param out_skipped On successful return, the field will be set to true iff
//     the buffer was skipped; and false iff the buffer was processed. This
//     field is optional and nullptr can be passed here to ignore the field.
// @return Zero on success, or negative error code.
int dvrTrackingFeatureExtractorProcessBuffer(
    DvrTrackingFeatureExtractor* extractor, DvrReadBuffer* buffer,
    const DvrTrackingBufferMetadata* metadata, bool* out_skipped);

__END_DECLS

#endif  // ANDROID_DVR_TRACKING_H_
