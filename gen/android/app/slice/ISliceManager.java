/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.slice;
/** @hide */
public interface ISliceManager extends android.os.IInterface
{
  /** Default implementation for ISliceManager. */
  public static class Default implements android.app.slice.ISliceManager
  {
    @Override public void pinSlice(java.lang.String pkg, android.net.Uri uri, android.app.slice.SliceSpec[] specs, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void unpinSlice(java.lang.String pkg, android.net.Uri uri, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public boolean hasSliceAccess(java.lang.String pkg) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.app.slice.SliceSpec[] getPinnedSpecs(android.net.Uri uri, java.lang.String pkg) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.Uri[] getPinnedSlices(java.lang.String pkg) throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] getBackupPayload(int user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void applyRestore(byte[] payload, int user) throws android.os.RemoteException
    {
    }
    // Perms.

    @Override public void grantSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException
    {
    }
    @Override public void revokeSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException
    {
    }
    @Override public int checkSlicePermission(android.net.Uri uri, java.lang.String callingPkg, java.lang.String pkg, int pid, int uid, java.lang.String[] autoGrantPermissions) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void grantPermissionFromUser(android.net.Uri uri, java.lang.String pkg, java.lang.String callingPkg, boolean allSlices) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.slice.ISliceManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.slice.ISliceManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.slice.ISliceManager interface,
     * generating a proxy if needed.
     */
    public static android.app.slice.ISliceManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.slice.ISliceManager))) {
        return ((android.app.slice.ISliceManager)iin);
      }
      return new android.app.slice.ISliceManager.Stub.Proxy(obj);
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
        case TRANSACTION_pinSlice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.slice.SliceSpec[] _arg2;
          _arg2 = data.createTypedArray(android.app.slice.SliceSpec.CREATOR);
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          this.pinSlice(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unpinSlice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          this.unpinSlice(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasSliceAccess:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.hasSliceAccess(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPinnedSpecs:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.app.slice.SliceSpec[] _result = this.getPinnedSpecs(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getPinnedSlices:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri[] _result = this.getPinnedSlices(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getBackupPayload:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _result = this.getBackupPayload(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_applyRestore:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          this.applyRestore(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_grantSlicePermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.grantSlicePermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_revokeSlicePermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.revokeSlicePermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkSlicePermission:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String[] _arg5;
          _arg5 = data.createStringArray();
          int _result = this.checkSlicePermission(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_grantPermissionFromUser:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.grantPermissionFromUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.slice.ISliceManager
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
      @Override public void pinSlice(java.lang.String pkg, android.net.Uri uri, android.app.slice.SliceSpec[] specs, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedArray(specs, 0);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pinSlice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pinSlice(pkg, uri, specs, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unpinSlice(java.lang.String pkg, android.net.Uri uri, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unpinSlice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unpinSlice(pkg, uri, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean hasSliceAccess(java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasSliceAccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasSliceAccess(pkg);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.app.slice.SliceSpec[] getPinnedSpecs(android.net.Uri uri, java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.slice.SliceSpec[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPinnedSpecs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPinnedSpecs(uri, pkg);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.app.slice.SliceSpec.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.Uri[] getPinnedSlices(java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPinnedSlices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPinnedSlices(pkg);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.Uri.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public byte[] getBackupPayload(int user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(user);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBackupPayload, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBackupPayload(user);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void applyRestore(byte[] payload, int user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(payload);
          _data.writeInt(user);
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyRestore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyRestore(payload, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Perms.

      @Override public void grantSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          _data.writeString(toPkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantSlicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantSlicePermission(callingPkg, toPkg, uri);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void revokeSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          _data.writeString(toPkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeSlicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeSlicePermission(callingPkg, toPkg, uri);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int checkSlicePermission(android.net.Uri uri, java.lang.String callingPkg, java.lang.String pkg, int pid, int uid, java.lang.String[] autoGrantPermissions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPkg);
          _data.writeString(pkg);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStringArray(autoGrantPermissions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkSlicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkSlicePermission(uri, callingPkg, pkg, pid, uid, autoGrantPermissions);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void grantPermissionFromUser(android.net.Uri uri, java.lang.String pkg, java.lang.String callingPkg, boolean allSlices) throws android.os.RemoteException
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
          _data.writeString(pkg);
          _data.writeString(callingPkg);
          _data.writeInt(((allSlices)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantPermissionFromUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantPermissionFromUser(uri, pkg, callingPkg, allSlices);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.slice.ISliceManager sDefaultImpl;
    }
    static final int TRANSACTION_pinSlice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unpinSlice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_hasSliceAccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getPinnedSpecs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getPinnedSlices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getBackupPayload = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_applyRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_grantSlicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_revokeSlicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_checkSlicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_grantPermissionFromUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.app.slice.ISliceManager impl) {
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
    public static android.app.slice.ISliceManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void pinSlice(java.lang.String pkg, android.net.Uri uri, android.app.slice.SliceSpec[] specs, android.os.IBinder token) throws android.os.RemoteException;
  public void unpinSlice(java.lang.String pkg, android.net.Uri uri, android.os.IBinder token) throws android.os.RemoteException;
  public boolean hasSliceAccess(java.lang.String pkg) throws android.os.RemoteException;
  public android.app.slice.SliceSpec[] getPinnedSpecs(android.net.Uri uri, java.lang.String pkg) throws android.os.RemoteException;
  public android.net.Uri[] getPinnedSlices(java.lang.String pkg) throws android.os.RemoteException;
  public byte[] getBackupPayload(int user) throws android.os.RemoteException;
  public void applyRestore(byte[] payload, int user) throws android.os.RemoteException;
  // Perms.

  public void grantSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException;
  public void revokeSlicePermission(java.lang.String callingPkg, java.lang.String toPkg, android.net.Uri uri) throws android.os.RemoteException;
  public int checkSlicePermission(android.net.Uri uri, java.lang.String callingPkg, java.lang.String pkg, int pid, int uid, java.lang.String[] autoGrantPermissions) throws android.os.RemoteException;
  public void grantPermissionFromUser(android.net.Uri uri, java.lang.String pkg, java.lang.String callingPkg, boolean allSlices) throws android.os.RemoteException;
}
