/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.projection;
/** {@hide} */
public interface IMediaProjectionManager extends android.os.IInterface
{
  /** Default implementation for IMediaProjectionManager. */
  public static class Default implements android.media.projection.IMediaProjectionManager
  {
    @Override public boolean hasProjectionPermission(int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.media.projection.IMediaProjection createProjection(int uid, java.lang.String packageName, int type, boolean permanentGrant) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isValidMediaProjection(android.media.projection.IMediaProjection projection) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.media.projection.MediaProjectionInfo getActiveProjectionInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void stopActiveProjection() throws android.os.RemoteException
    {
    }
    @Override public void addCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void removeCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.projection.IMediaProjectionManager
  {
    private static final java.lang.String DESCRIPTOR = "android.media.projection.IMediaProjectionManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.projection.IMediaProjectionManager interface,
     * generating a proxy if needed.
     */
    public static android.media.projection.IMediaProjectionManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.projection.IMediaProjectionManager))) {
        return ((android.media.projection.IMediaProjectionManager)iin);
      }
      return new android.media.projection.IMediaProjectionManager.Stub.Proxy(obj);
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
        case TRANSACTION_hasProjectionPermission:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.hasProjectionPermission(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createProjection:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.media.projection.IMediaProjection _result = this.createProjection(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_isValidMediaProjection:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjection _arg0;
          _arg0 = android.media.projection.IMediaProjection.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isValidMediaProjection(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getActiveProjectionInfo:
        {
          data.enforceInterface(descriptor);
          android.media.projection.MediaProjectionInfo _result = this.getActiveProjectionInfo();
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
        case TRANSACTION_stopActiveProjection:
        {
          data.enforceInterface(descriptor);
          this.stopActiveProjection();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addCallback:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjectionWatcherCallback _arg0;
          _arg0 = android.media.projection.IMediaProjectionWatcherCallback.Stub.asInterface(data.readStrongBinder());
          this.addCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeCallback:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjectionWatcherCallback _arg0;
          _arg0 = android.media.projection.IMediaProjectionWatcherCallback.Stub.asInterface(data.readStrongBinder());
          this.removeCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.projection.IMediaProjectionManager
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
      @Override public boolean hasProjectionPermission(int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasProjectionPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasProjectionPermission(uid, packageName);
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
      @Override public android.media.projection.IMediaProjection createProjection(int uid, java.lang.String packageName, int type, boolean permanentGrant) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.projection.IMediaProjection _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeInt(type);
          _data.writeInt(((permanentGrant)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createProjection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createProjection(uid, packageName, type, permanentGrant);
          }
          _reply.readException();
          _result = android.media.projection.IMediaProjection.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean isValidMediaProjection(android.media.projection.IMediaProjection projection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((projection!=null))?(projection.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isValidMediaProjection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isValidMediaProjection(projection);
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
      @Override public android.media.projection.MediaProjectionInfo getActiveProjectionInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.projection.MediaProjectionInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveProjectionInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveProjectionInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.projection.MediaProjectionInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void stopActiveProjection() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopActiveProjection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopActiveProjection();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.projection.IMediaProjectionManager sDefaultImpl;
    }
    static final int TRANSACTION_hasProjectionPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_createProjection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isValidMediaProjection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getActiveProjectionInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_stopActiveProjection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.media.projection.IMediaProjectionManager impl) {
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
    public static android.media.projection.IMediaProjectionManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/projection/IMediaProjectionManager.aidl:27:1:27:25")
  public boolean hasProjectionPermission(int uid, java.lang.String packageName) throws android.os.RemoteException;
  public android.media.projection.IMediaProjection createProjection(int uid, java.lang.String packageName, int type, boolean permanentGrant) throws android.os.RemoteException;
  public boolean isValidMediaProjection(android.media.projection.IMediaProjection projection) throws android.os.RemoteException;
  public android.media.projection.MediaProjectionInfo getActiveProjectionInfo() throws android.os.RemoteException;
  public void stopActiveProjection() throws android.os.RemoteException;
  public void addCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException;
  public void removeCallback(android.media.projection.IMediaProjectionWatcherCallback callback) throws android.os.RemoteException;
}
