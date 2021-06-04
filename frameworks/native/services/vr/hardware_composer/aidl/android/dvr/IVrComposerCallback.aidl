package android.dvr;

import android.dvr.ParcelableComposerFrame;
import android.dvr.ParcelableUniqueFd;

/**
 * A system app will implement and register this callback with VRComposer
 * to receive the layers SurfaceFlinger presented when in VR mode.
 *
 * @hide */
interface IVrComposerCallback {
  /**
   * Called by the VR HWC service when a new frame is ready to be presented.
   *
   * @param frame The new frame VR HWC wants to present.
   * @return A fence FD used to signal when the previous frame is no longer
   * used by the client. This may be an invalid fence (-1) if the client is not
   * using the previous frame, in which case the previous frame may be re-used
   * at any point in time.
   */
  ParcelableUniqueFd onNewFrame(in ParcelableComposerFrame frame);
}
