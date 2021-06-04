#include "RenderArea.h"

namespace android {

float RenderArea::getCaptureFillValue(CaptureFill captureFill) {
    switch(captureFill) {
        case CaptureFill::CLEAR:
            return 0.0f;
        case CaptureFill::OPAQUE:
        default:
            return 1.0f;
    }
}

} // namespace android
