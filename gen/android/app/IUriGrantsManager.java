/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * Interface for managing an app's permission to access a particular URI.
 * {@hide}
 */
public interface IUriGrantsManager extends android.os.IInterface
{
  /** Default implementation for IUriGrantsManager. */
  public static class Default implements android.app.IUriGrantsManager
  {
    @Override public void takePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException
    {
    }
    @Override public void releasePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException
    {
    }
    @Override public void grantUriPermissionFromOwner(android.os.IBinder owner, int fromUid, java.lang.String targetPkg, android.net.Uri uri, int mode, int sourceUserId, int targetUserId) throws android.os.RemoteException
    {
    }
    /**
         * Gets the URI permissions granted to an arbitrary package (or all packages if null)
         * NOTE: this is different from getUriPermissions(), which returns the URIs the package
         * granted to another packages (instead of those granted to it).
         */
    @Override public android.content.pm.ParceledListSlice getGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /** Clears the URI permissions granted to an arbitrary package. */
    @Override public void clearGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getUriPermissions(java.lang.String packageName, boolean incoming, boolean persistedOnly) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IUriGrantsManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IUriGrantsManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IUriGrantsManager interface,
     * generating a proxy if needed.
     */
    public static android.app.IUriGrantsManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IUriGrantsManager))) {
        return ((android.app.IUriGrantsManager)iin);
      }
      return new android.app.IUriGrantsManager.Stub.Proxy(obj);
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
        case TRANSACTION_takePersistableUriPermission:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.takePersistableUriPermission(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releasePersistableUriPermission:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.releasePersistableUriPermission(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_grantUriPermissionFromOwner:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.net.Uri _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          this.grantUriPermissionFromOwner(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGrantedUriPermissions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getGrantedUriPermissions(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_clearGrantedUriPermissions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.clearGrantedUriPermissions(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUriPermissions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.content.pm.ParceledListSlice _result = this.getUriPermissions(_arg0, _arg1, _arg2);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IUriGrantsManager
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
      @Override public void takePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(modeFlags);
          _data.writeString(toPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_takePersistableUriPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().takePersistableUriPermission(uri, modeFlags, toPackage, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releasePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(modeFlags);
          _data.writeString(toPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releasePersistableUriPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releasePersistableUriPermission(uri, modeFlags, toPackage, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void grantUriPermissionFromOwner(android.os.IBinder owner, int fromUid, java.lang.String targetPkg, android.net.Uri uri, int mode, int sourceUserId, int targetUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(owner);
          _data.writeInt(fromUid);
          _data.writeString(targetPkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(mode);
          _data.writeInt(sourceUserId);
          _data.writeInt(targetUserId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantUriPermissionFromOwner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantUriPermissionFromOwner(owner, fromUid, targetPkg, uri, mode, sourceUserId, targetUserId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Gets the URI permissions granted to an arbitrary package (or all packages if null)
           * NOTE: this is different from getUriPermissions(), which returns the URIs the package
           * granted to another packages (instead of those granted to it).
           */
      @Override public android.content.pm.ParceledListSlice getGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGrantedUriPermissions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGrantedUriPermissions(packageName, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Clears the URI permissions granted to an arbitrary package. */
      @Override public void clearGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearGrantedUriPermissions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearGrantedUriPermissions(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getUriPermissions(java.lang.String packageName, boolean incoming, boolean persistedOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((incoming)?(1):(0)));
          _data.writeInt(((persistedOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUriPermissions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUriPermissions(packageName, incoming, persistedOnly);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.app.IUriGrantsManager sDefaultImpl;
    }
    static final int TRANSACTION_takePersistableUriPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_releasePersistableUriPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_grantUriPermissionFromOwner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getGrantedUriPermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_clearGrantedUriPermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getUriPermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.app.IUriGrantsManager impl) {
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
    public static android.app.IUriGrantsManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void takePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException;
  public void releasePersistableUriPermission(android.net.Uri uri, int modeFlags, java.lang.String toPackage, int userId) throws android.os.RemoteException;
  public void grantUriPermissionFromOwner(android.os.IBinder owner, int fromUid, java.lang.String targetPkg, android.net.Uri uri, int mode, int sourceUserId, int targetUserId) throws android.os.RemoteException;
  /**
       * Gets the URI permissions granted to an arbitrary package (or all packages if null)
       * NOTE: this is different from getUriPermissions(), which returns the URIs the package
       * granted to another packages (instead of those granted to it).
       */
  public android.content.pm.ParceledListSlice getGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /** Clears the URI permissions granted to an arbitrary package. */
  public void clearGrantedUriPermissions(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getUriPermissions(java.lang.String packageName, boolean incoming, boolean persistedOnly) throws android.os.RemoteException;
}
