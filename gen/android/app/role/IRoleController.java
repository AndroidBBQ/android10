/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.role;
/**
 * @hide
 */
public interface IRoleController extends android.os.IInterface
{
  /** Default implementation for IRoleController. */
  public static class Default implements android.app.role.IRoleController
  {
    @Override public void grantDefaultRoles(android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onAddRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onRemoveRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onClearRoleHolders(java.lang.String roleName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void isApplicationQualifiedForRole(java.lang.String roleName, java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void isRoleVisible(java.lang.String roleName, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.role.IRoleController
  {
    private static final java.lang.String DESCRIPTOR = "android.app.role.IRoleController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.role.IRoleController interface,
     * generating a proxy if needed.
     */
    public static android.app.role.IRoleController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.role.IRoleController))) {
        return ((android.app.role.IRoleController)iin);
      }
      return new android.app.role.IRoleController.Stub.Proxy(obj);
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
        case TRANSACTION_grantDefaultRoles:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.grantDefaultRoles(_arg0);
          return true;
        }
        case TRANSACTION_onAddRoleHolder:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.onAddRoleHolder(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onRemoveRoleHolder:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.onRemoveRoleHolder(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onClearRoleHolders:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onClearRoleHolders(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_isApplicationQualifiedForRole:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.isApplicationQualifiedForRole(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_isRoleVisible:
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
          this.isRoleVisible(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.role.IRoleController
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
      @Override public void grantDefaultRoles(android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantDefaultRoles, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantDefaultRoles(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAddRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAddRoleHolder, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAddRoleHolder(roleName, packageName, flags, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRemoveRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRemoveRoleHolder, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRemoveRoleHolder(roleName, packageName, flags, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onClearRoleHolders(java.lang.String roleName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onClearRoleHolders, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onClearRoleHolders(roleName, flags, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isApplicationQualifiedForRole(java.lang.String roleName, java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isApplicationQualifiedForRole, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isApplicationQualifiedForRole(roleName, packageName, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isRoleVisible(java.lang.String roleName, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRoleVisible, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isRoleVisible(roleName, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.role.IRoleController sDefaultImpl;
    }
    static final int TRANSACTION_grantDefaultRoles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAddRoleHolder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onRemoveRoleHolder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onClearRoleHolders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isApplicationQualifiedForRole = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isRoleVisible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.app.role.IRoleController impl) {
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
    public static android.app.role.IRoleController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void grantDefaultRoles(android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void onAddRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void onRemoveRoleHolder(java.lang.String roleName, java.lang.String packageName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void onClearRoleHolders(java.lang.String roleName, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void isApplicationQualifiedForRole(java.lang.String roleName, java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void isRoleVisible(java.lang.String roleName, android.os.RemoteCallback callback) throws android.os.RemoteException;
}
