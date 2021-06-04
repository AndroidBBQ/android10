/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.gatekeeper;
/**
 * Interface for communication with GateKeeper, the
 * secure password storage daemon.
 *
 * This must be kept manually in sync with system/core/gatekeeperd
 * until AIDL can generate both C++ and Java bindings.
 *
 * @hide
 */
public interface IGateKeeperService extends android.os.IInterface
{
  /** Default implementation for IGateKeeperService. */
  public static class Default implements android.service.gatekeeper.IGateKeeperService
  {
    /**
         * Enrolls a password, returning the handle to the enrollment to be stored locally.
         * @param uid The Android user ID associated to this enrollment
         * @param currentPasswordHandle The previously enrolled handle, or null if none
         * @param currentPassword The previously enrolled plaintext password, or null if none.
         *                        If provided, must verify against the currentPasswordHandle.
         * @param desiredPassword The new desired password, for which a handle will be returned
         *                        upon success.
         * @return an EnrollResponse or null on failure
         */
    @Override public android.service.gatekeeper.GateKeeperResponse enroll(int uid, byte[] currentPasswordHandle, byte[] currentPassword, byte[] desiredPassword) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Verifies an enrolled handle against a provided, plaintext blob.
         * @param uid The Android user ID associated to this enrollment
         * @param enrolledPasswordHandle The handle against which the provided password will be
         *                               verified.
         * @param The plaintext blob to verify against enrolledPassword.
         * @return a VerifyResponse, or null on failure.
         */
    @Override public android.service.gatekeeper.GateKeeperResponse verify(int uid, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Verifies an enrolled handle against a provided, plaintext blob.
         * @param uid The Android user ID associated to this enrollment
         * @param challenge a challenge to authenticate agaisnt the device credential. If successful
         *                  authentication occurs, this value will be written to the returned
         *                  authentication attestation.
         * @param enrolledPasswordHandle The handle against which the provided password will be
         *                               verified.
         * @param The plaintext blob to verify against enrolledPassword.
         * @return a VerifyResponse with an attestation, or null on failure.
         */
    @Override public android.service.gatekeeper.GateKeeperResponse verifyChallenge(int uid, long challenge, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Retrieves the secure identifier for the user with the provided Android ID,
         * or 0 if none is found.
         * @param uid the Android user id
         */
    @Override public long getSecureUserId(int uid) throws android.os.RemoteException
    {
      return 0L;
    }
    /**
         * Clears secure user id associated with the provided Android ID.
         * Must be called when password is set to NONE.
         * @param uid the Android user id.
         */
    @Override public void clearSecureUserId(int uid) throws android.os.RemoteException
    {
    }
    /**
         * Notifies gatekeeper that device setup has been completed and any potentially still existing
         * state from before a factory reset can be cleaned up (if it has not been already).
         */
    @Override public void reportDeviceSetupComplete() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.gatekeeper.IGateKeeperService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.gatekeeper.IGateKeeperService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.gatekeeper.IGateKeeperService interface,
     * generating a proxy if needed.
     */
    public static android.service.gatekeeper.IGateKeeperService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.gatekeeper.IGateKeeperService))) {
        return ((android.service.gatekeeper.IGateKeeperService)iin);
      }
      return new android.service.gatekeeper.IGateKeeperService.Stub.Proxy(obj);
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
        case TRANSACTION_enroll:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          android.service.gatekeeper.GateKeeperResponse _result = this.enroll(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_verify:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          android.service.gatekeeper.GateKeeperResponse _result = this.verify(_arg0, _arg1, _arg2);
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
        case TRANSACTION_verifyChallenge:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          android.service.gatekeeper.GateKeeperResponse _result = this.verifyChallenge(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_getSecureUserId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _result = this.getSecureUserId(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_clearSecureUserId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.clearSecureUserId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportDeviceSetupComplete:
        {
          data.enforceInterface(descriptor);
          this.reportDeviceSetupComplete();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.gatekeeper.IGateKeeperService
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
      /**
           * Enrolls a password, returning the handle to the enrollment to be stored locally.
           * @param uid The Android user ID associated to this enrollment
           * @param currentPasswordHandle The previously enrolled handle, or null if none
           * @param currentPassword The previously enrolled plaintext password, or null if none.
           *                        If provided, must verify against the currentPasswordHandle.
           * @param desiredPassword The new desired password, for which a handle will be returned
           *                        upon success.
           * @return an EnrollResponse or null on failure
           */
      @Override public android.service.gatekeeper.GateKeeperResponse enroll(int uid, byte[] currentPasswordHandle, byte[] currentPassword, byte[] desiredPassword) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.service.gatekeeper.GateKeeperResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeByteArray(currentPasswordHandle);
          _data.writeByteArray(currentPassword);
          _data.writeByteArray(desiredPassword);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enroll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enroll(uid, currentPasswordHandle, currentPassword, desiredPassword);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.service.gatekeeper.GateKeeperResponse.CREATOR.createFromParcel(_reply);
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
      /**
           * Verifies an enrolled handle against a provided, plaintext blob.
           * @param uid The Android user ID associated to this enrollment
           * @param enrolledPasswordHandle The handle against which the provided password will be
           *                               verified.
           * @param The plaintext blob to verify against enrolledPassword.
           * @return a VerifyResponse, or null on failure.
           */
      @Override public android.service.gatekeeper.GateKeeperResponse verify(int uid, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.service.gatekeeper.GateKeeperResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeByteArray(enrolledPasswordHandle);
          _data.writeByteArray(providedPassword);
          boolean _status = mRemote.transact(Stub.TRANSACTION_verify, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().verify(uid, enrolledPasswordHandle, providedPassword);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.service.gatekeeper.GateKeeperResponse.CREATOR.createFromParcel(_reply);
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
      /**
           * Verifies an enrolled handle against a provided, plaintext blob.
           * @param uid The Android user ID associated to this enrollment
           * @param challenge a challenge to authenticate agaisnt the device credential. If successful
           *                  authentication occurs, this value will be written to the returned
           *                  authentication attestation.
           * @param enrolledPasswordHandle The handle against which the provided password will be
           *                               verified.
           * @param The plaintext blob to verify against enrolledPassword.
           * @return a VerifyResponse with an attestation, or null on failure.
           */
      @Override public android.service.gatekeeper.GateKeeperResponse verifyChallenge(int uid, long challenge, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.service.gatekeeper.GateKeeperResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeLong(challenge);
          _data.writeByteArray(enrolledPasswordHandle);
          _data.writeByteArray(providedPassword);
          boolean _status = mRemote.transact(Stub.TRANSACTION_verifyChallenge, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().verifyChallenge(uid, challenge, enrolledPasswordHandle, providedPassword);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.service.gatekeeper.GateKeeperResponse.CREATOR.createFromParcel(_reply);
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
      /**
           * Retrieves the secure identifier for the user with the provided Android ID,
           * or 0 if none is found.
           * @param uid the Android user id
           */
      @Override public long getSecureUserId(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSecureUserId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSecureUserId(uid);
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
      /**
           * Clears secure user id associated with the provided Android ID.
           * Must be called when password is set to NONE.
           * @param uid the Android user id.
           */
      @Override public void clearSecureUserId(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearSecureUserId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearSecureUserId(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Notifies gatekeeper that device setup has been completed and any potentially still existing
           * state from before a factory reset can be cleaned up (if it has not been already).
           */
      @Override public void reportDeviceSetupComplete() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportDeviceSetupComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportDeviceSetupComplete();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.gatekeeper.IGateKeeperService sDefaultImpl;
    }
    static final int TRANSACTION_enroll = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_verify = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_verifyChallenge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getSecureUserId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_clearSecureUserId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_reportDeviceSetupComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.service.gatekeeper.IGateKeeperService impl) {
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
    public static android.service.gatekeeper.IGateKeeperService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Enrolls a password, returning the handle to the enrollment to be stored locally.
       * @param uid The Android user ID associated to this enrollment
       * @param currentPasswordHandle The previously enrolled handle, or null if none
       * @param currentPassword The previously enrolled plaintext password, or null if none.
       *                        If provided, must verify against the currentPasswordHandle.
       * @param desiredPassword The new desired password, for which a handle will be returned
       *                        upon success.
       * @return an EnrollResponse or null on failure
       */
  public android.service.gatekeeper.GateKeeperResponse enroll(int uid, byte[] currentPasswordHandle, byte[] currentPassword, byte[] desiredPassword) throws android.os.RemoteException;
  /**
       * Verifies an enrolled handle against a provided, plaintext blob.
       * @param uid The Android user ID associated to this enrollment
       * @param enrolledPasswordHandle The handle against which the provided password will be
       *                               verified.
       * @param The plaintext blob to verify against enrolledPassword.
       * @return a VerifyResponse, or null on failure.
       */
  public android.service.gatekeeper.GateKeeperResponse verify(int uid, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException;
  /**
       * Verifies an enrolled handle against a provided, plaintext blob.
       * @param uid The Android user ID associated to this enrollment
       * @param challenge a challenge to authenticate agaisnt the device credential. If successful
       *                  authentication occurs, this value will be written to the returned
       *                  authentication attestation.
       * @param enrolledPasswordHandle The handle against which the provided password will be
       *                               verified.
       * @param The plaintext blob to verify against enrolledPassword.
       * @return a VerifyResponse with an attestation, or null on failure.
       */
  public android.service.gatekeeper.GateKeeperResponse verifyChallenge(int uid, long challenge, byte[] enrolledPasswordHandle, byte[] providedPassword) throws android.os.RemoteException;
  /**
       * Retrieves the secure identifier for the user with the provided Android ID,
       * or 0 if none is found.
       * @param uid the Android user id
       */
  public long getSecureUserId(int uid) throws android.os.RemoteException;
  /**
       * Clears secure user id associated with the provided Android ID.
       * Must be called when password is set to NONE.
       * @param uid the Android user id.
       */
  public void clearSecureUserId(int uid) throws android.os.RemoteException;
  /**
       * Notifies gatekeeper that device setup has been completed and any potentially still existing
       * state from before a factory reset can be cleaned up (if it has not been already).
       */
  public void reportDeviceSetupComplete() throws android.os.RemoteException;
}
