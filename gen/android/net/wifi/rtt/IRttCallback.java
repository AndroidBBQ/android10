/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.rtt;
/**
 * Interface for RTT result callback.
 *
 * @hide
 */
public interface IRttCallback extends android.os.IInterface
{
  /** Default implementation for IRttCallback. */
  public static class Default implements android.net.wifi.rtt.IRttCallback
  {
    /**
         * Service to manager callback indicating failure.
         */
    @Override public void onRangingFailure(int status) throws android.os.RemoteException
    {
    }
    /**
         * Service to manager callback indicating success and providing results.
         */
    @Override public void onRangingResults(java.util.List<android.net.wifi.rtt.RangingResult> results) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.rtt.IRttCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.rtt.IRttCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.rtt.IRttCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.rtt.IRttCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.rtt.IRttCallback))) {
        return ((android.net.wifi.rtt.IRttCallback)iin);
      }
      return new android.net.wifi.rtt.IRttCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onRangingFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onRangingFailure(_arg0);
          return true;
        }
        case TRANSACTION_onRangingResults:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.rtt.RangingResult> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.rtt.RangingResult.CREATOR);
          this.onRangingResults(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.rtt.IRttCallback
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
           * Service to manager callback indicating failure.
           */
      @Override public void onRangingFailure(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRangingFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRangingFailure(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Service to manager callback indicating success and providing results.
           */
      @Override public void onRangingResults(java.util.List<android.net.wifi.rtt.RangingResult> results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(results);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRangingResults, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRangingResults(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.rtt.IRttCallback sDefaultImpl;
    }
    static final int TRANSACTION_onRangingFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onRangingResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.wifi.rtt.IRttCallback impl) {
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
    public static android.net.wifi.rtt.IRttCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Service to manager callback indicating failure.
       */
  public void onRangingFailure(int status) throws android.os.RemoteException;
  /**
       * Service to manager callback indicating success and providing results.
       */
  public void onRangingResults(java.util.List<android.net.wifi.rtt.RangingResult> results) throws android.os.RemoteException;
}
