/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/** @hide */
public interface IAppFocusOwnershipCallback extends android.os.IInterface
{
  /** Default implementation for IAppFocusOwnershipCallback. */
  public static class Default implements android.car.IAppFocusOwnershipCallback
  {
    @Override public void onAppFocusOwnershipLost(int appType) throws android.os.RemoteException
    {
    }
    @Override public void onAppFocusOwnershipGranted(int appType) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.IAppFocusOwnershipCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.IAppFocusOwnershipCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.IAppFocusOwnershipCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.IAppFocusOwnershipCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.IAppFocusOwnershipCallback))) {
        return ((android.car.IAppFocusOwnershipCallback)iin);
      }
      return new android.car.IAppFocusOwnershipCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onAppFocusOwnershipLost:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onAppFocusOwnershipLost(_arg0);
          return true;
        }
        case TRANSACTION_onAppFocusOwnershipGranted:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onAppFocusOwnershipGranted(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.IAppFocusOwnershipCallback
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
      @Override public void onAppFocusOwnershipLost(int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAppFocusOwnershipLost, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAppFocusOwnershipLost(appType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAppFocusOwnershipGranted(int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAppFocusOwnershipGranted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAppFocusOwnershipGranted(appType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.IAppFocusOwnershipCallback sDefaultImpl;
    }
    static final int TRANSACTION_onAppFocusOwnershipLost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAppFocusOwnershipGranted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.IAppFocusOwnershipCallback impl) {
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
    public static android.car.IAppFocusOwnershipCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onAppFocusOwnershipLost(int appType) throws android.os.RemoteException;
  public void onAppFocusOwnershipGranted(int appType) throws android.os.RemoteException;
}
