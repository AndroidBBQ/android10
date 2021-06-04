/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms;
/**
 * @hide
 */
public interface IMbmsGroupCallSessionCallback extends android.os.IInterface
{
  /** Default implementation for IMbmsGroupCallSessionCallback. */
  public static class Default implements android.telephony.mbms.IMbmsGroupCallSessionCallback
  {
    @Override public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException
    {
    }
    @Override public void onAvailableSaisUpdated(java.util.List currentSai, java.util.List availableSais) throws android.os.RemoteException
    {
    }
    @Override public void onServiceInterfaceAvailable(java.lang.String interfaceName, int index) throws android.os.RemoteException
    {
    }
    @Override public void onMiddlewareReady() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.IMbmsGroupCallSessionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.IMbmsGroupCallSessionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.IMbmsGroupCallSessionCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.IMbmsGroupCallSessionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.IMbmsGroupCallSessionCallback))) {
        return ((android.telephony.mbms.IMbmsGroupCallSessionCallback)iin);
      }
      return new android.telephony.mbms.IMbmsGroupCallSessionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onError(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onAvailableSaisUpdated:
        {
          data.enforceInterface(descriptor);
          java.util.List _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readArrayList(cl);
          java.util.List _arg1;
          _arg1 = data.readArrayList(cl);
          this.onAvailableSaisUpdated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onServiceInterfaceAvailable:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onServiceInterfaceAvailable(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMiddlewareReady:
        {
          data.enforceInterface(descriptor);
          this.onMiddlewareReady();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.IMbmsGroupCallSessionCallback
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
      @Override public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errorCode);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(errorCode, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAvailableSaisUpdated(java.util.List currentSai, java.util.List availableSais) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeList(currentSai);
          _data.writeList(availableSais);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAvailableSaisUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAvailableSaisUpdated(currentSai, availableSais);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onServiceInterfaceAvailable(java.lang.String interfaceName, int index) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(interfaceName);
          _data.writeInt(index);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onServiceInterfaceAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onServiceInterfaceAvailable(interfaceName, index);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMiddlewareReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMiddlewareReady, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMiddlewareReady();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.mbms.IMbmsGroupCallSessionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAvailableSaisUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onServiceInterfaceAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onMiddlewareReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.telephony.mbms.IMbmsGroupCallSessionCallback impl) {
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
    public static android.telephony.mbms.IMbmsGroupCallSessionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException;
  public void onAvailableSaisUpdated(java.util.List currentSai, java.util.List availableSais) throws android.os.RemoteException;
  public void onServiceInterfaceAvailable(java.lang.String interfaceName, int index) throws android.os.RemoteException;
  public void onMiddlewareReady() throws android.os.RemoteException;
}
