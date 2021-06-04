/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm.permission;
/**
 * Interface for communication with the permission presenter service.
 *
 * @hide
 * @deprecated Only available to keep
 *             android.permissionpresenterservice.RuntimePermissionPresenterService functional
 */
public interface IRuntimePermissionPresenter extends android.os.IInterface
{
  /** Default implementation for IRuntimePermissionPresenter. */
  public static class Default implements android.content.pm.permission.IRuntimePermissionPresenter
  {
    @Override public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.permission.IRuntimePermissionPresenter
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.permission.IRuntimePermissionPresenter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.permission.IRuntimePermissionPresenter interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.permission.IRuntimePermissionPresenter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.permission.IRuntimePermissionPresenter))) {
        return ((android.content.pm.permission.IRuntimePermissionPresenter)iin);
      }
      return new android.content.pm.permission.IRuntimePermissionPresenter.Stub.Proxy(obj);
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
        case TRANSACTION_getAppPermissions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.RemoteCallback _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.getAppPermissions(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.permission.IRuntimePermissionPresenter
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
      @Override public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppPermissions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAppPermissions(packageName, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.pm.permission.IRuntimePermissionPresenter sDefaultImpl;
    }
    static final int TRANSACTION_getAppPermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.content.pm.permission.IRuntimePermissionPresenter impl) {
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
    public static android.content.pm.permission.IRuntimePermissionPresenter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException;
}
