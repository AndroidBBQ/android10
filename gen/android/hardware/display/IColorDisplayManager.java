/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.display;
/** @hide */
public interface IColorDisplayManager extends android.os.IInterface
{
  /** Default implementation for IColorDisplayManager. */
  public static class Default implements android.hardware.display.IColorDisplayManager
  {
    @Override public boolean isDeviceColorManaged() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setSaturationLevel(int saturationLevel) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAppSaturationLevel(java.lang.String packageName, int saturationLevel) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isSaturationActivated() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getTransformCapabilities() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isNightDisplayActivated() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setNightDisplayActivated(boolean activated) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getNightDisplayColorTemperature() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean setNightDisplayColorTemperature(int temperature) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getNightDisplayAutoMode() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getNightDisplayAutoModeRaw() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean setNightDisplayAutoMode(int autoMode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.hardware.display.Time getNightDisplayCustomStartTime() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setNightDisplayCustomStartTime(android.hardware.display.Time time) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.hardware.display.Time getNightDisplayCustomEndTime() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setNightDisplayCustomEndTime(android.hardware.display.Time time) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getColorMode() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setColorMode(int colorMode) throws android.os.RemoteException
    {
    }
    @Override public boolean isDisplayWhiteBalanceEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setDisplayWhiteBalanceEnabled(boolean enabled) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.display.IColorDisplayManager
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.display.IColorDisplayManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.display.IColorDisplayManager interface,
     * generating a proxy if needed.
     */
    public static android.hardware.display.IColorDisplayManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.display.IColorDisplayManager))) {
        return ((android.hardware.display.IColorDisplayManager)iin);
      }
      return new android.hardware.display.IColorDisplayManager.Stub.Proxy(obj);
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
        case TRANSACTION_isDeviceColorManaged:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDeviceColorManaged();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setSaturationLevel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.setSaturationLevel(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAppSaturationLevel:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setAppSaturationLevel(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSaturationActivated:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSaturationActivated();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getTransformCapabilities:
        {
          data.enforceInterface(descriptor);
          int _result = this.getTransformCapabilities();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isNightDisplayActivated:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isNightDisplayActivated();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setNightDisplayActivated:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setNightDisplayActivated(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNightDisplayColorTemperature:
        {
          data.enforceInterface(descriptor);
          int _result = this.getNightDisplayColorTemperature();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setNightDisplayColorTemperature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.setNightDisplayColorTemperature(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNightDisplayAutoMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getNightDisplayAutoMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getNightDisplayAutoModeRaw:
        {
          data.enforceInterface(descriptor);
          int _result = this.getNightDisplayAutoModeRaw();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setNightDisplayAutoMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.setNightDisplayAutoMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNightDisplayCustomStartTime:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.Time _result = this.getNightDisplayCustomStartTime();
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
        case TRANSACTION_setNightDisplayCustomStartTime:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.Time _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.display.Time.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.setNightDisplayCustomStartTime(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNightDisplayCustomEndTime:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.Time _result = this.getNightDisplayCustomEndTime();
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
        case TRANSACTION_setNightDisplayCustomEndTime:
        {
          data.enforceInterface(descriptor);
          android.hardware.display.Time _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.display.Time.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.setNightDisplayCustomEndTime(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getColorMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getColorMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setColorMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setColorMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDisplayWhiteBalanceEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDisplayWhiteBalanceEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDisplayWhiteBalanceEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setDisplayWhiteBalanceEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.display.IColorDisplayManager
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
      @Override public boolean isDeviceColorManaged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDeviceColorManaged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDeviceColorManaged();
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
      @Override public boolean setSaturationLevel(int saturationLevel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(saturationLevel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSaturationLevel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setSaturationLevel(saturationLevel);
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
      @Override public boolean setAppSaturationLevel(java.lang.String packageName, int saturationLevel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(saturationLevel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppSaturationLevel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAppSaturationLevel(packageName, saturationLevel);
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
      @Override public boolean isSaturationActivated() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSaturationActivated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSaturationActivated();
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
      @Override public int getTransformCapabilities() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTransformCapabilities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTransformCapabilities();
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
      @Override public boolean isNightDisplayActivated() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNightDisplayActivated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNightDisplayActivated();
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
      @Override public boolean setNightDisplayActivated(boolean activated) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((activated)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightDisplayActivated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightDisplayActivated(activated);
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
      @Override public int getNightDisplayColorTemperature() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightDisplayColorTemperature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightDisplayColorTemperature();
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
      @Override public boolean setNightDisplayColorTemperature(int temperature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(temperature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightDisplayColorTemperature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightDisplayColorTemperature(temperature);
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
      @Override public int getNightDisplayAutoMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightDisplayAutoMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightDisplayAutoMode();
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
      @Override public int getNightDisplayAutoModeRaw() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightDisplayAutoModeRaw, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightDisplayAutoModeRaw();
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
      @Override public boolean setNightDisplayAutoMode(int autoMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(autoMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightDisplayAutoMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightDisplayAutoMode(autoMode);
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
      @Override public android.hardware.display.Time getNightDisplayCustomStartTime() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.Time _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightDisplayCustomStartTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightDisplayCustomStartTime();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.Time.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setNightDisplayCustomStartTime(android.hardware.display.Time time) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((time!=null)) {
            _data.writeInt(1);
            time.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightDisplayCustomStartTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightDisplayCustomStartTime(time);
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
      @Override public android.hardware.display.Time getNightDisplayCustomEndTime() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.display.Time _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightDisplayCustomEndTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightDisplayCustomEndTime();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.display.Time.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setNightDisplayCustomEndTime(android.hardware.display.Time time) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((time!=null)) {
            _data.writeInt(1);
            time.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightDisplayCustomEndTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightDisplayCustomEndTime(time);
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
      @Override public int getColorMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getColorMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getColorMode();
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
      @Override public void setColorMode(int colorMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(colorMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setColorMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setColorMode(colorMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isDisplayWhiteBalanceEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDisplayWhiteBalanceEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDisplayWhiteBalanceEnabled();
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
      @Override public boolean setDisplayWhiteBalanceEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplayWhiteBalanceEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDisplayWhiteBalanceEnabled(enabled);
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
      public static android.hardware.display.IColorDisplayManager sDefaultImpl;
    }
    static final int TRANSACTION_isDeviceColorManaged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setSaturationLevel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setAppSaturationLevel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isSaturationActivated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getTransformCapabilities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isNightDisplayActivated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setNightDisplayActivated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getNightDisplayColorTemperature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setNightDisplayColorTemperature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getNightDisplayAutoMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getNightDisplayAutoModeRaw = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setNightDisplayAutoMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getNightDisplayCustomStartTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setNightDisplayCustomStartTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getNightDisplayCustomEndTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setNightDisplayCustomEndTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getColorMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setColorMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_isDisplayWhiteBalanceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setDisplayWhiteBalanceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    public static boolean setDefaultImpl(android.hardware.display.IColorDisplayManager impl) {
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
    public static android.hardware.display.IColorDisplayManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isDeviceColorManaged() throws android.os.RemoteException;
  public boolean setSaturationLevel(int saturationLevel) throws android.os.RemoteException;
  public boolean setAppSaturationLevel(java.lang.String packageName, int saturationLevel) throws android.os.RemoteException;
  public boolean isSaturationActivated() throws android.os.RemoteException;
  public int getTransformCapabilities() throws android.os.RemoteException;
  public boolean isNightDisplayActivated() throws android.os.RemoteException;
  public boolean setNightDisplayActivated(boolean activated) throws android.os.RemoteException;
  public int getNightDisplayColorTemperature() throws android.os.RemoteException;
  public boolean setNightDisplayColorTemperature(int temperature) throws android.os.RemoteException;
  public int getNightDisplayAutoMode() throws android.os.RemoteException;
  public int getNightDisplayAutoModeRaw() throws android.os.RemoteException;
  public boolean setNightDisplayAutoMode(int autoMode) throws android.os.RemoteException;
  public android.hardware.display.Time getNightDisplayCustomStartTime() throws android.os.RemoteException;
  public boolean setNightDisplayCustomStartTime(android.hardware.display.Time time) throws android.os.RemoteException;
  public android.hardware.display.Time getNightDisplayCustomEndTime() throws android.os.RemoteException;
  public boolean setNightDisplayCustomEndTime(android.hardware.display.Time time) throws android.os.RemoteException;
  public int getColorMode() throws android.os.RemoteException;
  public void setColorMode(int colorMode) throws android.os.RemoteException;
  public boolean isDisplayWhiteBalanceEnabled() throws android.os.RemoteException;
  public boolean setDisplayWhiteBalanceEnabled(boolean enabled) throws android.os.RemoteException;
}
