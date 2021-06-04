/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.trust;
/**
 * System private API to comunicate with trust service.
 *
 * {@hide}
 */
public interface ITrustManager extends android.os.IInterface
{
  /** Default implementation for ITrustManager. */
  public static class Default implements android.app.trust.ITrustManager
  {
    @Override public void reportUnlockAttempt(boolean successful, int userId) throws android.os.RemoteException
    {
    }
    @Override public void reportUnlockLockout(int timeoutMs, int userId) throws android.os.RemoteException
    {
    }
    @Override public void reportEnabledTrustAgentsChanged(int userId) throws android.os.RemoteException
    {
    }
    @Override public void registerTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException
    {
    }
    @Override public void reportKeyguardShowingChanged() throws android.os.RemoteException
    {
    }
    @Override public void setDeviceLockedForUser(int userId, boolean locked) throws android.os.RemoteException
    {
    }
    @Override public boolean isDeviceLocked(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isDeviceSecure(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isTrustUsuallyManaged(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void unlockedByBiometricForUser(int userId, android.hardware.biometrics.BiometricSourceType source) throws android.os.RemoteException
    {
    }
    @Override public void clearAllBiometricRecognized(android.hardware.biometrics.BiometricSourceType target) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.trust.ITrustManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.trust.ITrustManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.trust.ITrustManager interface,
     * generating a proxy if needed.
     */
    public static android.app.trust.ITrustManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.trust.ITrustManager))) {
        return ((android.app.trust.ITrustManager)iin);
      }
      return new android.app.trust.ITrustManager.Stub.Proxy(obj);
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
        case TRANSACTION_reportUnlockAttempt:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.reportUnlockAttempt(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportUnlockLockout:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.reportUnlockLockout(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportEnabledTrustAgentsChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.reportEnabledTrustAgentsChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTrustListener:
        {
          data.enforceInterface(descriptor);
          android.app.trust.ITrustListener _arg0;
          _arg0 = android.app.trust.ITrustListener.Stub.asInterface(data.readStrongBinder());
          this.registerTrustListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTrustListener:
        {
          data.enforceInterface(descriptor);
          android.app.trust.ITrustListener _arg0;
          _arg0 = android.app.trust.ITrustListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterTrustListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportKeyguardShowingChanged:
        {
          data.enforceInterface(descriptor);
          this.reportKeyguardShowingChanged();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDeviceLockedForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setDeviceLockedForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDeviceLocked:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDeviceLocked(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDeviceSecure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDeviceSecure(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isTrustUsuallyManaged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isTrustUsuallyManaged(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unlockedByBiometricForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.biometrics.BiometricSourceType _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.biometrics.BiometricSourceType.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.unlockedByBiometricForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearAllBiometricRecognized:
        {
          data.enforceInterface(descriptor);
          android.hardware.biometrics.BiometricSourceType _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.biometrics.BiometricSourceType.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.clearAllBiometricRecognized(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.trust.ITrustManager
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
      @Override public void reportUnlockAttempt(boolean successful, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((successful)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportUnlockAttempt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportUnlockAttempt(successful, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportUnlockLockout(int timeoutMs, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeoutMs);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportUnlockLockout, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportUnlockLockout(timeoutMs, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportEnabledTrustAgentsChanged(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportEnabledTrustAgentsChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportEnabledTrustAgentsChanged(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((trustListener!=null))?(trustListener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTrustListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTrustListener(trustListener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((trustListener!=null))?(trustListener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTrustListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTrustListener(trustListener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportKeyguardShowingChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportKeyguardShowingChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportKeyguardShowingChanged();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setDeviceLockedForUser(int userId, boolean locked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((locked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeviceLockedForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeviceLockedForUser(userId, locked);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isDeviceLocked(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDeviceLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDeviceLocked(userId);
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
      @Override public boolean isDeviceSecure(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDeviceSecure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDeviceSecure(userId);
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
      @Override public boolean isTrustUsuallyManaged(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTrustUsuallyManaged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTrustUsuallyManaged(userId);
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
      @Override public void unlockedByBiometricForUser(int userId, android.hardware.biometrics.BiometricSourceType source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          if ((source!=null)) {
            _data.writeInt(1);
            source.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unlockedByBiometricForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unlockedByBiometricForUser(userId, source);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearAllBiometricRecognized(android.hardware.biometrics.BiometricSourceType target) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((target!=null)) {
            _data.writeInt(1);
            target.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearAllBiometricRecognized, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearAllBiometricRecognized(target);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.trust.ITrustManager sDefaultImpl;
    }
    static final int TRANSACTION_reportUnlockAttempt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_reportUnlockLockout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_reportEnabledTrustAgentsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_registerTrustListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_unregisterTrustListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_reportKeyguardShowingChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setDeviceLockedForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isDeviceLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_isDeviceSecure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isTrustUsuallyManaged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_unlockedByBiometricForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_clearAllBiometricRecognized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(android.app.trust.ITrustManager impl) {
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
    public static android.app.trust.ITrustManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void reportUnlockAttempt(boolean successful, int userId) throws android.os.RemoteException;
  public void reportUnlockLockout(int timeoutMs, int userId) throws android.os.RemoteException;
  public void reportEnabledTrustAgentsChanged(int userId) throws android.os.RemoteException;
  public void registerTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException;
  public void unregisterTrustListener(android.app.trust.ITrustListener trustListener) throws android.os.RemoteException;
  public void reportKeyguardShowingChanged() throws android.os.RemoteException;
  public void setDeviceLockedForUser(int userId, boolean locked) throws android.os.RemoteException;
  public boolean isDeviceLocked(int userId) throws android.os.RemoteException;
  public boolean isDeviceSecure(int userId) throws android.os.RemoteException;
  public boolean isTrustUsuallyManaged(int userId) throws android.os.RemoteException;
  public void unlockedByBiometricForUser(int userId, android.hardware.biometrics.BiometricSourceType source) throws android.os.RemoteException;
  public void clearAllBiometricRecognized(android.hardware.biometrics.BiometricSourceType target) throws android.os.RemoteException;
}
