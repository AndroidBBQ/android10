/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware;
/**
 * @hide
 */
public interface ISensorPrivacyListener extends android.os.IInterface
{
  /** Default implementation for ISensorPrivacyListener. */
  public static class Default implements android.hardware.ISensorPrivacyListener
  {
    // Since these transactions are also called from native code, these must be kept in sync with
    // the ones in
    //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyListener.aidl
    // =============== Beginning of transactions used on native side as well ======================

    @Override public void onSensorPrivacyChanged(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.ISensorPrivacyListener
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.ISensorPrivacyListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.ISensorPrivacyListener interface,
     * generating a proxy if needed.
     */
    public static android.hardware.ISensorPrivacyListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.ISensorPrivacyListener))) {
        return ((android.hardware.ISensorPrivacyListener)iin);
      }
      return new android.hardware.ISensorPrivacyListener.Stub.Proxy(obj);
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
        case TRANSACTION_onSensorPrivacyChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onSensorPrivacyChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.ISensorPrivacyListener
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
      // Since these transactions are also called from native code, these must be kept in sync with
      // the ones in
      //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyListener.aidl
      // =============== Beginning of transactions used on native side as well ======================

      @Override public void onSensorPrivacyChanged(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSensorPrivacyChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSensorPrivacyChanged(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.ISensorPrivacyListener sDefaultImpl;
    }
    static final int TRANSACTION_onSensorPrivacyChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.ISensorPrivacyListener impl) {
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
    public static android.hardware.ISensorPrivacyListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Since these transactions are also called from native code, these must be kept in sync with
  // the ones in
  //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyListener.aidl
  // =============== Beginning of transactions used on native side as well ======================

  public void onSensorPrivacyChanged(boolean enabled) throws android.os.RemoteException;
}
