/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * An interface used by the Context Hub Service to invoke callbacks notifying the complete of a
 * transaction. The callbacks are unique for each type of transaction, and the service is
 * responsible for invoking the correct callback.
 *
 * @hide
 */
public interface IContextHubTransactionCallback extends android.os.IInterface
{
  /** Default implementation for IContextHubTransactionCallback. */
  public static class Default implements android.hardware.location.IContextHubTransactionCallback
  {
    // Callback to be invoked when a query request completes

    @Override public void onQueryResponse(int result, java.util.List<android.hardware.location.NanoAppState> nanoappList) throws android.os.RemoteException
    {
    }
    // Callback to be invoked when a non-query request completes

    @Override public void onTransactionComplete(int result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IContextHubTransactionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IContextHubTransactionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IContextHubTransactionCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IContextHubTransactionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IContextHubTransactionCallback))) {
        return ((android.hardware.location.IContextHubTransactionCallback)iin);
      }
      return new android.hardware.location.IContextHubTransactionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onQueryResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.hardware.location.NanoAppState> _arg1;
          _arg1 = data.createTypedArrayList(android.hardware.location.NanoAppState.CREATOR);
          this.onQueryResponse(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onTransactionComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onTransactionComplete(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IContextHubTransactionCallback
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
      // Callback to be invoked when a query request completes

      @Override public void onQueryResponse(int result, java.util.List<android.hardware.location.NanoAppState> nanoappList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          _data.writeTypedList(nanoappList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQueryResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQueryResponse(result, nanoappList);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback to be invoked when a non-query request completes

      @Override public void onTransactionComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTransactionComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTransactionComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.location.IContextHubTransactionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onQueryResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onTransactionComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.hardware.location.IContextHubTransactionCallback impl) {
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
    public static android.hardware.location.IContextHubTransactionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Callback to be invoked when a query request completes

  public void onQueryResponse(int result, java.util.List<android.hardware.location.NanoAppState> nanoappList) throws android.os.RemoteException;
  // Callback to be invoked when a non-query request completes

  public void onTransactionComplete(int result) throws android.os.RemoteException;
}
