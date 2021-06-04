/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** {@hide} */
public interface IUserSwitchObserver extends android.os.IInterface
{
  /** Default implementation for IUserSwitchObserver. */
  public static class Default implements android.app.IUserSwitchObserver
  {
    @Override public void onUserSwitching(int newUserId, android.os.IRemoteCallback reply) throws android.os.RemoteException
    {
    }
    @Override public void onUserSwitchComplete(int newUserId) throws android.os.RemoteException
    {
    }
    @Override public void onForegroundProfileSwitch(int newProfileId) throws android.os.RemoteException
    {
    }
    @Override public void onLockedBootComplete(int newUserId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IUserSwitchObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IUserSwitchObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IUserSwitchObserver interface,
     * generating a proxy if needed.
     */
    public static android.app.IUserSwitchObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IUserSwitchObserver))) {
        return ((android.app.IUserSwitchObserver)iin);
      }
      return new android.app.IUserSwitchObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onUserSwitching:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IRemoteCallback _arg1;
          _arg1 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          this.onUserSwitching(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onUserSwitchComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onUserSwitchComplete(_arg0);
          return true;
        }
        case TRANSACTION_onForegroundProfileSwitch:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onForegroundProfileSwitch(_arg0);
          return true;
        }
        case TRANSACTION_onLockedBootComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onLockedBootComplete(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IUserSwitchObserver
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
      @Override public void onUserSwitching(int newUserId, android.os.IRemoteCallback reply) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newUserId);
          _data.writeStrongBinder((((reply!=null))?(reply.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserSwitching, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserSwitching(newUserId, reply);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUserSwitchComplete(int newUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newUserId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserSwitchComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserSwitchComplete(newUserId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onForegroundProfileSwitch(int newProfileId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newProfileId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onForegroundProfileSwitch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onForegroundProfileSwitch(newProfileId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLockedBootComplete(int newUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newUserId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLockedBootComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLockedBootComplete(newUserId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IUserSwitchObserver sDefaultImpl;
    }
    static final int TRANSACTION_onUserSwitching = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onUserSwitchComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onForegroundProfileSwitch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onLockedBootComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.app.IUserSwitchObserver impl) {
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
    public static android.app.IUserSwitchObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUserSwitching(int newUserId, android.os.IRemoteCallback reply) throws android.os.RemoteException;
  public void onUserSwitchComplete(int newUserId) throws android.os.RemoteException;
  public void onForegroundProfileSwitch(int newProfileId) throws android.os.RemoteException;
  public void onLockedBootComplete(int newUserId) throws android.os.RemoteException;
}
