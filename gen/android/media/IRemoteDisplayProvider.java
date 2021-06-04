/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * {@hide}
 */
public interface IRemoteDisplayProvider extends android.os.IInterface
{
  /** Default implementation for IRemoteDisplayProvider. */
  public static class Default implements android.media.IRemoteDisplayProvider
  {
    @Override public void setCallback(android.media.IRemoteDisplayCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setDiscoveryMode(int mode) throws android.os.RemoteException
    {
    }
    @Override public void connect(java.lang.String id) throws android.os.RemoteException
    {
    }
    @Override public void disconnect(java.lang.String id) throws android.os.RemoteException
    {
    }
    @Override public void setVolume(java.lang.String id, int volume) throws android.os.RemoteException
    {
    }
    @Override public void adjustVolume(java.lang.String id, int delta) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IRemoteDisplayProvider
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IRemoteDisplayProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IRemoteDisplayProvider interface,
     * generating a proxy if needed.
     */
    public static android.media.IRemoteDisplayProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IRemoteDisplayProvider))) {
        return ((android.media.IRemoteDisplayProvider)iin);
      }
      return new android.media.IRemoteDisplayProvider.Stub.Proxy(obj);
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
        case TRANSACTION_setCallback:
        {
          data.enforceInterface(descriptor);
          android.media.IRemoteDisplayCallback _arg0;
          _arg0 = android.media.IRemoteDisplayCallback.Stub.asInterface(data.readStrongBinder());
          this.setCallback(_arg0);
          return true;
        }
        case TRANSACTION_setDiscoveryMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDiscoveryMode(_arg0);
          return true;
        }
        case TRANSACTION_connect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.connect(_arg0);
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.disconnect(_arg0);
          return true;
        }
        case TRANSACTION_setVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setVolume(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_adjustVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.adjustVolume(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IRemoteDisplayProvider
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
      @Override public void setCallback(android.media.IRemoteDisplayCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDiscoveryMode(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDiscoveryMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDiscoveryMode(mode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void connect(java.lang.String id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_connect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connect(id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disconnect(java.lang.String id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect(id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setVolume(java.lang.String id, int volume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          _data.writeInt(volume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolume(id, volume);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void adjustVolume(java.lang.String id, int delta) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          _data.writeInt(delta);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adjustVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adjustVolume(id, delta);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IRemoteDisplayProvider sDefaultImpl;
    }
    static final int TRANSACTION_setCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setDiscoveryMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_adjustVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.media.IRemoteDisplayProvider impl) {
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
    public static android.media.IRemoteDisplayProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setCallback(android.media.IRemoteDisplayCallback callback) throws android.os.RemoteException;
  public void setDiscoveryMode(int mode) throws android.os.RemoteException;
  public void connect(java.lang.String id) throws android.os.RemoteException;
  public void disconnect(java.lang.String id) throws android.os.RemoteException;
  public void setVolume(java.lang.String id, int volume) throws android.os.RemoteException;
  public void adjustVolume(java.lang.String id, int delta) throws android.os.RemoteException;
}
