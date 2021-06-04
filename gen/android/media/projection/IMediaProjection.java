/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.projection;
/** {@hide} */
public interface IMediaProjection extends android.os.IInterface
{
  /** Default implementation for IMediaProjection. */
  public static class Default implements android.media.projection.IMediaProjection
  {
    @Override public void start(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void stop() throws android.os.RemoteException
    {
    }
    @Override public boolean canProjectAudio() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean canProjectVideo() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean canProjectSecureVideo() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int applyVirtualDisplayFlags(int flags) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void registerCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.projection.IMediaProjection
  {
    private static final java.lang.String DESCRIPTOR = "android.media.projection.IMediaProjection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.projection.IMediaProjection interface,
     * generating a proxy if needed.
     */
    public static android.media.projection.IMediaProjection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.projection.IMediaProjection))) {
        return ((android.media.projection.IMediaProjection)iin);
      }
      return new android.media.projection.IMediaProjection.Stub.Proxy(obj);
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
        case TRANSACTION_start:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjectionCallback _arg0;
          _arg0 = android.media.projection.IMediaProjectionCallback.Stub.asInterface(data.readStrongBinder());
          this.start(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stop:
        {
          data.enforceInterface(descriptor);
          this.stop();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_canProjectAudio:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.canProjectAudio();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_canProjectVideo:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.canProjectVideo();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_canProjectSecureVideo:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.canProjectSecureVideo();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_applyVirtualDisplayFlags:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.applyVirtualDisplayFlags(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjectionCallback _arg0;
          _arg0 = android.media.projection.IMediaProjectionCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          android.media.projection.IMediaProjectionCallback _arg0;
          _arg0 = android.media.projection.IMediaProjectionCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.projection.IMediaProjection
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
      @Override public void start(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_start, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().start(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stop() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stop, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stop();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean canProjectAudio() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canProjectAudio, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canProjectAudio();
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
      @Override public boolean canProjectVideo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canProjectVideo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canProjectVideo();
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
      @Override public boolean canProjectSecureVideo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canProjectSecureVideo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canProjectSecureVideo();
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
      @Override public int applyVirtualDisplayFlags(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyVirtualDisplayFlags, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().applyVirtualDisplayFlags(flags);
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
      @Override public void registerCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.projection.IMediaProjection sDefaultImpl;
    }
    static final int TRANSACTION_start = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_canProjectAudio = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_canProjectVideo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_canProjectSecureVideo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_applyVirtualDisplayFlags = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.media.projection.IMediaProjection impl) {
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
    public static android.media.projection.IMediaProjection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void start(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException;
  public void stop() throws android.os.RemoteException;
  public boolean canProjectAudio() throws android.os.RemoteException;
  public boolean canProjectVideo() throws android.os.RemoteException;
  public boolean canProjectSecureVideo() throws android.os.RemoteException;
  public int applyVirtualDisplayFlags(int flags) throws android.os.RemoteException;
  public void registerCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException;
  public void unregisterCallback(android.media.projection.IMediaProjectionCallback callback) throws android.os.RemoteException;
}
