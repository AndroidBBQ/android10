/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.widget;
/** {@hide} */
public interface ILockSettings extends android.os.IInterface
{
  /** Default implementation for ILockSettings. */
  public static class Default implements com.android.internal.widget.ILockSettings
  {
    @Override public void setBoolean(java.lang.String key, boolean value, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setLong(java.lang.String key, long value, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setString(java.lang.String key, java.lang.String value, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean getBoolean(java.lang.String key, boolean defaultValue, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public long getLong(java.lang.String key, long defaultValue, int userId) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public java.lang.String getString(java.lang.String key, java.lang.String defaultValue, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setLockCredential(byte[] credential, int type, byte[] savedCredential, int requestedQuality, int userId, boolean allowUntrustedChange) throws android.os.RemoteException
    {
    }
    @Override public void resetKeyStore(int userId) throws android.os.RemoteException
    {
    }
    @Override public com.android.internal.widget.VerifyCredentialResponse checkCredential(byte[] credential, int type, int userId, com.android.internal.widget.ICheckCredentialProgressCallback progressCallback) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.widget.VerifyCredentialResponse verifyCredential(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.widget.VerifyCredentialResponse verifyTiedProfileChallenge(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean checkVoldPassword(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean havePattern(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean havePassword(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public byte[] getHashFactor(byte[] currentCredential, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setSeparateProfileChallengeEnabled(int userId, boolean enabled, byte[] managedUserPassword) throws android.os.RemoteException
    {
    }
    @Override public boolean getSeparateProfileChallengeEnabled(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void registerStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException
    {
    }
    @Override public void unregisterStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException
    {
    }
    @Override public void requireStrongAuth(int strongAuthReason, int userId) throws android.os.RemoteException
    {
    }
    @Override public void systemReady() throws android.os.RemoteException
    {
    }
    @Override public void userPresent(int userId) throws android.os.RemoteException
    {
    }
    @Override public int getStrongAuthForUser(int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean hasPendingEscrowToken(int userId) throws android.os.RemoteException
    {
      return false;
    }
    // Keystore RecoveryController methods.
    // {@code ServiceSpecificException} may be thrown to signal an error, which caller can
    // convert to  {@code RecoveryManagerException}.

    @Override public void initRecoveryServiceWithSigFile(java.lang.String rootCertificateAlias, byte[] recoveryServiceCertFile, byte[] recoveryServiceSigFile) throws android.os.RemoteException
    {
    }
    @Override public android.security.keystore.recovery.KeyChainSnapshot getKeyChainSnapshot() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String generateKey(java.lang.String alias) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String generateKeyWithMetadata(java.lang.String alias, byte[] metadata) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String importKey(java.lang.String alias, byte[] keyBytes) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String importKeyWithMetadata(java.lang.String alias, byte[] keyBytes, byte[] metadata) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getKey(java.lang.String alias) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void removeKey(java.lang.String alias) throws android.os.RemoteException
    {
    }
    @Override public void setSnapshotCreatedPendingIntent(android.app.PendingIntent intent) throws android.os.RemoteException
    {
    }
    @Override public void setServerParams(byte[] serverParams) throws android.os.RemoteException
    {
    }
    @Override public void setRecoveryStatus(java.lang.String alias, int status) throws android.os.RemoteException
    {
    }
    @Override public java.util.Map getRecoveryStatus() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setRecoverySecretTypes(int[] secretTypes) throws android.os.RemoteException
    {
    }
    @Override public int[] getRecoverySecretTypes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] startRecoverySessionWithCertPath(java.lang.String sessionId, java.lang.String rootCertificateAlias, android.security.keystore.recovery.RecoveryCertPath verifierCertPath, byte[] vaultParams, byte[] vaultChallenge, java.util.List<android.security.keystore.recovery.KeyChainProtectionParams> secrets) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.Map recoverKeyChainSnapshot(java.lang.String sessionId, byte[] recoveryKeyBlob, java.util.List<android.security.keystore.recovery.WrappedApplicationKey> applicationKeys) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void closeSession(java.lang.String sessionId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.widget.ILockSettings
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.widget.ILockSettings";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.widget.ILockSettings interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.widget.ILockSettings asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.widget.ILockSettings))) {
        return ((com.android.internal.widget.ILockSettings)iin);
      }
      return new com.android.internal.widget.ILockSettings.Stub.Proxy(obj);
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
        case TRANSACTION_setBoolean:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.setBoolean(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setLong:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.setLong(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setString:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.setString(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getBoolean:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.getBoolean(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLong:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          long _result = this.getLong(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getString:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _result = this.getString(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setLockCredential:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.setLockCredential(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resetKeyStore:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.resetKeyStore(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkCredential:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.widget.ICheckCredentialProgressCallback _arg3;
          _arg3 = com.android.internal.widget.ICheckCredentialProgressCallback.Stub.asInterface(data.readStrongBinder());
          com.android.internal.widget.VerifyCredentialResponse _result = this.checkCredential(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_verifyCredential:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          int _arg3;
          _arg3 = data.readInt();
          com.android.internal.widget.VerifyCredentialResponse _result = this.verifyCredential(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_verifyTiedProfileChallenge:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          int _arg3;
          _arg3 = data.readInt();
          com.android.internal.widget.VerifyCredentialResponse _result = this.verifyTiedProfileChallenge(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_checkVoldPassword:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.checkVoldPassword(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_havePattern:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.havePattern(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_havePassword:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.havePassword(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getHashFactor:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _result = this.getHashFactor(_arg0, _arg1);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_setSeparateProfileChallengeEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.setSeparateProfileChallengeEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSeparateProfileChallengeEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getSeparateProfileChallengeEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerStrongAuthTracker:
        {
          data.enforceInterface(descriptor);
          android.app.trust.IStrongAuthTracker _arg0;
          _arg0 = android.app.trust.IStrongAuthTracker.Stub.asInterface(data.readStrongBinder());
          this.registerStrongAuthTracker(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterStrongAuthTracker:
        {
          data.enforceInterface(descriptor);
          android.app.trust.IStrongAuthTracker _arg0;
          _arg0 = android.app.trust.IStrongAuthTracker.Stub.asInterface(data.readStrongBinder());
          this.unregisterStrongAuthTracker(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requireStrongAuth:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.requireStrongAuth(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_systemReady:
        {
          data.enforceInterface(descriptor);
          this.systemReady();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_userPresent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.userPresent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getStrongAuthForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getStrongAuthForUser(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasPendingEscrowToken:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.hasPendingEscrowToken(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_initRecoveryServiceWithSigFile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.initRecoveryServiceWithSigFile(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getKeyChainSnapshot:
        {
          data.enforceInterface(descriptor);
          android.security.keystore.recovery.KeyChainSnapshot _result = this.getKeyChainSnapshot();
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
        case TRANSACTION_generateKey:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.generateKey(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_generateKeyWithMetadata:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          java.lang.String _result = this.generateKeyWithMetadata(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_importKey:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          java.lang.String _result = this.importKey(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_importKeyWithMetadata:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          java.lang.String _result = this.importKeyWithMetadata(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getKey:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.getKey(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_removeKey:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeKey(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSnapshotCreatedPendingIntent:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setSnapshotCreatedPendingIntent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setServerParams:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.setServerParams(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRecoveryStatus:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setRecoveryStatus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRecoveryStatus:
        {
          data.enforceInterface(descriptor);
          java.util.Map _result = this.getRecoveryStatus();
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_setRecoverySecretTypes:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          this.setRecoverySecretTypes(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRecoverySecretTypes:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getRecoverySecretTypes();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_startRecoverySessionWithCertPath:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.security.keystore.recovery.RecoveryCertPath _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.security.keystore.recovery.RecoveryCertPath.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          byte[] _arg3;
          _arg3 = data.createByteArray();
          byte[] _arg4;
          _arg4 = data.createByteArray();
          java.util.List<android.security.keystore.recovery.KeyChainProtectionParams> _arg5;
          _arg5 = data.createTypedArrayList(android.security.keystore.recovery.KeyChainProtectionParams.CREATOR);
          byte[] _result = this.startRecoverySessionWithCertPath(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_recoverKeyChainSnapshot:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          java.util.List<android.security.keystore.recovery.WrappedApplicationKey> _arg2;
          _arg2 = data.createTypedArrayList(android.security.keystore.recovery.WrappedApplicationKey.CREATOR);
          java.util.Map _result = this.recoverKeyChainSnapshot(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_closeSession:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.closeSession(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.widget.ILockSettings
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
      @Override public void setBoolean(java.lang.String key, boolean value, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(((value)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBoolean, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBoolean(key, value, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setLong(java.lang.String key, long value, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeLong(value);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLong, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLong(key, value, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setString(java.lang.String key, java.lang.String value, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeString(value);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setString(key, value, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean getBoolean(java.lang.String key, boolean defaultValue, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(((defaultValue)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBoolean, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBoolean(key, defaultValue, userId);
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
      @Override public long getLong(java.lang.String key, long defaultValue, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeLong(defaultValue);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLong, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLong(key, defaultValue, userId);
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
      @Override public java.lang.String getString(java.lang.String key, java.lang.String defaultValue, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeString(defaultValue);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getString(key, defaultValue, userId);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setLockCredential(byte[] credential, int type, byte[] savedCredential, int requestedQuality, int userId, boolean allowUntrustedChange) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(credential);
          _data.writeInt(type);
          _data.writeByteArray(savedCredential);
          _data.writeInt(requestedQuality);
          _data.writeInt(userId);
          _data.writeInt(((allowUntrustedChange)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLockCredential, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLockCredential(credential, type, savedCredential, requestedQuality, userId, allowUntrustedChange);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resetKeyStore(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetKeyStore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetKeyStore(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public com.android.internal.widget.VerifyCredentialResponse checkCredential(byte[] credential, int type, int userId, com.android.internal.widget.ICheckCredentialProgressCallback progressCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.widget.VerifyCredentialResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(credential);
          _data.writeInt(type);
          _data.writeInt(userId);
          _data.writeStrongBinder((((progressCallback!=null))?(progressCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkCredential, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkCredential(credential, type, userId, progressCallback);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.widget.VerifyCredentialResponse.CREATOR.createFromParcel(_reply);
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
      @Override public com.android.internal.widget.VerifyCredentialResponse verifyCredential(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.widget.VerifyCredentialResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(credential);
          _data.writeInt(type);
          _data.writeLong(challenge);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_verifyCredential, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().verifyCredential(credential, type, challenge, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.widget.VerifyCredentialResponse.CREATOR.createFromParcel(_reply);
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
      @Override public com.android.internal.widget.VerifyCredentialResponse verifyTiedProfileChallenge(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.widget.VerifyCredentialResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(credential);
          _data.writeInt(type);
          _data.writeLong(challenge);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_verifyTiedProfileChallenge, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().verifyTiedProfileChallenge(credential, type, challenge, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.widget.VerifyCredentialResponse.CREATOR.createFromParcel(_reply);
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
      @Override public boolean checkVoldPassword(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkVoldPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkVoldPassword(userId);
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
      @Override public boolean havePattern(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_havePattern, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().havePattern(userId);
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
      @Override public boolean havePassword(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_havePassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().havePassword(userId);
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
      @Override public byte[] getHashFactor(byte[] currentCredential, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(currentCredential);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getHashFactor, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getHashFactor(currentCredential, userId);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setSeparateProfileChallengeEnabled(int userId, boolean enabled, byte[] managedUserPassword) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeByteArray(managedUserPassword);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSeparateProfileChallengeEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSeparateProfileChallengeEnabled(userId, enabled, managedUserPassword);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean getSeparateProfileChallengeEnabled(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSeparateProfileChallengeEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSeparateProfileChallengeEnabled(userId);
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
      @Override public void registerStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((tracker!=null))?(tracker.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerStrongAuthTracker, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerStrongAuthTracker(tracker);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((tracker!=null))?(tracker.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterStrongAuthTracker, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterStrongAuthTracker(tracker);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requireStrongAuth(int strongAuthReason, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(strongAuthReason);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requireStrongAuth, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requireStrongAuth(strongAuthReason, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void systemReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_systemReady, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().systemReady();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void userPresent(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_userPresent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().userPresent(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getStrongAuthForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStrongAuthForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStrongAuthForUser(userId);
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
      @Override public boolean hasPendingEscrowToken(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasPendingEscrowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasPendingEscrowToken(userId);
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
      // Keystore RecoveryController methods.
      // {@code ServiceSpecificException} may be thrown to signal an error, which caller can
      // convert to  {@code RecoveryManagerException}.

      @Override public void initRecoveryServiceWithSigFile(java.lang.String rootCertificateAlias, byte[] recoveryServiceCertFile, byte[] recoveryServiceSigFile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rootCertificateAlias);
          _data.writeByteArray(recoveryServiceCertFile);
          _data.writeByteArray(recoveryServiceSigFile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_initRecoveryServiceWithSigFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initRecoveryServiceWithSigFile(rootCertificateAlias, recoveryServiceCertFile, recoveryServiceSigFile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.security.keystore.recovery.KeyChainSnapshot getKeyChainSnapshot() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.security.keystore.recovery.KeyChainSnapshot _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKeyChainSnapshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKeyChainSnapshot();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.security.keystore.recovery.KeyChainSnapshot.CREATOR.createFromParcel(_reply);
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
      @Override public java.lang.String generateKey(java.lang.String alias) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          boolean _status = mRemote.transact(Stub.TRANSACTION_generateKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().generateKey(alias);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String generateKeyWithMetadata(java.lang.String alias, byte[] metadata) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          _data.writeByteArray(metadata);
          boolean _status = mRemote.transact(Stub.TRANSACTION_generateKeyWithMetadata, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().generateKeyWithMetadata(alias, metadata);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String importKey(java.lang.String alias, byte[] keyBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          _data.writeByteArray(keyBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_importKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().importKey(alias, keyBytes);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String importKeyWithMetadata(java.lang.String alias, byte[] keyBytes, byte[] metadata) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          _data.writeByteArray(keyBytes);
          _data.writeByteArray(metadata);
          boolean _status = mRemote.transact(Stub.TRANSACTION_importKeyWithMetadata, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().importKeyWithMetadata(alias, keyBytes, metadata);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getKey(java.lang.String alias) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKey(alias);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void removeKey(java.lang.String alias) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeKey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeKey(alias);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSnapshotCreatedPendingIntent(android.app.PendingIntent intent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSnapshotCreatedPendingIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSnapshotCreatedPendingIntent(intent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setServerParams(byte[] serverParams) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(serverParams);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setServerParams, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setServerParams(serverParams);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRecoveryStatus(java.lang.String alias, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRecoveryStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRecoveryStatus(alias, status);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.Map getRecoveryStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRecoveryStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRecoveryStatus();
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setRecoverySecretTypes(int[] secretTypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(secretTypes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRecoverySecretTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRecoverySecretTypes(secretTypes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getRecoverySecretTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRecoverySecretTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRecoverySecretTypes();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public byte[] startRecoverySessionWithCertPath(java.lang.String sessionId, java.lang.String rootCertificateAlias, android.security.keystore.recovery.RecoveryCertPath verifierCertPath, byte[] vaultParams, byte[] vaultChallenge, java.util.List<android.security.keystore.recovery.KeyChainProtectionParams> secrets) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(sessionId);
          _data.writeString(rootCertificateAlias);
          if ((verifierCertPath!=null)) {
            _data.writeInt(1);
            verifierCertPath.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(vaultParams);
          _data.writeByteArray(vaultChallenge);
          _data.writeTypedList(secrets);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecoverySessionWithCertPath, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startRecoverySessionWithCertPath(sessionId, rootCertificateAlias, verifierCertPath, vaultParams, vaultChallenge, secrets);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.Map recoverKeyChainSnapshot(java.lang.String sessionId, byte[] recoveryKeyBlob, java.util.List<android.security.keystore.recovery.WrappedApplicationKey> applicationKeys) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(sessionId);
          _data.writeByteArray(recoveryKeyBlob);
          _data.writeTypedList(applicationKeys);
          boolean _status = mRemote.transact(Stub.TRANSACTION_recoverKeyChainSnapshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().recoverKeyChainSnapshot(sessionId, recoveryKeyBlob, applicationKeys);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void closeSession(java.lang.String sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSession(sessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.widget.ILockSettings sDefaultImpl;
    }
    static final int TRANSACTION_setBoolean = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setLong = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getBoolean = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getLong = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setLockCredential = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_resetKeyStore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_checkCredential = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_verifyCredential = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_verifyTiedProfileChallenge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_checkVoldPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_havePattern = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_havePassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getHashFactor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setSeparateProfileChallengeEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getSeparateProfileChallengeEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_registerStrongAuthTracker = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_unregisterStrongAuthTracker = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_requireStrongAuth = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_systemReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_userPresent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getStrongAuthForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_hasPendingEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_initRecoveryServiceWithSigFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getKeyChainSnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_generateKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_generateKeyWithMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_importKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_importKeyWithMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_removeKey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_setSnapshotCreatedPendingIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_setServerParams = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_setRecoveryStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getRecoveryStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_setRecoverySecretTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getRecoverySecretTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_startRecoverySessionWithCertPath = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_recoverKeyChainSnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_closeSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    public static boolean setDefaultImpl(com.android.internal.widget.ILockSettings impl) {
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
    public static com.android.internal.widget.ILockSettings getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:33:1:33:25")
  public void setBoolean(java.lang.String key, boolean value, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:35:1:35:25")
  public void setLong(java.lang.String key, long value, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:37:1:37:25")
  public void setString(java.lang.String key, java.lang.String value, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:39:1:39:25")
  public boolean getBoolean(java.lang.String key, boolean defaultValue, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:41:1:41:25")
  public long getLong(java.lang.String key, long defaultValue, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:43:1:43:25")
  public java.lang.String getString(java.lang.String key, java.lang.String defaultValue, int userId) throws android.os.RemoteException;
  public void setLockCredential(byte[] credential, int type, byte[] savedCredential, int requestedQuality, int userId, boolean allowUntrustedChange) throws android.os.RemoteException;
  public void resetKeyStore(int userId) throws android.os.RemoteException;
  public com.android.internal.widget.VerifyCredentialResponse checkCredential(byte[] credential, int type, int userId, com.android.internal.widget.ICheckCredentialProgressCallback progressCallback) throws android.os.RemoteException;
  public com.android.internal.widget.VerifyCredentialResponse verifyCredential(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException;
  public com.android.internal.widget.VerifyCredentialResponse verifyTiedProfileChallenge(byte[] credential, int type, long challenge, int userId) throws android.os.RemoteException;
  public boolean checkVoldPassword(int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:52:1:52:25")
  public boolean havePattern(int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/widget/ILockSettings.aidl:54:1:54:25")
  public boolean havePassword(int userId) throws android.os.RemoteException;
  public byte[] getHashFactor(byte[] currentCredential, int userId) throws android.os.RemoteException;
  public void setSeparateProfileChallengeEnabled(int userId, boolean enabled, byte[] managedUserPassword) throws android.os.RemoteException;
  public boolean getSeparateProfileChallengeEnabled(int userId) throws android.os.RemoteException;
  public void registerStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException;
  public void unregisterStrongAuthTracker(android.app.trust.IStrongAuthTracker tracker) throws android.os.RemoteException;
  public void requireStrongAuth(int strongAuthReason, int userId) throws android.os.RemoteException;
  public void systemReady() throws android.os.RemoteException;
  public void userPresent(int userId) throws android.os.RemoteException;
  public int getStrongAuthForUser(int userId) throws android.os.RemoteException;
  public boolean hasPendingEscrowToken(int userId) throws android.os.RemoteException;
  // Keystore RecoveryController methods.
  // {@code ServiceSpecificException} may be thrown to signal an error, which caller can
  // convert to  {@code RecoveryManagerException}.

  public void initRecoveryServiceWithSigFile(java.lang.String rootCertificateAlias, byte[] recoveryServiceCertFile, byte[] recoveryServiceSigFile) throws android.os.RemoteException;
  public android.security.keystore.recovery.KeyChainSnapshot getKeyChainSnapshot() throws android.os.RemoteException;
  public java.lang.String generateKey(java.lang.String alias) throws android.os.RemoteException;
  public java.lang.String generateKeyWithMetadata(java.lang.String alias, byte[] metadata) throws android.os.RemoteException;
  public java.lang.String importKey(java.lang.String alias, byte[] keyBytes) throws android.os.RemoteException;
  public java.lang.String importKeyWithMetadata(java.lang.String alias, byte[] keyBytes, byte[] metadata) throws android.os.RemoteException;
  public java.lang.String getKey(java.lang.String alias) throws android.os.RemoteException;
  public void removeKey(java.lang.String alias) throws android.os.RemoteException;
  public void setSnapshotCreatedPendingIntent(android.app.PendingIntent intent) throws android.os.RemoteException;
  public void setServerParams(byte[] serverParams) throws android.os.RemoteException;
  public void setRecoveryStatus(java.lang.String alias, int status) throws android.os.RemoteException;
  public java.util.Map getRecoveryStatus() throws android.os.RemoteException;
  public void setRecoverySecretTypes(int[] secretTypes) throws android.os.RemoteException;
  public int[] getRecoverySecretTypes() throws android.os.RemoteException;
  public byte[] startRecoverySessionWithCertPath(java.lang.String sessionId, java.lang.String rootCertificateAlias, android.security.keystore.recovery.RecoveryCertPath verifierCertPath, byte[] vaultParams, byte[] vaultChallenge, java.util.List<android.security.keystore.recovery.KeyChainProtectionParams> secrets) throws android.os.RemoteException;
  public java.util.Map recoverKeyChainSnapshot(java.lang.String sessionId, byte[] recoveryKeyBlob, java.util.List<android.security.keystore.recovery.WrappedApplicationKey> applicationKeys) throws android.os.RemoteException;
  public void closeSession(java.lang.String sessionId) throws android.os.RemoteException;
}
