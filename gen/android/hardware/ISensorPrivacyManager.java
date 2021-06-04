/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware;
/** @hide */
public interface ISensorPrivacyManager extends android.os.IInterface
{
  /** Default implementation for ISensorPrivacyManager. */
  public static class Default implements android.hardware.ISensorPrivacyManager
  {
    // Since these transactions are also called from native code, these must be kept in sync with
    // the ones in
    //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyManager.aidl
    // =============== Beginning of transactions used on native side as well ======================

    @Override public void addSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException
    {
    }
    @Override public boolean isSensorPrivacyEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setSensorPrivacy(boolean enable) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.ISensorPrivacyManager
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.ISensorPrivacyManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.ISensorPrivacyManager interface,
     * generating a proxy if needed.
     */
    public static android.hardware.ISensorPrivacyManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.ISensorPrivacyManager))) {
        return ((android.hardware.ISensorPrivacyManager)iin);
      }
      return new android.hardware.ISensorPrivacyManager.Stub.Proxy(obj);
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
        case TRANSACTION_addSensorPrivacyListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.ISensorPrivacyListener _arg0;
          _arg0 = android.hardware.ISensorPrivacyListener.Stub.asInterface(data.readStrongBinder());
          this.addSensorPrivacyListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeSensorPrivacyListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.ISensorPrivacyListener _arg0;
          _arg0 = android.hardware.ISensorPrivacyListener.Stub.asInterface(data.readStrongBinder());
          this.removeSensorPrivacyListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isSensorPrivacyEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSensorPrivacyEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setSensorPrivacy:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSensorPrivacy(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.ISensorPrivacyManager
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
      //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyManager.aidl
      // =============== Beginning of transactions used on native side as well ======================

      @Override public void addSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSensorPrivacyListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSensorPrivacyListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSensorPrivacyListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSensorPrivacyListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isSensorPrivacyEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSensorPrivacyEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSensorPrivacyEnabled();
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
      @Override public void setSensorPrivacy(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSensorPrivacy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSensorPrivacy(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.ISensorPrivacyManager sDefaultImpl;
    }
    static final int TRANSACTION_addSensorPrivacyListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeSensorPrivacyListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isSensorPrivacyEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setSensorPrivacy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.hardware.ISensorPrivacyManager impl) {
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
    public static android.hardware.ISensorPrivacyManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Since these transactions are also called from native code, these must be kept in sync with
  // the ones in
  //   frameworks/native/libs/sensorprivacy/aidl/android/hardware/ISensorPrivacyManager.aidl
  // =============== Beginning of transactions used on native side as well ======================

  public void addSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException;
  public void removeSensorPrivacyListener(android.hardware.ISensorPrivacyListener listener) throws android.os.RemoteException;
  public boolean isSensorPrivacyEnabled() throws android.os.RemoteException;
  public void setSensorPrivacy(boolean enable) throws android.os.RemoteException;
}
