/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for Traffic state callback.
 *
 * @hide
 */
public interface ITrafficStateCallback extends android.os.IInterface
{
  /** Default implementation for ITrafficStateCallback. */
  public static class Default implements android.net.wifi.ITrafficStateCallback
  {
    /**
        * Callback invoked to inform clients about the current traffic state.
        *
        * @param state One of the values: {@link #DATA_ACTIVITY_NONE}, {@link #DATA_ACTIVITY_IN},
        * {@link #DATA_ACTIVITY_OUT} & {@link #DATA_ACTIVITY_INOUT}.
        * @hide
        */
    @Override public void onStateChanged(int state) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.ITrafficStateCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.ITrafficStateCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.ITrafficStateCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.ITrafficStateCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.ITrafficStateCallback))) {
        return ((android.net.wifi.ITrafficStateCallback)iin);
      }
      return new android.net.wifi.ITrafficStateCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onStateChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.ITrafficStateCallback
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
      /**
          * Callback invoked to inform clients about the current traffic state.
          *
          * @param state One of the values: {@link #DATA_ACTIVITY_NONE}, {@link #DATA_ACTIVITY_IN},
          * {@link #DATA_ACTIVITY_OUT} & {@link #DATA_ACTIVITY_INOUT}.
          * @hide
          */
      @Override public void onStateChanged(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStateChanged(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.ITrafficStateCallback sDefaultImpl;
    }
    static final int TRANSACTION_onStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.net.wifi.ITrafficStateCallback impl) {
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
    public static android.net.wifi.ITrafficStateCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
      * Callback invoked to inform clients about the current traffic state.
      *
      * @param state One of the values: {@link #DATA_ACTIVITY_NONE}, {@link #DATA_ACTIVITY_IN},
      * {@link #DATA_ACTIVITY_OUT} & {@link #DATA_ACTIVITY_INOUT}.
      * @hide
      */
  public void onStateChanged(int state) throws android.os.RemoteException;
}
