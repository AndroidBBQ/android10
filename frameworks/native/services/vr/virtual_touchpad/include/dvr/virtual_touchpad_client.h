#ifndef ANDROID_DVR_VIRTUAL_TOUCHPAD_C_CLIENT_H
#define ANDROID_DVR_VIRTUAL_TOUCHPAD_C_CLIENT_H

#include <dvr/dvr_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DvrVirtualTouchpad DvrVirtualTouchpad;

// Creates a new virtual touchpad client.
//
// @return Pointer to the created virtual touchpad client; nullptr on failure.
//
DvrVirtualTouchpad* dvrVirtualTouchpadCreate();

// Destroys a virtual touchpad client.
//
// @param client Pointer to the virtual touchpad client to be destroyed.
//
void dvrVirtualTouchpadDestroy(DvrVirtualTouchpad* client);

// Initialize the virtual touchpad.
//
// In the current server implementation, attachment creates and configures
// the kernel virtual touchpad device(s). A single client may be attached
// and detached repeatedly, e.g. on entering and leaving VR mode.
//
// @param client Pointer to the virtual touchpad client to be attached.
// @return Zero on success, status_t-style error code on failure.
//
int dvrVirtualTouchpadAttach(DvrVirtualTouchpad* client);

// Shut down the virtual touchpad.
//
// @param client Pointer to the virtual touchpad client to be detached.
// @return Zero on success, status_t-style error code on failure.
//
int dvrVirtualTouchpadDetach(DvrVirtualTouchpad* client);

// Generate a simulated touch event.
//
// @param client Pointer to the virtual touchpad client.
// @param touchpad Selects touchpad.
// @param x Horizontal touch position.
// @param y Vertical touch position.
// @param pressure Touch pressure; use 0.0 for no touch (lift or hover).
// @return Zero on success, status_t-style error code on failure.
//
int dvrVirtualTouchpadTouch(DvrVirtualTouchpad* client, int touchpad, float x,
                            float y, float pressure);

// Generate a simulated touchpad button state event.
//
// @param client Pointer to the virtual touchpad client.
// @param touchpad Selects touchpad.
// @param buttons A union of MotionEvent BUTTON_* values.
// @return Zero on success, status_t-style error code on failure.
//
int dvrVirtualTouchpadButtonState(DvrVirtualTouchpad* client, int touchpad,
                                  int buttons);

// Generate a simulated scroll event.
//
// @param client Pointer to the virtual touchpad client.
// @param touchpad Selects touchpad.
// @param x Horizontal scroll increment.
// @param y Vertical scroll increment.
// @return Zero on success, status_t-style error code on failure.
//
int dvrVirtualTouchpadScroll(DvrVirtualTouchpad* client, int touchpad, float x,
                             float y);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_DVR_VIRTUAL_TOUCHPAD_CLIENT_H
