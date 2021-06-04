/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * System private interface to the window manager.
 *
 * {@hide}
 */
public interface IWindowManager extends android.os.IInterface
{
  /** Default implementation for IWindowManager. */
  public static class Default implements android.view.IWindowManager
  {
    /**
         * ===== NOTICE =====
         * The first three methods must remain the first three methods. Scripts
         * and tools rely on their transaction number to work properly.
         */// This is used for debugging

    @Override public boolean startViewServer(int port) throws android.os.RemoteException
    {
      return false;
    }
    // Transaction #1

    @Override public boolean stopViewServer() throws android.os.RemoteException
    {
      return false;
    }
    // Transaction #2

    @Override public boolean isViewServerRunning() throws android.os.RemoteException
    {
      return false;
    }
    // Transaction #3

    @Override public android.view.IWindowSession openSession(android.view.IWindowSessionCallback callback) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void getInitialDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException
    {
    }
    @Override public void getBaseDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException
    {
    }
    @Override public void setForcedDisplaySize(int displayId, int width, int height) throws android.os.RemoteException
    {
    }
    @Override public void clearForcedDisplaySize(int displayId) throws android.os.RemoteException
    {
    }
    @Override public int getInitialDisplayDensity(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getBaseDisplayDensity(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setForcedDisplayDensityForUser(int displayId, int density, int userId) throws android.os.RemoteException
    {
    }
    @Override public void clearForcedDisplayDensityForUser(int displayId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setForcedDisplayScalingMode(int displayId, int mode) throws android.os.RemoteException
    {
    }
    // 0 = auto, 1 = disable

    @Override public void setOverscan(int displayId, int left, int top, int right, int bottom) throws android.os.RemoteException
    {
    }
    // These can only be called when holding the MANAGE_APP_TOKENS permission.

    @Override public void setEventDispatching(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void addWindowToken(android.os.IBinder token, int type, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void removeWindowToken(android.os.IBinder token, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void prepareAppTransition(int transit, boolean alwaysKeepCurrent) throws android.os.RemoteException
    {
    }
    /**
         * Like overridePendingAppTransitionMultiThumb, but uses a future to supply the specs. This is
         * used for recents, where generating the thumbnails of the specs takes a non-trivial amount of
         * time, so we want to move that off the critical path for starting the new activity.
         */
    @Override public void overridePendingAppTransitionMultiThumbFuture(android.view.IAppTransitionAnimationSpecsFuture specsFuture, android.os.IRemoteCallback startedCallback, boolean scaleUp, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void overridePendingAppTransitionRemote(android.view.RemoteAnimationAdapter remoteAnimationAdapter, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void executeAppTransition() throws android.os.RemoteException
    {
    }
    /**
          * Used by system ui to report that recents has shown itself.
          * @deprecated to be removed once prebuilts are updated
          */
    @Override public void endProlongedAnimations() throws android.os.RemoteException
    {
    }
    @Override public void startFreezingScreen(int exitAnim, int enterAnim) throws android.os.RemoteException
    {
    }
    @Override public void stopFreezingScreen() throws android.os.RemoteException
    {
    }
    // these require DISABLE_KEYGUARD permission
    /** @deprecated use Activity.setShowWhenLocked instead. */
    @Override public void disableKeyguard(android.os.IBinder token, java.lang.String tag, int userId) throws android.os.RemoteException
    {
    }
    /** @deprecated use Activity.setShowWhenLocked instead. */
    @Override public void reenableKeyguard(android.os.IBinder token, int userId) throws android.os.RemoteException
    {
    }
    @Override public void exitKeyguardSecurely(android.view.IOnKeyguardExitResult callback) throws android.os.RemoteException
    {
    }
    @Override public boolean isKeyguardLocked() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isKeyguardSecure(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void dismissKeyguard(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    // Requires INTERACT_ACROSS_USERS_FULL permission

    @Override public void setSwitchingUser(boolean switching) throws android.os.RemoteException
    {
    }
    @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
    {
    }
    // These can only be called with the SET_ANIMATON_SCALE permission.

    @Override public float getAnimationScale(int which) throws android.os.RemoteException
    {
      return 0.0f;
    }
    @Override public float[] getAnimationScales() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setAnimationScale(int which, float scale) throws android.os.RemoteException
    {
    }
    @Override public void setAnimationScales(float[] scales) throws android.os.RemoteException
    {
    }
    @Override public float getCurrentAnimatorScale() throws android.os.RemoteException
    {
      return 0.0f;
    }
    // For testing

    @Override public void setInTouchMode(boolean showFocus) throws android.os.RemoteException
    {
    }
    // For StrictMode flashing a red border on violations from the UI
    // thread.  The uid/pid is implicit from the Binder call, and the Window
    // Manager uses that to determine whether or not the red border should
    // actually be shown.  (it will be ignored that pid doesn't have windows
    // on screen)

    @Override public void showStrictModeViolation(boolean on) throws android.os.RemoteException
    {
    }
    // Proxy to set the system property for whether the flashing
    // should be enabled.  The 'enabled' value is null or blank for
    // the system default (differs per build variant) or any valid
    // boolean string as parsed by SystemProperties.getBoolean().

    @Override public void setStrictModeVisualIndicatorPreference(java.lang.String enabled) throws android.os.RemoteException
    {
    }
    /**
         * Set whether screen capture is disabled for all windows of a specific user from
         * the device policy cache.
         */
    @Override public void refreshScreenCaptureDisabled(int userId) throws android.os.RemoteException
    {
    }
    // These can only be called with the SET_ORIENTATION permission.
    /**
         * Update the current screen rotation based on the current state of
         * the world.
         * @param alwaysSendConfiguration Flag to force a new configuration to
         * be evaluated.  This can be used when there are other parameters in
         * configuration that are changing.
         * @param forceRelayout If true, the window manager will always do a relayout
         * of its windows even if the rotation hasn't changed.
         */
    @Override public void updateRotation(boolean alwaysSendConfiguration, boolean forceRelayout) throws android.os.RemoteException
    {
    }
    /**
         * Retrieve the current orientation of the primary screen.
         * @return Constant as per {@link android.view.Surface.Rotation}.
         *
         * @see android.view.Display#DEFAULT_DISPLAY
         */
    @Override public int getDefaultDisplayRotation() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Watch the rotation of the specified screen.  Returns the current rotation,
         * calls back when it changes.
         */
    @Override public int watchRotation(android.view.IRotationWatcher watcher, int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Remove a rotation watcher set using watchRotation.
         * @hide
         */
    @Override public void removeRotationWatcher(android.view.IRotationWatcher watcher) throws android.os.RemoteException
    {
    }
    /**
         * Determine the preferred edge of the screen to pin the compact options menu against.
         *
         * @param displayId Id of the display where the menu window currently resides.
         * @return a Gravity value for the options menu panel.
         * @hide
         */
    @Override public int getPreferredOptionsPanelGravity(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Equivalent to calling {@link #freezeDisplayRotation(int, int)} with {@link
         * android.view.Display#DEFAULT_DISPLAY} and given rotation.
         */
    @Override public void freezeRotation(int rotation) throws android.os.RemoteException
    {
    }
    /**
         * Equivalent to calling {@link #thawDisplayRotation(int)} with {@link
         * android.view.Display#DEFAULT_DISPLAY}.
         */
    @Override public void thawRotation() throws android.os.RemoteException
    {
    }
    /**
         * Equivelant to call {@link #isDisplayRotationFrozen(int)} with {@link
         * android.view.Display#DEFAULT_DISPLAY}.
         */
    @Override public boolean isRotationFrozen() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Lock the display orientation to the specified rotation, or to the current
         * rotation if -1. Sensor input will be ignored until thawRotation() is called.
         *
         * @param displayId the ID of display which rotation should be frozen.
         * @param rotation one of {@link android.view.Surface#ROTATION_0},
         *        {@link android.view.Surface#ROTATION_90}, {@link android.view.Surface#ROTATION_180},
         *        {@link android.view.Surface#ROTATION_270} or -1 to freeze it to current rotation.
         * @hide
         */
    @Override public void freezeDisplayRotation(int displayId, int rotation) throws android.os.RemoteException
    {
    }
    /**
         * Release the orientation lock imposed by freezeRotation() on the display.
         *
         * @param displayId the ID of display which rotation should be thawed.
         * @hide
         */
    @Override public void thawDisplayRotation(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Gets whether the rotation is frozen on the display.
         *
         * @param displayId the ID of display which frozen is needed.
         * @return Whether the rotation is frozen.
         */
    @Override public boolean isDisplayRotationFrozen(int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Screenshot the current wallpaper layer, including the whole screen.
         */
    @Override public android.graphics.Bitmap screenshotWallpaper() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Registers a wallpaper visibility listener.
         * @return Current visibility.
         */
    @Override public boolean registerWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Remove a visibility watcher that was added using registerWallpaperVisibilityListener.
         */
    @Override public void unregisterWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Registers a system gesture exclusion listener for a given display.
         */
    @Override public void registerSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Unregisters a system gesture exclusion listener for a given display.
         */
    @Override public void unregisterSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Used only for assist -- request a screenshot of the current application.
         */
    @Override public boolean requestAssistScreenshot(android.app.IAssistDataReceiver receiver) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Called by the status bar to notify Views of changes to System UI visiblity.
         */
    @Override public void statusBarVisibilityChanged(int displayId, int visibility) throws android.os.RemoteException
    {
    }
    /**
        * When set to {@code true} the system bars will always be shown. This is true even if an app
        * requests to be fullscreen by setting the system ui visibility flags. The
        * functionality was added for the automotive case as a way to guarantee required content stays
        * on screen at all times.
        *
        * @hide
        */
    @Override public void setForceShowSystemBars(boolean show) throws android.os.RemoteException
    {
    }
    /**
         * Called by System UI to notify of changes to the visibility of Recents.
         */
    @Override public void setRecentsVisibility(boolean visible) throws android.os.RemoteException
    {
    }
    /**
         * Called by System UI to notify of changes to the visibility of PIP.
         */
    @Override public void setPipVisibility(boolean visible) throws android.os.RemoteException
    {
    }
    /**
         * Called by System UI to notify of changes to the visibility and height of the shelf.
         */
    @Override public void setShelfHeight(boolean visible, int shelfHeight) throws android.os.RemoteException
    {
    }
    /**
         * Called by System UI to enable or disable haptic feedback on the navigation bar buttons.
         */
    @Override public void setNavBarVirtualKeyHapticFeedbackEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    /**
         * Device has a software navigation bar (separate from the status bar) on specific display.
         *
         * @param displayId the id of display to check if there is a software navigation bar.
         */
    @Override public boolean hasNavigationBar(int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the position of the nav bar
         */
    @Override public int getNavBarPosition(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Lock the device immediately with the specified options (can be null).
         */
    @Override public void lockNow(android.os.Bundle options) throws android.os.RemoteException
    {
    }
    /**
         * Device is in safe mode.
         */
    @Override public boolean isSafeModeEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Enables the screen if all conditions are met.
         */
    @Override public void enableScreenIfNeeded() throws android.os.RemoteException
    {
    }
    /**
         * Clears the frame statistics for a given window.
         *
         * @param token The window token.
         * @return Whether the frame statistics were cleared.
         */
    @Override public boolean clearWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Gets the content frame statistics for a given window.
         *
         * @param token The window token.
         * @return The frame statistics or null if the window does not exist.
         */
    @Override public android.view.WindowContentFrameStats getWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @return the dock side the current docked stack is at; must be one of the
         *         WindowManagerGlobal.DOCKED_* values
         */
    @Override public int getDockedStackSide() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the region the user can touch the divider. This region will be excluded from the region
         * which is used to cause a focus switch when dispatching touch.
         */
    @Override public void setDockedStackDividerTouchRegion(android.graphics.Rect touchableRegion) throws android.os.RemoteException
    {
    }
    /**
         * Registers a listener that will be called when the dock divider changes its visibility or when
         * the docked stack gets added/removed.
         */
    @Override public void registerDockedStackListener(android.view.IDockedStackListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Registers a listener that will be called when the pinned stack state changes.
         */
    @Override public void registerPinnedStackListener(int displayId, android.view.IPinnedStackListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Updates the dim layer used while resizing.
         *
         * @param visible Whether the dim layer should be visible.
         * @param targetWindowingMode The windowing mode of the stack the dim layer should be placed on.
         * @param alpha The translucency of the dim layer, between 0 and 1.
         */
    @Override public void setResizeDimLayer(boolean visible, int targetWindowingMode, float alpha) throws android.os.RemoteException
    {
    }
    /**
         * Requests Keyboard Shortcuts from the displayed window.
         *
         * @param receiver The receiver to deliver the results to.
         */
    @Override public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException
    {
    }
    /**
         * Retrieves the current stable insets from the primary display.
         */
    @Override public void getStableInsets(int displayId, android.graphics.Rect outInsets) throws android.os.RemoteException
    {
    }
    /**
         * Set the forwarded insets on the display.
         * <p>
         * This is only used in case a virtual display is displayed on another display that has insets,
         * and the bounds of the virtual display is overlapping with the insets from the host display.
         * In that case, the contents on the virtual display won't be placed over the forwarded insets.
         * Only the owner of the display is permitted to set the forwarded insets on it.
         */
    @Override public void setForwardedInsets(int displayId, android.graphics.Insets insets) throws android.os.RemoteException
    {
    }
    /**
         * Register shortcut key. Shortcut code is packed as:
         * (MetaState << Integer.SIZE) | KeyCode
         * @hide
         */
    @Override public void registerShortcutKey(long shortcutCode, com.android.internal.policy.IShortcutService keySubscriber) throws android.os.RemoteException
    {
    }
    /**
         * Create an input consumer by name and display id.
         */
    @Override public void createInputConsumer(android.os.IBinder token, java.lang.String name, int displayId, android.view.InputChannel inputChannel) throws android.os.RemoteException
    {
    }
    /**
         * Destroy an input consumer by name and display id.
         * This method will also dispose the input channels associated with that InputConsumer.
         */
    @Override public boolean destroyInputConsumer(java.lang.String name, int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return the touch region for the current IME window, or an empty region if there is none.
         */
    @Override public android.graphics.Region getCurrentImeTouchRegion() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Registers an IDisplayFoldListener.
         */
    @Override public void registerDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Unregisters an IDisplayFoldListener.
         */
    @Override public void unregisterDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Starts a window trace.
         */
    @Override public void startWindowTrace() throws android.os.RemoteException
    {
    }
    /**
         * Stops a window trace.
         */
    @Override public void stopWindowTrace() throws android.os.RemoteException
    {
    }
    /**
         * Returns true if window trace is enabled.
         */
    @Override public boolean isWindowTraceEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Requests that the WindowManager sends
         * WindowManagerPolicyConstants#ACTION_USER_ACTIVITY_NOTIFICATION on the next user activity.
         */
    @Override public void requestUserActivityNotification() throws android.os.RemoteException
    {
    }
    /**
         * Notify WindowManager that it should not override the info in DisplayManager for the specified
         * display. This can disable letter- or pillar-boxing applied in DisplayManager when the metrics
         * of the logical display reported from WindowManager do not correspond to the metrics of the
         * physical display it is based on.
         *
         * @param displayId The id of the display.
         */
    @Override public void dontOverrideDisplayInfo(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Gets the windowing mode of the display.
         *
         * @param displayId The id of the display.
         * @return {@link WindowConfiguration.WindowingMode}
         */
    @Override public int getWindowingMode(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the windowing mode of the display.
         *
         * @param displayId The id of the display.
         * @param mode {@link WindowConfiguration.WindowingMode}
         */
    @Override public void setWindowingMode(int displayId, int mode) throws android.os.RemoteException
    {
    }
    /**
         * Gets current remove content mode of the display.
         * <p>
         * What actions should be performed with the display's content when it is removed. Default
         * behavior for public displays in this case is to move all activities to the primary display
         * and make it focused. For private display is to destroy all activities.
         * </p>
         *
         * @param displayId The id of the display.
         * @return The remove content mode of the display.
         * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
         * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
         */
    @Override public int getRemoveContentMode(int displayId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the remove content mode of the display.
         * <p>
         * This mode indicates what actions should be performed with the display's content when it is
         * removed.
         * </p>
         *
         * @param displayId The id of the display.
         * @param mode Remove content mode.
         * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
         * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
         */
    @Override public void setRemoveContentMode(int displayId, int mode) throws android.os.RemoteException
    {
    }
    /**
         * Indicates that the display should show its content when non-secure keyguard is shown.
         * <p>
         * This flag identifies secondary displays that will continue showing content if keyguard can be
         * dismissed without entering credentials.
         * </p><p>
         * An example of usage is a virtual display which content is displayed on external hardware
         * display that is not visible to the system directly.
         * </p>
         *
         * @param displayId The id of the display.
         * @return {@code true} if the display should show its content when non-secure keyguard is
         *         shown.
         * @see KeyguardManager#isDeviceSecure()
         * @see KeyguardManager#isDeviceLocked()
         */
    @Override public boolean shouldShowWithInsecureKeyguard(int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets that the display should show its content when non-secure keyguard is shown.
         *
         * @param displayId The id of the display.
         * @param shouldShow Indicates that the display should show its content when non-secure keyguard
         *                  is shown.
         * @see KeyguardManager#isDeviceSecure()
         * @see KeyguardManager#isDeviceLocked()
         */
    @Override public void setShouldShowWithInsecureKeyguard(int displayId, boolean shouldShow) throws android.os.RemoteException
    {
    }
    /**
         * Indicates the display should show system decors.
         * <p>
         * System decors include status bar, navigation bar, launcher.
         * </p>
         *
         * @param displayId The id of the display.
         * @return {@code true} if the display should show system decors.
         */
    @Override public boolean shouldShowSystemDecors(int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets that the display should show system decors.
         * <p>
         * System decors include status bar, navigation bar, launcher.
         * </p>
         *
         * @param displayId The id of the display.
         * @param shouldShow Indicates that the display should show system decors.
         */
    @Override public void setShouldShowSystemDecors(int displayId, boolean shouldShow) throws android.os.RemoteException
    {
    }
    /**
         * Indicates that the display should show IME.
         *
         * @param displayId The id of the display.
         * @return {@code true} if the display should show IME.
         * @see KeyguardManager#isDeviceSecure()
         * @see KeyguardManager#isDeviceLocked()
         */
    @Override public boolean shouldShowIme(int displayId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets that the display should show IME.
         *
         * @param displayId The id of the display.
         * @param shouldShow Indicates that the display should show IME.
         * @see KeyguardManager#isDeviceSecure()
         * @see KeyguardManager#isDeviceLocked()
         */
    @Override public void setShouldShowIme(int displayId, boolean shouldShow) throws android.os.RemoteException
    {
    }
    /**
         * Waits for transactions to get applied before injecting input.
         * This includes waiting for the input windows to get sent to InputManager.
         *
         * This is needed for testing since the system add windows and injects input
         * quick enough that the windows don't have time to get sent to InputManager.
         */
    @Override public boolean injectInputAfterTransactionsApplied(android.view.InputEvent ev, int mode) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Waits until all animations have completed and input information has been sent from
         * WindowManager to native InputManager.
         *
         * This is needed for testing since we need to ensure input information has been propagated to
         * native InputManager before proceeding with tests.
         */
    @Override public void syncInputTransactions() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IWindowManager
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IWindowManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IWindowManager interface,
     * generating a proxy if needed.
     */
    public static android.view.IWindowManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IWindowManager))) {
        return ((android.view.IWindowManager)iin);
      }
      return new android.view.IWindowManager.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_startViewServer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.startViewServer(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopViewServer:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.stopViewServer();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isViewServerRunning:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isViewServerRunning();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_openSession:
        {
          data.enforceInterface(descriptor);
          android.view.IWindowSessionCallback _arg0;
          _arg0 = android.view.IWindowSessionCallback.Stub.asInterface(data.readStrongBinder());
          android.view.IWindowSession _result = this.openSession(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getInitialDisplaySize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Point _arg1;
          _arg1 = new android.graphics.Point();
          this.getInitialDisplaySize(_arg0, _arg1);
          reply.writeNoException();
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getBaseDisplaySize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Point _arg1;
          _arg1 = new android.graphics.Point();
          this.getBaseDisplaySize(_arg0, _arg1);
          reply.writeNoException();
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_setForcedDisplaySize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setForcedDisplaySize(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearForcedDisplaySize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.clearForcedDisplaySize(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInitialDisplayDensity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getInitialDisplayDensity(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getBaseDisplayDensity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getBaseDisplayDensity(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setForcedDisplayDensityForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setForcedDisplayDensityForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearForcedDisplayDensityForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.clearForcedDisplayDensityForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setForcedDisplayScalingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setForcedDisplayScalingMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setOverscan:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.setOverscan(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setEventDispatching:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setEventDispatching(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addWindowToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.addWindowToken(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeWindowToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.removeWindowToken(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepareAppTransition:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.prepareAppTransition(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_overridePendingAppTransitionMultiThumbFuture:
        {
          data.enforceInterface(descriptor);
          android.view.IAppTransitionAnimationSpecsFuture _arg0;
          _arg0 = android.view.IAppTransitionAnimationSpecsFuture.Stub.asInterface(data.readStrongBinder());
          android.os.IRemoteCallback _arg1;
          _arg1 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.overridePendingAppTransitionMultiThumbFuture(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_overridePendingAppTransitionRemote:
        {
          data.enforceInterface(descriptor);
          android.view.RemoteAnimationAdapter _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.RemoteAnimationAdapter.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.overridePendingAppTransitionRemote(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_executeAppTransition:
        {
          data.enforceInterface(descriptor);
          this.executeAppTransition();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_endProlongedAnimations:
        {
          data.enforceInterface(descriptor);
          this.endProlongedAnimations();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startFreezingScreen:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.startFreezingScreen(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopFreezingScreen:
        {
          data.enforceInterface(descriptor);
          this.stopFreezingScreen();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableKeyguard:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.disableKeyguard(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reenableKeyguard:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.reenableKeyguard(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_exitKeyguardSecurely:
        {
          data.enforceInterface(descriptor);
          android.view.IOnKeyguardExitResult _arg0;
          _arg0 = android.view.IOnKeyguardExitResult.Stub.asInterface(data.readStrongBinder());
          this.exitKeyguardSecurely(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isKeyguardLocked:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isKeyguardLocked();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isKeyguardSecure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isKeyguardSecure(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_dismissKeyguard:
        {
          data.enforceInterface(descriptor);
          com.android.internal.policy.IKeyguardDismissCallback _arg0;
          _arg0 = com.android.internal.policy.IKeyguardDismissCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.CharSequence _arg1;
          if (0!=data.readInt()) {
            _arg1 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.dismissKeyguard(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSwitchingUser:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSwitchingUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeSystemDialogs:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.closeSystemDialogs(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAnimationScale:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _result = this.getAnimationScale(_arg0);
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_getAnimationScales:
        {
          data.enforceInterface(descriptor);
          float[] _result = this.getAnimationScales();
          reply.writeNoException();
          reply.writeFloatArray(_result);
          return true;
        }
        case TRANSACTION_setAnimationScale:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _arg1;
          _arg1 = data.readFloat();
          this.setAnimationScale(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAnimationScales:
        {
          data.enforceInterface(descriptor);
          float[] _arg0;
          _arg0 = data.createFloatArray();
          this.setAnimationScales(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentAnimatorScale:
        {
          data.enforceInterface(descriptor);
          float _result = this.getCurrentAnimatorScale();
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_setInTouchMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setInTouchMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showStrictModeViolation:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.showStrictModeViolation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setStrictModeVisualIndicatorPreference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setStrictModeVisualIndicatorPreference(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_refreshScreenCaptureDisabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.refreshScreenCaptureDisabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateRotation:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.updateRotation(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDefaultDisplayRotation:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDefaultDisplayRotation();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_watchRotation:
        {
          data.enforceInterface(descriptor);
          android.view.IRotationWatcher _arg0;
          _arg0 = android.view.IRotationWatcher.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.watchRotation(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeRotationWatcher:
        {
          data.enforceInterface(descriptor);
          android.view.IRotationWatcher _arg0;
          _arg0 = android.view.IRotationWatcher.Stub.asInterface(data.readStrongBinder());
          this.removeRotationWatcher(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPreferredOptionsPanelGravity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getPreferredOptionsPanelGravity(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_freezeRotation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.freezeRotation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_thawRotation:
        {
          data.enforceInterface(descriptor);
          this.thawRotation();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isRotationFrozen:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isRotationFrozen();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_freezeDisplayRotation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.freezeDisplayRotation(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_thawDisplayRotation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.thawDisplayRotation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDisplayRotationFrozen:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDisplayRotationFrozen(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_screenshotWallpaper:
        {
          data.enforceInterface(descriptor);
          android.graphics.Bitmap _result = this.screenshotWallpaper();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_registerWallpaperVisibilityListener:
        {
          data.enforceInterface(descriptor);
          android.view.IWallpaperVisibilityListener _arg0;
          _arg0 = android.view.IWallpaperVisibilityListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.registerWallpaperVisibilityListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterWallpaperVisibilityListener:
        {
          data.enforceInterface(descriptor);
          android.view.IWallpaperVisibilityListener _arg0;
          _arg0 = android.view.IWallpaperVisibilityListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.unregisterWallpaperVisibilityListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerSystemGestureExclusionListener:
        {
          data.enforceInterface(descriptor);
          android.view.ISystemGestureExclusionListener _arg0;
          _arg0 = android.view.ISystemGestureExclusionListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.registerSystemGestureExclusionListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterSystemGestureExclusionListener:
        {
          data.enforceInterface(descriptor);
          android.view.ISystemGestureExclusionListener _arg0;
          _arg0 = android.view.ISystemGestureExclusionListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.unregisterSystemGestureExclusionListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestAssistScreenshot:
        {
          data.enforceInterface(descriptor);
          android.app.IAssistDataReceiver _arg0;
          _arg0 = android.app.IAssistDataReceiver.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.requestAssistScreenshot(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_statusBarVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.statusBarVisibilityChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setForceShowSystemBars:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setForceShowSystemBars(_arg0);
          return true;
        }
        case TRANSACTION_setRecentsVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setRecentsVisibility(_arg0);
          return true;
        }
        case TRANSACTION_setPipVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setPipVisibility(_arg0);
          return true;
        }
        case TRANSACTION_setShelfHeight:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setShelfHeight(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setNavBarVirtualKeyHapticFeedbackEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setNavBarVirtualKeyHapticFeedbackEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasNavigationBar:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.hasNavigationBar(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNavBarPosition:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getNavBarPosition(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_lockNow:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.lockNow(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isSafeModeEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSafeModeEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableScreenIfNeeded:
        {
          data.enforceInterface(descriptor);
          this.enableScreenIfNeeded();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearWindowContentFrameStats:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.clearWindowContentFrameStats(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getWindowContentFrameStats:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.view.WindowContentFrameStats _result = this.getWindowContentFrameStats(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getDockedStackSide:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDockedStackSide();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDockedStackDividerTouchRegion:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setDockedStackDividerTouchRegion(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerDockedStackListener:
        {
          data.enforceInterface(descriptor);
          android.view.IDockedStackListener _arg0;
          _arg0 = android.view.IDockedStackListener.Stub.asInterface(data.readStrongBinder());
          this.registerDockedStackListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerPinnedStackListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.IPinnedStackListener _arg1;
          _arg1 = android.view.IPinnedStackListener.Stub.asInterface(data.readStrongBinder());
          this.registerPinnedStackListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setResizeDimLayer:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          float _arg2;
          _arg2 = data.readFloat();
          this.setResizeDimLayer(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestAppKeyboardShortcuts:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.requestAppKeyboardShortcuts(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getStableInsets:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Rect _arg1;
          _arg1 = new android.graphics.Rect();
          this.getStableInsets(_arg0, _arg1);
          reply.writeNoException();
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_setForwardedInsets:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Insets _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Insets.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setForwardedInsets(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerShortcutKey:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          com.android.internal.policy.IShortcutService _arg1;
          _arg1 = com.android.internal.policy.IShortcutService.Stub.asInterface(data.readStrongBinder());
          this.registerShortcutKey(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createInputConsumer:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.view.InputChannel _arg3;
          _arg3 = new android.view.InputChannel();
          this.createInputConsumer(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          if ((_arg3!=null)) {
            reply.writeInt(1);
            _arg3.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_destroyInputConsumer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.destroyInputConsumer(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentImeTouchRegion:
        {
          data.enforceInterface(descriptor);
          android.graphics.Region _result = this.getCurrentImeTouchRegion();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_registerDisplayFoldListener:
        {
          data.enforceInterface(descriptor);
          android.view.IDisplayFoldListener _arg0;
          _arg0 = android.view.IDisplayFoldListener.Stub.asInterface(data.readStrongBinder());
          this.registerDisplayFoldListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterDisplayFoldListener:
        {
          data.enforceInterface(descriptor);
          android.view.IDisplayFoldListener _arg0;
          _arg0 = android.view.IDisplayFoldListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterDisplayFoldListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWindowTrace:
        {
          data.enforceInterface(descriptor);
          this.startWindowTrace();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWindowTrace:
        {
          data.enforceInterface(descriptor);
          this.stopWindowTrace();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isWindowTraceEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isWindowTraceEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestUserActivityNotification:
        {
          data.enforceInterface(descriptor);
          this.requestUserActivityNotification();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dontOverrideDisplayInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.dontOverrideDisplayInfo(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWindowingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getWindowingMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setWindowingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setWindowingMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRemoveContentMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getRemoveContentMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setRemoveContentMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setRemoveContentMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shouldShowWithInsecureKeyguard:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.shouldShowWithInsecureKeyguard(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setShouldShowWithInsecureKeyguard:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setShouldShowWithInsecureKeyguard(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shouldShowSystemDecors:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.shouldShowSystemDecors(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setShouldShowSystemDecors:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setShouldShowSystemDecors(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shouldShowIme:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.shouldShowIme(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setShouldShowIme:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setShouldShowIme(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_injectInputAfterTransactionsApplied:
        {
          data.enforceInterface(descriptor);
          android.view.InputEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InputEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.injectInputAfterTransactionsApplied(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_syncInputTransactions:
        {
          data.enforceInterface(descriptor);
          this.syncInputTransactions();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IWindowManager
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      /**
           * ===== NOTICE =====
           * The first three methods must remain the first three methods. Scripts
           * and tools rely on their transaction number to work properly.
           */// This is used for debugging

      @Override public boolean startViewServer(int port) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(port);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startViewServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startViewServer(port);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Transaction #1

      @Override public boolean stopViewServer() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopViewServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopViewServer();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Transaction #2

      @Override public boolean isViewServerRunning() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isViewServerRunning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isViewServerRunning();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Transaction #3

      @Override public android.view.IWindowSession openSession(android.view.IWindowSessionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.IWindowSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openSession(callback);
          }
          _reply.readException();
          _result = android.view.IWindowSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void getInitialDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInitialDisplaySize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getInitialDisplaySize(displayId, size);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            size.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getBaseDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBaseDisplaySize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getBaseDisplaySize(displayId, size);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            size.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setForcedDisplaySize(int displayId, int width, int height) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(width);
          _data.writeInt(height);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForcedDisplaySize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForcedDisplaySize(displayId, width, height);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearForcedDisplaySize(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearForcedDisplaySize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearForcedDisplaySize(displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getInitialDisplayDensity(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInitialDisplayDensity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInitialDisplayDensity(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getBaseDisplayDensity(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBaseDisplayDensity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBaseDisplayDensity(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setForcedDisplayDensityForUser(int displayId, int density, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(density);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForcedDisplayDensityForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForcedDisplayDensityForUser(displayId, density, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearForcedDisplayDensityForUser(int displayId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearForcedDisplayDensityForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearForcedDisplayDensityForUser(displayId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setForcedDisplayScalingMode(int displayId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForcedDisplayScalingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForcedDisplayScalingMode(displayId, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // 0 = auto, 1 = disable

      @Override public void setOverscan(int displayId, int left, int top, int right, int bottom) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(left);
          _data.writeInt(top);
          _data.writeInt(right);
          _data.writeInt(bottom);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOverscan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOverscan(displayId, left, top, right, bottom);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These can only be called when holding the MANAGE_APP_TOKENS permission.

      @Override public void setEventDispatching(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEventDispatching, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setEventDispatching(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addWindowToken(android.os.IBinder token, int type, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(type);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addWindowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addWindowToken(token, type, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeWindowToken(android.os.IBinder token, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeWindowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeWindowToken(token, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void prepareAppTransition(int transit, boolean alwaysKeepCurrent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(transit);
          _data.writeInt(((alwaysKeepCurrent)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareAppTransition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareAppTransition(transit, alwaysKeepCurrent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Like overridePendingAppTransitionMultiThumb, but uses a future to supply the specs. This is
           * used for recents, where generating the thumbnails of the specs takes a non-trivial amount of
           * time, so we want to move that off the critical path for starting the new activity.
           */
      @Override public void overridePendingAppTransitionMultiThumbFuture(android.view.IAppTransitionAnimationSpecsFuture specsFuture, android.os.IRemoteCallback startedCallback, boolean scaleUp, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((specsFuture!=null))?(specsFuture.asBinder()):(null)));
          _data.writeStrongBinder((((startedCallback!=null))?(startedCallback.asBinder()):(null)));
          _data.writeInt(((scaleUp)?(1):(0)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_overridePendingAppTransitionMultiThumbFuture, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().overridePendingAppTransitionMultiThumbFuture(specsFuture, startedCallback, scaleUp, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void overridePendingAppTransitionRemote(android.view.RemoteAnimationAdapter remoteAnimationAdapter, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((remoteAnimationAdapter!=null)) {
            _data.writeInt(1);
            remoteAnimationAdapter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_overridePendingAppTransitionRemote, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().overridePendingAppTransitionRemote(remoteAnimationAdapter, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void executeAppTransition() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_executeAppTransition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().executeAppTransition();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
            * Used by system ui to report that recents has shown itself.
            * @deprecated to be removed once prebuilts are updated
            */
      @Override public void endProlongedAnimations() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_endProlongedAnimations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().endProlongedAnimations();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startFreezingScreen(int exitAnim, int enterAnim) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(exitAnim);
          _data.writeInt(enterAnim);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startFreezingScreen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startFreezingScreen(exitAnim, enterAnim);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopFreezingScreen() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopFreezingScreen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopFreezingScreen();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // these require DISABLE_KEYGUARD permission
      /** @deprecated use Activity.setShowWhenLocked instead. */
      @Override public void disableKeyguard(android.os.IBinder token, java.lang.String tag, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(tag);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableKeyguard(token, tag, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** @deprecated use Activity.setShowWhenLocked instead. */
      @Override public void reenableKeyguard(android.os.IBinder token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reenableKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reenableKeyguard(token, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void exitKeyguardSecurely(android.view.IOnKeyguardExitResult callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_exitKeyguardSecurely, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().exitKeyguardSecurely(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isKeyguardLocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isKeyguardLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isKeyguardLocked();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean isKeyguardSecure(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isKeyguardSecure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isKeyguardSecure(userId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void dismissKeyguard(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissKeyguard(callback, message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires INTERACT_ACROSS_USERS_FULL permission

      @Override public void setSwitchingUser(boolean switching) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((switching)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSwitchingUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSwitchingUser(switching);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSystemDialogs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSystemDialogs(reason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These can only be called with the SET_ANIMATON_SCALE permission.

      @Override public float getAnimationScale(int which) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(which);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAnimationScale, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAnimationScale(which);
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public float[] getAnimationScales() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAnimationScales, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAnimationScales();
          }
          _reply.readException();
          _result = _reply.createFloatArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setAnimationScale(int which, float scale) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(which);
          _data.writeFloat(scale);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAnimationScale, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAnimationScale(which, scale);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAnimationScales(float[] scales) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloatArray(scales);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAnimationScales, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAnimationScales(scales);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public float getCurrentAnimatorScale() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentAnimatorScale, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentAnimatorScale();
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // For testing

      @Override public void setInTouchMode(boolean showFocus) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showFocus)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInTouchMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInTouchMode(showFocus);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // For StrictMode flashing a red border on violations from the UI
      // thread.  The uid/pid is implicit from the Binder call, and the Window
      // Manager uses that to determine whether or not the red border should
      // actually be shown.  (it will be ignored that pid doesn't have windows
      // on screen)

      @Override public void showStrictModeViolation(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showStrictModeViolation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showStrictModeViolation(on);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Proxy to set the system property for whether the flashing
      // should be enabled.  The 'enabled' value is null or blank for
      // the system default (differs per build variant) or any valid
      // boolean string as parsed by SystemProperties.getBoolean().

      @Override public void setStrictModeVisualIndicatorPreference(java.lang.String enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(enabled);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStrictModeVisualIndicatorPreference, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStrictModeVisualIndicatorPreference(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set whether screen capture is disabled for all windows of a specific user from
           * the device policy cache.
           */
      @Override public void refreshScreenCaptureDisabled(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_refreshScreenCaptureDisabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().refreshScreenCaptureDisabled(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These can only be called with the SET_ORIENTATION permission.
      /**
           * Update the current screen rotation based on the current state of
           * the world.
           * @param alwaysSendConfiguration Flag to force a new configuration to
           * be evaluated.  This can be used when there are other parameters in
           * configuration that are changing.
           * @param forceRelayout If true, the window manager will always do a relayout
           * of its windows even if the rotation hasn't changed.
           */
      @Override public void updateRotation(boolean alwaysSendConfiguration, boolean forceRelayout) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((alwaysSendConfiguration)?(1):(0)));
          _data.writeInt(((forceRelayout)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateRotation(alwaysSendConfiguration, forceRelayout);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Retrieve the current orientation of the primary screen.
           * @return Constant as per {@link android.view.Surface.Rotation}.
           *
           * @see android.view.Display#DEFAULT_DISPLAY
           */
      @Override public int getDefaultDisplayRotation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultDisplayRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultDisplayRotation();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Watch the rotation of the specified screen.  Returns the current rotation,
           * calls back when it changes.
           */
      @Override public int watchRotation(android.view.IRotationWatcher watcher, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((watcher!=null))?(watcher.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_watchRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().watchRotation(watcher, displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Remove a rotation watcher set using watchRotation.
           * @hide
           */
      @Override public void removeRotationWatcher(android.view.IRotationWatcher watcher) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((watcher!=null))?(watcher.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRotationWatcher, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeRotationWatcher(watcher);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Determine the preferred edge of the screen to pin the compact options menu against.
           *
           * @param displayId Id of the display where the menu window currently resides.
           * @return a Gravity value for the options menu panel.
           * @hide
           */
      @Override public int getPreferredOptionsPanelGravity(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreferredOptionsPanelGravity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreferredOptionsPanelGravity(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Equivalent to calling {@link #freezeDisplayRotation(int, int)} with {@link
           * android.view.Display#DEFAULT_DISPLAY} and given rotation.
           */
      @Override public void freezeRotation(int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_freezeRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().freezeRotation(rotation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Equivalent to calling {@link #thawDisplayRotation(int)} with {@link
           * android.view.Display#DEFAULT_DISPLAY}.
           */
      @Override public void thawRotation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_thawRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().thawRotation();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Equivelant to call {@link #isDisplayRotationFrozen(int)} with {@link
           * android.view.Display#DEFAULT_DISPLAY}.
           */
      @Override public boolean isRotationFrozen() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRotationFrozen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRotationFrozen();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Lock the display orientation to the specified rotation, or to the current
           * rotation if -1. Sensor input will be ignored until thawRotation() is called.
           *
           * @param displayId the ID of display which rotation should be frozen.
           * @param rotation one of {@link android.view.Surface#ROTATION_0},
           *        {@link android.view.Surface#ROTATION_90}, {@link android.view.Surface#ROTATION_180},
           *        {@link android.view.Surface#ROTATION_270} or -1 to freeze it to current rotation.
           * @hide
           */
      @Override public void freezeDisplayRotation(int displayId, int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_freezeDisplayRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().freezeDisplayRotation(displayId, rotation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Release the orientation lock imposed by freezeRotation() on the display.
           *
           * @param displayId the ID of display which rotation should be thawed.
           * @hide
           */
      @Override public void thawDisplayRotation(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_thawDisplayRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().thawDisplayRotation(displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Gets whether the rotation is frozen on the display.
           *
           * @param displayId the ID of display which frozen is needed.
           * @return Whether the rotation is frozen.
           */
      @Override public boolean isDisplayRotationFrozen(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDisplayRotationFrozen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDisplayRotationFrozen(displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Screenshot the current wallpaper layer, including the whole screen.
           */
      @Override public android.graphics.Bitmap screenshotWallpaper() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Bitmap _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_screenshotWallpaper, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().screenshotWallpaper();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Registers a wallpaper visibility listener.
           * @return Current visibility.
           */
      @Override public boolean registerWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerWallpaperVisibilityListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerWallpaperVisibilityListener(listener, displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Remove a visibility watcher that was added using registerWallpaperVisibilityListener.
           */
      @Override public void unregisterWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterWallpaperVisibilityListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterWallpaperVisibilityListener(listener, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Registers a system gesture exclusion listener for a given display.
           */
      @Override public void registerSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerSystemGestureExclusionListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerSystemGestureExclusionListener(listener, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Unregisters a system gesture exclusion listener for a given display.
           */
      @Override public void unregisterSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterSystemGestureExclusionListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterSystemGestureExclusionListener(listener, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Used only for assist -- request a screenshot of the current application.
           */
      @Override public boolean requestAssistScreenshot(android.app.IAssistDataReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAssistScreenshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestAssistScreenshot(receiver);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Called by the status bar to notify Views of changes to System UI visiblity.
           */
      @Override public void statusBarVisibilityChanged(int displayId, int visibility) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(visibility);
          boolean _status = mRemote.transact(Stub.TRANSACTION_statusBarVisibilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().statusBarVisibilityChanged(displayId, visibility);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
          * When set to {@code true} the system bars will always be shown. This is true even if an app
          * requests to be fullscreen by setting the system ui visibility flags. The
          * functionality was added for the automotive case as a way to guarantee required content stays
          * on screen at all times.
          *
          * @hide
          */
      @Override public void setForceShowSystemBars(boolean show) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((show)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForceShowSystemBars, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForceShowSystemBars(show);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called by System UI to notify of changes to the visibility of Recents.
           */
      @Override public void setRecentsVisibility(boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRecentsVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRecentsVisibility(visible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called by System UI to notify of changes to the visibility of PIP.
           */
      @Override public void setPipVisibility(boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPipVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPipVisibility(visible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called by System UI to notify of changes to the visibility and height of the shelf.
           */
      @Override public void setShelfHeight(boolean visible, int shelfHeight) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          _data.writeInt(shelfHeight);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setShelfHeight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setShelfHeight(visible, shelfHeight);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Called by System UI to enable or disable haptic feedback on the navigation bar buttons.
           */
      @Override public void setNavBarVirtualKeyHapticFeedbackEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNavBarVirtualKeyHapticFeedbackEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNavBarVirtualKeyHapticFeedbackEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Device has a software navigation bar (separate from the status bar) on specific display.
           *
           * @param displayId the id of display to check if there is a software navigation bar.
           */
      @Override public boolean hasNavigationBar(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasNavigationBar, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasNavigationBar(displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the position of the nav bar
           */
      @Override public int getNavBarPosition(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNavBarPosition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNavBarPosition(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Lock the device immediately with the specified options (can be null).
           */
      @Override public void lockNow(android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_lockNow, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().lockNow(options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Device is in safe mode.
           */
      @Override public boolean isSafeModeEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSafeModeEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSafeModeEnabled();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Enables the screen if all conditions are met.
           */
      @Override public void enableScreenIfNeeded() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableScreenIfNeeded, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableScreenIfNeeded();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Clears the frame statistics for a given window.
           *
           * @param token The window token.
           * @return Whether the frame statistics were cleared.
           */
      @Override public boolean clearWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearWindowContentFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearWindowContentFrameStats(token);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Gets the content frame statistics for a given window.
           *
           * @param token The window token.
           * @return The frame statistics or null if the window does not exist.
           */
      @Override public android.view.WindowContentFrameStats getWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.WindowContentFrameStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowContentFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowContentFrameStats(token);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.WindowContentFrameStats.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @return the dock side the current docked stack is at; must be one of the
           *         WindowManagerGlobal.DOCKED_* values
           */
      @Override public int getDockedStackSide() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDockedStackSide, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDockedStackSide();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets the region the user can touch the divider. This region will be excluded from the region
           * which is used to cause a focus switch when dispatching touch.
           */
      @Override public void setDockedStackDividerTouchRegion(android.graphics.Rect touchableRegion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((touchableRegion!=null)) {
            _data.writeInt(1);
            touchableRegion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDockedStackDividerTouchRegion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDockedStackDividerTouchRegion(touchableRegion);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Registers a listener that will be called when the dock divider changes its visibility or when
           * the docked stack gets added/removed.
           */
      @Override public void registerDockedStackListener(android.view.IDockedStackListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDockedStackListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerDockedStackListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Registers a listener that will be called when the pinned stack state changes.
           */
      @Override public void registerPinnedStackListener(int displayId, android.view.IPinnedStackListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPinnedStackListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPinnedStackListener(displayId, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Updates the dim layer used while resizing.
           *
           * @param visible Whether the dim layer should be visible.
           * @param targetWindowingMode The windowing mode of the stack the dim layer should be placed on.
           * @param alpha The translucency of the dim layer, between 0 and 1.
           */
      @Override public void setResizeDimLayer(boolean visible, int targetWindowingMode, float alpha) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          _data.writeInt(targetWindowingMode);
          _data.writeFloat(alpha);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setResizeDimLayer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setResizeDimLayer(visible, targetWindowingMode, alpha);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Requests Keyboard Shortcuts from the displayed window.
           *
           * @param receiver The receiver to deliver the results to.
           */
      @Override public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAppKeyboardShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestAppKeyboardShortcuts(receiver, deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Retrieves the current stable insets from the primary display.
           */
      @Override public void getStableInsets(int displayId, android.graphics.Rect outInsets) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStableInsets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getStableInsets(displayId, outInsets);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            outInsets.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set the forwarded insets on the display.
           * <p>
           * This is only used in case a virtual display is displayed on another display that has insets,
           * and the bounds of the virtual display is overlapping with the insets from the host display.
           * In that case, the contents on the virtual display won't be placed over the forwarded insets.
           * Only the owner of the display is permitted to set the forwarded insets on it.
           */
      @Override public void setForwardedInsets(int displayId, android.graphics.Insets insets) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          if ((insets!=null)) {
            _data.writeInt(1);
            insets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForwardedInsets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForwardedInsets(displayId, insets);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Register shortcut key. Shortcut code is packed as:
           * (MetaState << Integer.SIZE) | KeyCode
           * @hide
           */
      @Override public void registerShortcutKey(long shortcutCode, com.android.internal.policy.IShortcutService keySubscriber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(shortcutCode);
          _data.writeStrongBinder((((keySubscriber!=null))?(keySubscriber.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerShortcutKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerShortcutKey(shortcutCode, keySubscriber);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Create an input consumer by name and display id.
           */
      @Override public void createInputConsumer(android.os.IBinder token, java.lang.String name, int displayId, android.view.InputChannel inputChannel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(name);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createInputConsumer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createInputConsumer(token, name, displayId, inputChannel);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            inputChannel.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Destroy an input consumer by name and display id.
           * This method will also dispose the input channels associated with that InputConsumer.
           */
      @Override public boolean destroyInputConsumer(java.lang.String name, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroyInputConsumer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().destroyInputConsumer(name, displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Return the touch region for the current IME window, or an empty region if there is none.
           */
      @Override public android.graphics.Region getCurrentImeTouchRegion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Region _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentImeTouchRegion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentImeTouchRegion();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Region.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Registers an IDisplayFoldListener.
           */
      @Override public void registerDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDisplayFoldListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerDisplayFoldListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Unregisters an IDisplayFoldListener.
           */
      @Override public void unregisterDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDisplayFoldListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDisplayFoldListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Starts a window trace.
           */
      @Override public void startWindowTrace() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWindowTrace, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWindowTrace();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Stops a window trace.
           */
      @Override public void stopWindowTrace() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWindowTrace, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWindowTrace();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns true if window trace is enabled.
           */
      @Override public boolean isWindowTraceEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isWindowTraceEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isWindowTraceEnabled();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Requests that the WindowManager sends
           * WindowManagerPolicyConstants#ACTION_USER_ACTIVITY_NOTIFICATION on the next user activity.
           */
      @Override public void requestUserActivityNotification() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUserActivityNotification, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestUserActivityNotification();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Notify WindowManager that it should not override the info in DisplayManager for the specified
           * display. This can disable letter- or pillar-boxing applied in DisplayManager when the metrics
           * of the logical display reported from WindowManager do not correspond to the metrics of the
           * physical display it is based on.
           *
           * @param displayId The id of the display.
           */
      @Override public void dontOverrideDisplayInfo(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dontOverrideDisplayInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dontOverrideDisplayInfo(displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Gets the windowing mode of the display.
           *
           * @param displayId The id of the display.
           * @return {@link WindowConfiguration.WindowingMode}
           */
      @Override public int getWindowingMode(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowingMode(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets the windowing mode of the display.
           *
           * @param displayId The id of the display.
           * @param mode {@link WindowConfiguration.WindowingMode}
           */
      @Override public void setWindowingMode(int displayId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWindowingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWindowingMode(displayId, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Gets current remove content mode of the display.
           * <p>
           * What actions should be performed with the display's content when it is removed. Default
           * behavior for public displays in this case is to move all activities to the primary display
           * and make it focused. For private display is to destroy all activities.
           * </p>
           *
           * @param displayId The id of the display.
           * @return The remove content mode of the display.
           * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
           * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
           */
      @Override public int getRemoveContentMode(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRemoveContentMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRemoveContentMode(displayId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets the remove content mode of the display.
           * <p>
           * This mode indicates what actions should be performed with the display's content when it is
           * removed.
           * </p>
           *
           * @param displayId The id of the display.
           * @param mode Remove content mode.
           * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
           * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
           */
      @Override public void setRemoveContentMode(int displayId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRemoveContentMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRemoveContentMode(displayId, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Indicates that the display should show its content when non-secure keyguard is shown.
           * <p>
           * This flag identifies secondary displays that will continue showing content if keyguard can be
           * dismissed without entering credentials.
           * </p><p>
           * An example of usage is a virtual display which content is displayed on external hardware
           * display that is not visible to the system directly.
           * </p>
           *
           * @param displayId The id of the display.
           * @return {@code true} if the display should show its content when non-secure keyguard is
           *         shown.
           * @see KeyguardManager#isDeviceSecure()
           * @see KeyguardManager#isDeviceLocked()
           */
      @Override public boolean shouldShowWithInsecureKeyguard(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldShowWithInsecureKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldShowWithInsecureKeyguard(displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets that the display should show its content when non-secure keyguard is shown.
           *
           * @param displayId The id of the display.
           * @param shouldShow Indicates that the display should show its content when non-secure keyguard
           *                  is shown.
           * @see KeyguardManager#isDeviceSecure()
           * @see KeyguardManager#isDeviceLocked()
           */
      @Override public void setShouldShowWithInsecureKeyguard(int displayId, boolean shouldShow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((shouldShow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setShouldShowWithInsecureKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setShouldShowWithInsecureKeyguard(displayId, shouldShow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Indicates the display should show system decors.
           * <p>
           * System decors include status bar, navigation bar, launcher.
           * </p>
           *
           * @param displayId The id of the display.
           * @return {@code true} if the display should show system decors.
           */
      @Override public boolean shouldShowSystemDecors(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldShowSystemDecors, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldShowSystemDecors(displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets that the display should show system decors.
           * <p>
           * System decors include status bar, navigation bar, launcher.
           * </p>
           *
           * @param displayId The id of the display.
           * @param shouldShow Indicates that the display should show system decors.
           */
      @Override public void setShouldShowSystemDecors(int displayId, boolean shouldShow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((shouldShow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setShouldShowSystemDecors, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setShouldShowSystemDecors(displayId, shouldShow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Indicates that the display should show IME.
           *
           * @param displayId The id of the display.
           * @return {@code true} if the display should show IME.
           * @see KeyguardManager#isDeviceSecure()
           * @see KeyguardManager#isDeviceLocked()
           */
      @Override public boolean shouldShowIme(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldShowIme, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldShowIme(displayId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets that the display should show IME.
           *
           * @param displayId The id of the display.
           * @param shouldShow Indicates that the display should show IME.
           * @see KeyguardManager#isDeviceSecure()
           * @see KeyguardManager#isDeviceLocked()
           */
      @Override public void setShouldShowIme(int displayId, boolean shouldShow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((shouldShow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setShouldShowIme, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setShouldShowIme(displayId, shouldShow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Waits for transactions to get applied before injecting input.
           * This includes waiting for the input windows to get sent to InputManager.
           *
           * This is needed for testing since the system add windows and injects input
           * quick enough that the windows don't have time to get sent to InputManager.
           */
      @Override public boolean injectInputAfterTransactionsApplied(android.view.InputEvent ev, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ev!=null)) {
            _data.writeInt(1);
            ev.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectInputAfterTransactionsApplied, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().injectInputAfterTransactionsApplied(ev, mode);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Waits until all animations have completed and input information has been sent from
           * WindowManager to native InputManager.
           *
           * This is needed for testing since we need to ensure input information has been propagated to
           * native InputManager before proceeding with tests.
           */
      @Override public void syncInputTransactions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_syncInputTransactions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().syncInputTransactions();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.view.IWindowManager sDefaultImpl;
    }
    static final int TRANSACTION_startViewServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopViewServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isViewServerRunning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_openSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getInitialDisplaySize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getBaseDisplaySize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setForcedDisplaySize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_clearForcedDisplaySize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getInitialDisplayDensity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getBaseDisplayDensity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setForcedDisplayDensityForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_clearForcedDisplayDensityForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setForcedDisplayScalingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setOverscan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setEventDispatching = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_addWindowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_removeWindowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_prepareAppTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_overridePendingAppTransitionMultiThumbFuture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_overridePendingAppTransitionRemote = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_executeAppTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_endProlongedAnimations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_startFreezingScreen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_stopFreezingScreen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_disableKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_reenableKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_exitKeyguardSecurely = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_isKeyguardLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_isKeyguardSecure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_dismissKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_setSwitchingUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_closeSystemDialogs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getAnimationScale = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getAnimationScales = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_setAnimationScale = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_setAnimationScales = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getCurrentAnimatorScale = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_setInTouchMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_showStrictModeViolation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_setStrictModeVisualIndicatorPreference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_refreshScreenCaptureDisabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_updateRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getDefaultDisplayRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_watchRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_removeRotationWatcher = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_getPreferredOptionsPanelGravity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_freezeRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_thawRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_isRotationFrozen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_freezeDisplayRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_thawDisplayRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_isDisplayRotationFrozen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_screenshotWallpaper = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_registerWallpaperVisibilityListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_unregisterWallpaperVisibilityListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_registerSystemGestureExclusionListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_unregisterSystemGestureExclusionListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_requestAssistScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_statusBarVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_setForceShowSystemBars = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_setRecentsVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_setPipVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_setShelfHeight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_setNavBarVirtualKeyHapticFeedbackEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_hasNavigationBar = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_getNavBarPosition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_lockNow = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_isSafeModeEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_enableScreenIfNeeded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_clearWindowContentFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_getWindowContentFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_getDockedStackSide = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_setDockedStackDividerTouchRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_registerDockedStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_registerPinnedStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_setResizeDimLayer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_requestAppKeyboardShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_getStableInsets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_setForwardedInsets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_registerShortcutKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_createInputConsumer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_destroyInputConsumer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_getCurrentImeTouchRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_registerDisplayFoldListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_unregisterDisplayFoldListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_startWindowTrace = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_stopWindowTrace = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_isWindowTraceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_requestUserActivityNotification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_dontOverrideDisplayInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    static final int TRANSACTION_getWindowingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 90);
    static final int TRANSACTION_setWindowingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 91);
    static final int TRANSACTION_getRemoveContentMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 92);
    static final int TRANSACTION_setRemoveContentMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 93);
    static final int TRANSACTION_shouldShowWithInsecureKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 94);
    static final int TRANSACTION_setShouldShowWithInsecureKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 95);
    static final int TRANSACTION_shouldShowSystemDecors = (android.os.IBinder.FIRST_CALL_TRANSACTION + 96);
    static final int TRANSACTION_setShouldShowSystemDecors = (android.os.IBinder.FIRST_CALL_TRANSACTION + 97);
    static final int TRANSACTION_shouldShowIme = (android.os.IBinder.FIRST_CALL_TRANSACTION + 98);
    static final int TRANSACTION_setShouldShowIme = (android.os.IBinder.FIRST_CALL_TRANSACTION + 99);
    static final int TRANSACTION_injectInputAfterTransactionsApplied = (android.os.IBinder.FIRST_CALL_TRANSACTION + 100);
    static final int TRANSACTION_syncInputTransactions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 101);
    public static boolean setDefaultImpl(android.view.IWindowManager impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static android.view.IWindowManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * ===== NOTICE =====
       * The first three methods must remain the first three methods. Scripts
       * and tools rely on their transaction number to work properly.
       */// This is used for debugging

  public boolean startViewServer(int port) throws android.os.RemoteException;
  // Transaction #1

  public boolean stopViewServer() throws android.os.RemoteException;
  // Transaction #2

  public boolean isViewServerRunning() throws android.os.RemoteException;
  // Transaction #3

  public android.view.IWindowSession openSession(android.view.IWindowSessionCallback callback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:78:1:78:25")
  public void getInitialDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:80:1:80:25")
  public void getBaseDisplaySize(int displayId, android.graphics.Point size) throws android.os.RemoteException;
  public void setForcedDisplaySize(int displayId, int width, int height) throws android.os.RemoteException;
  public void clearForcedDisplaySize(int displayId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:84:1:84:25")
  public int getInitialDisplayDensity(int displayId) throws android.os.RemoteException;
  public int getBaseDisplayDensity(int displayId) throws android.os.RemoteException;
  public void setForcedDisplayDensityForUser(int displayId, int density, int userId) throws android.os.RemoteException;
  public void clearForcedDisplayDensityForUser(int displayId, int userId) throws android.os.RemoteException;
  public void setForcedDisplayScalingMode(int displayId, int mode) throws android.os.RemoteException;
  // 0 = auto, 1 = disable

  public void setOverscan(int displayId, int left, int top, int right, int bottom) throws android.os.RemoteException;
  // These can only be called when holding the MANAGE_APP_TOKENS permission.

  public void setEventDispatching(boolean enabled) throws android.os.RemoteException;
  public void addWindowToken(android.os.IBinder token, int type, int displayId) throws android.os.RemoteException;
  public void removeWindowToken(android.os.IBinder token, int displayId) throws android.os.RemoteException;
  public void prepareAppTransition(int transit, boolean alwaysKeepCurrent) throws android.os.RemoteException;
  /**
       * Like overridePendingAppTransitionMultiThumb, but uses a future to supply the specs. This is
       * used for recents, where generating the thumbnails of the specs takes a non-trivial amount of
       * time, so we want to move that off the critical path for starting the new activity.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:104:1:104:25")
  public void overridePendingAppTransitionMultiThumbFuture(android.view.IAppTransitionAnimationSpecsFuture specsFuture, android.os.IRemoteCallback startedCallback, boolean scaleUp, int displayId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:108:1:108:25")
  public void overridePendingAppTransitionRemote(android.view.RemoteAnimationAdapter remoteAnimationAdapter, int displayId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:111:1:111:25")
  public void executeAppTransition() throws android.os.RemoteException;
  /**
        * Used by system ui to report that recents has shown itself.
        * @deprecated to be removed once prebuilts are updated
        */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:118:1:118:25")
  public void endProlongedAnimations() throws android.os.RemoteException;
  public void startFreezingScreen(int exitAnim, int enterAnim) throws android.os.RemoteException;
  public void stopFreezingScreen() throws android.os.RemoteException;
  // these require DISABLE_KEYGUARD permission
  /** @deprecated use Activity.setShowWhenLocked instead. */
  public void disableKeyguard(android.os.IBinder token, java.lang.String tag, int userId) throws android.os.RemoteException;
  /** @deprecated use Activity.setShowWhenLocked instead. */
  public void reenableKeyguard(android.os.IBinder token, int userId) throws android.os.RemoteException;
  public void exitKeyguardSecurely(android.view.IOnKeyguardExitResult callback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:130:1:130:25")
  public boolean isKeyguardLocked() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:132:1:132:25")
  public boolean isKeyguardSecure(int userId) throws android.os.RemoteException;
  public void dismissKeyguard(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException;
  // Requires INTERACT_ACROSS_USERS_FULL permission

  public void setSwitchingUser(boolean switching) throws android.os.RemoteException;
  public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException;
  // These can only be called with the SET_ANIMATON_SCALE permission.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:142:1:142:25")
  public float getAnimationScale(int which) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:144:1:144:25")
  public float[] getAnimationScales() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:146:1:146:25")
  public void setAnimationScale(int which, float scale) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:148:1:148:25")
  public void setAnimationScales(float[] scales) throws android.os.RemoteException;
  public float getCurrentAnimatorScale() throws android.os.RemoteException;
  // For testing

  public void setInTouchMode(boolean showFocus) throws android.os.RemoteException;
  // For StrictMode flashing a red border on violations from the UI
  // thread.  The uid/pid is implicit from the Binder call, and the Window
  // Manager uses that to determine whether or not the red border should
  // actually be shown.  (it will be ignored that pid doesn't have windows
  // on screen)

  public void showStrictModeViolation(boolean on) throws android.os.RemoteException;
  // Proxy to set the system property for whether the flashing
  // should be enabled.  The 'enabled' value is null or blank for
  // the system default (differs per build variant) or any valid
  // boolean string as parsed by SystemProperties.getBoolean().

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:167:1:167:25")
  public void setStrictModeVisualIndicatorPreference(java.lang.String enabled) throws android.os.RemoteException;
  /**
       * Set whether screen capture is disabled for all windows of a specific user from
       * the device policy cache.
       */
  public void refreshScreenCaptureDisabled(int userId) throws android.os.RemoteException;
  // These can only be called with the SET_ORIENTATION permission.
  /**
       * Update the current screen rotation based on the current state of
       * the world.
       * @param alwaysSendConfiguration Flag to force a new configuration to
       * be evaluated.  This can be used when there are other parameters in
       * configuration that are changing.
       * @param forceRelayout If true, the window manager will always do a relayout
       * of its windows even if the rotation hasn't changed.
       */
  public void updateRotation(boolean alwaysSendConfiguration, boolean forceRelayout) throws android.os.RemoteException;
  /**
       * Retrieve the current orientation of the primary screen.
       * @return Constant as per {@link android.view.Surface.Rotation}.
       *
       * @see android.view.Display#DEFAULT_DISPLAY
       */
  public int getDefaultDisplayRotation() throws android.os.RemoteException;
  /**
       * Watch the rotation of the specified screen.  Returns the current rotation,
       * calls back when it changes.
       */
  public int watchRotation(android.view.IRotationWatcher watcher, int displayId) throws android.os.RemoteException;
  /**
       * Remove a rotation watcher set using watchRotation.
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:206:1:206:25")
  public void removeRotationWatcher(android.view.IRotationWatcher watcher) throws android.os.RemoteException;
  /**
       * Determine the preferred edge of the screen to pin the compact options menu against.
       *
       * @param displayId Id of the display where the menu window currently resides.
       * @return a Gravity value for the options menu panel.
       * @hide
       */
  public int getPreferredOptionsPanelGravity(int displayId) throws android.os.RemoteException;
  /**
       * Equivalent to calling {@link #freezeDisplayRotation(int, int)} with {@link
       * android.view.Display#DEFAULT_DISPLAY} and given rotation.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:222:1:222:25")
  public void freezeRotation(int rotation) throws android.os.RemoteException;
  /**
       * Equivalent to calling {@link #thawDisplayRotation(int)} with {@link
       * android.view.Display#DEFAULT_DISPLAY}.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:229:1:229:25")
  public void thawRotation() throws android.os.RemoteException;
  /**
       * Equivelant to call {@link #isDisplayRotationFrozen(int)} with {@link
       * android.view.Display#DEFAULT_DISPLAY}.
       */
  public boolean isRotationFrozen() throws android.os.RemoteException;
  /**
       * Lock the display orientation to the specified rotation, or to the current
       * rotation if -1. Sensor input will be ignored until thawRotation() is called.
       *
       * @param displayId the ID of display which rotation should be frozen.
       * @param rotation one of {@link android.view.Surface#ROTATION_0},
       *        {@link android.view.Surface#ROTATION_90}, {@link android.view.Surface#ROTATION_180},
       *        {@link android.view.Surface#ROTATION_270} or -1 to freeze it to current rotation.
       * @hide
       */
  public void freezeDisplayRotation(int displayId, int rotation) throws android.os.RemoteException;
  /**
       * Release the orientation lock imposed by freezeRotation() on the display.
       *
       * @param displayId the ID of display which rotation should be thawed.
       * @hide
       */
  public void thawDisplayRotation(int displayId) throws android.os.RemoteException;
  /**
       * Gets whether the rotation is frozen on the display.
       *
       * @param displayId the ID of display which frozen is needed.
       * @return Whether the rotation is frozen.
       */
  public boolean isDisplayRotationFrozen(int displayId) throws android.os.RemoteException;
  /**
       * Screenshot the current wallpaper layer, including the whole screen.
       */
  public android.graphics.Bitmap screenshotWallpaper() throws android.os.RemoteException;
  /**
       * Registers a wallpaper visibility listener.
       * @return Current visibility.
       */
  public boolean registerWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException;
  /**
       * Remove a visibility watcher that was added using registerWallpaperVisibilityListener.
       */
  public void unregisterWallpaperVisibilityListener(android.view.IWallpaperVisibilityListener listener, int displayId) throws android.os.RemoteException;
  /**
       * Registers a system gesture exclusion listener for a given display.
       */
  public void registerSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException;
  /**
       * Unregisters a system gesture exclusion listener for a given display.
       */
  public void unregisterSystemGestureExclusionListener(android.view.ISystemGestureExclusionListener listener, int displayId) throws android.os.RemoteException;
  /**
       * Used only for assist -- request a screenshot of the current application.
       */
  public boolean requestAssistScreenshot(android.app.IAssistDataReceiver receiver) throws android.os.RemoteException;
  /**
       * Called by the status bar to notify Views of changes to System UI visiblity.
       */
  public void statusBarVisibilityChanged(int displayId, int visibility) throws android.os.RemoteException;
  /**
      * When set to {@code true} the system bars will always be shown. This is true even if an app
      * requests to be fullscreen by setting the system ui visibility flags. The
      * functionality was added for the automotive case as a way to guarantee required content stays
      * on screen at all times.
      *
      * @hide
      */
  public void setForceShowSystemBars(boolean show) throws android.os.RemoteException;
  /**
       * Called by System UI to notify of changes to the visibility of Recents.
       */
  public void setRecentsVisibility(boolean visible) throws android.os.RemoteException;
  /**
       * Called by System UI to notify of changes to the visibility of PIP.
       */
  public void setPipVisibility(boolean visible) throws android.os.RemoteException;
  /**
       * Called by System UI to notify of changes to the visibility and height of the shelf.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:329:1:329:25")
  public void setShelfHeight(boolean visible, int shelfHeight) throws android.os.RemoteException;
  /**
       * Called by System UI to enable or disable haptic feedback on the navigation bar buttons.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:335:1:335:25")
  public void setNavBarVirtualKeyHapticFeedbackEnabled(boolean enabled) throws android.os.RemoteException;
  /**
       * Device has a software navigation bar (separate from the status bar) on specific display.
       *
       * @param displayId the id of display to check if there is a software navigation bar.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:343:1:343:25")
  public boolean hasNavigationBar(int displayId) throws android.os.RemoteException;
  /**
       * Get the position of the nav bar
       */
  public int getNavBarPosition(int displayId) throws android.os.RemoteException;
  /**
       * Lock the device immediately with the specified options (can be null).
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:354:1:354:25")
  public void lockNow(android.os.Bundle options) throws android.os.RemoteException;
  /**
       * Device is in safe mode.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:360:1:360:25")
  public boolean isSafeModeEnabled() throws android.os.RemoteException;
  /**
       * Enables the screen if all conditions are met.
       */
  public void enableScreenIfNeeded() throws android.os.RemoteException;
  /**
       * Clears the frame statistics for a given window.
       *
       * @param token The window token.
       * @return Whether the frame statistics were cleared.
       */
  public boolean clearWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException;
  /**
       * Gets the content frame statistics for a given window.
       *
       * @param token The window token.
       * @return The frame statistics or null if the window does not exist.
       */
  public android.view.WindowContentFrameStats getWindowContentFrameStats(android.os.IBinder token) throws android.os.RemoteException;
  /**
       * @return the dock side the current docked stack is at; must be one of the
       *         WindowManagerGlobal.DOCKED_* values
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:388:1:388:25")
  public int getDockedStackSide() throws android.os.RemoteException;
  /**
       * Sets the region the user can touch the divider. This region will be excluded from the region
       * which is used to cause a focus switch when dispatching touch.
       */
  public void setDockedStackDividerTouchRegion(android.graphics.Rect touchableRegion) throws android.os.RemoteException;
  /**
       * Registers a listener that will be called when the dock divider changes its visibility or when
       * the docked stack gets added/removed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:401:1:401:25")
  public void registerDockedStackListener(android.view.IDockedStackListener listener) throws android.os.RemoteException;
  /**
       * Registers a listener that will be called when the pinned stack state changes.
       */
  public void registerPinnedStackListener(int displayId, android.view.IPinnedStackListener listener) throws android.os.RemoteException;
  /**
       * Updates the dim layer used while resizing.
       *
       * @param visible Whether the dim layer should be visible.
       * @param targetWindowingMode The windowing mode of the stack the dim layer should be placed on.
       * @param alpha The translucency of the dim layer, between 0 and 1.
       */
  public void setResizeDimLayer(boolean visible, int targetWindowingMode, float alpha) throws android.os.RemoteException;
  /**
       * Requests Keyboard Shortcuts from the displayed window.
       *
       * @param receiver The receiver to deliver the results to.
       */
  public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException;
  /**
       * Retrieves the current stable insets from the primary display.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:428:1:428:25")
  public void getStableInsets(int displayId, android.graphics.Rect outInsets) throws android.os.RemoteException;
  /**
       * Set the forwarded insets on the display.
       * <p>
       * This is only used in case a virtual display is displayed on another display that has insets,
       * and the bounds of the virtual display is overlapping with the insets from the host display.
       * In that case, the contents on the virtual display won't be placed over the forwarded insets.
       * Only the owner of the display is permitted to set the forwarded insets on it.
       */
  public void setForwardedInsets(int displayId, android.graphics.Insets insets) throws android.os.RemoteException;
  /**
       * Register shortcut key. Shortcut code is packed as:
       * (MetaState << Integer.SIZE) | KeyCode
       * @hide
       */
  public void registerShortcutKey(long shortcutCode, com.android.internal.policy.IShortcutService keySubscriber) throws android.os.RemoteException;
  /**
       * Create an input consumer by name and display id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:451:1:451:25")
  public void createInputConsumer(android.os.IBinder token, java.lang.String name, int displayId, android.view.InputChannel inputChannel) throws android.os.RemoteException;
  /**
       * Destroy an input consumer by name and display id.
       * This method will also dispose the input channels associated with that InputConsumer.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowManager.aidl:459:1:459:25")
  public boolean destroyInputConsumer(java.lang.String name, int displayId) throws android.os.RemoteException;
  /**
       * Return the touch region for the current IME window, or an empty region if there is none.
       */
  public android.graphics.Region getCurrentImeTouchRegion() throws android.os.RemoteException;
  /**
       * Registers an IDisplayFoldListener.
       */
  public void registerDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException;
  /**
       * Unregisters an IDisplayFoldListener.
       */
  public void unregisterDisplayFoldListener(android.view.IDisplayFoldListener listener) throws android.os.RemoteException;
  /**
       * Starts a window trace.
       */
  public void startWindowTrace() throws android.os.RemoteException;
  /**
       * Stops a window trace.
       */
  public void stopWindowTrace() throws android.os.RemoteException;
  /**
       * Returns true if window trace is enabled.
       */
  public boolean isWindowTraceEnabled() throws android.os.RemoteException;
  /**
       * Requests that the WindowManager sends
       * WindowManagerPolicyConstants#ACTION_USER_ACTIVITY_NOTIFICATION on the next user activity.
       */
  public void requestUserActivityNotification() throws android.os.RemoteException;
  /**
       * Notify WindowManager that it should not override the info in DisplayManager for the specified
       * display. This can disable letter- or pillar-boxing applied in DisplayManager when the metrics
       * of the logical display reported from WindowManager do not correspond to the metrics of the
       * physical display it is based on.
       *
       * @param displayId The id of the display.
       */
  public void dontOverrideDisplayInfo(int displayId) throws android.os.RemoteException;
  /**
       * Gets the windowing mode of the display.
       *
       * @param displayId The id of the display.
       * @return {@link WindowConfiguration.WindowingMode}
       */
  public int getWindowingMode(int displayId) throws android.os.RemoteException;
  /**
       * Sets the windowing mode of the display.
       *
       * @param displayId The id of the display.
       * @param mode {@link WindowConfiguration.WindowingMode}
       */
  public void setWindowingMode(int displayId, int mode) throws android.os.RemoteException;
  /**
       * Gets current remove content mode of the display.
       * <p>
       * What actions should be performed with the display's content when it is removed. Default
       * behavior for public displays in this case is to move all activities to the primary display
       * and make it focused. For private display is to destroy all activities.
       * </p>
       *
       * @param displayId The id of the display.
       * @return The remove content mode of the display.
       * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
       * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
       */
  public int getRemoveContentMode(int displayId) throws android.os.RemoteException;
  /**
       * Sets the remove content mode of the display.
       * <p>
       * This mode indicates what actions should be performed with the display's content when it is
       * removed.
       * </p>
       *
       * @param displayId The id of the display.
       * @param mode Remove content mode.
       * @see WindowManager#REMOVE_CONTENT_MODE_MOVE_TO_PRIMARY
       * @see WindowManager#REMOVE_CONTENT_MODE_DESTROY
       */
  public void setRemoveContentMode(int displayId, int mode) throws android.os.RemoteException;
  /**
       * Indicates that the display should show its content when non-secure keyguard is shown.
       * <p>
       * This flag identifies secondary displays that will continue showing content if keyguard can be
       * dismissed without entering credentials.
       * </p><p>
       * An example of usage is a virtual display which content is displayed on external hardware
       * display that is not visible to the system directly.
       * </p>
       *
       * @param displayId The id of the display.
       * @return {@code true} if the display should show its content when non-secure keyguard is
       *         shown.
       * @see KeyguardManager#isDeviceSecure()
       * @see KeyguardManager#isDeviceLocked()
       */
  public boolean shouldShowWithInsecureKeyguard(int displayId) throws android.os.RemoteException;
  /**
       * Sets that the display should show its content when non-secure keyguard is shown.
       *
       * @param displayId The id of the display.
       * @param shouldShow Indicates that the display should show its content when non-secure keyguard
       *                  is shown.
       * @see KeyguardManager#isDeviceSecure()
       * @see KeyguardManager#isDeviceLocked()
       */
  public void setShouldShowWithInsecureKeyguard(int displayId, boolean shouldShow) throws android.os.RemoteException;
  /**
       * Indicates the display should show system decors.
       * <p>
       * System decors include status bar, navigation bar, launcher.
       * </p>
       *
       * @param displayId The id of the display.
       * @return {@code true} if the display should show system decors.
       */
  public boolean shouldShowSystemDecors(int displayId) throws android.os.RemoteException;
  /**
       * Sets that the display should show system decors.
       * <p>
       * System decors include status bar, navigation bar, launcher.
       * </p>
       *
       * @param displayId The id of the display.
       * @param shouldShow Indicates that the display should show system decors.
       */
  public void setShouldShowSystemDecors(int displayId, boolean shouldShow) throws android.os.RemoteException;
  /**
       * Indicates that the display should show IME.
       *
       * @param displayId The id of the display.
       * @return {@code true} if the display should show IME.
       * @see KeyguardManager#isDeviceSecure()
       * @see KeyguardManager#isDeviceLocked()
       */
  public boolean shouldShowIme(int displayId) throws android.os.RemoteException;
  /**
       * Sets that the display should show IME.
       *
       * @param displayId The id of the display.
       * @param shouldShow Indicates that the display should show IME.
       * @see KeyguardManager#isDeviceSecure()
       * @see KeyguardManager#isDeviceLocked()
       */
  public void setShouldShowIme(int displayId, boolean shouldShow) throws android.os.RemoteException;
  /**
       * Waits for transactions to get applied before injecting input.
       * This includes waiting for the input windows to get sent to InputManager.
       *
       * This is needed for testing since the system add windows and injects input
       * quick enough that the windows don't have time to get sent to InputManager.
       */
  public boolean injectInputAfterTransactionsApplied(android.view.InputEvent ev, int mode) throws android.os.RemoteException;
  /**
       * Waits until all animations have completed and input information has been sent from
       * WindowManager to native InputManager.
       *
       * This is needed for testing since we need to ensure input information has been propagated to
       * native InputManager before proceeding with tests.
       */
  public void syncInputTransactions() throws android.os.RemoteException;
}
