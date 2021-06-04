/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.statusbar;
/** @hide */
public interface IStatusBar extends android.os.IInterface
{
  /** Default implementation for IStatusBar. */
  public static class Default implements com.android.internal.statusbar.IStatusBar
  {
    @Override public void setIcon(java.lang.String slot, com.android.internal.statusbar.StatusBarIcon icon) throws android.os.RemoteException
    {
    }
    @Override public void removeIcon(java.lang.String slot) throws android.os.RemoteException
    {
    }
    @Override public void disable(int displayId, int state1, int state2) throws android.os.RemoteException
    {
    }
    @Override public void animateExpandNotificationsPanel() throws android.os.RemoteException
    {
    }
    @Override public void animateExpandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException
    {
    }
    @Override public void animateCollapsePanels() throws android.os.RemoteException
    {
    }
    @Override public void togglePanel() throws android.os.RemoteException
    {
    }
    @Override public void showWirelessChargingAnimation(int batteryLevel) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI side of a visibility flag change on the specified display.
         *
         * @param displayId the id of the display to notify
         * @param vis the visibility flags except SYSTEM_UI_FLAG_LIGHT_STATUS_BAR which will be reported
         *            separately in fullscreenStackVis and dockedStackVis
         * @param fullscreenStackVis the flags which only apply in the region of the fullscreen stack,
         *                           which is currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
         * @param dockedStackVis the flags that only apply in the region of the docked stack, which is
         *                       currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
         * @param mask which flags to change
         * @param fullscreenBounds the current bounds of the fullscreen stack, in screen coordinates
         * @param dockedBounds the current bounds of the docked stack, in screen coordinates
         * @param navbarColorManagedByIme {@code true} if navigation bar color is managed by IME.
         */
    @Override public void setSystemUiVisibility(int displayId, int vis, int fullscreenStackVis, int dockedStackVis, int mask, android.graphics.Rect fullscreenBounds, android.graphics.Rect dockedBounds, boolean navbarColorManagedByIme) throws android.os.RemoteException
    {
    }
    @Override public void topAppWindowChanged(int displayId, boolean menuVisible) throws android.os.RemoteException
    {
    }
    @Override public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException
    {
    }
    @Override public void setWindowState(int display, int window, int state) throws android.os.RemoteException
    {
    }
    @Override public void showRecentApps(boolean triggeredFromAltTab) throws android.os.RemoteException
    {
    }
    @Override public void hideRecentApps(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException
    {
    }
    @Override public void toggleRecentApps() throws android.os.RemoteException
    {
    }
    @Override public void toggleSplitScreen() throws android.os.RemoteException
    {
    }
    @Override public void preloadRecentApps() throws android.os.RemoteException
    {
    }
    @Override public void cancelPreloadRecentApps() throws android.os.RemoteException
    {
    }
    @Override public void showScreenPinningRequest(int taskId) throws android.os.RemoteException
    {
    }
    @Override public void dismissKeyboardShortcutsMenu() throws android.os.RemoteException
    {
    }
    @Override public void toggleKeyboardShortcutsMenu(int deviceId) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI on the specified display that an app transition is pending to delay
         * applying some flags with visual impact until {@link #appTransitionReady} is called.
         *
         * @param displayId the id of the display to notify
         */
    @Override public void appTransitionPending(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI on the specified display that a pending app transition has been cancelled.
         *
         * @param displayId the id of the display to notify
         */
    @Override public void appTransitionCancelled(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI on the specified display that an app transition is now being executed.
         *
         * @param displayId the id of the display to notify
         * @param statusBarAnimationsStartTime the desired start time for all visual animations in the
         *        status bar caused by this app transition in uptime millis
         * @param statusBarAnimationsDuration the duration for all visual animations in the status
         *        bar caused by this app transition in millis
         */
    @Override public void appTransitionStarting(int displayId, long statusBarAnimationsStartTime, long statusBarAnimationsDuration) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI on the specified display that an app transition is done.
         *
         * @param displayId the id of the display to notify
         */
    @Override public void appTransitionFinished(int displayId) throws android.os.RemoteException
    {
    }
    @Override public void showAssistDisclosure() throws android.os.RemoteException
    {
    }
    @Override public void startAssist(android.os.Bundle args) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the status bar that a camera launch gesture has been detected.
         *
         * @param source the identifier for the gesture, see {@link StatusBarManager}
         */
    @Override public void onCameraLaunchGestureDetected(int source) throws android.os.RemoteException
    {
    }
    /**
         * Shows the picture-in-picture menu if an activity is in picture-in-picture mode.
         */
    @Override public void showPictureInPictureMenu() throws android.os.RemoteException
    {
    }
    /**
         * Shows the global actions menu.
         */
    @Override public void showGlobalActionsMenu() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the status bar that a new rotation suggestion is available.
         */
    @Override public void onProposedRotationChanged(int rotation, boolean isValid) throws android.os.RemoteException
    {
    }
    /**
         * Set whether the top app currently hides the statusbar.
         *
         * @param hidesStatusBar whether it is being hidden
         */
    @Override public void setTopAppHidesStatusBar(boolean hidesStatusBar) throws android.os.RemoteException
    {
    }
    @Override public void addQsTile(android.content.ComponentName tile) throws android.os.RemoteException
    {
    }
    @Override public void remQsTile(android.content.ComponentName tile) throws android.os.RemoteException
    {
    }
    @Override public void clickQsTile(android.content.ComponentName tile) throws android.os.RemoteException
    {
    }
    @Override public void handleSystemKey(int key) throws android.os.RemoteException
    {
    }
    /**
         * Methods to show toast messages for screen pinning
         */
    @Override public void showPinningEnterExitToast(boolean entering) throws android.os.RemoteException
    {
    }
    @Override public void showPinningEscapeToast() throws android.os.RemoteException
    {
    }
    @Override public void showShutdownUi(boolean isReboot, java.lang.String reason) throws android.os.RemoteException
    {
    }
    // Used to show the dialog when BiometricService starts authentication

    @Override public void showBiometricDialog(android.os.Bundle bundle, android.hardware.biometrics.IBiometricServiceReceiverInternal receiver, int type, boolean requireConfirmation, int userId) throws android.os.RemoteException
    {
    }
    // Used to hide the dialog when a biometric is authenticated

    @Override public void onBiometricAuthenticated(boolean authenticated, java.lang.String failureReason) throws android.os.RemoteException
    {
    }
    // Used to set a temporary message, e.g. fingerprint not recognized, finger moved too fast, etc

    @Override public void onBiometricHelp(java.lang.String message) throws android.os.RemoteException
    {
    }
    // Used to set a message - the dialog will dismiss after a certain amount of time

    @Override public void onBiometricError(java.lang.String error) throws android.os.RemoteException
    {
    }
    // Used to hide the biometric dialog when the AuthenticationClient is stopped

    @Override public void hideBiometricDialog() throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI that the display is ready to show system decorations.
         */
    @Override public void onDisplayReady(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Notifies System UI whether the recents animation is running or not.
         */
    @Override public void onRecentsAnimationStateChanged(boolean running) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.statusbar.IStatusBar
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.statusbar.IStatusBar";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.statusbar.IStatusBar interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.statusbar.IStatusBar asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.statusbar.IStatusBar))) {
        return ((com.android.internal.statusbar.IStatusBar)iin);
      }
      return new com.android.internal.statusbar.IStatusBar.Stub.Proxy(obj);
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
        case TRANSACTION_setIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.statusbar.StatusBarIcon _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.internal.statusbar.StatusBarIcon.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setIcon(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeIcon(_arg0);
          return true;
        }
        case TRANSACTION_disable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.disable(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_animateExpandNotificationsPanel:
        {
          data.enforceInterface(descriptor);
          this.animateExpandNotificationsPanel();
          return true;
        }
        case TRANSACTION_animateExpandSettingsPanel:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.animateExpandSettingsPanel(_arg0);
          return true;
        }
        case TRANSACTION_animateCollapsePanels:
        {
          data.enforceInterface(descriptor);
          this.animateCollapsePanels();
          return true;
        }
        case TRANSACTION_togglePanel:
        {
          data.enforceInterface(descriptor);
          this.togglePanel();
          return true;
        }
        case TRANSACTION_showWirelessChargingAnimation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.showWirelessChargingAnimation(_arg0);
          return true;
        }
        case TRANSACTION_setSystemUiVisibility:
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
          android.graphics.Rect _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          android.graphics.Rect _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          boolean _arg7;
          _arg7 = (0!=data.readInt());
          this.setSystemUiVisibility(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_topAppWindowChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.topAppWindowChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setImeWindowStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.setImeWindowStatus(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_setWindowState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setWindowState(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_showRecentApps:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.showRecentApps(_arg0);
          return true;
        }
        case TRANSACTION_hideRecentApps:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.hideRecentApps(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_toggleRecentApps:
        {
          data.enforceInterface(descriptor);
          this.toggleRecentApps();
          return true;
        }
        case TRANSACTION_toggleSplitScreen:
        {
          data.enforceInterface(descriptor);
          this.toggleSplitScreen();
          return true;
        }
        case TRANSACTION_preloadRecentApps:
        {
          data.enforceInterface(descriptor);
          this.preloadRecentApps();
          return true;
        }
        case TRANSACTION_cancelPreloadRecentApps:
        {
          data.enforceInterface(descriptor);
          this.cancelPreloadRecentApps();
          return true;
        }
        case TRANSACTION_showScreenPinningRequest:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.showScreenPinningRequest(_arg0);
          return true;
        }
        case TRANSACTION_dismissKeyboardShortcutsMenu:
        {
          data.enforceInterface(descriptor);
          this.dismissKeyboardShortcutsMenu();
          return true;
        }
        case TRANSACTION_toggleKeyboardShortcutsMenu:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.toggleKeyboardShortcutsMenu(_arg0);
          return true;
        }
        case TRANSACTION_appTransitionPending:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.appTransitionPending(_arg0);
          return true;
        }
        case TRANSACTION_appTransitionCancelled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.appTransitionCancelled(_arg0);
          return true;
        }
        case TRANSACTION_appTransitionStarting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          this.appTransitionStarting(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_appTransitionFinished:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.appTransitionFinished(_arg0);
          return true;
        }
        case TRANSACTION_showAssistDisclosure:
        {
          data.enforceInterface(descriptor);
          this.showAssistDisclosure();
          return true;
        }
        case TRANSACTION_startAssist:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startAssist(_arg0);
          return true;
        }
        case TRANSACTION_onCameraLaunchGestureDetected:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onCameraLaunchGestureDetected(_arg0);
          return true;
        }
        case TRANSACTION_showPictureInPictureMenu:
        {
          data.enforceInterface(descriptor);
          this.showPictureInPictureMenu();
          return true;
        }
        case TRANSACTION_showGlobalActionsMenu:
        {
          data.enforceInterface(descriptor);
          this.showGlobalActionsMenu();
          return true;
        }
        case TRANSACTION_onProposedRotationChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onProposedRotationChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setTopAppHidesStatusBar:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setTopAppHidesStatusBar(_arg0);
          return true;
        }
        case TRANSACTION_addQsTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.addQsTile(_arg0);
          return true;
        }
        case TRANSACTION_remQsTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.remQsTile(_arg0);
          return true;
        }
        case TRANSACTION_clickQsTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.clickQsTile(_arg0);
          return true;
        }
        case TRANSACTION_handleSystemKey:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.handleSystemKey(_arg0);
          return true;
        }
        case TRANSACTION_showPinningEnterExitToast:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.showPinningEnterExitToast(_arg0);
          return true;
        }
        case TRANSACTION_showPinningEscapeToast:
        {
          data.enforceInterface(descriptor);
          this.showPinningEscapeToast();
          return true;
        }
        case TRANSACTION_showShutdownUi:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.showShutdownUi(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_showBiometricDialog:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.hardware.biometrics.IBiometricServiceReceiverInternal _arg1;
          _arg1 = android.hardware.biometrics.IBiometricServiceReceiverInternal.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          int _arg4;
          _arg4 = data.readInt();
          this.showBiometricDialog(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onBiometricAuthenticated:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onBiometricAuthenticated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onBiometricHelp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onBiometricHelp(_arg0);
          return true;
        }
        case TRANSACTION_onBiometricError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onBiometricError(_arg0);
          return true;
        }
        case TRANSACTION_hideBiometricDialog:
        {
          data.enforceInterface(descriptor);
          this.hideBiometricDialog();
          return true;
        }
        case TRANSACTION_onDisplayReady:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDisplayReady(_arg0);
          return true;
        }
        case TRANSACTION_onRecentsAnimationStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onRecentsAnimationStateChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.statusbar.IStatusBar
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
      @Override public void setIcon(java.lang.String slot, com.android.internal.statusbar.StatusBarIcon icon) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(slot);
          if ((icon!=null)) {
            _data.writeInt(1);
            icon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIcon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIcon(slot, icon);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeIcon(java.lang.String slot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(slot);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeIcon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeIcon(slot);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disable(int displayId, int state1, int state2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(state1);
          _data.writeInt(state2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disable(displayId, state1, state2);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void animateExpandNotificationsPanel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_animateExpandNotificationsPanel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().animateExpandNotificationsPanel();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void animateExpandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(subPanel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_animateExpandSettingsPanel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().animateExpandSettingsPanel(subPanel);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void animateCollapsePanels() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_animateCollapsePanels, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().animateCollapsePanels();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void togglePanel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_togglePanel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().togglePanel();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showWirelessChargingAnimation(int batteryLevel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(batteryLevel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showWirelessChargingAnimation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showWirelessChargingAnimation(batteryLevel);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI side of a visibility flag change on the specified display.
           *
           * @param displayId the id of the display to notify
           * @param vis the visibility flags except SYSTEM_UI_FLAG_LIGHT_STATUS_BAR which will be reported
           *            separately in fullscreenStackVis and dockedStackVis
           * @param fullscreenStackVis the flags which only apply in the region of the fullscreen stack,
           *                           which is currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
           * @param dockedStackVis the flags that only apply in the region of the docked stack, which is
           *                       currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
           * @param mask which flags to change
           * @param fullscreenBounds the current bounds of the fullscreen stack, in screen coordinates
           * @param dockedBounds the current bounds of the docked stack, in screen coordinates
           * @param navbarColorManagedByIme {@code true} if navigation bar color is managed by IME.
           */
      @Override public void setSystemUiVisibility(int displayId, int vis, int fullscreenStackVis, int dockedStackVis, int mask, android.graphics.Rect fullscreenBounds, android.graphics.Rect dockedBounds, boolean navbarColorManagedByIme) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(vis);
          _data.writeInt(fullscreenStackVis);
          _data.writeInt(dockedStackVis);
          _data.writeInt(mask);
          if ((fullscreenBounds!=null)) {
            _data.writeInt(1);
            fullscreenBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((dockedBounds!=null)) {
            _data.writeInt(1);
            dockedBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((navbarColorManagedByIme)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemUiVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemUiVisibility(displayId, vis, fullscreenStackVis, dockedStackVis, mask, fullscreenBounds, dockedBounds, navbarColorManagedByIme);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void topAppWindowChanged(int displayId, boolean menuVisible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((menuVisible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_topAppWindowChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().topAppWindowChanged(displayId, menuVisible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeStrongBinder(token);
          _data.writeInt(vis);
          _data.writeInt(backDisposition);
          _data.writeInt(((showImeSwitcher)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImeWindowStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImeWindowStatus(displayId, token, vis, backDisposition, showImeSwitcher);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setWindowState(int display, int window, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(display);
          _data.writeInt(window);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWindowState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWindowState(display, window, state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showRecentApps(boolean triggeredFromAltTab) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((triggeredFromAltTab)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showRecentApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showRecentApps(triggeredFromAltTab);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hideRecentApps(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((triggeredFromAltTab)?(1):(0)));
          _data.writeInt(((triggeredFromHomeKey)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideRecentApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideRecentApps(triggeredFromAltTab, triggeredFromHomeKey);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void toggleRecentApps() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleRecentApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleRecentApps();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void toggleSplitScreen() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleSplitScreen, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleSplitScreen();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void preloadRecentApps() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_preloadRecentApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().preloadRecentApps();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelPreloadRecentApps() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelPreloadRecentApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelPreloadRecentApps();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showScreenPinningRequest(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showScreenPinningRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showScreenPinningRequest(taskId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dismissKeyboardShortcutsMenu() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissKeyboardShortcutsMenu, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissKeyboardShortcutsMenu();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void toggleKeyboardShortcutsMenu(int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleKeyboardShortcutsMenu, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleKeyboardShortcutsMenu(deviceId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI on the specified display that an app transition is pending to delay
           * applying some flags with visual impact until {@link #appTransitionReady} is called.
           *
           * @param displayId the id of the display to notify
           */
      @Override public void appTransitionPending(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appTransitionPending, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appTransitionPending(displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI on the specified display that a pending app transition has been cancelled.
           *
           * @param displayId the id of the display to notify
           */
      @Override public void appTransitionCancelled(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appTransitionCancelled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appTransitionCancelled(displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI on the specified display that an app transition is now being executed.
           *
           * @param displayId the id of the display to notify
           * @param statusBarAnimationsStartTime the desired start time for all visual animations in the
           *        status bar caused by this app transition in uptime millis
           * @param statusBarAnimationsDuration the duration for all visual animations in the status
           *        bar caused by this app transition in millis
           */
      @Override public void appTransitionStarting(int displayId, long statusBarAnimationsStartTime, long statusBarAnimationsDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeLong(statusBarAnimationsStartTime);
          _data.writeLong(statusBarAnimationsDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appTransitionStarting, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appTransitionStarting(displayId, statusBarAnimationsStartTime, statusBarAnimationsDuration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI on the specified display that an app transition is done.
           *
           * @param displayId the id of the display to notify
           */
      @Override public void appTransitionFinished(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appTransitionFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appTransitionFinished(displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showAssistDisclosure() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showAssistDisclosure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showAssistDisclosure();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startAssist(android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAssist, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startAssist(args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the status bar that a camera launch gesture has been detected.
           *
           * @param source the identifier for the gesture, see {@link StatusBarManager}
           */
      @Override public void onCameraLaunchGestureDetected(int source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(source);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCameraLaunchGestureDetected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCameraLaunchGestureDetected(source);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Shows the picture-in-picture menu if an activity is in picture-in-picture mode.
           */
      @Override public void showPictureInPictureMenu() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showPictureInPictureMenu, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showPictureInPictureMenu();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Shows the global actions menu.
           */
      @Override public void showGlobalActionsMenu() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showGlobalActionsMenu, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showGlobalActionsMenu();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the status bar that a new rotation suggestion is available.
           */
      @Override public void onProposedRotationChanged(int rotation, boolean isValid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rotation);
          _data.writeInt(((isValid)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProposedRotationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProposedRotationChanged(rotation, isValid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Set whether the top app currently hides the statusbar.
           *
           * @param hidesStatusBar whether it is being hidden
           */
      @Override public void setTopAppHidesStatusBar(boolean hidesStatusBar) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hidesStatusBar)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTopAppHidesStatusBar, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTopAppHidesStatusBar(hidesStatusBar);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addQsTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addQsTile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addQsTile(tile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void remQsTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_remQsTile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remQsTile(tile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clickQsTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_clickQsTile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clickQsTile(tile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handleSystemKey(int key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleSystemKey, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleSystemKey(key);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Methods to show toast messages for screen pinning
           */
      @Override public void showPinningEnterExitToast(boolean entering) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((entering)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showPinningEnterExitToast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showPinningEnterExitToast(entering);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showPinningEscapeToast() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showPinningEscapeToast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showPinningEscapeToast();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showShutdownUi(boolean isReboot, java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isReboot)?(1):(0)));
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showShutdownUi, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showShutdownUi(isReboot, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Used to show the dialog when BiometricService starts authentication

      @Override public void showBiometricDialog(android.os.Bundle bundle, android.hardware.biometrics.IBiometricServiceReceiverInternal receiver, int type, boolean requireConfirmation, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((bundle!=null)) {
            _data.writeInt(1);
            bundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(type);
          _data.writeInt(((requireConfirmation)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showBiometricDialog, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showBiometricDialog(bundle, receiver, type, requireConfirmation, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Used to hide the dialog when a biometric is authenticated

      @Override public void onBiometricAuthenticated(boolean authenticated, java.lang.String failureReason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((authenticated)?(1):(0)));
          _data.writeString(failureReason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricAuthenticated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricAuthenticated(authenticated, failureReason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Used to set a temporary message, e.g. fingerprint not recognized, finger moved too fast, etc

      @Override public void onBiometricHelp(java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricHelp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricHelp(message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Used to set a message - the dialog will dismiss after a certain amount of time

      @Override public void onBiometricError(java.lang.String error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Used to hide the biometric dialog when the AuthenticationClient is stopped

      @Override public void hideBiometricDialog() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideBiometricDialog, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideBiometricDialog();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI that the display is ready to show system decorations.
           */
      @Override public void onDisplayReady(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDisplayReady, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDisplayReady(displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies System UI whether the recents animation is running or not.
           */
      @Override public void onRecentsAnimationStateChanged(boolean running) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((running)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRecentsAnimationStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRecentsAnimationStateChanged(running);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.statusbar.IStatusBar sDefaultImpl;
    }
    static final int TRANSACTION_setIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_disable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_animateExpandNotificationsPanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_animateExpandSettingsPanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_animateCollapsePanels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_togglePanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_showWirelessChargingAnimation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setSystemUiVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_topAppWindowChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setImeWindowStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setWindowState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_showRecentApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_hideRecentApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_toggleRecentApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_toggleSplitScreen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_preloadRecentApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_cancelPreloadRecentApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_showScreenPinningRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_dismissKeyboardShortcutsMenu = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_toggleKeyboardShortcutsMenu = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_appTransitionPending = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_appTransitionCancelled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_appTransitionStarting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_appTransitionFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_showAssistDisclosure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_startAssist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_onCameraLaunchGestureDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_showPictureInPictureMenu = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_showGlobalActionsMenu = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_onProposedRotationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setTopAppHidesStatusBar = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_addQsTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_remQsTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_clickQsTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_handleSystemKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_showPinningEnterExitToast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_showPinningEscapeToast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_showShutdownUi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_showBiometricDialog = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_onBiometricAuthenticated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_onBiometricHelp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_onBiometricError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_hideBiometricDialog = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_onDisplayReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_onRecentsAnimationStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    public static boolean setDefaultImpl(com.android.internal.statusbar.IStatusBar impl) {
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
    public static com.android.internal.statusbar.IStatusBar getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setIcon(java.lang.String slot, com.android.internal.statusbar.StatusBarIcon icon) throws android.os.RemoteException;
  public void removeIcon(java.lang.String slot) throws android.os.RemoteException;
  public void disable(int displayId, int state1, int state2) throws android.os.RemoteException;
  public void animateExpandNotificationsPanel() throws android.os.RemoteException;
  public void animateExpandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException;
  public void animateCollapsePanels() throws android.os.RemoteException;
  public void togglePanel() throws android.os.RemoteException;
  public void showWirelessChargingAnimation(int batteryLevel) throws android.os.RemoteException;
  /**
       * Notifies System UI side of a visibility flag change on the specified display.
       *
       * @param displayId the id of the display to notify
       * @param vis the visibility flags except SYSTEM_UI_FLAG_LIGHT_STATUS_BAR which will be reported
       *            separately in fullscreenStackVis and dockedStackVis
       * @param fullscreenStackVis the flags which only apply in the region of the fullscreen stack,
       *                           which is currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
       * @param dockedStackVis the flags that only apply in the region of the docked stack, which is
       *                       currently only SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
       * @param mask which flags to change
       * @param fullscreenBounds the current bounds of the fullscreen stack, in screen coordinates
       * @param dockedBounds the current bounds of the docked stack, in screen coordinates
       * @param navbarColorManagedByIme {@code true} if navigation bar color is managed by IME.
       */
  public void setSystemUiVisibility(int displayId, int vis, int fullscreenStackVis, int dockedStackVis, int mask, android.graphics.Rect fullscreenBounds, android.graphics.Rect dockedBounds, boolean navbarColorManagedByIme) throws android.os.RemoteException;
  public void topAppWindowChanged(int displayId, boolean menuVisible) throws android.os.RemoteException;
  public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException;
  public void setWindowState(int display, int window, int state) throws android.os.RemoteException;
  public void showRecentApps(boolean triggeredFromAltTab) throws android.os.RemoteException;
  public void hideRecentApps(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException;
  public void toggleRecentApps() throws android.os.RemoteException;
  public void toggleSplitScreen() throws android.os.RemoteException;
  public void preloadRecentApps() throws android.os.RemoteException;
  public void cancelPreloadRecentApps() throws android.os.RemoteException;
  public void showScreenPinningRequest(int taskId) throws android.os.RemoteException;
  public void dismissKeyboardShortcutsMenu() throws android.os.RemoteException;
  public void toggleKeyboardShortcutsMenu(int deviceId) throws android.os.RemoteException;
  /**
       * Notifies System UI on the specified display that an app transition is pending to delay
       * applying some flags with visual impact until {@link #appTransitionReady} is called.
       *
       * @param displayId the id of the display to notify
       */
  public void appTransitionPending(int displayId) throws android.os.RemoteException;
  /**
       * Notifies System UI on the specified display that a pending app transition has been cancelled.
       *
       * @param displayId the id of the display to notify
       */
  public void appTransitionCancelled(int displayId) throws android.os.RemoteException;
  /**
       * Notifies System UI on the specified display that an app transition is now being executed.
       *
       * @param displayId the id of the display to notify
       * @param statusBarAnimationsStartTime the desired start time for all visual animations in the
       *        status bar caused by this app transition in uptime millis
       * @param statusBarAnimationsDuration the duration for all visual animations in the status
       *        bar caused by this app transition in millis
       */
  public void appTransitionStarting(int displayId, long statusBarAnimationsStartTime, long statusBarAnimationsDuration) throws android.os.RemoteException;
  /**
       * Notifies System UI on the specified display that an app transition is done.
       *
       * @param displayId the id of the display to notify
       */
  public void appTransitionFinished(int displayId) throws android.os.RemoteException;
  public void showAssistDisclosure() throws android.os.RemoteException;
  public void startAssist(android.os.Bundle args) throws android.os.RemoteException;
  /**
       * Notifies the status bar that a camera launch gesture has been detected.
       *
       * @param source the identifier for the gesture, see {@link StatusBarManager}
       */
  public void onCameraLaunchGestureDetected(int source) throws android.os.RemoteException;
  /**
       * Shows the picture-in-picture menu if an activity is in picture-in-picture mode.
       */
  public void showPictureInPictureMenu() throws android.os.RemoteException;
  /**
       * Shows the global actions menu.
       */
  public void showGlobalActionsMenu() throws android.os.RemoteException;
  /**
       * Notifies the status bar that a new rotation suggestion is available.
       */
  public void onProposedRotationChanged(int rotation, boolean isValid) throws android.os.RemoteException;
  /**
       * Set whether the top app currently hides the statusbar.
       *
       * @param hidesStatusBar whether it is being hidden
       */
  public void setTopAppHidesStatusBar(boolean hidesStatusBar) throws android.os.RemoteException;
  public void addQsTile(android.content.ComponentName tile) throws android.os.RemoteException;
  public void remQsTile(android.content.ComponentName tile) throws android.os.RemoteException;
  public void clickQsTile(android.content.ComponentName tile) throws android.os.RemoteException;
  public void handleSystemKey(int key) throws android.os.RemoteException;
  /**
       * Methods to show toast messages for screen pinning
       */
  public void showPinningEnterExitToast(boolean entering) throws android.os.RemoteException;
  public void showPinningEscapeToast() throws android.os.RemoteException;
  public void showShutdownUi(boolean isReboot, java.lang.String reason) throws android.os.RemoteException;
  // Used to show the dialog when BiometricService starts authentication

  public void showBiometricDialog(android.os.Bundle bundle, android.hardware.biometrics.IBiometricServiceReceiverInternal receiver, int type, boolean requireConfirmation, int userId) throws android.os.RemoteException;
  // Used to hide the dialog when a biometric is authenticated

  public void onBiometricAuthenticated(boolean authenticated, java.lang.String failureReason) throws android.os.RemoteException;
  // Used to set a temporary message, e.g. fingerprint not recognized, finger moved too fast, etc

  public void onBiometricHelp(java.lang.String message) throws android.os.RemoteException;
  // Used to set a message - the dialog will dismiss after a certain amount of time

  public void onBiometricError(java.lang.String error) throws android.os.RemoteException;
  // Used to hide the biometric dialog when the AuthenticationClient is stopped

  public void hideBiometricDialog() throws android.os.RemoteException;
  /**
       * Notifies System UI that the display is ready to show system decorations.
       */
  public void onDisplayReady(int displayId) throws android.os.RemoteException;
  /**
       * Notifies System UI whether the recents animation is running or not.
       */
  public void onRecentsAnimationStateChanged(boolean running) throws android.os.RemoteException;
}
