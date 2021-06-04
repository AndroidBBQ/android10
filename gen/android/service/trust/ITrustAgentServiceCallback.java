/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.trust;
/**
 * Communication channel from the TrustAgentService back to TrustManagerService.
 * @hide
 */
public interface ITrustAgentServiceCallback extends android.os.IInterface
{
  /** Default implementation for ITrustAgentServiceCallback. */
  public static class Default implements android.service.trust.ITrustAgentServiceCallback
  {
    @Override public void grantTrust(java.lang.CharSequence message, long durationMs, int flags) throws android.os.RemoteException
    {
    }
    @Override public void revokeTrust() throws android.os.RemoteException
    {
    }
    @Override public void setManagingTrust(boolean managingTrust) throws android.os.RemoteException
    {
    }
    @Override public void onConfigureCompleted(boolean result, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException
    {
    }
    @Override public void isEscrowTokenActive(long handle, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException
    {
    }
    @Override public void unlockUserWithToken(long handle, byte[] token, int userId) throws android.os.RemoteException
    {
    }
    @Override public void showKeyguardErrorMessage(java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.trust.ITrustAgentServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.trust.ITrustAgentServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.trust.ITrustAgentServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.trust.ITrustAgentServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.trust.ITrustAgentServiceCallback))) {
        return ((android.service.trust.ITrustAgentServiceCallback)iin);
      }
      return new android.service.trust.ITrustAgentServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_grantTrust:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.grantTrust(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_revokeTrust:
        {
          data.enforceInterface(descriptor);
          this.revokeTrust();
          return true;
        }
        case TRANSACTION_setManagingTrust:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setManagingTrust(_arg0);
          return true;
        }
        case TRANSACTION_onConfigureCompleted:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.onConfigureCompleted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addEscrowToken:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          this.addEscrowToken(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_isEscrowTokenActive:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.isEscrowTokenActive(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeEscrowToken:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.removeEscrowToken(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unlockUserWithToken:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          int _arg2;
          _arg2 = data.readInt();
          this.unlockUserWithToken(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_showKeyguardErrorMessage:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.showKeyguardErrorMessage(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.trust.ITrustAgentServiceCallback
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
      @Override public void grantTrust(java.lang.CharSequence message, long durationMs, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(durationMs);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantTrust, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantTrust(message, durationMs, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void revokeTrust() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeTrust, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeTrust();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setManagingTrust(boolean managingTrust) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((managingTrust)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setManagingTrust, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setManagingTrust(managingTrust);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConfigureCompleted(boolean result, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((result)?(1):(0)));
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfigureCompleted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfigureCompleted(result, token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addEscrowToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addEscrowToken(token, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isEscrowTokenActive(long handle, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEscrowTokenActive, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isEscrowTokenActive(handle, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeEscrowToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeEscrowToken(handle, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unlockUserWithToken(long handle, byte[] token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeByteArray(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unlockUserWithToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unlockUserWithToken(handle, token, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showKeyguardErrorMessage(java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_showKeyguardErrorMessage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showKeyguardErrorMessage(message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.trust.ITrustAgentServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_grantTrust = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_revokeTrust = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setManagingTrust = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onConfigureCompleted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isEscrowTokenActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_unlockUserWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_showKeyguardErrorMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.service.trust.ITrustAgentServiceCallback impl) {
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
    public static android.service.trust.ITrustAgentServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void grantTrust(java.lang.CharSequence message, long durationMs, int flags) throws android.os.RemoteException;
  public void revokeTrust() throws android.os.RemoteException;
  public void setManagingTrust(boolean managingTrust) throws android.os.RemoteException;
  public void onConfigureCompleted(boolean result, android.os.IBinder token) throws android.os.RemoteException;
  public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException;
  public void isEscrowTokenActive(long handle, int userId) throws android.os.RemoteException;
  public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException;
  public void unlockUserWithToken(long handle, byte[] token, int userId) throws android.os.RemoteException;
  public void showKeyguardErrorMessage(java.lang.CharSequence message) throws android.os.RemoteException;
}
