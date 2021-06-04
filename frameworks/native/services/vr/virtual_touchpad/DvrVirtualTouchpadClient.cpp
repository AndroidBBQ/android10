#include "VirtualTouchpadClient.h"
#include "dvr/virtual_touchpad_client.h"

struct DvrVirtualTouchpad {};

#ifdef __cplusplus
extern "C" {
#endif

namespace {
android::dvr::VirtualTouchpad* FromC(DvrVirtualTouchpad* client) {
  return reinterpret_cast<android::dvr::VirtualTouchpad*>(client);
}
}  // namespace

DvrVirtualTouchpad* dvrVirtualTouchpadCreate() {
  return reinterpret_cast<DvrVirtualTouchpad*>(
      android::dvr::VirtualTouchpadClient::Create().release());
}

void dvrVirtualTouchpadDestroy(DvrVirtualTouchpad* client) {
  delete FromC(client);
}

int dvrVirtualTouchpadAttach(DvrVirtualTouchpad* client) {
  return FromC(client)->Attach();
}

int dvrVirtualTouchpadDetach(DvrVirtualTouchpad* client) {
  return FromC(client)->Detach();
}

int dvrVirtualTouchpadTouch(DvrVirtualTouchpad* client, int touchpad, float x,
                            float y, float pressure) {
  return FromC(client)->Touch(touchpad, x, y, pressure);
}

int dvrVirtualTouchpadButtonState(DvrVirtualTouchpad* client, int touchpad,
                                  int buttons) {
  return FromC(client)->ButtonState(touchpad, buttons);
}

int dvrVirtualTouchpadScroll(DvrVirtualTouchpad* client, int touchpad, float x,
                             float y) {
  return FromC(client)->Scroll(touchpad, x, y);
}

#ifdef __cplusplus
}  // extern "C"
#endif
