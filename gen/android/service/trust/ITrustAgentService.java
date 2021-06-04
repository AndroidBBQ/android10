/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.trust;
/**
 * Communication channel from TrustManagerService to the TrustAgent.
 * @hide
 */
public interface ITrustAgentService extends android.os.IInterface
{
  /** Default implementation for ITrustAgentService. */
  public static class Default implements android.service.trust.ITrustAgentService
  {
    @Override public void onUnlockAttempt(boolean successful) throws android.os.RemoteException
    {
    }
    @Override public void onUnlockLockout(int timeoutMs) throws android.os.RemoteException
    {
    }
    @Override public void onTrustTimeout() throws android.os.RemoteException
    {
    }
    @Override public void onDeviceLocked() throws android.os.RemoteException
    {
    }
    @Override public void onDeviceUnlocked() throws android.os.RemoteException
    {
    }
    @Override public void onConfigure(java.util.List<android.os.PersistableBundle> options, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void setCallback(android.service.trust.ITrustAgentServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onEscrowTokenAdded(byte[] token, long handle, android.os.UserHandle user) throws android.os.RemoteException
    {
    }
    @Override public void onTokenStateReceived(long handle, int tokenState) throws android.os.RemoteException
    {
    }
    @Override public void onEscrowTokenRemoved(long handle, boolean successful) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.trust.ITrustAgentService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.trust.ITrustAgentService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.trust.ITrustAgentService interface,
     * generating a proxy if needed.
     */
    public static android.service.trust.ITrustAgentService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.trust.ITrustAgentService))) {
        return ((android.service.trust.ITrustAgentService)iin);
      }
      return new android.service.trust.ITrustAgentService.Stub.Proxy(obj);
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
        case TRANSACTION_onUnlockAttempt:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onUnlockAttempt(_arg0);
          return true;
        }
        case TRANSACTION_onUnlockLockout:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onUnlockLockout(_arg0);
          return true;
        }
        case TRANSACTION_onTrustTimeout:
        {
          data.enforceInterface(descriptor);
          this.onTrustTimeout();
          return true;
        }
        case TRANSACTION_onDeviceLocked:
        {
          data.enforceInterface(descriptor);
          this.onDeviceLocked();
          return true;
        }
        case TRANSACTION_onDeviceUnlocked:
        {
          data.enforceInterface(descriptor);
          this.onDeviceUnlocked();
          return true;
        }
        case TRANSACTION_onConfigure:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.os.PersistableBundle> _arg0;
          _arg0 = data.createTypedArrayList(android.os.PersistableBundle.CREATOR);
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.onConfigure(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setCallback:
        {
          data.enforceInterface(descriptor);
          android.service.trust.ITrustAgentServiceCallback _arg0;
          _arg0 = android.service.trust.ITrustAgentServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.setCallback(_arg0);
          return true;
        }
        case TRANSACTION_onEscrowTokenAdded:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          long _arg1;
          _arg1 = data.readLong();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onEscrowTokenAdded(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onTokenStateReceived:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.onTokenStateReceived(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onEscrowTokenRemoved:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onEscrowTokenRemoved(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.trust.ITrustAgentService
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
      @Override public void onUnlockAttempt(boolean successful) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((successful)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUnlockAttempt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUnlockAttempt(successful);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUnlockLockout(int timeoutMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeoutMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUnlockLockout, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUnlockLockout(timeoutMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTrustTimeout() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustTimeout, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustTimeout();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeviceLocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceLocked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceLocked();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeviceUnlocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceUnlocked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceUnlocked();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConfigure(java.util.List<android.os.PersistableBundle> options, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(options);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfigure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfigure(options, token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setCallback(android.service.trust.ITrustAgentServiceCallback callback) throws android.os.RemoteException
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
      @Override public void onEscrowTokenAdded(byte[] token, long handle, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          _data.writeLong(handle);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenAdded(token, handle, user);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTokenStateReceived(long handle, int tokenState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(tokenState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTokenStateReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTokenStateReceived(handle, tokenState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onEscrowTokenRemoved(long handle, boolean successful) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(((successful)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenRemoved(handle, successful);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.trust.ITrustAgentService sDefaultImpl;
    }
    static final int TRANSACTION_onUnlockAttempt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onUnlockLockout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onTrustTimeout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onDeviceLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onDeviceUnlocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onConfigure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onEscrowTokenAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onTokenStateReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onEscrowTokenRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.service.trust.ITrustAgentService impl) {
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
    public static android.service.trust.ITrustAgentService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUnlockAttempt(boolean successful) throws android.os.RemoteException;
  public void onUnlockLockout(int timeoutMs) throws android.os.RemoteException;
  public void onTrustTimeout() throws android.os.RemoteException;
  public void onDeviceLocked() throws android.os.RemoteException;
  public void onDeviceUnlocked() throws android.os.RemoteException;
  public void onConfigure(java.util.List<android.os.PersistableBundle> options, android.os.IBinder token) throws android.os.RemoteException;
  public void setCallback(android.service.trust.ITrustAgentServiceCallback callback) throws android.os.RemoteException;
  public void onEscrowTokenAdded(byte[] token, long handle, android.os.UserHandle user) throws android.os.RemoteException;
  public void onTokenStateReceived(long handle, int tokenState) throws android.os.RemoteException;
  public void onEscrowTokenRemoved(long handle, boolean successful) throws android.os.RemoteException;
}
