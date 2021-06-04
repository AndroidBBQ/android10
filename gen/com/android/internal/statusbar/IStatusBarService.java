/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.statusbar;
/** @hide */
public interface IStatusBarService extends android.os.IInterface
{
  /** Default implementation for IStatusBarService. */
  public static class Default implements com.android.internal.statusbar.IStatusBarService
  {
    @Override public void expandNotificationsPanel() throws android.os.RemoteException
    {
    }
    @Override public void collapsePanels() throws android.os.RemoteException
    {
    }
    @Override public void togglePanel() throws android.os.RemoteException
    {
    }
    @Override public void disable(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException
    {
    }
    @Override public void disableForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException
    {
    }
    @Override public void disable2(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException
    {
    }
    @Override public void disable2ForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException
    {
    }
    @Override public int[] getDisableFlags(android.os.IBinder token, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setIcon(java.lang.String slot, java.lang.String iconPackage, int iconId, int iconLevel, java.lang.String contentDescription) throws android.os.RemoteException
    {
    }
    @Override public void setIconVisibility(java.lang.String slot, boolean visible) throws android.os.RemoteException
    {
    }
    @Override public void removeIcon(java.lang.String slot) throws android.os.RemoteException
    {
    }
    @Override public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException
    {
    }
    @Override public void expandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException
    {
    }
    // ---- Methods below are for use by the status bar policy services ----
    // You need the STATUS_BAR_SERVICE permission

    @Override public com.android.internal.statusbar.RegisterStatusBarResult registerStatusBar(com.android.internal.statusbar.IStatusBar callbacks) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void onPanelRevealed(boolean clearNotificationEffects, int numItems) throws android.os.RemoteException
    {
    }
    @Override public void onPanelHidden() throws android.os.RemoteException
    {
    }
    // Mark current notifications as "seen" and stop ringing, vibrating, blinking.

    @Override public void clearNotificationEffects() throws android.os.RemoteException
    {
    }
    @Override public void onNotificationClick(java.lang.String key, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationActionClick(java.lang.String key, int actionIndex, android.app.Notification.Action action, com.android.internal.statusbar.NotificationVisibility nv, boolean generatedByAssistant) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationError(java.lang.String pkg, java.lang.String tag, int id, int uid, int initialPid, java.lang.String message, int userId) throws android.os.RemoteException
    {
    }
    @Override public void onClearAllNotifications(int userId) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationClear(java.lang.String pkg, java.lang.String tag, int id, int userId, java.lang.String key, int dismissalSurface, int dismissalSentiment, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationVisibilityChanged(com.android.internal.statusbar.NotificationVisibility[] newlyVisibleKeys, com.android.internal.statusbar.NotificationVisibility[] noLongerVisibleKeys) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded, int notificationLocation) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationDirectReplied(java.lang.String key) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationSmartSuggestionsAdded(java.lang.String key, int smartReplyCount, int smartActionCount, boolean generatedByAsssistant, boolean editBeforeSending) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationSmartReplySent(java.lang.String key, int replyIndex, java.lang.CharSequence reply, int notificationLocation, boolean modifiedBeforeSending) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationSettingsViewed(java.lang.String key) throws android.os.RemoteException
    {
    }
    @Override public void setSystemUiVisibility(int displayId, int vis, int mask, java.lang.String cause) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationBubbleChanged(java.lang.String key, boolean isBubble) throws android.os.RemoteException
    {
    }
    @Override public void onGlobalActionsShown() throws android.os.RemoteException
    {
    }
    @Override public void onGlobalActionsHidden() throws android.os.RemoteException
    {
    }
    /**
         * These methods are needed for global actions control which the UI is shown in sysui.
         */
    @Override public void shutdown() throws android.os.RemoteException
    {
    }
    @Override public void reboot(boolean safeMode) throws android.os.RemoteException
    {
    }
    @Override public void addTile(android.content.ComponentName tile) throws android.os.RemoteException
    {
    }
    @Override public void remTile(android.content.ComponentName tile) throws android.os.RemoteException
    {
    }
    @Override public void clickTile(android.content.ComponentName tile) throws android.os.RemoteException
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
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.statusbar.IStatusBarService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.statusbar.IStatusBarService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.statusbar.IStatusBarService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.statusbar.IStatusBarService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.statusbar.IStatusBarService))) {
        return ((com.android.internal.statusbar.IStatusBarService)iin);
      }
      return new com.android.internal.statusbar.IStatusBarService.Stub.Proxy(obj);
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
        case TRANSACTION_expandNotificationsPanel:
        {
          data.enforceInterface(descriptor);
          this.expandNotificationsPanel();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_collapsePanels:
        {
          data.enforceInterface(descriptor);
          this.collapsePanels();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_togglePanel:
        {
          data.enforceInterface(descriptor);
          this.togglePanel();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.disable(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.disableForUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disable2:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.disable2(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disable2ForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.disable2ForUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDisableFlags:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int[] _result = this.getDisableFlags(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_setIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.setIcon(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setIconVisibility:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setIconVisibility(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeIcon(_arg0);
          reply.writeNoException();
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
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_expandSettingsPanel:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.expandSettingsPanel(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerStatusBar:
        {
          data.enforceInterface(descriptor);
          com.android.internal.statusbar.IStatusBar _arg0;
          _arg0 = com.android.internal.statusbar.IStatusBar.Stub.asInterface(data.readStrongBinder());
          com.android.internal.statusbar.RegisterStatusBarResult _result = this.registerStatusBar(_arg0);
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
        case TRANSACTION_onPanelRevealed:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onPanelRevealed(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onPanelHidden:
        {
          data.enforceInterface(descriptor);
          this.onPanelHidden();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearNotificationEffects:
        {
          data.enforceInterface(descriptor);
          this.clearNotificationEffects();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationClick:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.statusbar.NotificationVisibility _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.internal.statusbar.NotificationVisibility.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onNotificationClick(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationActionClick:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.app.Notification.Action _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.Notification.Action.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          com.android.internal.statusbar.NotificationVisibility _arg3;
          if ((0!=data.readInt())) {
            _arg3 = com.android.internal.statusbar.NotificationVisibility.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.onNotificationActionClick(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          int _arg6;
          _arg6 = data.readInt();
          this.onNotificationError(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onClearAllNotifications:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onClearAllNotifications(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationClear:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          com.android.internal.statusbar.NotificationVisibility _arg7;
          if ((0!=data.readInt())) {
            _arg7 = com.android.internal.statusbar.NotificationVisibility.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          this.onNotificationClear(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          com.android.internal.statusbar.NotificationVisibility[] _arg0;
          _arg0 = data.createTypedArray(com.android.internal.statusbar.NotificationVisibility.CREATOR);
          com.android.internal.statusbar.NotificationVisibility[] _arg1;
          _arg1 = data.createTypedArray(com.android.internal.statusbar.NotificationVisibility.CREATOR);
          this.onNotificationVisibilityChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationExpansionChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.onNotificationExpansionChanged(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationDirectReplied:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onNotificationDirectReplied(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationSmartSuggestionsAdded:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.onNotificationSmartSuggestionsAdded(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationSmartReplySent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.CharSequence _arg2;
          if (0!=data.readInt()) {
            _arg2 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.onNotificationSmartReplySent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationSettingsViewed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onNotificationSettingsViewed(_arg0);
          reply.writeNoException();
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
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setSystemUiVisibility(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onNotificationBubbleChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onNotificationBubbleChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onGlobalActionsShown:
        {
          data.enforceInterface(descriptor);
          this.onGlobalActionsShown();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onGlobalActionsHidden:
        {
          data.enforceInterface(descriptor);
          this.onGlobalActionsHidden();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          this.shutdown();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reboot:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.reboot(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.addTile(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_remTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.remTile(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clickTile:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.clickTile(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handleSystemKey:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.handleSystemKey(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showPinningEnterExitToast:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.showPinningEnterExitToast(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showPinningEscapeToast:
        {
          data.enforceInterface(descriptor);
          this.showPinningEscapeToast();
          reply.writeNoException();
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
          reply.writeNoException();
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
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onBiometricHelp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onBiometricHelp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onBiometricError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onBiometricError(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hideBiometricDialog:
        {
          data.enforceInterface(descriptor);
          this.hideBiometricDialog();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.statusbar.IStatusBarService
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
      @Override public void expandNotificationsPanel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_expandNotificationsPanel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().expandNotificationsPanel();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void collapsePanels() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_collapsePanels, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().collapsePanels();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void togglePanel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_togglePanel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().togglePanel();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disable(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(what);
          _data.writeStrongBinder(token);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disable(what, token, pkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disableForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(what);
          _data.writeStrongBinder(token);
          _data.writeString(pkg);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableForUser(what, token, pkg, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disable2(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(what);
          _data.writeStrongBinder(token);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disable2, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disable2(what, token, pkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disable2ForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(what);
          _data.writeStrongBinder(token);
          _data.writeString(pkg);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disable2ForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disable2ForUser(what, token, pkg, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getDisableFlags(android.os.IBinder token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisableFlags, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDisableFlags(token, userId);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setIcon(java.lang.String slot, java.lang.String iconPackage, int iconId, int iconLevel, java.lang.String contentDescription) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(slot);
          _data.writeString(iconPackage);
          _data.writeInt(iconId);
          _data.writeInt(iconLevel);
          _data.writeString(contentDescription);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIcon(slot, iconPackage, iconId, iconLevel, contentDescription);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setIconVisibility(java.lang.String slot, boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(slot);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIconVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIconVisibility(slot, visible);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeIcon(java.lang.String slot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(slot);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeIcon(slot);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeStrongBinder(token);
          _data.writeInt(vis);
          _data.writeInt(backDisposition);
          _data.writeInt(((showImeSwitcher)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImeWindowStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImeWindowStatus(displayId, token, vis, backDisposition, showImeSwitcher);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void expandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(subPanel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_expandSettingsPanel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().expandSettingsPanel(subPanel);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // ---- Methods below are for use by the status bar policy services ----
      // You need the STATUS_BAR_SERVICE permission

      @Override public com.android.internal.statusbar.RegisterStatusBarResult registerStatusBar(com.android.internal.statusbar.IStatusBar callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.statusbar.RegisterStatusBarResult _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerStatusBar, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerStatusBar(callbacks);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.statusbar.RegisterStatusBarResult.CREATOR.createFromParcel(_reply);
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
      @Override public void onPanelRevealed(boolean clearNotificationEffects, int numItems) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((clearNotificationEffects)?(1):(0)));
          _data.writeInt(numItems);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPanelRevealed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPanelRevealed(clearNotificationEffects, numItems);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onPanelHidden() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPanelHidden, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPanelHidden();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Mark current notifications as "seen" and stop ringing, vibrating, blinking.

      @Override public void clearNotificationEffects() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearNotificationEffects, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearNotificationEffects();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationClick(java.lang.String key, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          if ((nv!=null)) {
            _data.writeInt(1);
            nv.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationClick, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationClick(key, nv);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationActionClick(java.lang.String key, int actionIndex, android.app.Notification.Action action, com.android.internal.statusbar.NotificationVisibility nv, boolean generatedByAssistant) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(actionIndex);
          if ((action!=null)) {
            _data.writeInt(1);
            action.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((nv!=null)) {
            _data.writeInt(1);
            nv.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((generatedByAssistant)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationActionClick, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationActionClick(key, actionIndex, action, nv, generatedByAssistant);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationError(java.lang.String pkg, java.lang.String tag, int id, int uid, int initialPid, java.lang.String message, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeString(tag);
          _data.writeInt(id);
          _data.writeInt(uid);
          _data.writeInt(initialPid);
          _data.writeString(message);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationError(pkg, tag, id, uid, initialPid, message, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onClearAllNotifications(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onClearAllNotifications, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onClearAllNotifications(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationClear(java.lang.String pkg, java.lang.String tag, int id, int userId, java.lang.String key, int dismissalSurface, int dismissalSentiment, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeString(tag);
          _data.writeInt(id);
          _data.writeInt(userId);
          _data.writeString(key);
          _data.writeInt(dismissalSurface);
          _data.writeInt(dismissalSentiment);
          if ((nv!=null)) {
            _data.writeInt(1);
            nv.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationClear, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationClear(pkg, tag, id, userId, key, dismissalSurface, dismissalSentiment, nv);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationVisibilityChanged(com.android.internal.statusbar.NotificationVisibility[] newlyVisibleKeys, com.android.internal.statusbar.NotificationVisibility[] noLongerVisibleKeys) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(newlyVisibleKeys, 0);
          _data.writeTypedArray(noLongerVisibleKeys, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationVisibilityChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationVisibilityChanged(newlyVisibleKeys, noLongerVisibleKeys);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded, int notificationLocation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(((userAction)?(1):(0)));
          _data.writeInt(((expanded)?(1):(0)));
          _data.writeInt(notificationLocation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationExpansionChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationExpansionChanged(key, userAction, expanded, notificationLocation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationDirectReplied(java.lang.String key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationDirectReplied, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationDirectReplied(key);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationSmartSuggestionsAdded(java.lang.String key, int smartReplyCount, int smartActionCount, boolean generatedByAsssistant, boolean editBeforeSending) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(smartReplyCount);
          _data.writeInt(smartActionCount);
          _data.writeInt(((generatedByAsssistant)?(1):(0)));
          _data.writeInt(((editBeforeSending)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationSmartSuggestionsAdded, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationSmartSuggestionsAdded(key, smartReplyCount, smartActionCount, generatedByAsssistant, editBeforeSending);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationSmartReplySent(java.lang.String key, int replyIndex, java.lang.CharSequence reply, int notificationLocation, boolean modifiedBeforeSending) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(replyIndex);
          if (reply!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(reply, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(notificationLocation);
          _data.writeInt(((modifiedBeforeSending)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationSmartReplySent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationSmartReplySent(key, replyIndex, reply, notificationLocation, modifiedBeforeSending);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationSettingsViewed(java.lang.String key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationSettingsViewed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationSettingsViewed(key);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSystemUiVisibility(int displayId, int vis, int mask, java.lang.String cause) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(vis);
          _data.writeInt(mask);
          _data.writeString(cause);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemUiVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemUiVisibility(displayId, vis, mask, cause);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onNotificationBubbleChanged(java.lang.String key, boolean isBubble) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(((isBubble)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationBubbleChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationBubbleChanged(key, isBubble);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onGlobalActionsShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGlobalActionsShown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGlobalActionsShown();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onGlobalActionsHidden() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGlobalActionsHidden, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGlobalActionsHidden();
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
           * These methods are needed for global actions control which the UI is shown in sysui.
           */
      @Override public void shutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reboot(boolean safeMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((safeMode)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reboot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reboot(safeMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addTile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addTile(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void remTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_remTile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remTile(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clickTile(android.content.ComponentName tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_clickTile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clickTile(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void handleSystemKey(int key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleSystemKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleSystemKey(key);
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
           * Methods to show toast messages for screen pinning
           */
      @Override public void showPinningEnterExitToast(boolean entering) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((entering)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showPinningEnterExitToast, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showPinningEnterExitToast(entering);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showPinningEscapeToast() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showPinningEscapeToast, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showPinningEscapeToast();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used to show the dialog when BiometricService starts authentication

      @Override public void showBiometricDialog(android.os.Bundle bundle, android.hardware.biometrics.IBiometricServiceReceiverInternal receiver, int type, boolean requireConfirmation, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_showBiometricDialog, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showBiometricDialog(bundle, receiver, type, requireConfirmation, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used to hide the dialog when a biometric is authenticated

      @Override public void onBiometricAuthenticated(boolean authenticated, java.lang.String failureReason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((authenticated)?(1):(0)));
          _data.writeString(failureReason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricAuthenticated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricAuthenticated(authenticated, failureReason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used to set a temporary message, e.g. fingerprint not recognized, finger moved too fast, etc

      @Override public void onBiometricHelp(java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricHelp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricHelp(message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used to set a message - the dialog will dismiss after a certain amount of time

      @Override public void onBiometricError(java.lang.String error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBiometricError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBiometricError(error);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used to hide the biometric dialog when the AuthenticationClient is stopped

      @Override public void hideBiometricDialog() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideBiometricDialog, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideBiometricDialog();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.statusbar.IStatusBarService sDefaultImpl;
    }
    static final int TRANSACTION_expandNotificationsPanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_collapsePanels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_togglePanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_disableForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_disable2 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_disable2ForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getDisableFlags = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setIconVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_removeIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setImeWindowStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_expandSettingsPanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_registerStatusBar = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onPanelRevealed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onPanelHidden = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_clearNotificationEffects = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_onNotificationClick = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_onNotificationActionClick = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_onNotificationError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_onClearAllNotifications = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_onNotificationClear = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_onNotificationVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_onNotificationExpansionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_onNotificationDirectReplied = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_onNotificationSmartSuggestionsAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_onNotificationSmartReplySent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_onNotificationSettingsViewed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_setSystemUiVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_onNotificationBubbleChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_onGlobalActionsShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_onGlobalActionsHidden = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_reboot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_addTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_remTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_clickTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_handleSystemKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_showPinningEnterExitToast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_showPinningEscapeToast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_showBiometricDialog = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_onBiometricAuthenticated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_onBiometricHelp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_onBiometricError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_hideBiometricDialog = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    public static boolean setDefaultImpl(com.android.internal.statusbar.IStatusBarService impl) {
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
    public static com.android.internal.statusbar.IStatusBarService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:35:1:35:25")
  public void expandNotificationsPanel() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:37:1:37:25")
  public void collapsePanels() throws android.os.RemoteException;
  public void togglePanel() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:40:1:40:25")
  public void disable(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException;
  public void disableForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException;
  public void disable2(int what, android.os.IBinder token, java.lang.String pkg) throws android.os.RemoteException;
  public void disable2ForUser(int what, android.os.IBinder token, java.lang.String pkg, int userId) throws android.os.RemoteException;
  public int[] getDisableFlags(android.os.IBinder token, int userId) throws android.os.RemoteException;
  public void setIcon(java.lang.String slot, java.lang.String iconPackage, int iconId, int iconLevel, java.lang.String contentDescription) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:47:1:47:25")
  public void setIconVisibility(java.lang.String slot, boolean visible) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:49:1:49:25")
  public void removeIcon(java.lang.String slot) throws android.os.RemoteException;
  public void setImeWindowStatus(int displayId, android.os.IBinder token, int vis, int backDisposition, boolean showImeSwitcher) throws android.os.RemoteException;
  public void expandSettingsPanel(java.lang.String subPanel) throws android.os.RemoteException;
  // ---- Methods below are for use by the status bar policy services ----
  // You need the STATUS_BAR_SERVICE permission

  public com.android.internal.statusbar.RegisterStatusBarResult registerStatusBar(com.android.internal.statusbar.IStatusBar callbacks) throws android.os.RemoteException;
  public void onPanelRevealed(boolean clearNotificationEffects, int numItems) throws android.os.RemoteException;
  public void onPanelHidden() throws android.os.RemoteException;
  // Mark current notifications as "seen" and stop ringing, vibrating, blinking.

  public void clearNotificationEffects() throws android.os.RemoteException;
  public void onNotificationClick(java.lang.String key, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException;
  public void onNotificationActionClick(java.lang.String key, int actionIndex, android.app.Notification.Action action, com.android.internal.statusbar.NotificationVisibility nv, boolean generatedByAssistant) throws android.os.RemoteException;
  public void onNotificationError(java.lang.String pkg, java.lang.String tag, int id, int uid, int initialPid, java.lang.String message, int userId) throws android.os.RemoteException;
  public void onClearAllNotifications(int userId) throws android.os.RemoteException;
  public void onNotificationClear(java.lang.String pkg, java.lang.String tag, int id, int userId, java.lang.String key, int dismissalSurface, int dismissalSentiment, com.android.internal.statusbar.NotificationVisibility nv) throws android.os.RemoteException;
  public void onNotificationVisibilityChanged(com.android.internal.statusbar.NotificationVisibility[] newlyVisibleKeys, com.android.internal.statusbar.NotificationVisibility[] noLongerVisibleKeys) throws android.os.RemoteException;
  public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded, int notificationLocation) throws android.os.RemoteException;
  public void onNotificationDirectReplied(java.lang.String key) throws android.os.RemoteException;
  public void onNotificationSmartSuggestionsAdded(java.lang.String key, int smartReplyCount, int smartActionCount, boolean generatedByAsssistant, boolean editBeforeSending) throws android.os.RemoteException;
  public void onNotificationSmartReplySent(java.lang.String key, int replyIndex, java.lang.CharSequence reply, int notificationLocation, boolean modifiedBeforeSending) throws android.os.RemoteException;
  public void onNotificationSettingsViewed(java.lang.String key) throws android.os.RemoteException;
  public void setSystemUiVisibility(int displayId, int vis, int mask, java.lang.String cause) throws android.os.RemoteException;
  public void onNotificationBubbleChanged(java.lang.String key, boolean isBubble) throws android.os.RemoteException;
  public void onGlobalActionsShown() throws android.os.RemoteException;
  public void onGlobalActionsHidden() throws android.os.RemoteException;
  /**
       * These methods are needed for global actions control which the UI is shown in sysui.
       */
  public void shutdown() throws android.os.RemoteException;
  public void reboot(boolean safeMode) throws android.os.RemoteException;
  public void addTile(android.content.ComponentName tile) throws android.os.RemoteException;
  public void remTile(android.content.ComponentName tile) throws android.os.RemoteException;
  public void clickTile(android.content.ComponentName tile) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/statusbar/IStatusBarService.aidl:93:1:93:25")
  public void handleSystemKey(int key) throws android.os.RemoteException;
  /**
       * Methods to show toast messages for screen pinning
       */
  public void showPinningEnterExitToast(boolean entering) throws android.os.RemoteException;
  public void showPinningEscapeToast() throws android.os.RemoteException;
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
}
