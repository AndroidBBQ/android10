/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.biometrics;
/**
 * Communication channel from BiometricPrompt and BiometricManager to BiometricService. The
 * interface does not expose specific biometric modalities. The system will use the default
 * biometric for apps. On devices with more than one, the choice is dictated by user preference in
 * Settings.
 * @hide
 */
public interface IBiometricService extends android.os.IInterface
{
  /** Default implementation for IBiometricService. */
  public static class Default implements android.hardware.biometrics.IBiometricService
  {
    // Requests authentication. The service choose the appropriate biometric to use, and show
    // the corresponding BiometricDialog.
    // TODO(b/123378871): Remove callback when moved.

    @Override public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiver receiver, java.lang.String opPackageName, android.os.Bundle bundle, android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException
    {
    }
    // Cancel authentication for the given sessionId

    @Override public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // Checks if biometrics can be used.

    @Override public int canAuthenticate(java.lang.String opPackageName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    // Checks if any biometrics are enrolled.

    @Override public boolean hasEnrolledBiometrics(int userId) throws android.os.RemoteException
    {
      return false;
    }
    // Register callback for when keyguard biometric eligibility changes.

    @Override public void registerEnabledOnKeyguardCallback(android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback callback) throws android.os.RemoteException
    {
    }
    // Explicitly set the active user.

    @Override public void setActiveUser(int userId) throws android.os.RemoteException
    {
    }
    // Notify BiometricService when <Biometric>Service is ready to start the prepared client.
    // Client lifecycle is still managed in <Biometric>Service.

    @Override public void onReadyForAuthentication(int cookie, boolean requireConfirmation, int userId) throws android.os.RemoteException
    {
    }
    // Reset the lockout when user authenticates with strong auth (e.g. PIN, pattern or password)

    @Override public void resetLockout(byte[] token) throws android.os.RemoteException
    {
    }
    // TODO(b/123378871): Remove when moved.
    // CDCA needs to send results to BiometricService if it was invoked using BiometricPrompt's
    // setAllowDeviceCredential method, since there's no way for us to intercept onActivityResult.
    // CDCA is launched from BiometricService (startActivityAsUser) instead of *ForResult.

    @Override public void onConfirmDeviceCredentialSuccess() throws android.os.RemoteException
    {
    }
    // TODO(b/123378871): Remove when moved.

    @Override public void onConfirmDeviceCredentialError(int error, java.lang.String message) throws android.os.RemoteException
    {
    }
    // TODO(b/123378871): Remove when moved.
    // When ConfirmLock* is invoked from BiometricPrompt, it needs to register a callback so that
    // it can receive the cancellation signal.

    @Override public void registerCancellationCallback(android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.biometrics.IBiometricService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.biometrics.IBiometricService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.biometrics.IBiometricService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.biometrics.IBiometricService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.biometrics.IBiometricService))) {
        return ((android.hardware.biometrics.IBiometricService)iin);
      }
      return new android.hardware.biometrics.IBiometricService.Stub.Proxy(obj);
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
        case TRANSACTION_authenticate:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          android.hardware.biometrics.IBiometricServiceReceiver _arg3;
          _arg3 = android.hardware.biometrics.IBiometricServiceReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback _arg6;
          _arg6 = android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback.Stub.asInterface(data.readStrongBinder());
          this.authenticate(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelAuthentication:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.cancelAuthentication(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_canAuthenticate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.canAuthenticate(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasEnrolledBiometrics:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.hasEnrolledBiometrics(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerEnabledOnKeyguardCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback _arg0;
          _arg0 = android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback.Stub.asInterface(data.readStrongBinder());
          this.registerEnabledOnKeyguardCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setActiveUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setActiveUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onReadyForAuthentication:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.onReadyForAuthentication(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resetLockout:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.resetLockout(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onConfirmDeviceCredentialSuccess:
        {
          data.enforceInterface(descriptor);
          this.onConfirmDeviceCredentialSuccess();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onConfirmDeviceCredentialError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onConfirmDeviceCredentialError(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerCancellationCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback _arg0;
          _arg0 = android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCancellationCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.biometrics.IBiometricService
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
      // Requests authentication. The service choose the appropriate biometric to use, and show
      // the corresponding BiometricDialog.
      // TODO(b/123378871): Remove callback when moved.

      @Override public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiver receiver, java.lang.String opPackageName, android.os.Bundle bundle, android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeLong(sessionId);
          _data.writeInt(userId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          if ((bundle!=null)) {
            _data.writeInt(1);
            bundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_authenticate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().authenticate(token, sessionId, userId, receiver, opPackageName, bundle, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Cancel authentication for the given sessionId

      @Override public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAuthentication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAuthentication(token, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Checks if biometrics can be used.

      @Override public int canAuthenticate(java.lang.String opPackageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(opPackageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canAuthenticate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canAuthenticate(opPackageName, userId);
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
      // Checks if any biometrics are enrolled.

      @Override public boolean hasEnrolledBiometrics(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasEnrolledBiometrics, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasEnrolledBiometrics(userId);
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
      // Register callback for when keyguard biometric eligibility changes.

      @Override public void registerEnabledOnKeyguardCallback(android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerEnabledOnKeyguardCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerEnabledOnKeyguardCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Explicitly set the active user.

      @Override public void setActiveUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActiveUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActiveUser(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Notify BiometricService when <Biometric>Service is ready to start the prepared client.
      // Client lifecycle is still managed in <Biometric>Service.

      @Override public void onReadyForAuthentication(int cookie, boolean requireConfirmation, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cookie);
          _data.writeInt(((requireConfirmation)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReadyForAuthentication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReadyForAuthentication(cookie, requireConfirmation, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Reset the lockout when user authenticates with strong auth (e.g. PIN, pattern or password)

      @Override public void resetLockout(byte[] token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetLockout, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetLockout(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // TODO(b/123378871): Remove when moved.
      // CDCA needs to send results to BiometricService if it was invoked using BiometricPrompt's
      // setAllowDeviceCredential method, since there's no way for us to intercept onActivityResult.
      // CDCA is launched from BiometricService (startActivityAsUser) instead of *ForResult.

      @Override public void onConfirmDeviceCredentialSuccess() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfirmDeviceCredentialSuccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfirmDeviceCredentialSuccess();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // TODO(b/123378871): Remove when moved.

      @Override public void onConfirmDeviceCredentialError(int error, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfirmDeviceCredentialError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfirmDeviceCredentialError(error, message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // TODO(b/123378871): Remove when moved.
      // When ConfirmLock* is invoked from BiometricPrompt, it needs to register a callback so that
      // it can receive the cancellation signal.

      @Override public void registerCancellationCallback(android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCancellationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCancellationCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.biometrics.IBiometricService sDefaultImpl;
    }
    static final int TRANSACTION_authenticate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_cancelAuthentication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_canAuthenticate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_hasEnrolledBiometrics = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_registerEnabledOnKeyguardCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setActiveUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onReadyForAuthentication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_resetLockout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onConfirmDeviceCredentialSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onConfirmDeviceCredentialError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_registerCancellationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.hardware.biometrics.IBiometricService impl) {
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
    public static android.hardware.biometrics.IBiometricService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Requests authentication. The service choose the appropriate biometric to use, and show
  // the corresponding BiometricDialog.
  // TODO(b/123378871): Remove callback when moved.

  public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiver receiver, java.lang.String opPackageName, android.os.Bundle bundle, android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException;
  // Cancel authentication for the given sessionId

  public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException;
  // Checks if biometrics can be used.

  public int canAuthenticate(java.lang.String opPackageName, int userId) throws android.os.RemoteException;
  // Checks if any biometrics are enrolled.

  public boolean hasEnrolledBiometrics(int userId) throws android.os.RemoteException;
  // Register callback for when keyguard biometric eligibility changes.

  public void registerEnabledOnKeyguardCallback(android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback callback) throws android.os.RemoteException;
  // Explicitly set the active user.

  public void setActiveUser(int userId) throws android.os.RemoteException;
  // Notify BiometricService when <Biometric>Service is ready to start the prepared client.
  // Client lifecycle is still managed in <Biometric>Service.

  public void onReadyForAuthentication(int cookie, boolean requireConfirmation, int userId) throws android.os.RemoteException;
  // Reset the lockout when user authenticates with strong auth (e.g. PIN, pattern or password)

  public void resetLockout(byte[] token) throws android.os.RemoteException;
  // TODO(b/123378871): Remove when moved.
  // CDCA needs to send results to BiometricService if it was invoked using BiometricPrompt's
  // setAllowDeviceCredential method, since there's no way for us to intercept onActivityResult.
  // CDCA is launched from BiometricService (startActivityAsUser) instead of *ForResult.

  public void onConfirmDeviceCredentialSuccess() throws android.os.RemoteException;
  // TODO(b/123378871): Remove when moved.

  public void onConfirmDeviceCredentialError(int error, java.lang.String message) throws android.os.RemoteException;
  // TODO(b/123378871): Remove when moved.
  // When ConfirmLock* is invoked from BiometricPrompt, it needs to register a callback so that
  // it can receive the cancellation signal.

  public void registerCancellationCallback(android.hardware.biometrics.IBiometricConfirmDeviceCredentialCallback callback) throws android.os.RemoteException;
}
