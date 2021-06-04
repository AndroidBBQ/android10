/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.storage;
/**
 * Callback class for receiving events from StorageManagerService about Opaque Binary
 * Blobs (OBBs).
 *
 * Don't change the existing transaction Ids as they could be used in the native code.
 * When adding a new method, assign the next available transaction id.
 *
 * @hide - Applications should use StorageManager to interact with OBBs.
 */
public interface IObbActionListener extends android.os.IInterface
{
  /** Default implementation for IObbActionListener. */
  public static class Default implements android.os.storage.IObbActionListener
  {
    /**
         * Return from an OBB action result.
         *
         * @param filename the path to the OBB the operation was performed on
         * @param nonce identifier that is meaningful to the receiver
         * @param status status code as defined in {@link OnObbStateChangeListener}
         */
    @Override public void onObbResult(java.lang.String filename, int nonce, int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.storage.IObbActionListener
  {
    private static final java.lang.String DESCRIPTOR = "android.os.storage.IObbActionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.storage.IObbActionListener interface,
     * generating a proxy if needed.
     */
    public static android.os.storage.IObbActionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.storage.IObbActionListener))) {
        return ((android.os.storage.IObbActionListener)iin);
      }
      return new android.os.storage.IObbActionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onObbResult:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onObbResult(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.storage.IObbActionListener
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
           * Return from an OBB action result.
           *
           * @param filename the path to the OBB the operation was performed on
           * @param nonce identifier that is meaningful to the receiver
           * @param status status code as defined in {@link OnObbStateChangeListener}
           */
      @Override public void onObbResult(java.lang.String filename, int nonce, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(filename);
          _data.writeInt(nonce);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onObbResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onObbResult(filename, nonce, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.storage.IObbActionListener sDefaultImpl;
    }
    static final int TRANSACTION_onObbResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.os.storage.IObbActionListener impl) {
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
    public static android.os.storage.IObbActionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Return from an OBB action result.
       *
       * @param filename the path to the OBB the operation was performed on
       * @param nonce identifier that is meaningful to the receiver
       * @param status status code as defined in {@link OnObbStateChangeListener}
       */
  public void onObbResult(java.lang.String filename, int nonce, int status) throws android.os.RemoteException;
}
