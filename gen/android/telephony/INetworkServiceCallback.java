/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony;
/**
 * Network service call back interface
 * @hide
 */
public interface INetworkServiceCallback extends android.os.IInterface
{
  /** Default implementation for INetworkServiceCallback. */
  public static class Default implements android.telephony.INetworkServiceCallback
  {
    @Override public void onRequestNetworkRegistrationInfoComplete(int result, android.telephony.NetworkRegistrationInfo state) throws android.os.RemoteException
    {
    }
    @Override public void onNetworkStateChanged() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.INetworkServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.INetworkServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.INetworkServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.INetworkServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.INetworkServiceCallback))) {
        return ((android.telephony.INetworkServiceCallback)iin);
      }
      return new android.telephony.INetworkServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onRequestNetworkRegistrationInfoComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.NetworkRegistrationInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.NetworkRegistrationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onRequestNetworkRegistrationInfoComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onNetworkStateChanged:
        {
          data.enforceInterface(descriptor);
          this.onNetworkStateChanged();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.INetworkServiceCallback
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
      @Override public void onRequestNetworkRegistrationInfoComplete(int result, android.telephony.NetworkRegistrationInfo state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRequestNetworkRegistrationInfoComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRequestNetworkRegistrationInfoComplete(result, state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNetworkStateChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNetworkStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNetworkStateChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.INetworkServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onRequestNetworkRegistrationInfoComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onNetworkStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.telephony.INetworkServiceCallback impl) {
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
    public static android.telephony.INetworkServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onRequestNetworkRegistrationInfoComplete(int result, android.telephony.NetworkRegistrationInfo state) throws android.os.RemoteException;
  public void onNetworkStateChanged() throws android.os.RemoteException;
}
