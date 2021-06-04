/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.display;
/** @hide */
public interface IDisplayManager extends android.os.IInterface
{
  /** Default implementation for IDisplayManager. */
  public static class Default implements android.hardware.display.IDisplayManager
  {
    @Override public android.view.DisplayInfo getDisplayInfo(int displayId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int[] getDisplayIds() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isUidPresentOnDisplay(int uid, int displayId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void registerCallback(android.hardware.display.IDisplayManagerCallback callback) throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.
    // The process must have previously registered a callback.

    @Override public void startWifiDisplayScan() throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void stopWifiDisplayScan() throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void connectWifiDisplay(java.lang.String address) throws android.os.RemoteException
    {
    }
    // No permissions required.

    @Override public void disconnectWifiDisplay() throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void renameWifiDisplay(java.lang.String address, java.lang.String alias) throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void forgetWifiDisplay(java.lang.String address) throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void pauseWifiDisplay() throws android.os.RemoteException
    {
    }
    // Requires CONFIGURE_WIFI_DISPLAY permission.

    @Override public void resumeWifiDisplay() throws android.os.RemoteException
    {
    }
    // No permissions required.

    @Override public android.hardware.display.WifiDisplayStatus getWifiDisplayStatus() throws android.os.RemoteException
    {
      return null;
    }
    // Requires CONFIGURE_DISPLAY_COLOR_MODE

    @Override public void requestColorMode(int displayId, int colorMode) throws android.os.RemoteException
    {
    }
    // Requires CAPTURE_VIDEO_OUTPUT, CAPTURE_SECURE_VIDEO_OUTPUT, or an appropriate
    // MediaProjection token for certain combinations of flags.

    @Override public int createVirtualDisplay(android.hardware.display.IVirtualDisplayCallback callback, android.media.projection.IMediaProjection projectionToken, java.lang.String packageName, java.lang.String name, int width, int height, int densityDpi, android.view.Surface surface, int flags, java.lang.String uniqueId) throws android.os.RemoteException
    {
      return 0;
    }
    // No permissions required, but must be same Uid as the creator.

    @Override public void resizeVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token, int width, int height, int densityDpi) throws android.os.RemoteException
    {
    }
    // No permissions required but must be same Uid as the creator.

    @Override public void setVirtualDisplaySurface(android.hardware.display.IVirtualDisplayCallback token, android.view.Surface surface) throws android.os.RemoteException
    {
    }
    // No permissions required but must be same Uid as the creator.

    @Override public void releaseVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token) throws android.os.RemoteException
    {
    }
    // No permissions required but must be same Uid as the creator.

    @Override public void setVirtualDisplayState(android.hardware.display.IVirtualDisplayCallback token, boolean isOn) throws android.os.RemoteException
    {
    }
    // Get a stable metric for the device's display size. No permissions required.

    @Override public android.graphics.Point getStableDisplaySize() throws android.os.RemoteException
    {
      return null;
    }
    // Requires BRIGHTNESS_SLIDER_USAGE permission.

    @Override public android.content.pm.ParceledListSlice getBrightnessEvents(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    // Requires ACCESS_AMBIENT_LIGHT_STATS permission.

    @Override public android.content.pm.ParceledListSlice getAmbientBrightnessStats() throws android.os.RemoteException
    {
      return null;
    }
    // Sets the global brightness configuration for a given user. Requires
    // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user being configured is not
    // the same as the calling user.

    @Override public void setBrightnessConfigurationForUser(android.hardware.display.BrightnessConfiguration c, int userId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    // Gets the global brightness configuration for a given user. Requires
    // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user is not
    // the same as the calling user.

    @Override public android.hardware.display.BrightnessConfiguration getBrightnessConfigurationForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    // Gets the default brightness configuration if configured.

    @Override public android.hardware.display.BrightnessConfiguration getDefaultBrightnessConfiguration() throws android.os.RemoteException
    {
      return null;
    }
    // Temporarily sets the display brightness.

    @Override public void setTemporaryBrightness(int brightness) throws android.os.RemoteException
    {
    }
    // Temporarily sets the auto brightness adjustment factor.

    @Override public void setTemporaryAutoBrightnessAdjustment(float adjustment) throws android.os.RemoteException
    {
    }
    // Get the minimum brightness curve.

    @Override public android.hardware.display.Curve getMinimumBrightnessCurve() throws android.os.RemoteException
    {
      return null;
    }
    // Gets the id of the preferred wide gamut color space for all displays.
    // The wide gamut color space is returned from composition pipeline
    // based on hardware capability.

    @Override public int getPreferredWideGamutColorSpaceId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.display.IDisplayManager
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.display.IDisplayManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.display.IDisplayManager interface,
     * generating a proxy if needed.
     */
    public static android.hardware.display.IDisplayManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.display.IDisplayManager))) {
        return ((android.hardware.display.IDisplayManager)iin);
      }
      return new android.hardware.display.IDisplayManager.Stub.Proxy(obj);
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
        case TRANSACTION_getDisplayInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.DisplayInfo _result = this.getDisplayInfo(_arg0);
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
        case TRANSACTION_getDisplayIds:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getDisplayIds();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_isUidPresentOnDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isUidPresentOnDisplay(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IDisplayManagerCallback _arg0;
          _arg0 = android.hardware.display.IDisplayManagerCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWifiDisplayScan:
        {
          data.enforceInterface(descriptor);
          this.startWifiDisplayScan();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWifiDisplayScan:
        {
          data.enforceInterface(descriptor);
          this.stopWifiDisplayScan();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_connectWifiDisplay:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.connectWifiDisplay(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disconnectWifiDisplay:
        {
          data.enforceInterface(descriptor);
          this.disconnectWifiDisplay();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_renameWifiDisplay:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.renameWifiDisplay(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forgetWifiDisplay:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.forgetWifiDisplay(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_pauseWifiDisplay:
        {
          data.enforceInterface(descriptor);
          this.pauseWifiDisplay();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resumeWifiDisplay:
        {
          data.enforceInterface(descriptor);
          this.resumeWifiDisplay();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWifiDisplayStatus:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.WifiDisplayStatus _result = this.getWifiDisplayStatus();
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
        case TRANSACTION_requestColorMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.requestColorMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createVirtualDisplay:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IVirtualDisplayCallback _arg0;
          _arg0 = android.hardware.display.IVirtualDisplayCallback.Stub.asInterface(data.readStrongBinder());
          android.media.projection.IMediaProjection _arg1;
          _arg1 = android.media.projection.IMediaProjection.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          android.view.Surface _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          int _arg8;
          _arg8 = data.readInt();
          java.lang.String _arg9;
          _arg9 = data.readString();
          int _result = this.createVirtualDisplay(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_resizeVirtualDisplay:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IVirtualDisplayCallback _arg0;
          _arg0 = android.hardware.display.IVirtualDisplayCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.resizeVirtualDisplay(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVirtualDisplaySurface:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IVirtualDisplayCallback _arg0;
          _arg0 = android.hardware.display.IVirtualDisplayCallback.Stub.asInterface(data.readStrongBinder());
          android.view.Surface _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setVirtualDisplaySurface(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseVirtualDisplay:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IVirtualDisplayCallback _arg0;
          _arg0 = android.hardware.display.IVirtualDisplayCallback.Stub.asInterface(data.readStrongBinder());
          this.releaseVirtualDisplay(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVirtualDisplayState:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.IVirtualDisplayCallback _arg0;
          _arg0 = android.hardware.display.IVirtualDisplayCallback.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setVirtualDisplayState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getStableDisplaySize:
        {
          data.enforceInterface(descriptor);
          android.graphics.Point _result = this.getStableDisplaySize();
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
        case TRANSACTION_getBrightnessEvents:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _result = this.getBrightnessEvents(_arg0);
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
        case TRANSACTION_getAmbientBrightnessStats:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _result = this.getAmbientBrightnessStats();
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
        case TRANSACTION_setBrightnessConfigurationForUser:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.BrightnessConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.display.BrightnessConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setBrightnessConfigurationForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getBrightnessConfigurationForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.display.BrightnessConfiguration _result = this.getBrightnessConfigurationForUser(_arg0);
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
        case TRANSACTION_getDefaultBrightnessConfiguration:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.BrightnessConfiguration _result = this.getDefaultBrightnessConfiguration();
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
        case TRANSACTION_setTemporaryBrightness:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setTemporaryBrightness(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTemporaryAutoBrightnessAdjustment:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setTemporaryAutoBrightnessAdjustment(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMinimumBrightnessCurve:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.Curve _result = this.getMinimumBrightnessCurve();
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
        case TRANSACTION_getPreferredWideGamutColorSpaceId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getPreferredWideGamutColorSpaceId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.display.IDisplayManager
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
      @Override public android.view.DisplayInfo getDisplayInfo(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.DisplayInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisplayInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDisplayInfo(displayId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.DisplayInfo.CREATOR.createFromParcel(_reply);
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
      @Override public int[] getDisplayIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisplayIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDisplayIds();
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
      @Override public boolean isUidPresentOnDisplay(int uid, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUidPresentOnDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUidPresentOnDisplay(uid, displayId);
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
      @Override public void registerCallback(android.hardware.display.IDisplayManagerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.
      // The process must have previously registered a callback.

      @Override public void startWifiDisplayScan() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWifiDisplayScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWifiDisplayScan();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void stopWifiDisplayScan() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWifiDisplayScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWifiDisplayScan();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void connectWifiDisplay(java.lang.String address) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          boolean _status = mRemote.transact(Stub.TRANSACTION_connectWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connectWifiDisplay(address);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // No permissions required.

      @Override public void disconnectWifiDisplay() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnectWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnectWifiDisplay();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void renameWifiDisplay(java.lang.String address, java.lang.String alias) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          _data.writeString(alias);
          boolean _status = mRemote.transact(Stub.TRANSACTION_renameWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().renameWifiDisplay(address, alias);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void forgetWifiDisplay(java.lang.String address) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forgetWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forgetWifiDisplay(address);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void pauseWifiDisplay() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pauseWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pauseWifiDisplay();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CONFIGURE_WIFI_DISPLAY permission.

      @Override public void resumeWifiDisplay() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resumeWifiDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resumeWifiDisplay();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // No permissions required.

      @Override public android.hardware.display.WifiDisplayStatus getWifiDisplayStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.WifiDisplayStatus _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiDisplayStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiDisplayStatus();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.WifiDisplayStatus.CREATOR.createFromParcel(_reply);
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
      // Requires CONFIGURE_DISPLAY_COLOR_MODE

      @Override public void requestColorMode(int displayId, int colorMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(colorMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestColorMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestColorMode(displayId, colorMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires CAPTURE_VIDEO_OUTPUT, CAPTURE_SECURE_VIDEO_OUTPUT, or an appropriate
      // MediaProjection token for certain combinations of flags.

      @Override public int createVirtualDisplay(android.hardware.display.IVirtualDisplayCallback callback, android.media.projection.IMediaProjection projectionToken, java.lang.String packageName, java.lang.String name, int width, int height, int densityDpi, android.view.Surface surface, int flags, java.lang.String uniqueId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeStrongBinder((((projectionToken!=null))?(projectionToken.asBinder()):(null)));
          _data.writeString(packageName);
          _data.writeString(name);
          _data.writeInt(width);
          _data.writeInt(height);
          _data.writeInt(densityDpi);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          _data.writeString(uniqueId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createVirtualDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createVirtualDisplay(callback, projectionToken, packageName, name, width, height, densityDpi, surface, flags, uniqueId);
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
      // No permissions required, but must be same Uid as the creator.

      @Override public void resizeVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token, int width, int height, int densityDpi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((token!=null))?(token.asBinder()):(null)));
          _data.writeInt(width);
          _data.writeInt(height);
          _data.writeInt(densityDpi);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resizeVirtualDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resizeVirtualDisplay(token, width, height, densityDpi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // No permissions required but must be same Uid as the creator.

      @Override public void setVirtualDisplaySurface(android.hardware.display.IVirtualDisplayCallback token, android.view.Surface surface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((token!=null))?(token.asBinder()):(null)));
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVirtualDisplaySurface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVirtualDisplaySurface(token, surface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // No permissions required but must be same Uid as the creator.

      @Override public void releaseVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((token!=null))?(token.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseVirtualDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseVirtualDisplay(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // No permissions required but must be same Uid as the creator.

      @Override public void setVirtualDisplayState(android.hardware.display.IVirtualDisplayCallback token, boolean isOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((token!=null))?(token.asBinder()):(null)));
          _data.writeInt(((isOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVirtualDisplayState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVirtualDisplayState(token, isOn);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get a stable metric for the device's display size. No permissions required.

      @Override public android.graphics.Point getStableDisplaySize() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Point _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStableDisplaySize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStableDisplaySize();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Point.CREATOR.createFromParcel(_reply);
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
      // Requires BRIGHTNESS_SLIDER_USAGE permission.

      @Override public android.content.pm.ParceledListSlice getBrightnessEvents(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBrightnessEvents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBrightnessEvents(callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      // Requires ACCESS_AMBIENT_LIGHT_STATS permission.

      @Override public android.content.pm.ParceledListSlice getAmbientBrightnessStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAmbientBrightnessStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAmbientBrightnessStats();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      // Sets the global brightness configuration for a given user. Requires
      // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user being configured is not
      // the same as the calling user.

      @Override public void setBrightnessConfigurationForUser(android.hardware.display.BrightnessConfiguration c, int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((c!=null)) {
            _data.writeInt(1);
            c.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBrightnessConfigurationForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBrightnessConfigurationForUser(c, userId, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Gets the global brightness configuration for a given user. Requires
      // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user is not
      // the same as the calling user.

      @Override public android.hardware.display.BrightnessConfiguration getBrightnessConfigurationForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.BrightnessConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBrightnessConfigurationForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBrightnessConfigurationForUser(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.BrightnessConfiguration.CREATOR.createFromParcel(_reply);
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
      // Gets the default brightness configuration if configured.

      @Override public android.hardware.display.BrightnessConfiguration getDefaultBrightnessConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.BrightnessConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultBrightnessConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultBrightnessConfiguration();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.BrightnessConfiguration.CREATOR.createFromParcel(_reply);
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
      // Temporarily sets the display brightness.

      @Override public void setTemporaryBrightness(int brightness) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(brightness);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTemporaryBrightness, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTemporaryBrightness(brightness);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Temporarily sets the auto brightness adjustment factor.

      @Override public void setTemporaryAutoBrightnessAdjustment(float adjustment) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(adjustment);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTemporaryAutoBrightnessAdjustment, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTemporaryAutoBrightnessAdjustment(adjustment);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get the minimum brightness curve.

      @Override public android.hardware.display.Curve getMinimumBrightnessCurve() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.Curve _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMinimumBrightnessCurve, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMinimumBrightnessCurve();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.Curve.CREATOR.createFromParcel(_reply);
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
      // Gets the id of the preferred wide gamut color space for all displays.
      // The wide gamut color space is returned from composition pipeline
      // based on hardware capability.

      @Override public int getPreferredWideGamutColorSpaceId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreferredWideGamutColorSpaceId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreferredWideGamutColorSpaceId();
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
      public static android.hardware.display.IDisplayManager sDefaultImpl;
    }
    static final int TRANSACTION_getDisplayInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getDisplayIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isUidPresentOnDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startWifiDisplayScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_stopWifiDisplayScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_connectWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_disconnectWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_renameWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_forgetWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_pauseWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_resumeWifiDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getWifiDisplayStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_requestColorMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_createVirtualDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_resizeVirtualDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setVirtualDisplaySurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_releaseVirtualDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setVirtualDisplayState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getStableDisplaySize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getBrightnessEvents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getAmbientBrightnessStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setBrightnessConfigurationForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_getBrightnessConfigurationForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getDefaultBrightnessConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setTemporaryBrightness = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_setTemporaryAutoBrightnessAdjustment = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getMinimumBrightnessCurve = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getPreferredWideGamutColorSpaceId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    public static boolean setDefaultImpl(android.hardware.display.IDisplayManager impl) {
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
    public static android.hardware.display.IDisplayManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/hardware/display/IDisplayManager.aidl:33:1:33:25")
  public android.view.DisplayInfo getDisplayInfo(int displayId) throws android.os.RemoteException;
  public int[] getDisplayIds() throws android.os.RemoteException;
  public boolean isUidPresentOnDisplay(int uid, int displayId) throws android.os.RemoteException;
  public void registerCallback(android.hardware.display.IDisplayManagerCallback callback) throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.
  // The process must have previously registered a callback.

  public void startWifiDisplayScan() throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void stopWifiDisplayScan() throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void connectWifiDisplay(java.lang.String address) throws android.os.RemoteException;
  // No permissions required.

  public void disconnectWifiDisplay() throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void renameWifiDisplay(java.lang.String address, java.lang.String alias) throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void forgetWifiDisplay(java.lang.String address) throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void pauseWifiDisplay() throws android.os.RemoteException;
  // Requires CONFIGURE_WIFI_DISPLAY permission.

  public void resumeWifiDisplay() throws android.os.RemoteException;
  // No permissions required.

  public android.hardware.display.WifiDisplayStatus getWifiDisplayStatus() throws android.os.RemoteException;
  // Requires CONFIGURE_DISPLAY_COLOR_MODE

  public void requestColorMode(int displayId, int colorMode) throws android.os.RemoteException;
  // Requires CAPTURE_VIDEO_OUTPUT, CAPTURE_SECURE_VIDEO_OUTPUT, or an appropriate
  // MediaProjection token for certain combinations of flags.

  public int createVirtualDisplay(android.hardware.display.IVirtualDisplayCallback callback, android.media.projection.IMediaProjection projectionToken, java.lang.String packageName, java.lang.String name, int width, int height, int densityDpi, android.view.Surface surface, int flags, java.lang.String uniqueId) throws android.os.RemoteException;
  // No permissions required, but must be same Uid as the creator.

  public void resizeVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token, int width, int height, int densityDpi) throws android.os.RemoteException;
  // No permissions required but must be same Uid as the creator.

  public void setVirtualDisplaySurface(android.hardware.display.IVirtualDisplayCallback token, android.view.Surface surface) throws android.os.RemoteException;
  // No permissions required but must be same Uid as the creator.

  public void releaseVirtualDisplay(android.hardware.display.IVirtualDisplayCallback token) throws android.os.RemoteException;
  // No permissions required but must be same Uid as the creator.

  public void setVirtualDisplayState(android.hardware.display.IVirtualDisplayCallback token, boolean isOn) throws android.os.RemoteException;
  // Get a stable metric for the device's display size. No permissions required.

  public android.graphics.Point getStableDisplaySize() throws android.os.RemoteException;
  // Requires BRIGHTNESS_SLIDER_USAGE permission.

  public android.content.pm.ParceledListSlice getBrightnessEvents(java.lang.String callingPackage) throws android.os.RemoteException;
  // Requires ACCESS_AMBIENT_LIGHT_STATS permission.

  public android.content.pm.ParceledListSlice getAmbientBrightnessStats() throws android.os.RemoteException;
  // Sets the global brightness configuration for a given user. Requires
  // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user being configured is not
  // the same as the calling user.

  public void setBrightnessConfigurationForUser(android.hardware.display.BrightnessConfiguration c, int userId, java.lang.String packageName) throws android.os.RemoteException;
  // Gets the global brightness configuration for a given user. Requires
  // CONFIGURE_DISPLAY_BRIGHTNESS, and INTERACT_ACROSS_USER if the user is not
  // the same as the calling user.

  public android.hardware.display.BrightnessConfiguration getBrightnessConfigurationForUser(int userId) throws android.os.RemoteException;
  // Gets the default brightness configuration if configured.

  public android.hardware.display.BrightnessConfiguration getDefaultBrightnessConfiguration() throws android.os.RemoteException;
  // Temporarily sets the display brightness.

  public void setTemporaryBrightness(int brightness) throws android.os.RemoteException;
  // Temporarily sets the auto brightness adjustment factor.

  public void setTemporaryAutoBrightnessAdjustment(float adjustment) throws android.os.RemoteException;
  // Get the minimum brightness curve.

  public android.hardware.display.Curve getMinimumBrightnessCurve() throws android.os.RemoteException;
  // Gets the id of the preferred wide gamut color space for all displays.
  // The wide gamut color space is returned from composition pipeline
  // based on hardware capability.

  public int getPreferredWideGamutColorSpaceId() throws android.os.RemoteException;
}
