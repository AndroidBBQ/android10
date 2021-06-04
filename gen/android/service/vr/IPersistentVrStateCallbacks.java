/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.vr;
/** @hide */
public interface IPersistentVrStateCallbacks extends android.os.IInterface
{
  /** Default implementation for IPersistentVrStateCallbacks. */
  public static class Default implements android.service.vr.IPersistentVrStateCallbacks
  {
    @Override public void onPersistentVrStateChanged(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.vr.IPersistentVrStateCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "android.service.vr.IPersistentVrStateCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.vr.IPersistentVrStateCallbacks interface,
     * generating a proxy if needed.
     */
    public static android.service.vr.IPersistentVrStateCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.vr.IPersistentVrStateCallbacks))) {
        return ((android.service.vr.IPersistentVrStateCallbacks)iin);
      }
      return new android.service.vr.IPersistentVrStateCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_onPersistentVrStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onPersistentVrStateChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.vr.IPersistentVrStateCallbacks
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
      @Override public void onPersistentVrStateChanged(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPersistentVrStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPersistentVrStateChanged(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.vr.IPersistentVrStateCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_onPersistentVrStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.vr.IPersistentVrStateCallbacks impl) {
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
    public static android.service.vr.IPersistentVrStateCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onPersistentVrStateChanged(boolean enabled) throws android.os.RemoteException;
}
