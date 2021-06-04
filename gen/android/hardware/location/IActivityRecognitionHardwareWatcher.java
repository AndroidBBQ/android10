/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * Activity Recognition Hardware watcher. This interface can be used to receive interfaces to
 * implementations of {@link IActivityRecognitionHardware}.
 *
 * @deprecated use {@link IActivityRecognitionHardwareClient} instead.

 * @hide
 */
public interface IActivityRecognitionHardwareWatcher extends android.os.IInterface
{
  /** Default implementation for IActivityRecognitionHardwareWatcher. */
  public static class Default implements android.hardware.location.IActivityRecognitionHardwareWatcher
  {
    /**
         * Hardware Activity-Recognition availability event.
         */
    @Override public void onInstanceChanged(android.hardware.location.IActivityRecognitionHardware instance) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IActivityRecognitionHardwareWatcher
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IActivityRecognitionHardwareWatcher";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IActivityRecognitionHardwareWatcher interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IActivityRecognitionHardwareWatcher asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IActivityRecognitionHardwareWatcher))) {
        return ((android.hardware.location.IActivityRecognitionHardwareWatcher)iin);
      }
      return new android.hardware.location.IActivityRecognitionHardwareWatcher.Stub.Proxy(obj);
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
        case TRANSACTION_onInstanceChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.IActivityRecognitionHardware _arg0;
          _arg0 = android.hardware.location.IActivityRecognitionHardware.Stub.asInterface(data.readStrongBinder());
          this.onInstanceChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IActivityRecognitionHardwareWatcher
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
           * Hardware Activity-Recognition availability event.
           */
      @Override public void onInstanceChanged(android.hardware.location.IActivityRecognitionHardware instance) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((instance!=null))?(instance.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInstanceChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInstanceChanged(instance);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.location.IActivityRecognitionHardwareWatcher sDefaultImpl;
    }
    static final int TRANSACTION_onInstanceChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.location.IActivityRecognitionHardwareWatcher impl) {
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
    public static android.hardware.location.IActivityRecognitionHardwareWatcher getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Hardware Activity-Recognition availability event.
       */
  public void onInstanceChanged(android.hardware.location.IActivityRecognitionHardware instance) throws android.os.RemoteException;
}
