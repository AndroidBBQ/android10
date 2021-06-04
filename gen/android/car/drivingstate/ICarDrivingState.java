/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.drivingstate;
/**
 * Binder interface for {@link android.car.drivingstate.CarDrivingStateManager}.
 * Check {@link android.car.drivingstate.CarDrivingStateManager} APIs for expected behavior of
 * each call.
 *
 * @hide
 */
public interface ICarDrivingState extends android.os.IInterface
{
  /** Default implementation for ICarDrivingState. */
  public static class Default implements android.car.drivingstate.ICarDrivingState
  {
    @Override public void registerDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public android.car.drivingstate.CarDrivingStateEvent getCurrentDrivingState() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void injectDrivingState(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.drivingstate.ICarDrivingState
  {
    private static final java.lang.String DESCRIPTOR = "android.car.drivingstate.ICarDrivingState";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.drivingstate.ICarDrivingState interface,
     * generating a proxy if needed.
     */
    public static android.car.drivingstate.ICarDrivingState asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.drivingstate.ICarDrivingState))) {
        return ((android.car.drivingstate.ICarDrivingState)iin);
      }
      return new android.car.drivingstate.ICarDrivingState.Stub.Proxy(obj);
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
        case TRANSACTION_registerDrivingStateChangeListener:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.ICarDrivingStateChangeListener _arg0;
          _arg0 = android.car.drivingstate.ICarDrivingStateChangeListener.Stub.asInterface(data.readStrongBinder());
          this.registerDrivingStateChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterDrivingStateChangeListener:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.ICarDrivingStateChangeListener _arg0;
          _arg0 = android.car.drivingstate.ICarDrivingStateChangeListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterDrivingStateChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentDrivingState:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.CarDrivingStateEvent _result = this.getCurrentDrivingState();
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
        case TRANSACTION_injectDrivingState:
        {
          data.enforceInterface(descriptor);
          android.car.drivingstate.CarDrivingStateEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.drivingstate.CarDrivingStateEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.injectDrivingState(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.drivingstate.ICarDrivingState
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
      @Override public void registerDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDrivingStateChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerDrivingStateChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDrivingStateChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDrivingStateChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.car.drivingstate.CarDrivingStateEvent getCurrentDrivingState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.drivingstate.CarDrivingStateEvent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentDrivingState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentDrivingState();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.drivingstate.CarDrivingStateEvent.CREATOR.createFromParcel(_reply);
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
      @Override public void injectDrivingState(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectDrivingState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().injectDrivingState(event);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.drivingstate.ICarDrivingState sDefaultImpl;
    }
    static final int TRANSACTION_registerDrivingStateChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterDrivingStateChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCurrentDrivingState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_injectDrivingState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.car.drivingstate.ICarDrivingState impl) {
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
    public static android.car.drivingstate.ICarDrivingState getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException;
  public void unregisterDrivingStateChangeListener(android.car.drivingstate.ICarDrivingStateChangeListener listener) throws android.os.RemoteException;
  public android.car.drivingstate.CarDrivingStateEvent getCurrentDrivingState() throws android.os.RemoteException;
  public void injectDrivingState(android.car.drivingstate.CarDrivingStateEvent event) throws android.os.RemoteException;
}
