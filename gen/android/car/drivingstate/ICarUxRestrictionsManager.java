/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.drivingstate;
/**
 * Binder interface for {@link android.car.drivingstate.CarUxRestrictionsManager}.
 * Check {@link android.car.drivingstate.CarUxRestrictionsManager} APIs for expected behavior of
 * each call.
 *
 * @hide
 */
public interface ICarUxRestrictionsManager extends android.os.IInterface
{
  /** Default implementation for ICarUxRestrictionsManager. */
  public static class Default implements android.car.drivingstate.ICarUxRestrictionsManager
  {
    @Override public void registerUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void unregisterUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public android.car.drivingstate.CarUxRestrictions getCurrentUxRestrictions(int displayId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean saveUxRestrictionsConfigurationForNextBoot(java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> configs) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getStagedConfigs() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getConfigs() throws android.os.RemoteException
    {
      return null;
    }
    // 6 removed. Do not use - boolean setRestrictionMode(int mode) = 6;
    // 7 removed. Do not use - int getRestrictionMode() = 7;

    @Override public boolean setRestrictionMode(java.lang.String mode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String getRestrictionMode() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.drivingstate.ICarUxRestrictionsManager
  {
    private static final java.lang.String DESCRIPTOR = "android.car.drivingstate.ICarUxRestrictionsManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.drivingstate.ICarUxRestrictionsManager interface,
     * generating a proxy if needed.
     */
    public static android.car.drivingstate.ICarUxRestrictionsManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.drivingstate.ICarUxRestrictionsManager))) {
        return ((android.car.drivingstate.ICarUxRestrictionsManager)iin);
      }
      return new android.car.drivingstate.ICarUxRestrictionsManager.Stub.Proxy(obj);
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
        case TRANSACTION_registerUxRestrictionsChangeListener:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.ICarUxRestrictionsChangeListener _arg0;
          _arg0 = android.car.drivingstate.ICarUxRestrictionsChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.registerUxRestrictionsChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterUxRestrictionsChangeListener:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.ICarUxRestrictionsChangeListener _arg0;
          _arg0 = android.car.drivingstate.ICarUxRestrictionsChangeListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterUxRestrictionsChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentUxRestrictions:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.drivingstate.CarUxRestrictions _result = this.getCurrentUxRestrictions(_arg0);
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
        case TRANSACTION_saveUxRestrictionsConfigurationForNextBoot:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> _arg0;
          _arg0 = data.createTypedArrayList(android.car.drivingstate.CarUxRestrictionsConfiguration.CREATOR);
          boolean _result = this.saveUxRestrictionsConfigurationForNextBoot(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getStagedConfigs:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> _result = this.getStagedConfigs();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getConfigs:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> _result = this.getConfigs();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setRestrictionMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.setRestrictionMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getRestrictionMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getRestrictionMode();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.drivingstate.ICarUxRestrictionsManager
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
      @Override public void registerUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUxRestrictionsChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerUxRestrictionsChangeListener(listener, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUxRestrictionsChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUxRestrictionsChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.car.drivingstate.CarUxRestrictions getCurrentUxRestrictions(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.drivingstate.CarUxRestrictions _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentUxRestrictions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentUxRestrictions(displayId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.drivingstate.CarUxRestrictions.CREATOR.createFromParcel(_reply);
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
      @Override public boolean saveUxRestrictionsConfigurationForNextBoot(java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> configs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(configs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_saveUxRestrictionsConfigurationForNextBoot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().saveUxRestrictionsConfigurationForNextBoot(configs);
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
      @Override public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getStagedConfigs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStagedConfigs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStagedConfigs();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.drivingstate.CarUxRestrictionsConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getConfigs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigs();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.drivingstate.CarUxRestrictionsConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // 6 removed. Do not use - boolean setRestrictionMode(int mode) = 6;
      // 7 removed. Do not use - int getRestrictionMode() = 7;

      @Override public boolean setRestrictionMode(java.lang.String mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRestrictionMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRestrictionMode(mode);
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
      @Override public java.lang.String getRestrictionMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRestrictionMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRestrictionMode();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.car.drivingstate.ICarUxRestrictionsManager sDefaultImpl;
    }
    static final int TRANSACTION_registerUxRestrictionsChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterUxRestrictionsChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCurrentUxRestrictions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_saveUxRestrictionsConfigurationForNextBoot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getStagedConfigs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getConfigs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setRestrictionMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getRestrictionMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(android.car.drivingstate.ICarUxRestrictionsManager impl) {
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
    public static android.car.drivingstate.ICarUxRestrictionsManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener, int displayId) throws android.os.RemoteException;
  public void unregisterUxRestrictionsChangeListener(android.car.drivingstate.ICarUxRestrictionsChangeListener listener) throws android.os.RemoteException;
  public android.car.drivingstate.CarUxRestrictions getCurrentUxRestrictions(int displayId) throws android.os.RemoteException;
  public boolean saveUxRestrictionsConfigurationForNextBoot(java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> configs) throws android.os.RemoteException;
  public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getStagedConfigs() throws android.os.RemoteException;
  public java.util.List<android.car.drivingstate.CarUxRestrictionsConfiguration> getConfigs() throws android.os.RemoteException;
  // 6 removed. Do not use - boolean setRestrictionMode(int mode) = 6;
  // 7 removed. Do not use - int getRestrictionMode() = 7;

  public boolean setRestrictionMode(java.lang.String mode) throws android.os.RemoteException;
  public java.lang.String getRestrictionMode() throws android.os.RemoteException;
}
