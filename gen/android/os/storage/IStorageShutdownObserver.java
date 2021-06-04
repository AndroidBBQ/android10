/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.storage;
/**
 * Callback class for receiving events related to shutdown.
 *
 * Don't change the existing transaction Ids as they could be used in the native code.
 * When adding a new method, assign the next available transaction id.
 *
 * @hide - For internal consumption only.
 */
public interface IStorageShutdownObserver extends android.os.IInterface
{
  /** Default implementation for IStorageShutdownObserver. */
  public static class Default implements android.os.storage.IStorageShutdownObserver
  {
    /**
         * This method is called when the shutdown of StorageManagerService completed.
         *
         * @param statusCode indicates success or failure of the shutdown.
         */
    @Override public void onShutDownComplete(int statusCode) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.storage.IStorageShutdownObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.os.storage.IStorageShutdownObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.storage.IStorageShutdownObserver interface,
     * generating a proxy if needed.
     */
    public static android.os.storage.IStorageShutdownObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.storage.IStorageShutdownObserver))) {
        return ((android.os.storage.IStorageShutdownObserver)iin);
      }
      return new android.os.storage.IStorageShutdownObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onShutDownComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onShutDownComplete(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.storage.IStorageShutdownObserver
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
           * This method is called when the shutdown of StorageManagerService completed.
           *
           * @param statusCode indicates success or failure of the shutdown.
           */
      @Override public void onShutDownComplete(int statusCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(statusCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShutDownComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShutDownComplete(statusCode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.storage.IStorageShutdownObserver sDefaultImpl;
    }
    static final int TRANSACTION_onShutDownComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.os.storage.IStorageShutdownObserver impl) {
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
    public static android.os.storage.IStorageShutdownObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * This method is called when the shutdown of StorageManagerService completed.
       *
       * @param statusCode indicates success or failure of the shutdown.
       */
  public void onShutDownComplete(int statusCode) throws android.os.RemoteException;
}
