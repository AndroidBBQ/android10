package android.dvr;

import android.dvr.IVrComposerCallback;

/**
 * Service interface exposed by VR HWC exposed to system apps which allows one
 * system app to connect to get SurfaceFlinger's outputs (all displays). This
 * is active when SurfaceFlinger is in VR mode, where all 2D output is
 * redirected to VR HWC.
 *
 * @hide */
interface IVrComposer
{
  const String SERVICE_NAME = "vr_hwc";

  /**
   * Registers a callback used to receive frame notifications.
   */
  void registerObserver(in IVrComposerCallback callback);

  /**
   * Clears a previously registered frame notification callback.
   */
  void clearObserver();
}
