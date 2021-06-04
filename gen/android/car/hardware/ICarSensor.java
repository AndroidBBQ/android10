/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.hardware;
/** @hide */
public interface ICarSensor extends android.os.IInterface
{
  /** Default implementation for ICarSensor. */
  public static class Default implements android.car.hardware.ICarSensor
  {
    @Override public int[] getSupportedSensors() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * register a callback or update registration if already updated.
         * @param sensorType sensor to listen with this callback.
         * @param rate sensor rate.
         * @return false if requested sensors cannot be subscribed / started.
         */
    @Override public boolean registerOrUpdateSensorListener(int sensorType, int rate, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * get latest sensor event for the type. If there was no update after car connection, it will
         * return null immediately.
         */
    @Override public android.car.hardware.CarSensorEvent getLatestSensorEvent(int sensorType) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Stop listening for the given sensor type. All other sensors registered before will not
         * be affected.
         */
    @Override public void unregisterSensorListener(int sensorType, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException
    {
    }
    /**
         * get config flags and config array for the sensor type
         */
    @Override public android.car.hardware.CarSensorConfig getSensorConfig(int sensorType) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.hardware.ICarSensor
  {
    private static final java.lang.String DESCRIPTOR = "android.car.hardware.ICarSensor";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.hardware.ICarSensor interface,
     * generating a proxy if needed.
     */
    public static android.car.hardware.ICarSensor asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.hardware.ICarSensor))) {
        return ((android.car.hardware.ICarSensor)iin);
      }
      return new android.car.hardware.ICarSensor.Stub.Proxy(obj);
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
        case TRANSACTION_getSupportedSensors:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getSupportedSensors();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_registerOrUpdateSensorListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.car.hardware.ICarSensorEventListener _arg2;
          _arg2 = android.car.hardware.ICarSensorEventListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerOrUpdateSensorListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLatestSensorEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.hardware.CarSensorEvent _result = this.getLatestSensorEvent(_arg0);
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
        case TRANSACTION_unregisterSensorListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.hardware.ICarSensorEventListener _arg1;
          _arg1 = android.car.hardware.ICarSensorEventListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterSensorListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSensorConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.hardware.CarSensorConfig _result = this.getSensorConfig(_arg0);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.hardware.ICarSensor
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
      @Override public int[] getSupportedSensors() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedSensors, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedSensors();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * register a callback or update registration if already updated.
           * @param sensorType sensor to listen with this callback.
           * @param rate sensor rate.
           * @return false if requested sensors cannot be subscribed / started.
           */
      @Override public boolean registerOrUpdateSensorListener(int sensorType, int rate, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sensorType);
          _data.writeInt(rate);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerOrUpdateSensorListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerOrUpdateSensorListener(sensorType, rate, callback);
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
      /**
           * get latest sensor event for the type. If there was no update after car connection, it will
           * return null immediately.
           */
      @Override public android.car.hardware.CarSensorEvent getLatestSensorEvent(int sensorType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.hardware.CarSensorEvent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sensorType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLatestSensorEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLatestSensorEvent(sensorType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.hardware.CarSensorEvent.CREATOR.createFromParcel(_reply);
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
      /**
           * Stop listening for the given sensor type. All other sensors registered before will not
           * be affected.
           */
      @Override public void unregisterSensorListener(int sensorType, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sensorType);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterSensorListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterSensorListener(sensorType, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * get config flags and config array for the sensor type
           */
      @Override public android.car.hardware.CarSensorConfig getSensorConfig(int sensorType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.hardware.CarSensorConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sensorType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSensorConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSensorConfig(sensorType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.hardware.CarSensorConfig.CREATOR.createFromParcel(_reply);
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
      public static android.car.hardware.ICarSensor sDefaultImpl;
    }
    static final int TRANSACTION_getSupportedSensors = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerOrUpdateSensorListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getLatestSensorEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_unregisterSensorListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getSensorConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.car.hardware.ICarSensor impl) {
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
    public static android.car.hardware.ICarSensor getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int[] getSupportedSensors() throws android.os.RemoteException;
  /**
       * register a callback or update registration if already updated.
       * @param sensorType sensor to listen with this callback.
       * @param rate sensor rate.
       * @return false if requested sensors cannot be subscribed / started.
       */
  public boolean registerOrUpdateSensorListener(int sensorType, int rate, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException;
  /**
       * get latest sensor event for the type. If there was no update after car connection, it will
       * return null immediately.
       */
  public android.car.hardware.CarSensorEvent getLatestSensorEvent(int sensorType) throws android.os.RemoteException;
  /**
       * Stop listening for the given sensor type. All other sensors registered before will not
       * be affected.
       */
  public void unregisterSensorListener(int sensorType, android.car.hardware.ICarSensorEventListener callback) throws android.os.RemoteException;
  /**
       * get config flags and config array for the sensor type
       */
  public android.car.hardware.CarSensorConfig getSensorConfig(int sensorType) throws android.os.RemoteException;
}
