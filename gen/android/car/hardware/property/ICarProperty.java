/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.hardware.property;
/**
 * @hide
 */
public interface ICarProperty extends android.os.IInterface
{
  /** Default implementation for ICarProperty. */
  public static class Default implements android.car.hardware.property.ICarProperty
  {
    @Override public void registerListener(int propId, float rate, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterListener(int propId, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.car.hardware.CarPropertyConfig> getPropertyList() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.car.hardware.CarPropertyValue getProperty(int prop, int zone) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setProperty(android.car.hardware.CarPropertyValue prop) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getReadPermission(int propId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getWritePermission(int propId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.hardware.property.ICarProperty
  {
    private static final java.lang.String DESCRIPTOR = "android.car.hardware.property.ICarProperty";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.hardware.property.ICarProperty interface,
     * generating a proxy if needed.
     */
    public static android.car.hardware.property.ICarProperty asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.hardware.property.ICarProperty))) {
        return ((android.car.hardware.property.ICarProperty)iin);
      }
      return new android.car.hardware.property.ICarProperty.Stub.Proxy(obj);
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
        case TRANSACTION_registerListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _arg1;
          _arg1 = data.readFloat();
          android.car.hardware.property.ICarPropertyEventListener _arg2;
          _arg2 = android.car.hardware.property.ICarPropertyEventListener.Stub.asInterface(data.readStrongBinder());
          this.registerListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.hardware.property.ICarPropertyEventListener _arg1;
          _arg1 = android.car.hardware.property.ICarPropertyEventListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPropertyList:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.hardware.CarPropertyConfig> _result = this.getPropertyList();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getProperty:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.car.hardware.CarPropertyValue _result = this.getProperty(_arg0, _arg1);
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
        case TRANSACTION_setProperty:
        {
          data.enforceInterface(descriptor);
          android.car.hardware.CarPropertyValue _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.hardware.CarPropertyValue.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setProperty(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getReadPermission:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getReadPermission(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getWritePermission:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getWritePermission(_arg0);
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
    private static class Proxy implements android.car.hardware.property.ICarProperty
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
      @Override public void registerListener(int propId, float rate, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(propId);
          _data.writeFloat(rate);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerListener(propId, rate, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterListener(int propId, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(propId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterListener(propId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.car.hardware.CarPropertyConfig> getPropertyList() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.hardware.CarPropertyConfig> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPropertyList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPropertyList();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.hardware.CarPropertyConfig.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.car.hardware.CarPropertyValue getProperty(int prop, int zone) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.hardware.CarPropertyValue _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(prop);
          _data.writeInt(zone);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProperty, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProperty(prop, zone);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.hardware.CarPropertyValue.CREATOR.createFromParcel(_reply);
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
      @Override public void setProperty(android.car.hardware.CarPropertyValue prop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((prop!=null)) {
            _data.writeInt(1);
            prop.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProperty, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProperty(prop);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getReadPermission(int propId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(propId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getReadPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getReadPermission(propId);
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
      @Override public java.lang.String getWritePermission(int propId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(propId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWritePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWritePermission(propId);
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
      public static android.car.hardware.property.ICarProperty sDefaultImpl;
    }
    static final int TRANSACTION_registerListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getPropertyList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getProperty = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setProperty = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getReadPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getWritePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.car.hardware.property.ICarProperty impl) {
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
    public static android.car.hardware.property.ICarProperty getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerListener(int propId, float rate, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException;
  public void unregisterListener(int propId, android.car.hardware.property.ICarPropertyEventListener callback) throws android.os.RemoteException;
  public java.util.List<android.car.hardware.CarPropertyConfig> getPropertyList() throws android.os.RemoteException;
  public android.car.hardware.CarPropertyValue getProperty(int prop, int zone) throws android.os.RemoteException;
  public void setProperty(android.car.hardware.CarPropertyValue prop) throws android.os.RemoteException;
  public java.lang.String getReadPermission(int propId) throws android.os.RemoteException;
  public java.lang.String getWritePermission(int propId) throws android.os.RemoteException;
}
