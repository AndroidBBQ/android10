/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.biometrics;
/**
 * Callback when lockout period expired and clients are allowed to authenticate again.
 * @hide
 */
public interface IBiometricServiceLockoutResetCallback extends android.os.IInterface
{
  /** Default implementation for IBiometricServiceLockoutResetCallback. */
  public static class Default implements android.hardware.biometrics.IBiometricServiceLockoutResetCallback
  {
    /**
         * A wakelock will be held until the reciever calls back into {@param callback}
         */
    @Override public void onLockoutReset(long deviceId, android.os.IRemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.biometrics.IBiometricServiceLockoutResetCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.biometrics.IBiometricServiceLockoutResetCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.biometrics.IBiometricServiceLockoutResetCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.biometrics.IBiometricServiceLockoutResetCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.biometrics.IBiometricServiceLockoutResetCallback))) {
        return ((android.hardware.biometrics.IBiometricServiceLockoutResetCallback)iin);
      }
      return new android.hardware.biometrics.IBiometricServiceLockoutResetCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onLockoutReset:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.IRemoteCallback _arg1;
          _arg1 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          this.onLockoutReset(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.biometrics.IBiometricServiceLockoutResetCallback
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
           * A wakelock will be held until the reciever calls back into {@param callback}
           */
      @Override public void onLockoutReset(long deviceId, android.os.IRemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLockoutReset, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLockoutReset(deviceId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.biometrics.IBiometricServiceLockoutResetCallback sDefaultImpl;
    }
    static final int TRANSACTION_onLockoutReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.biometrics.IBiometricServiceLockoutResetCallback impl) {
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
    public static android.hardware.biometrics.IBiometricServiceLockoutResetCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * A wakelock will be held until the reciever calls back into {@param callback}
       */
  public void onLockoutReset(long deviceId, android.os.IRemoteCallback callback) throws android.os.RemoteException;
}
