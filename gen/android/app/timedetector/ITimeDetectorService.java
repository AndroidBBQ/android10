/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.timedetector;
/**
 * System private API to comunicate with time detector service.
 *
 * <p>Used by parts of the Android system with signals associated with the device's time to provide
 * information to the Time Detector Service.
 *
 * <p>Use the {@link android.app.timedetector.TimeDetector} class rather than going through
 * this Binder interface directly. See {@link android.app.timedetector.TimeDetectorService} for
 * more complete documentation.
 *
 *
 * {@hide}
 */
public interface ITimeDetectorService extends android.os.IInterface
{
  /** Default implementation for ITimeDetectorService. */
  public static class Default implements android.app.timedetector.ITimeDetectorService
  {
    @Override public void suggestTime(android.app.timedetector.TimeSignal timeSignal) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.timedetector.ITimeDetectorService
  {
    private static final java.lang.String DESCRIPTOR = "android.app.timedetector.ITimeDetectorService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.timedetector.ITimeDetectorService interface,
     * generating a proxy if needed.
     */
    public static android.app.timedetector.ITimeDetectorService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.timedetector.ITimeDetectorService))) {
        return ((android.app.timedetector.ITimeDetectorService)iin);
      }
      return new android.app.timedetector.ITimeDetectorService.Stub.Proxy(obj);
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
        case TRANSACTION_suggestTime:
        {
          data.enforceInterface(descriptor);
          android.app.timedetector.TimeSignal _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.timedetector.TimeSignal.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.suggestTime(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.timedetector.ITimeDetectorService
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
      @Override public void suggestTime(android.app.timedetector.TimeSignal timeSignal) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((timeSignal!=null)) {
            _data.writeInt(1);
            timeSignal.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_suggestTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().suggestTime(timeSignal);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.timedetector.ITimeDetectorService sDefaultImpl;
    }
    static final int TRANSACTION_suggestTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.timedetector.ITimeDetectorService impl) {
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
    public static android.app.timedetector.ITimeDetectorService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void suggestTime(android.app.timedetector.TimeSignal timeSignal) throws android.os.RemoteException;
}
