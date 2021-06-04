/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.cluster;
/**
 * API to communicate between {@link CarInstrumentClusterManager} and Car Service.
 *
 * @deprecated CarInstrumentClusterManager is deprecated
 * @hide
 */
public interface IInstrumentClusterManagerService extends android.os.IInterface
{
  /** Default implementation for IInstrumentClusterManagerService. */
  public static class Default implements android.car.cluster.IInstrumentClusterManagerService
  {
    @Override public void startClusterActivity(android.content.Intent intent) throws android.os.RemoteException
    {
    }
    @Override public void registerCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.cluster.IInstrumentClusterManagerService
  {
    private static final java.lang.String DESCRIPTOR = "android.car.cluster.IInstrumentClusterManagerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.cluster.IInstrumentClusterManagerService interface,
     * generating a proxy if needed.
     */
    public static android.car.cluster.IInstrumentClusterManagerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.cluster.IInstrumentClusterManagerService))) {
        return ((android.car.cluster.IInstrumentClusterManagerService)iin);
      }
      return new android.car.cluster.IInstrumentClusterManagerService.Stub.Proxy(obj);
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
        case TRANSACTION_startClusterActivity:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startClusterActivity(_arg0);
          return true;
        }
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.car.cluster.IInstrumentClusterManagerCallback _arg0;
          _arg0 = android.car.cluster.IInstrumentClusterManagerCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCallback(_arg0);
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          android.car.cluster.IInstrumentClusterManagerCallback _arg0;
          _arg0 = android.car.cluster.IInstrumentClusterManagerCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterCallback(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.cluster.IInstrumentClusterManagerService
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
      @Override public void startClusterActivity(android.content.Intent intent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startClusterActivity, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startClusterActivity(intent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void registerCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.cluster.IInstrumentClusterManagerService sDefaultImpl;
    }
    static final int TRANSACTION_startClusterActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.car.cluster.IInstrumentClusterManagerService impl) {
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
    public static android.car.cluster.IInstrumentClusterManagerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startClusterActivity(android.content.Intent intent) throws android.os.RemoteException;
  public void registerCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException;
  public void unregisterCallback(android.car.cluster.IInstrumentClusterManagerCallback callback) throws android.os.RemoteException;
}
