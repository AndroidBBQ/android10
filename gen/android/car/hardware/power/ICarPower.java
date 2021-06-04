/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.hardware.power;
/** @hide */
public interface ICarPower extends android.os.IInterface
{
  /** Default implementation for ICarPower. */
  public static class Default implements android.car.hardware.power.ICarPower
  {
    @Override public void registerListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
    {
    }
    @Override public void requestShutdownOnNextSuspend() throws android.os.RemoteException
    {
    }
    @Override public void finished(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
    {
    }
    @Override public void scheduleNextWakeupTime(int seconds) throws android.os.RemoteException
    {
    }
    @Override public void registerListenerWithCompletion(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.hardware.power.ICarPower
  {
    private static final java.lang.String DESCRIPTOR = "android.car.hardware.power.ICarPower";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.hardware.power.ICarPower interface,
     * generating a proxy if needed.
     */
    public static android.car.hardware.power.ICarPower asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.hardware.power.ICarPower))) {
        return ((android.car.hardware.power.ICarPower)iin);
      }
      return new android.car.hardware.power.ICarPower.Stub.Proxy(obj);
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
        case TRANSACTION_registerListener:
        {
          data.enforceInterface(descriptor);
          android.car.hardware.power.ICarPowerStateListener _arg0;
          _arg0 = android.car.hardware.power.ICarPowerStateListener.Stub.asInterface(data.readStrongBinder());
          this.registerListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterListener:
        {
          data.enforceInterface(descriptor);
          android.car.hardware.power.ICarPowerStateListener _arg0;
          _arg0 = android.car.hardware.power.ICarPowerStateListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestShutdownOnNextSuspend:
        {
          data.enforceInterface(descriptor);
          this.requestShutdownOnNextSuspend();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finished:
        {
          data.enforceInterface(descriptor);
          android.car.hardware.power.ICarPowerStateListener _arg0;
          _arg0 = android.car.hardware.power.ICarPowerStateListener.Stub.asInterface(data.readStrongBinder());
          this.finished(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_scheduleNextWakeupTime:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.scheduleNextWakeupTime(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerListenerWithCompletion:
        {
          data.enforceInterface(descriptor);
          android.car.hardware.power.ICarPowerStateListener _arg0;
          _arg0 = android.car.hardware.power.ICarPowerStateListener.Stub.asInterface(data.readStrongBinder());
          this.registerListenerWithCompletion(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.hardware.power.ICarPower
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
      @Override public void registerListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestShutdownOnNextSuspend() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestShutdownOnNextSuspend, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestShutdownOnNextSuspend();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finished(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finished, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finished(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void scheduleNextWakeupTime(int seconds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seconds);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleNextWakeupTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleNextWakeupTime(seconds);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerListenerWithCompletion(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerListenerWithCompletion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerListenerWithCompletion(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.hardware.power.ICarPower sDefaultImpl;
    }
    static final int TRANSACTION_registerListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestShutdownOnNextSuspend = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_finished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_scheduleNextWakeupTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_registerListenerWithCompletion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.car.hardware.power.ICarPower impl) {
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
    public static android.car.hardware.power.ICarPower getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException;
  public void unregisterListener(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException;
  public void requestShutdownOnNextSuspend() throws android.os.RemoteException;
  public void finished(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException;
  public void scheduleNextWakeupTime(int seconds) throws android.os.RemoteException;
  public void registerListenerWithCompletion(android.car.hardware.power.ICarPowerStateListener listener) throws android.os.RemoteException;
}
