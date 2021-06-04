/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.biometrics;
/**
 * Communication channel from
 *   1) BiometricDialogImpl (SysUI) back to BiometricService
 *   2) <Biometric>Service back to BiometricService
 * Receives messages from the above and does some handling before forwarding to BiometricPrompt
 * via IBiometricServiceReceiver.
 * @hide
 */
public interface IBiometricServiceReceiverInternal extends android.os.IInterface
{
  /** Default implementation for IBiometricServiceReceiverInternal. */
  public static class Default implements android.hardware.biometrics.IBiometricServiceReceiverInternal
  {
    // Notify BiometricService that authentication was successful. If user confirmation is required,
    // the auth token must be submitted into KeyStore.

    @Override public void onAuthenticationSucceeded(boolean requireConfirmation, byte[] token) throws android.os.RemoteException
    {
    }
    // Notify BiometricService that an error has occurred.

    @Override public void onAuthenticationFailed(int cookie, boolean requireConfirmation) throws android.os.RemoteException
    {
    }
    // Notify BiometricService than an error has occured. Forward to the correct receiver depending
    // on the cookie.

    @Override public void onError(int cookie, int error, java.lang.String message) throws android.os.RemoteException
    {
    }
    // Notifies that a biometric has been acquired.

    @Override public void onAcquired(int acquiredInfo, java.lang.String message) throws android.os.RemoteException
    {
    }
    // Notifies that the SystemUI dialog has been dismissed.

    @Override public void onDialogDismissed(int reason) throws android.os.RemoteException
    {
    }
    // Notifies that the user has pressed the "try again" button on SystemUI

    @Override public void onTryAgainPressed() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.biometrics.IBiometricServiceReceiverInternal
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.biometrics.IBiometricServiceReceiverInternal";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.biometrics.IBiometricServiceReceiverInternal interface,
     * generating a proxy if needed.
     */
    public static android.hardware.biometrics.IBiometricServiceReceiverInternal asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.biometrics.IBiometricServiceReceiverInternal))) {
        return ((android.hardware.biometrics.IBiometricServiceReceiverInternal)iin);
      }
      return new android.hardware.biometrics.IBiometricServiceReceiverInternal.Stub.Proxy(obj);
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
        case TRANSACTION_onAuthenticationSucceeded:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.onAuthenticationSucceeded(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onAuthenticationFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onAuthenticationFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.onError(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAcquired:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onAcquired(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDialogDismissed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDialogDismissed(_arg0);
          return true;
        }
        case TRANSACTION_onTryAgainPressed:
        {
          data.enforceInterface(descriptor);
          this.onTryAgainPressed();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.biometrics.IBiometricServiceReceiverInternal
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
      // Notify BiometricService that authentication was successful. If user confirmation is required,
      // the auth token must be submitted into KeyStore.

      @Override public void onAuthenticationSucceeded(boolean requireConfirmation, byte[] token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((requireConfirmation)?(1):(0)));
          _data.writeByteArray(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAuthenticationSucceeded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAuthenticationSucceeded(requireConfirmation, token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Notify BiometricService that an error has occurred.

      @Override public void onAuthenticationFailed(int cookie, boolean requireConfirmation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cookie);
          _data.writeInt(((requireConfirmation)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAuthenticationFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAuthenticationFailed(cookie, requireConfirmation);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Notify BiometricService than an error has occured. Forward to the correct receiver depending
      // on the cookie.

      @Override public void onError(int cookie, int error, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cookie);
          _data.writeInt(error);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(cookie, error, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Notifies that a biometric has been acquired.

      @Override public void onAcquired(int acquiredInfo, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(acquiredInfo);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAcquired, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAcquired(acquiredInfo, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Notifies that the SystemUI dialog has been dismissed.

      @Override public void onDialogDismissed(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDialogDismissed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDialogDismissed(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Notifies that the user has pressed the "try again" button on SystemUI

      @Override public void onTryAgainPressed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTryAgainPressed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTryAgainPressed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.biometrics.IBiometricServiceReceiverInternal sDefaultImpl;
    }
    static final int TRANSACTION_onAuthenticationSucceeded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAuthenticationFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onAcquired = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onDialogDismissed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onTryAgainPressed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.hardware.biometrics.IBiometricServiceReceiverInternal impl) {
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
    public static android.hardware.biometrics.IBiometricServiceReceiverInternal getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Notify BiometricService that authentication was successful. If user confirmation is required,
  // the auth token must be submitted into KeyStore.

  public void onAuthenticationSucceeded(boolean requireConfirmation, byte[] token) throws android.os.RemoteException;
  // Notify BiometricService that an error has occurred.

  public void onAuthenticationFailed(int cookie, boolean requireConfirmation) throws android.os.RemoteException;
  // Notify BiometricService than an error has occured. Forward to the correct receiver depending
  // on the cookie.

  public void onError(int cookie, int error, java.lang.String message) throws android.os.RemoteException;
  // Notifies that a biometric has been acquired.

  public void onAcquired(int acquiredInfo, java.lang.String message) throws android.os.RemoteException;
  // Notifies that the SystemUI dialog has been dismissed.

  public void onDialogDismissed(int reason) throws android.os.RemoteException;
  // Notifies that the user has pressed the "try again" button on SystemUI

  public void onTryAgainPressed() throws android.os.RemoteException;
}
