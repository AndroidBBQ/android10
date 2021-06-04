/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * Provides APIs to get/set the IMS service feature/capability/parameters.
 * The config items include items provisioned by the operator.
 *
 * {@hide}
 */
public interface IImsConfig extends android.os.IInterface
{
  /** Default implementation for IImsConfig. */
  public static class Default implements android.telephony.ims.aidl.IImsConfig
  {
    @Override public void addImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException
    {
    }
    @Override public void removeImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException
    {
    }
    @Override public int getConfigInt(int item) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getConfigString(int item) throws android.os.RemoteException
    {
      return null;
    }
    // Return result code defined in ImsConfig#OperationStatusConstants

    @Override public int setConfigInt(int item, int value) throws android.os.RemoteException
    {
      return 0;
    }
    // Return result code defined in ImsConfig#OperationStatusConstants

    @Override public int setConfigString(int item, java.lang.String value) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsConfig
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsConfig";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsConfig interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsConfig asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsConfig))) {
        return ((android.telephony.ims.aidl.IImsConfig)iin);
      }
      return new android.telephony.ims.aidl.IImsConfig.Stub.Proxy(obj);
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
        case TRANSACTION_addImsConfigCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsConfigCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsConfigCallback.Stub.asInterface(data.readStrongBinder());
          this.addImsConfigCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeImsConfigCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsConfigCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsConfigCallback.Stub.asInterface(data.readStrongBinder());
          this.removeImsConfigCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getConfigInt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getConfigInt(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getConfigString:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getConfigString(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setConfigInt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.setConfigInt(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setConfigString:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.setConfigString(_arg0, _arg1);
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
    private static class Proxy implements android.telephony.ims.aidl.IImsConfig
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
      @Override public void addImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addImsConfigCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addImsConfigCallback(c);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeImsConfigCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeImsConfigCallback(c);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getConfigInt(int item) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigInt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigInt(item);
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
      @Override public java.lang.String getConfigString(int item) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigString(item);
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
      // Return result code defined in ImsConfig#OperationStatusConstants

      @Override public int setConfigInt(int item, int value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          _data.writeInt(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConfigInt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setConfigInt(item, value);
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
      // Return result code defined in ImsConfig#OperationStatusConstants

      @Override public int setConfigString(int item, java.lang.String value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          _data.writeString(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConfigString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setConfigString(item, value);
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
      public static android.telephony.ims.aidl.IImsConfig sDefaultImpl;
    }
    static final int TRANSACTION_addImsConfigCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeImsConfigCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getConfigInt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getConfigString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setConfigInt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setConfigString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsConfig impl) {
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
    public static android.telephony.ims.aidl.IImsConfig getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException;
  public void removeImsConfigCallback(android.telephony.ims.aidl.IImsConfigCallback c) throws android.os.RemoteException;
  public int getConfigInt(int item) throws android.os.RemoteException;
  public java.lang.String getConfigString(int item) throws android.os.RemoteException;
  // Return result code defined in ImsConfig#OperationStatusConstants

  public int setConfigInt(int item, int value) throws android.os.RemoteException;
  // Return result code defined in ImsConfig#OperationStatusConstants

  public int setConfigString(int item, java.lang.String value) throws android.os.RemoteException;
}
