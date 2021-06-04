/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.fingerprint;
/**
 * Communication channel from client to the fingerprint service.
 * @hide
 */
public interface IFingerprintService extends android.os.IInterface
{
  /** Default implementation for IFingerprintService. */
  public static class Default implements android.hardware.fingerprint.IFingerprintService
  {
    // Authenticate the given sessionId with a fingerprint. This is protected by
    // USE_FINGERPRINT/USE_BIOMETRIC permission. This is effectively deprecated, since it only comes
    // through FingerprintManager now.

    @Override public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // This method prepares the service to start authenticating, but doesn't start authentication.
    // This is protected by the MANAGE_BIOMETRIC signatuer permission. This method should only be
    // called from BiometricService. The additional uid, pid, userId arguments should be determined
    // by BiometricService. To start authentication after the clients are ready, use
    // startPreparedClient().

    @Override public void prepareForAuthentication(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException
    {
    }
    // Starts authentication with the previously prepared client.

    @Override public void startPreparedClient(int cookie) throws android.os.RemoteException
    {
    }
    // Cancel authentication for the given sessionId

    @Override public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // Same as above, except this is protected by the MANAGE_BIOMETRIC signature permission. Takes
    // an additional uid, pid, userid.

    @Override public void cancelAuthenticationFromService(android.os.IBinder token, java.lang.String opPackageName, int callingUid, int callingPid, int callingUserId, boolean fromClient) throws android.os.RemoteException
    {
    }
    // Start fingerprint enrollment

    @Override public void enroll(android.os.IBinder token, byte[] cryptoToken, int groupId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // Cancel enrollment in progress

    @Override public void cancelEnrollment(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    // Any errors resulting from this call will be returned to the listener

    @Override public void remove(android.os.IBinder token, int fingerId, int groupId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException
    {
    }
    // Rename the fingerprint specified by fingerId and groupId to the given name

    @Override public void rename(int fingerId, int groupId, java.lang.String name) throws android.os.RemoteException
    {
    }
    // Get a list of enrolled fingerprints in the given group.

    @Override public java.util.List<android.hardware.fingerprint.Fingerprint> getEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    // Determine if HAL is loaded and ready

    @Override public boolean isHardwareDetected(long deviceId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return false;
    }
    // Get a pre-enrollment authentication token

    @Override public long preEnroll(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0L;
    }
    // Finish an enrollment sequence and invalidate the authentication token

    @Override public int postEnroll(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0;
    }
    // Determine if a user has at least one enrolled fingerprint

    @Override public boolean hasEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return false;
    }
    // Gets the number of hardware devices
    // int getHardwareDeviceCount();
    // Gets the unique device id for hardware enumerated at i
    // long getHardwareDevice(int i);
    // Gets the authenticator ID for fingerprint

    @Override public long getAuthenticatorId(java.lang.String opPackageName) throws android.os.RemoteException
    {
      return 0L;
    }
    // Reset the timeout when user authenticates with strong auth (e.g. PIN, pattern or password)

    @Override public void resetTimeout(byte[] cryptoToken) throws android.os.RemoteException
    {
    }
    // Add a callback which gets notified when the fingerprint lockout period expired.

    @Override public void addLockoutResetCallback(android.hardware.biometrics.IBiometricServiceLockoutResetCallback callback) throws android.os.RemoteException
    {
    }
    // Explicitly set the active user (for enrolling work profile)

    @Override public void setActiveUser(int uid) throws android.os.RemoteException
    {
    }
    // Enumerate all fingerprints

    @Override public void enumerate(android.os.IBinder token, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException
    {
    }
    // Check if a client request is currently being handled

    @Override public boolean isClientActive() throws android.os.RemoteException
    {
      return false;
    }
    // Add a callback which gets notified when the service starts and stops handling client requests

    @Override public void addClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException
    {
    }
    // Removes a callback set by addClientActiveCallback

    @Override public void removeClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.fingerprint.IFingerprintService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.fingerprint.IFingerprintService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.fingerprint.IFingerprintService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.fingerprint.IFingerprintService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.fingerprint.IFingerprintService))) {
        return ((android.hardware.fingerprint.IFingerprintService)iin);
      }
      return new android.hardware.fingerprint.IFingerprintService.Stub.Proxy(obj);
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
          android.hardware.fingerprint.IFingerprintServiceReceiver _arg3;
          _arg3 = android.hardware.fingerprint.IFingerprintServiceReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.authenticate(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepareForAuthentication:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          android.hardware.biometrics.IBiometricServiceReceiverInternal _arg3;
          _arg3 = android.hardware.biometrics.IBiometricServiceReceiverInternal.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          int _arg7;
          _arg7 = data.readInt();
          int _arg8;
          _arg8 = data.readInt();
          this.prepareForAuthentication(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startPreparedClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.startPreparedClient(_arg0);
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
        case TRANSACTION_cancelAuthenticationFromService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.cancelAuthenticationFromService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enroll:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          int _arg2;
          _arg2 = data.readInt();
          android.hardware.fingerprint.IFingerprintServiceReceiver _arg3;
          _arg3 = android.hardware.fingerprint.IFingerprintServiceReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.enroll(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelEnrollment:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.cancelEnrollment(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_remove:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.hardware.fingerprint.IFingerprintServiceReceiver _arg4;
          _arg4 = android.hardware.fingerprint.IFingerprintServiceReceiver.Stub.asInterface(data.readStrongBinder());
          this.remove(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_rename:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.rename(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEnrolledFingerprints:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.hardware.fingerprint.Fingerprint> _result = this.getEnrolledFingerprints(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_isHardwareDetected:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isHardwareDetected(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_preEnroll:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _result = this.preEnroll(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_postEnroll:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.postEnroll(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasEnrolledFingerprints:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.hasEnrolledFingerprints(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAuthenticatorId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _result = this.getAuthenticatorId(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_resetTimeout:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.resetTimeout(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addLockoutResetCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.biometrics.IBiometricServiceLockoutResetCallback _arg0;
          _arg0 = android.hardware.biometrics.IBiometricServiceLockoutResetCallback.Stub.asInterface(data.readStrongBinder());
          this.addLockoutResetCallback(_arg0);
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
        case TRANSACTION_enumerate:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.fingerprint.IFingerprintServiceReceiver _arg2;
          _arg2 = android.hardware.fingerprint.IFingerprintServiceReceiver.Stub.asInterface(data.readStrongBinder());
          this.enumerate(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isClientActive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isClientActive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addClientActiveCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.fingerprint.IFingerprintClientActiveCallback _arg0;
          _arg0 = android.hardware.fingerprint.IFingerprintClientActiveCallback.Stub.asInterface(data.readStrongBinder());
          this.addClientActiveCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeClientActiveCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.fingerprint.IFingerprintClientActiveCallback _arg0;
          _arg0 = android.hardware.fingerprint.IFingerprintClientActiveCallback.Stub.asInterface(data.readStrongBinder());
          this.removeClientActiveCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.fingerprint.IFingerprintService
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
      // Authenticate the given sessionId with a fingerprint. This is protected by
      // USE_FINGERPRINT/USE_BIOMETRIC permission. This is effectively deprecated, since it only comes
      // through FingerprintManager now.

      @Override public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeLong(sessionId);
          _data.writeInt(userId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_authenticate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().authenticate(token, sessionId, userId, receiver, flags, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // This method prepares the service to start authenticating, but doesn't start authentication.
      // This is protected by the MANAGE_BIOMETRIC signatuer permission. This method should only be
      // called from BiometricService. The additional uid, pid, userId arguments should be determined
      // by BiometricService. To start authentication after the clients are ready, use
      // startPreparedClient().

      @Override public void prepareForAuthentication(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeLong(sessionId);
          _data.writeInt(userId);
          _data.writeStrongBinder((((wrapperReceiver!=null))?(wrapperReceiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          _data.writeInt(cookie);
          _data.writeInt(callingUid);
          _data.writeInt(callingPid);
          _data.writeInt(callingUserId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareForAuthentication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareForAuthentication(token, sessionId, userId, wrapperReceiver, opPackageName, cookie, callingUid, callingPid, callingUserId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Starts authentication with the previously prepared client.

      @Override public void startPreparedClient(int cookie) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cookie);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startPreparedClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startPreparedClient(cookie);
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
      // Same as above, except this is protected by the MANAGE_BIOMETRIC signature permission. Takes
      // an additional uid, pid, userid.

      @Override public void cancelAuthenticationFromService(android.os.IBinder token, java.lang.String opPackageName, int callingUid, int callingPid, int callingUserId, boolean fromClient) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(opPackageName);
          _data.writeInt(callingUid);
          _data.writeInt(callingPid);
          _data.writeInt(callingUserId);
          _data.writeInt(((fromClient)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAuthenticationFromService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAuthenticationFromService(token, opPackageName, callingUid, callingPid, callingUserId, fromClient);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Start fingerprint enrollment

      @Override public void enroll(android.os.IBinder token, byte[] cryptoToken, int groupId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeByteArray(cryptoToken);
          _data.writeInt(groupId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enroll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enroll(token, cryptoToken, groupId, receiver, flags, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Cancel enrollment in progress

      @Override public void cancelEnrollment(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelEnrollment, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelEnrollment(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Any errors resulting from this call will be returned to the listener

      @Override public void remove(android.os.IBinder token, int fingerId, int groupId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(fingerId);
          _data.writeInt(groupId);
          _data.writeInt(userId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_remove, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remove(token, fingerId, groupId, userId, receiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Rename the fingerprint specified by fingerId and groupId to the given name

      @Override public void rename(int fingerId, int groupId, java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(fingerId);
          _data.writeInt(groupId);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_rename, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rename(fingerId, groupId, name);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get a list of enrolled fingerprints in the given group.

      @Override public java.util.List<android.hardware.fingerprint.Fingerprint> getEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.fingerprint.Fingerprint> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(groupId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnrolledFingerprints, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnrolledFingerprints(groupId, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.fingerprint.Fingerprint.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Determine if HAL is loaded and ready

      @Override public boolean isHardwareDetected(long deviceId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isHardwareDetected, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isHardwareDetected(deviceId, opPackageName);
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
      // Get a pre-enrollment authentication token

      @Override public long preEnroll(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_preEnroll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().preEnroll(token);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Finish an enrollment sequence and invalidate the authentication token

      @Override public int postEnroll(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_postEnroll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().postEnroll(token);
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
      // Determine if a user has at least one enrolled fingerprint

      @Override public boolean hasEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(groupId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasEnrolledFingerprints, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasEnrolledFingerprints(groupId, opPackageName);
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
      // Gets the number of hardware devices
      // int getHardwareDeviceCount();
      // Gets the unique device id for hardware enumerated at i
      // long getHardwareDevice(int i);
      // Gets the authenticator ID for fingerprint

      @Override public long getAuthenticatorId(java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAuthenticatorId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAuthenticatorId(opPackageName);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Reset the timeout when user authenticates with strong auth (e.g. PIN, pattern or password)

      @Override public void resetTimeout(byte[] cryptoToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(cryptoToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetTimeout, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetTimeout(cryptoToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Add a callback which gets notified when the fingerprint lockout period expired.

      @Override public void addLockoutResetCallback(android.hardware.biometrics.IBiometricServiceLockoutResetCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addLockoutResetCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addLockoutResetCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Explicitly set the active user (for enrolling work profile)

      @Override public void setActiveUser(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActiveUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActiveUser(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Enumerate all fingerprints

      @Override public void enumerate(android.os.IBinder token, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(userId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enumerate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enumerate(token, userId, receiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Check if a client request is currently being handled

      @Override public boolean isClientActive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isClientActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isClientActive();
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
      // Add a callback which gets notified when the service starts and stops handling client requests

      @Override public void addClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClientActiveCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addClientActiveCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Removes a callback set by addClientActiveCallback

      @Override public void removeClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeClientActiveCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeClientActiveCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.fingerprint.IFingerprintService sDefaultImpl;
    }
    static final int TRANSACTION_authenticate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_prepareForAuthentication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startPreparedClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_cancelAuthentication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_cancelAuthenticationFromService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_enroll = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_cancelEnrollment = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_remove = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_rename = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getEnrolledFingerprints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isHardwareDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_preEnroll = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_postEnroll = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_hasEnrolledFingerprints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getAuthenticatorId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_resetTimeout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_addLockoutResetCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setActiveUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_enumerate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_isClientActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_addClientActiveCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_removeClientActiveCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    public static boolean setDefaultImpl(android.hardware.fingerprint.IFingerprintService impl) {
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
    public static android.hardware.fingerprint.IFingerprintService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Authenticate the given sessionId with a fingerprint. This is protected by
  // USE_FINGERPRINT/USE_BIOMETRIC permission. This is effectively deprecated, since it only comes
  // through FingerprintManager now.

  public void authenticate(android.os.IBinder token, long sessionId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException;
  // This method prepares the service to start authenticating, but doesn't start authentication.
  // This is protected by the MANAGE_BIOMETRIC signatuer permission. This method should only be
  // called from BiometricService. The additional uid, pid, userId arguments should be determined
  // by BiometricService. To start authentication after the clients are ready, use
  // startPreparedClient().

  public void prepareForAuthentication(android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException;
  // Starts authentication with the previously prepared client.

  public void startPreparedClient(int cookie) throws android.os.RemoteException;
  // Cancel authentication for the given sessionId

  public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException;
  // Same as above, except this is protected by the MANAGE_BIOMETRIC signature permission. Takes
  // an additional uid, pid, userid.

  public void cancelAuthenticationFromService(android.os.IBinder token, java.lang.String opPackageName, int callingUid, int callingPid, int callingUserId, boolean fromClient) throws android.os.RemoteException;
  // Start fingerprint enrollment

  public void enroll(android.os.IBinder token, byte[] cryptoToken, int groupId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException;
  // Cancel enrollment in progress

  public void cancelEnrollment(android.os.IBinder token) throws android.os.RemoteException;
  // Any errors resulting from this call will be returned to the listener

  public void remove(android.os.IBinder token, int fingerId, int groupId, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException;
  // Rename the fingerprint specified by fingerId and groupId to the given name

  public void rename(int fingerId, int groupId, java.lang.String name) throws android.os.RemoteException;
  // Get a list of enrolled fingerprints in the given group.

  public java.util.List<android.hardware.fingerprint.Fingerprint> getEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Determine if HAL is loaded and ready

  public boolean isHardwareDetected(long deviceId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Get a pre-enrollment authentication token

  public long preEnroll(android.os.IBinder token) throws android.os.RemoteException;
  // Finish an enrollment sequence and invalidate the authentication token

  public int postEnroll(android.os.IBinder token) throws android.os.RemoteException;
  // Determine if a user has at least one enrolled fingerprint

  public boolean hasEnrolledFingerprints(int groupId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Gets the number of hardware devices
  // int getHardwareDeviceCount();
  // Gets the unique device id for hardware enumerated at i
  // long getHardwareDevice(int i);
  // Gets the authenticator ID for fingerprint

  public long getAuthenticatorId(java.lang.String opPackageName) throws android.os.RemoteException;
  // Reset the timeout when user authenticates with strong auth (e.g. PIN, pattern or password)

  public void resetTimeout(byte[] cryptoToken) throws android.os.RemoteException;
  // Add a callback which gets notified when the fingerprint lockout period expired.

  public void addLockoutResetCallback(android.hardware.biometrics.IBiometricServiceLockoutResetCallback callback) throws android.os.RemoteException;
  // Explicitly set the active user (for enrolling work profile)

  public void setActiveUser(int uid) throws android.os.RemoteException;
  // Enumerate all fingerprints

  public void enumerate(android.os.IBinder token, int userId, android.hardware.fingerprint.IFingerprintServiceReceiver receiver) throws android.os.RemoteException;
  // Check if a client request is currently being handled

  public boolean isClientActive() throws android.os.RemoteException;
  // Add a callback which gets notified when the service starts and stops handling client requests

  public void addClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException;
  // Removes a callback set by addClientActiveCallback

  public void removeClientActiveCallback(android.hardware.fingerprint.IFingerprintClientActiveCallback callback) throws android.os.RemoteException;
}
