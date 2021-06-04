/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony;
/**
 * {@hide}
 */
public interface INetworkService extends android.os.IInterface
{
  /** Default implementation for INetworkService. */
  public static class Default implements android.telephony.INetworkService
  {
    @Override public void createNetworkServiceProvider(int slotId) throws android.os.RemoteException
    {
    }
    @Override public void removeNetworkServiceProvider(int slotId) throws android.os.RemoteException
    {
    }
    @Override public void requestNetworkRegistrationInfo(int slotId, int domain, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void registerForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.INetworkService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.INetworkService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.INetworkService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.INetworkService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.INetworkService))) {
        return ((android.telephony.INetworkService)iin);
      }
      return new android.telephony.INetworkService.Stub.Proxy(obj);
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
        case TRANSACTION_createNetworkServiceProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.createNetworkServiceProvider(_arg0);
          return true;
        }
        case TRANSACTION_removeNetworkServiceProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeNetworkServiceProvider(_arg0);
          return true;
        }
        case TRANSACTION_requestNetworkRegistrationInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.INetworkServiceCallback _arg2;
          _arg2 = android.telephony.INetworkServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.requestNetworkRegistrationInfo(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_registerForNetworkRegistrationInfoChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.INetworkServiceCallback _arg1;
          _arg1 = android.telephony.INetworkServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.registerForNetworkRegistrationInfoChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unregisterForNetworkRegistrationInfoChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.INetworkServiceCallback _arg1;
          _arg1 = android.telephony.INetworkServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterForNetworkRegistrationInfoChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.INetworkService
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
      @Override public void createNetworkServiceProvider(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createNetworkServiceProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createNetworkServiceProvider(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeNetworkServiceProvider(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNetworkServiceProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeNetworkServiceProvider(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestNetworkRegistrationInfo(int slotId, int domain, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(domain);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNetworkRegistrationInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestNetworkRegistrationInfo(slotId, domain, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void registerForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerForNetworkRegistrationInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerForNetworkRegistrationInfoChanged(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterForNetworkRegistrationInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterForNetworkRegistrationInfoChanged(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.INetworkService sDefaultImpl;
    }
    static final int TRANSACTION_createNetworkServiceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeNetworkServiceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestNetworkRegistrationInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_registerForNetworkRegistrationInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_unregisterForNetworkRegistrationInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.telephony.INetworkService impl) {
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
    public static android.telephony.INetworkService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void createNetworkServiceProvider(int slotId) throws android.os.RemoteException;
  public void removeNetworkServiceProvider(int slotId) throws android.os.RemoteException;
  public void requestNetworkRegistrationInfo(int slotId, int domain, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException;
  public void registerForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException;
  public void unregisterForNetworkRegistrationInfoChanged(int slotId, android.telephony.INetworkServiceCallback callback) throws android.os.RemoteException;
}
