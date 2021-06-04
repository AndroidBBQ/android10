/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.nfc;
/**
 * {@hide}
 */
public interface INfcDta extends android.os.IInterface
{
  /** Default implementation for INfcDta. */
  public static class Default implements android.nfc.INfcDta
  {
    @Override public void enableDta() throws android.os.RemoteException
    {
    }
    @Override public void disableDta() throws android.os.RemoteException
    {
    }
    @Override public boolean enableServer(java.lang.String serviceName, int serviceSap, int miu, int rwSize, int testCaseId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void disableServer() throws android.os.RemoteException
    {
    }
    @Override public boolean enableClient(java.lang.String serviceName, int miu, int rwSize, int testCaseId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void disableClient() throws android.os.RemoteException
    {
    }
    @Override public boolean registerMessageService(java.lang.String msgServiceName) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.nfc.INfcDta
  {
    private static final java.lang.String DESCRIPTOR = "android.nfc.INfcDta";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.nfc.INfcDta interface,
     * generating a proxy if needed.
     */
    public static android.nfc.INfcDta asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.nfc.INfcDta))) {
        return ((android.nfc.INfcDta)iin);
      }
      return new android.nfc.INfcDta.Stub.Proxy(obj);
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
        case TRANSACTION_enableDta:
        {
          data.enforceInterface(descriptor);
          this.enableDta();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableDta:
        {
          data.enforceInterface(descriptor);
          this.disableDta();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableServer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          boolean _result = this.enableServer(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableServer:
        {
          data.enforceInterface(descriptor);
          this.disableServer();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableClient:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.enableClient(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableClient:
        {
          data.enforceInterface(descriptor);
          this.disableClient();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerMessageService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.registerMessageService(_arg0);
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
    private static class Proxy implements android.nfc.INfcDta
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
      @Override public void enableDta() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableDta, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableDta();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disableDta() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableDta, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableDta();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean enableServer(java.lang.String serviceName, int serviceSap, int miu, int rwSize, int testCaseId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(serviceName);
          _data.writeInt(serviceSap);
          _data.writeInt(miu);
          _data.writeInt(rwSize);
          _data.writeInt(testCaseId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableServer(serviceName, serviceSap, miu, rwSize, testCaseId);
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
      @Override public void disableServer() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableServer();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean enableClient(java.lang.String serviceName, int miu, int rwSize, int testCaseId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(serviceName);
          _data.writeInt(miu);
          _data.writeInt(rwSize);
          _data.writeInt(testCaseId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableClient(serviceName, miu, rwSize, testCaseId);
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
      @Override public void disableClient() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableClient();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean registerMessageService(java.lang.String msgServiceName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(msgServiceName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerMessageService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerMessageService(msgServiceName);
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
      public static android.nfc.INfcDta sDefaultImpl;
    }
    static final int TRANSACTION_enableDta = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_disableDta = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_enableServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disableServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_enableClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_disableClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_registerMessageService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.nfc.INfcDta impl) {
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
    public static android.nfc.INfcDta getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void enableDta() throws android.os.RemoteException;
  public void disableDta() throws android.os.RemoteException;
  public boolean enableServer(java.lang.String serviceName, int serviceSap, int miu, int rwSize, int testCaseId) throws android.os.RemoteException;
  public void disableServer() throws android.os.RemoteException;
  public boolean enableClient(java.lang.String serviceName, int miu, int rwSize, int testCaseId) throws android.os.RemoteException;
  public void disableClient() throws android.os.RemoteException;
  public boolean registerMessageService(java.lang.String msgServiceName) throws android.os.RemoteException;
}
