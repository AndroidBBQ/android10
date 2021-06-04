/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.face;
/**
 * Communication channel from client to the face service. These methods are all require the
 * MANAGE_BIOMETRIC signature permission.
 * @hide
 */
public interface IFaceService extends android.os.IInterface
{
  /** Default implementation for IFaceService. */
  public static class Default implements android.hardware.face.IFaceService
  {
    // Authenticate the given sessionId with a face

    @Override public void authenticate(android.os.IBinder token, long sessionId, int userid, android.hardware.face.IFaceServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // This method prepares the service to start authenticating, but doesn't start authentication.
    // This is protected by the MANAGE_BIOMETRIC signatuer permission. This method should only be
    // called from BiometricService. The additional uid, pid, userId arguments should be determined
    // by BiometricService. To start authentication after the clients are ready, use
    // startPreparedClient().

    @Override public void prepareForAuthentication(boolean requireConfirmation, android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException
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
    // Same as above, with extra arguments.

    @Override public void cancelAuthenticationFromService(android.os.IBinder token, java.lang.String opPackageName, int callingUid, int callingPid, int callingUserId, boolean fromClient) throws android.os.RemoteException
    {
    }
    // Start face enrollment

    @Override public void enroll(int userId, android.os.IBinder token, byte[] cryptoToken, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName, int[] disabledFeatures) throws android.os.RemoteException
    {
    }
    // Cancel enrollment in progress

    @Override public void cancelEnrollment(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    // Any errors resulting from this call will be returned to the listener

    @Override public void remove(android.os.IBinder token, int faceId, int userId, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    // Rename the face specified by faceId to the given name

    @Override public void rename(int faceId, java.lang.String name) throws android.os.RemoteException
    {
    }
    // Get the enrolled face for user.

    @Override public java.util.List<android.hardware.face.Face> getEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    // Determine if HAL is loaded and ready

    @Override public boolean isHardwareDetected(long deviceId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return false;
    }
    // Get a pre-enrollment authentication token

    @Override public long generateChallenge(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0L;
    }
    // Finish an enrollment sequence and invalidate the authentication token

    @Override public int revokeChallenge(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0;
    }
    // Determine if a user has at least one enrolled face

    @Override public boolean hasEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return false;
    }
    // Gets the number of hardware devices
    // int getHardwareDeviceCount();
    // Gets the unique device id for hardware enumerated at i
    // long getHardwareDevice(int i);
    // Gets the authenticator ID for face

    @Override public long getAuthenticatorId(java.lang.String opPackageName) throws android.os.RemoteException
    {
      return 0L;
    }
    // Reset the lockout when user authenticates with strong auth (e.g. PIN, pattern or password)

    @Override public void resetLockout(byte[] token) throws android.os.RemoteException
    {
    }
    // Add a callback which gets notified when the face lockout period expired.

    @Override public void addLockoutResetCallback(android.hardware.biometrics.IBiometricServiceLockoutResetCallback callback) throws android.os.RemoteException
    {
    }
    // Explicitly set the active user (for enrolling work profile)

    @Override public void setActiveUser(int uid) throws android.os.RemoteException
    {
    }
    // Enumerate all faces

    @Override public void enumerate(android.os.IBinder token, int userId, android.hardware.face.IFaceServiceReceiver receiver) throws android.os.RemoteException
    {
    }
    @Override public void setFeature(int userId, int feature, boolean enabled, byte[] token, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void getFeature(int userId, int feature, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void userActivity() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.face.IFaceService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.face.IFaceService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.face.IFaceService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.face.IFaceService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.face.IFaceService))) {
        return ((android.hardware.face.IFaceService)iin);
      }
      return new android.hardware.face.IFaceService.Stub.Proxy(obj);
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
          android.hardware.face.IFaceServiceReceiver _arg3;
          _arg3 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
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
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          long _arg2;
          _arg2 = data.readLong();
          int _arg3;
          _arg3 = data.readInt();
          android.hardware.biometrics.IBiometricServiceReceiverInternal _arg4;
          _arg4 = android.hardware.biometrics.IBiometricServiceReceiverInternal.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg5;
          _arg5 = data.readString();
          int _arg6;
          _arg6 = data.readInt();
          int _arg7;
          _arg7 = data.readInt();
          int _arg8;
          _arg8 = data.readInt();
          int _arg9;
          _arg9 = data.readInt();
          this.prepareForAuthentication(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
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
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          android.hardware.face.IFaceServiceReceiver _arg3;
          _arg3 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          int[] _arg5;
          _arg5 = data.createIntArray();
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
          android.hardware.face.IFaceServiceReceiver _arg3;
          _arg3 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.remove(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_rename:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.rename(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEnrolledFaces:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.hardware.face.Face> _result = this.getEnrolledFaces(_arg0, _arg1);
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
        case TRANSACTION_generateChallenge:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _result = this.generateChallenge(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_revokeChallenge:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.revokeChallenge(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasEnrolledFaces:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.hasEnrolledFaces(_arg0, _arg1);
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
        case TRANSACTION_resetLockout:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.resetLockout(_arg0);
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
          android.hardware.face.IFaceServiceReceiver _arg2;
          _arg2 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
          this.enumerate(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          byte[] _arg3;
          _arg3 = data.createByteArray();
          android.hardware.face.IFaceServiceReceiver _arg4;
          _arg4 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.setFeature(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.face.IFaceServiceReceiver _arg2;
          _arg2 = android.hardware.face.IFaceServiceReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.getFeature(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_userActivity:
        {
          data.enforceInterface(descriptor);
          this.userActivity();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.face.IFaceService
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
      // Authenticate the given sessionId with a face

      @Override public void authenticate(android.os.IBinder token, long sessionId, int userid, android.hardware.face.IFaceServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeLong(sessionId);
          _data.writeInt(userid);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_authenticate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().authenticate(token, sessionId, userid, receiver, flags, opPackageName);
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

      @Override public void prepareForAuthentication(boolean requireConfirmation, android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((requireConfirmation)?(1):(0)));
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
            getDefaultImpl().prepareForAuthentication(requireConfirmation, token, sessionId, userId, wrapperReceiver, opPackageName, cookie, callingUid, callingPid, callingUserId);
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
      // Same as above, with extra arguments.

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
      // Start face enrollment

      @Override public void enroll(int userId, android.os.IBinder token, byte[] cryptoToken, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName, int[] disabledFeatures) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStrongBinder(token);
          _data.writeByteArray(cryptoToken);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          _data.writeIntArray(disabledFeatures);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enroll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enroll(userId, token, cryptoToken, receiver, opPackageName, disabledFeatures);
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

      @Override public void remove(android.os.IBinder token, int faceId, int userId, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(faceId);
          _data.writeInt(userId);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_remove, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remove(token, faceId, userId, receiver, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Rename the face specified by faceId to the given name

      @Override public void rename(int faceId, java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(faceId);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_rename, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rename(faceId, name);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get the enrolled face for user.

      @Override public java.util.List<android.hardware.face.Face> getEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.face.Face> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnrolledFaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnrolledFaces(userId, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.face.Face.CREATOR);
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

      @Override public long generateChallenge(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_generateChallenge, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().generateChallenge(token);
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

      @Override public int revokeChallenge(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeChallenge, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().revokeChallenge(token);
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
      // Determine if a user has at least one enrolled face

      @Override public boolean hasEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasEnrolledFaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasEnrolledFaces(userId, opPackageName);
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
      // Gets the authenticator ID for face

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
      // Add a callback which gets notified when the face lockout period expired.

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
      // Enumerate all faces

      @Override public void enumerate(android.os.IBinder token, int userId, android.hardware.face.IFaceServiceReceiver receiver) throws android.os.RemoteException
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
      @Override public void setFeature(int userId, int feature, boolean enabled, byte[] token, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(feature);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeByteArray(token);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFeature(userId, feature, enabled, token, receiver, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getFeature(int userId, int feature, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(feature);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getFeature(userId, feature, receiver, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void userActivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_userActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().userActivity();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.face.IFaceService sDefaultImpl;
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
    static final int TRANSACTION_getEnrolledFaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isHardwareDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_generateChallenge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_revokeChallenge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_hasEnrolledFaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getAuthenticatorId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_resetLockout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_addLockoutResetCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setActiveUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_enumerate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_userActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    public static boolean setDefaultImpl(android.hardware.face.IFaceService impl) {
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
    public static android.hardware.face.IFaceService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Authenticate the given sessionId with a face

  public void authenticate(android.os.IBinder token, long sessionId, int userid, android.hardware.face.IFaceServiceReceiver receiver, int flags, java.lang.String opPackageName) throws android.os.RemoteException;
  // This method prepares the service to start authenticating, but doesn't start authentication.
  // This is protected by the MANAGE_BIOMETRIC signatuer permission. This method should only be
  // called from BiometricService. The additional uid, pid, userId arguments should be determined
  // by BiometricService. To start authentication after the clients are ready, use
  // startPreparedClient().

  public void prepareForAuthentication(boolean requireConfirmation, android.os.IBinder token, long sessionId, int userId, android.hardware.biometrics.IBiometricServiceReceiverInternal wrapperReceiver, java.lang.String opPackageName, int cookie, int callingUid, int callingPid, int callingUserId) throws android.os.RemoteException;
  // Starts authentication with the previously prepared client.

  public void startPreparedClient(int cookie) throws android.os.RemoteException;
  // Cancel authentication for the given sessionId

  public void cancelAuthentication(android.os.IBinder token, java.lang.String opPackageName) throws android.os.RemoteException;
  // Same as above, with extra arguments.

  public void cancelAuthenticationFromService(android.os.IBinder token, java.lang.String opPackageName, int callingUid, int callingPid, int callingUserId, boolean fromClient) throws android.os.RemoteException;
  // Start face enrollment

  public void enroll(int userId, android.os.IBinder token, byte[] cryptoToken, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName, int[] disabledFeatures) throws android.os.RemoteException;
  // Cancel enrollment in progress

  public void cancelEnrollment(android.os.IBinder token) throws android.os.RemoteException;
  // Any errors resulting from this call will be returned to the listener

  public void remove(android.os.IBinder token, int faceId, int userId, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException;
  // Rename the face specified by faceId to the given name

  public void rename(int faceId, java.lang.String name) throws android.os.RemoteException;
  // Get the enrolled face for user.

  public java.util.List<android.hardware.face.Face> getEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Determine if HAL is loaded and ready

  public boolean isHardwareDetected(long deviceId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Get a pre-enrollment authentication token

  public long generateChallenge(android.os.IBinder token) throws android.os.RemoteException;
  // Finish an enrollment sequence and invalidate the authentication token

  public int revokeChallenge(android.os.IBinder token) throws android.os.RemoteException;
  // Determine if a user has at least one enrolled face

  public boolean hasEnrolledFaces(int userId, java.lang.String opPackageName) throws android.os.RemoteException;
  // Gets the number of hardware devices
  // int getHardwareDeviceCount();
  // Gets the unique device id for hardware enumerated at i
  // long getHardwareDevice(int i);
  // Gets the authenticator ID for face

  public long getAuthenticatorId(java.lang.String opPackageName) throws android.os.RemoteException;
  // Reset the lockout when user authenticates with strong auth (e.g. PIN, pattern or password)

  public void resetLockout(byte[] token) throws android.os.RemoteException;
  // Add a callback which gets notified when the face lockout period expired.

  public void addLockoutResetCallback(android.hardware.biometrics.IBiometricServiceLockoutResetCallback callback) throws android.os.RemoteException;
  // Explicitly set the active user (for enrolling work profile)

  public void setActiveUser(int uid) throws android.os.RemoteException;
  // Enumerate all faces

  public void enumerate(android.os.IBinder token, int userId, android.hardware.face.IFaceServiceReceiver receiver) throws android.os.RemoteException;
  public void setFeature(int userId, int feature, boolean enabled, byte[] token, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException;
  public void getFeature(int userId, int feature, android.hardware.face.IFaceServiceReceiver receiver, java.lang.String opPackageName) throws android.os.RemoteException;
  public void userActivity() throws android.os.RemoteException;
}
