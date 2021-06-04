/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * {@hide}
 */
public interface IBatteryPropertiesRegistrar extends android.os.IInterface
{
  /** Default implementation for IBatteryPropertiesRegistrar. */
  public static class Default implements android.os.IBatteryPropertiesRegistrar
  {
    @Override public int getProperty(int id, android.os.BatteryProperty prop) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void scheduleUpdate() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IBatteryPropertiesRegistrar
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IBatteryPropertiesRegistrar";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IBatteryPropertiesRegistrar interface,
     * generating a proxy if needed.
     */
    public static android.os.IBatteryPropertiesRegistrar asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IBatteryPropertiesRegistrar))) {
        return ((android.os.IBatteryPropertiesRegistrar)iin);
      }
      return new android.os.IBatteryPropertiesRegistrar.Stub.Proxy(obj);
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
        case TRANSACTION_getProperty:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.BatteryProperty _arg1;
          _arg1 = new android.os.BatteryProperty();
          int _result = this.getProperty(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_scheduleUpdate:
        {
          data.enforceInterface(descriptor);
          this.scheduleUpdate();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IBatteryPropertiesRegistrar
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
      @Override public int getProperty(int id, android.os.BatteryProperty prop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProperty, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProperty(id, prop);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            prop.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void scheduleUpdate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleUpdate();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.IBatteryPropertiesRegistrar sDefaultImpl;
    }
    static final int TRANSACTION_getProperty = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_scheduleUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.IBatteryPropertiesRegistrar impl) {
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
    public static android.os.IBatteryPropertiesRegistrar getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int getProperty(int id, android.os.BatteryProperty prop) throws android.os.RemoteException;
  public void scheduleUpdate() throws android.os.RemoteException;
}
