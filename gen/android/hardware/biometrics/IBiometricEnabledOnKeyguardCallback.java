/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.biometrics;
/**
 * @hide
 */
public interface IBiometricEnabledOnKeyguardCallback extends android.os.IInterface
{
  /** Default implementation for IBiometricEnabledOnKeyguardCallback. */
  public static class Default implements android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback
  {
    @Override public void onChanged(android.hardware.biometrics.BiometricSourceType type, boolean enabled, int userId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback))) {
        return ((android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback)iin);
      }
      return new android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.biometrics.BiometricSourceType _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.biometrics.BiometricSourceType.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.onChanged(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback
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
      @Override public void onChanged(android.hardware.biometrics.BiometricSourceType type, boolean enabled, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((type!=null)) {
            _data.writeInt(1);
            type.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onChanged(type, enabled, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback sDefaultImpl;
    }
    static final int TRANSACTION_onChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback impl) {
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
    public static android.hardware.biometrics.IBiometricEnabledOnKeyguardCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onChanged(android.hardware.biometrics.BiometricSourceType type, boolean enabled, int userId) throws android.os.RemoteException;
}
