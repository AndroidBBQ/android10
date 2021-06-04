#ifndef ANDROID_DVR_HARDWARE_COMPOSER_CLIENT_H
#define ANDROID_DVR_HARDWARE_COMPOSER_CLIENT_H

#include <dvr/dvr_hardware_composer_types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AHardwareBuffer AHardwareBuffer;
typedef struct DvrHwcClient DvrHwcClient;
typedef struct DvrHwcFrame DvrHwcFrame;

// Called when a new frame has arrived.
//
// @param client_state Pointer to client state passed in |dvrHwcCreateClient()|.
// @param frame New frame. Owned by the client.
// @return fence FD for the release of the last frame.
typedef int(*DvrHwcOnFrameCallback)(void* client_state, DvrHwcFrame* frame);

// @param callback Called when a new frame is available.
// @param client_state Pointer to client state passed back in the callback.
DvrHwcClient* dvrHwcClientCreate(DvrHwcOnFrameCallback callback,
                                 void* client_state);

// Called to free the DvrHwcClient pointer.
void dvrHwcClientDestroy(DvrHwcClient* client);

// Called to free the frame information.
// @param frame Pointer for the valid frame used for the query.
void dvrHwcFrameDestroy(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return Identifier for the display associated by the frame.
DvrHwcDisplay dvrHwcFrameGetDisplayId(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return width of the physical display associated with |frame|. This does not
// take into account any orientation changes.
int32_t dvrHwcFrameGetDisplayWidth(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return height of the physical display associated with |frame|. This does not
// take into account any orientation changes.
int32_t dvrHwcFrameGetDisplayHeight(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return True if the display has been removed. In this case the current frame
// does not contain any valid layers to display. It is a signal to clean up any
// display related state.
bool dvrHwcFrameGetDisplayRemoved(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return Number of layers in the frame.
size_t dvrHwcFrameGetLayerCount(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return The ID of the currently active display configuration.
uint32_t dvrHwcFrameGetActiveConfig(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return The ID of the current color mode. See HAL_COLOR_MODE_* for valid
// values.
uint32_t dvrHwcFrameGetColorMode(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @param out_matrix Output parameter for a float[16] array which will be filled
// with the color transform matrix.
// @param out_hint Output parameter which will contain the color transform hint.
// See HAL_COLOR_TRANSFORM_* for valid values.
void dvrHwcFrameGetColorTransform(DvrHwcFrame* frame, float* out_matrix,
                                  int32_t* out_hint);

// @param frame Pointer for the valid frame used for the query.
// @return The current power mode for the display. See HWC2_POWER_MODE_* for
// valid values.
uint32_t dvrHwcFrameGetPowerMode(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @return The current state of vsync. See HWC2_VSYNC_* for valid values.
uint32_t dvrHwcFrameGetVsyncEnabled(DvrHwcFrame* frame);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return A unique ID for the layer.
DvrHwcLayer dvrHwcFrameGetLayerId(DvrHwcFrame* frame, size_t layer_index);

// Return the graphic buffer associated with the layer at |layer_index| in
// |frame|.
//
// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return Graphic buffer. Caller owns the buffer and is responsible for freeing
// it. (see AHardwareBuffer_release())
AHardwareBuffer* dvrHwcFrameGetLayerBuffer(DvrHwcFrame* frame,
                                           size_t layer_index);

// Returns the fence FD for the layer at index |layer_index| in |frame|.
//
// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return Fence FD. Caller owns the FD and is responsible for closing it.
int dvrHwcFrameGetLayerFence(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return describing the portion of the display covered by the layer. Will
// not exceed the display dimensions.
DvrHwcRecti dvrHwcFrameGetLayerDisplayFrame(DvrHwcFrame* frame,
                                            size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return describing the portion of the layer that will fill the display
// frame. Will not exceed the layer dimensions.
DvrHwcRectf dvrHwcFrameGetLayerCrop(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The blend mode of the layer.
DvrHwcBlendMode dvrHwcFrameGetLayerBlendMode(DvrHwcFrame* frame,
                                             size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The alpha value to be applied to the whole layer. Will be in the
// [0.0, 1.0] range.
float dvrHwcFrameGetLayerAlpha(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The type of the layer assigned by the window manager.
uint32_t dvrHwcFrameGetLayerType(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The application id the layer belongs to.
uint32_t dvrHwcFrameGetLayerApplicationId(DvrHwcFrame* frame,
                                          size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The z-order for the layer.
uint32_t dvrHwcFrameGetLayerZOrder(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @param out_x Output parameter for the x coordinate of the cursor location.
// @param out_y Output parameter for the y coordinate of the cursor location.
void dvrHwcFrameGetLayerCursor(DvrHwcFrame* frame, size_t layer_index,
                               int32_t* out_x, int32_t* out_y);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The transformation that needs to be applied to the layer before
// presenting it. See DVR_HWC_TRANSFORM_* for valid values.
uint32_t dvrHwcFrameGetLayerTransform(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The dataspace which represents how the pixel values should be
// interpreted. See HAL_DATASPACE_* for valid values.
uint32_t dvrHwcFrameGetLayerDataspace(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The color of the layer if layer composition is SOLID_COLOR.
uint32_t dvrHwcFrameGetLayerColor(DvrHwcFrame* frame, size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The number of visible regions.
uint32_t dvrHwcFrameGetLayerNumVisibleRegions(DvrHwcFrame* frame,
                                              size_t layer_index);
// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @param index The index of the visible region for the layer.
// @return The rectangle describing the visible region.
DvrHwcRecti dvrHwcFrameGetLayerVisibleRegion(DvrHwcFrame* frame,
                                             size_t layer_index, size_t index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @return The number of damanged regions.
uint32_t dvrHwcFrameGetLayerNumDamagedRegions(DvrHwcFrame* frame,
                                              size_t layer_index);

// @param frame Pointer for the valid frame used for the query.
// @param layer_index The index of the layer in the frame.
// @param index The index of the damanged region for the layer.
// @return The rectangle describing the damaged region.
DvrHwcRecti dvrHwcFrameGetLayerDamagedRegion(DvrHwcFrame* frame,
                                             size_t layer_index, size_t index);
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_CLIENT_H
