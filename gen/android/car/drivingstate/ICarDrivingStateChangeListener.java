/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.drivingstate;
/**
 * Binder callback for onDrivingStateChange.
 * @hide
 */
public interface ICarDrivingStateChangeListener extends android.os.IInterface
{
  /** Default implementation for ICarDrivingStateChangeListener. */
  public static class Default implements android.car.drivingstate.ICarDrivingStateChangeListener
  {
    @Override public void onDrivingStateChanged(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.drivingstate.ICarDrivingStateChangeListener
  {
    private static final java.lang.String DESCRIPTOR = "android.car.drivingstate.ICarDrivingStateChangeListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.drivingstate.ICarDrivingStateChangeListener interface,
     * generating a proxy if needed.
     */
    public static android.car.drivingstate.ICarDrivingStateChangeListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.drivingstate.ICarDrivingStateChangeListener))) {
        return ((android.car.drivingstate.ICarDrivingStateChangeListener)iin);
      }
      return new android.car.drivingstate.ICarDrivingStateChangeListener.Stub.Proxy(obj);
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
        case TRANSACTION_onDrivingStateChanged:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.CarDrivingStateEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.drivingstate.CarDrivingStateEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDrivingStateChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.drivingstate.ICarDrivingStateChangeListener
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
      @Override public void onDrivingStateChanged(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDrivingStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDrivingStateChanged(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.drivingstate.ICarDrivingStateChangeListener sDefaultImpl;
    }
    static final int TRANSACTION_onDrivingStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.drivingstate.ICarDrivingStateChangeListener impl) {
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
    public static android.car.drivingstate.ICarDrivingStateChangeListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDrivingStateChanged(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException;
}
