/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.role;
/**
 * @hide
 */
public interface IOnRoleHoldersChangedListener extends android.os.IInterface
{
  /** Default implementation for IOnRoleHoldersChangedListener. */
  public static class Default implements android.app.role.IOnRoleHoldersChangedListener
  {
    @Override public void onRoleHoldersChanged(java.lang.String roleName, int userId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.role.IOnRoleHoldersChangedListener
  {
    private static final java.lang.String DESCRIPTOR = "android.app.role.IOnRoleHoldersChangedListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.role.IOnRoleHoldersChangedListener interface,
     * generating a proxy if needed.
     */
    public static android.app.role.IOnRoleHoldersChangedListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.role.IOnRoleHoldersChangedListener))) {
        return ((android.app.role.IOnRoleHoldersChangedListener)iin);
      }
      return new android.app.role.IOnRoleHoldersChangedListener.Stub.Proxy(obj);
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
        case TRANSACTION_onRoleHoldersChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onRoleHoldersChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.role.IOnRoleHoldersChangedListener
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
      @Override public void onRoleHoldersChanged(java.lang.String roleName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRoleHoldersChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRoleHoldersChanged(roleName, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.role.IOnRoleHoldersChangedListener sDefaultImpl;
    }
    static final int TRANSACTION_onRoleHoldersChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.role.IOnRoleHoldersChangedListener impl) {
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
    public static android.app.role.IOnRoleHoldersChangedListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onRoleHoldersChanged(java.lang.String roleName, int userId) throws android.os.RemoteException;
}
