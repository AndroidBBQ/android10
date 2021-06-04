/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.data;
/**
 * {@hide}
 */
public interface IQualifiedNetworksService extends android.os.IInterface
{
  /** Default implementation for IQualifiedNetworksService. */
  public static class Default implements android.telephony.data.IQualifiedNetworksService
  {
    @Override public void createNetworkAvailabilityProvider(int slotId, android.telephony.data.IQualifiedNetworksServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void removeNetworkAvailabilityProvider(int slotId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.data.IQualifiedNetworksService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.data.IQualifiedNetworksService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.data.IQualifiedNetworksService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.data.IQualifiedNetworksService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.data.IQualifiedNetworksService))) {
        return ((android.telephony.data.IQualifiedNetworksService)iin);
      }
      return new android.telephony.data.IQualifiedNetworksService.Stub.Proxy(obj);
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
        case TRANSACTION_createNetworkAvailabilityProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.IQualifiedNetworksServiceCallback _arg1;
          _arg1 = android.telephony.data.IQualifiedNetworksServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.createNetworkAvailabilityProvider(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeNetworkAvailabilityProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeNetworkAvailabilityProvider(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.data.IQualifiedNetworksService
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
      @Override public void createNetworkAvailabilityProvider(int slotId, android.telephony.data.IQualifiedNetworksServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createNetworkAvailabilityProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createNetworkAvailabilityProvider(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeNetworkAvailabilityProvider(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNetworkAvailabilityProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeNetworkAvailabilityProvider(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.data.IQualifiedNetworksService sDefaultImpl;
    }
    static final int TRANSACTION_createNetworkAvailabilityProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeNetworkAvailabilityProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.telephony.data.IQualifiedNetworksService impl) {
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
    public static android.telephony.data.IQualifiedNetworksService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void createNetworkAvailabilityProvider(int slotId, android.telephony.data.IQualifiedNetworksServiceCallback callback) throws android.os.RemoteException;
  public void removeNetworkAvailabilityProvider(int slotId) throws android.os.RemoteException;
}
