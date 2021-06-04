/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * {@hide}
 */
public interface IThermalService extends android.os.IInterface
{
  /** Default implementation for IThermalService. */
  public static class Default implements android.os.IThermalService
  {
    /**
          * Register a listener for thermal events.
          * @param listener the IThermalEventListener to be notified.
          * {@hide}
          */
    @Override public boolean registerThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Register a listener for thermal events on given temperature type.
          * @param listener the IThermalEventListener to be notified.
          * @param type the temperature type IThermalEventListener to be notified.
          * @return true if registered successfully.
          * {@hide}
          */
    @Override public boolean registerThermalEventListenerWithType(android.os.IThermalEventListener listener, int type) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Unregister a previously-registered listener for thermal events.
          * @param listener the IThermalEventListener to no longer be notified.
          * @return true if unregistered successfully.
          * {@hide}
          */
    @Override public boolean unregisterThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Get current temperature with its throttling status.
          * @return list of {@link android.os.Temperature}.
          * {@hide}
          */
    @Override public java.util.List<android.os.Temperature> getCurrentTemperatures() throws android.os.RemoteException
    {
      return null;
    }
    /**
          * Get current temperature with its throttling status on given temperature type.
          * @param type the temperature type to query.
          * @return list of {@link android.os.Temperature}.
          * {@hide}
          */
    @Override public java.util.List<android.os.Temperature> getCurrentTemperaturesWithType(int type) throws android.os.RemoteException
    {
      return null;
    }
    /**
          * Register a listener for thermal status change.
          * @param listener the {@link android.os.IThermalStatusListener} to be notified.
          * @return true if registered successfully.
          * {@hide}
          */
    @Override public boolean registerThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Unregister a previously-registered listener for thermal status.
          * @param listener the {@link android.os.IThermalStatusListener} to no longer be notified.
          * @return true if unregistered successfully.
          * {@hide}
          */
    @Override public boolean unregisterThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Get current thermal status.
          * @return status defined in {@link android.os.Temperature}.
          * {@hide}
          */
    @Override public int getCurrentThermalStatus() throws android.os.RemoteException
    {
      return 0;
    }
    /**
          * Get current cooling devices.
          * @return list of {@link android.os.CoolingDevice}.
          * {@hide}
          */
    @Override public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevices() throws android.os.RemoteException
    {
      return null;
    }
    /**
          * Get current cooling devices on given type.
          * @param type the cooling device type to query.
          * @return list of {@link android.os.CoolingDevice}.
          * {@hide}
          */
    @Override public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevicesWithType(int type) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IThermalService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IThermalService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IThermalService interface,
     * generating a proxy if needed.
     */
    public static android.os.IThermalService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IThermalService))) {
        return ((android.os.IThermalService)iin);
      }
      return new android.os.IThermalService.Stub.Proxy(obj);
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
        case TRANSACTION_registerThermalEventListener:
        {
          data.enforceInterface(descriptor);
          android.os.IThermalEventListener _arg0;
          _arg0 = android.os.IThermalEventListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerThermalEventListener(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerThermalEventListenerWithType:
        {
          data.enforceInterface(descriptor);
          android.os.IThermalEventListener _arg0;
          _arg0 = android.os.IThermalEventListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.registerThermalEventListenerWithType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterThermalEventListener:
        {
          data.enforceInterface(descriptor);
          android.os.IThermalEventListener _arg0;
          _arg0 = android.os.IThermalEventListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unregisterThermalEventListener(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentTemperatures:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.os.Temperature> _result = this.getCurrentTemperatures();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getCurrentTemperaturesWithType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.os.Temperature> _result = this.getCurrentTemperaturesWithType(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerThermalStatusListener:
        {
          data.enforceInterface(descriptor);
          android.os.IThermalStatusListener _arg0;
          _arg0 = android.os.IThermalStatusListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerThermalStatusListener(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterThermalStatusListener:
        {
          data.enforceInterface(descriptor);
          android.os.IThermalStatusListener _arg0;
          _arg0 = android.os.IThermalStatusListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unregisterThermalStatusListener(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentThermalStatus:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCurrentThermalStatus();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCurrentCoolingDevices:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.os.CoolingDevice> _result = this.getCurrentCoolingDevices();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getCurrentCoolingDevicesWithType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.os.CoolingDevice> _result = this.getCurrentCoolingDevicesWithType(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IThermalService
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
            * Register a listener for thermal events.
            * @param listener the IThermalEventListener to be notified.
            * {@hide}
            */
      @Override public boolean registerThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerThermalEventListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerThermalEventListener(listener);
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
            * Register a listener for thermal events on given temperature type.
            * @param listener the IThermalEventListener to be notified.
            * @param type the temperature type IThermalEventListener to be notified.
            * @return true if registered successfully.
            * {@hide}
            */
      @Override public boolean registerThermalEventListenerWithType(android.os.IThermalEventListener listener, int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerThermalEventListenerWithType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerThermalEventListenerWithType(listener, type);
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
            * Unregister a previously-registered listener for thermal events.
            * @param listener the IThermalEventListener to no longer be notified.
            * @return true if unregistered successfully.
            * {@hide}
            */
      @Override public boolean unregisterThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterThermalEventListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unregisterThermalEventListener(listener);
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
            * Get current temperature with its throttling status.
            * @return list of {@link android.os.Temperature}.
            * {@hide}
            */
      @Override public java.util.List<android.os.Temperature> getCurrentTemperatures() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.Temperature> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTemperatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTemperatures();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.os.Temperature.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Get current temperature with its throttling status on given temperature type.
            * @param type the temperature type to query.
            * @return list of {@link android.os.Temperature}.
            * {@hide}
            */
      @Override public java.util.List<android.os.Temperature> getCurrentTemperaturesWithType(int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.Temperature> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTemperaturesWithType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTemperaturesWithType(type);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.os.Temperature.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Register a listener for thermal status change.
            * @param listener the {@link android.os.IThermalStatusListener} to be notified.
            * @return true if registered successfully.
            * {@hide}
            */
      @Override public boolean registerThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerThermalStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerThermalStatusListener(listener);
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
            * Unregister a previously-registered listener for thermal status.
            * @param listener the {@link android.os.IThermalStatusListener} to no longer be notified.
            * @return true if unregistered successfully.
            * {@hide}
            */
      @Override public boolean unregisterThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterThermalStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unregisterThermalStatusListener(listener);
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
            * Get current thermal status.
            * @return status defined in {@link android.os.Temperature}.
            * {@hide}
            */
      @Override public int getCurrentThermalStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentThermalStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentThermalStatus();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Get current cooling devices.
            * @return list of {@link android.os.CoolingDevice}.
            * {@hide}
            */
      @Override public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevices() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.CoolingDevice> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentCoolingDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentCoolingDevices();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.os.CoolingDevice.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Get current cooling devices on given type.
            * @param type the cooling device type to query.
            * @return list of {@link android.os.CoolingDevice}.
            * {@hide}
            */
      @Override public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevicesWithType(int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.CoolingDevice> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentCoolingDevicesWithType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentCoolingDevicesWithType(type);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.os.CoolingDevice.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.IThermalService sDefaultImpl;
    }
    static final int TRANSACTION_registerThermalEventListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerThermalEventListenerWithType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterThermalEventListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getCurrentTemperatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getCurrentTemperaturesWithType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_registerThermalStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_unregisterThermalStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getCurrentThermalStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getCurrentCoolingDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getCurrentCoolingDevicesWithType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.os.IThermalService impl) {
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
    public static android.os.IThermalService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
        * Register a listener for thermal events.
        * @param listener the IThermalEventListener to be notified.
        * {@hide}
        */
  public boolean registerThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException;
  /**
        * Register a listener for thermal events on given temperature type.
        * @param listener the IThermalEventListener to be notified.
        * @param type the temperature type IThermalEventListener to be notified.
        * @return true if registered successfully.
        * {@hide}
        */
  public boolean registerThermalEventListenerWithType(android.os.IThermalEventListener listener, int type) throws android.os.RemoteException;
  /**
        * Unregister a previously-registered listener for thermal events.
        * @param listener the IThermalEventListener to no longer be notified.
        * @return true if unregistered successfully.
        * {@hide}
        */
  public boolean unregisterThermalEventListener(android.os.IThermalEventListener listener) throws android.os.RemoteException;
  /**
        * Get current temperature with its throttling status.
        * @return list of {@link android.os.Temperature}.
        * {@hide}
        */
  public java.util.List<android.os.Temperature> getCurrentTemperatures() throws android.os.RemoteException;
  /**
        * Get current temperature with its throttling status on given temperature type.
        * @param type the temperature type to query.
        * @return list of {@link android.os.Temperature}.
        * {@hide}
        */
  public java.util.List<android.os.Temperature> getCurrentTemperaturesWithType(int type) throws android.os.RemoteException;
  /**
        * Register a listener for thermal status change.
        * @param listener the {@link android.os.IThermalStatusListener} to be notified.
        * @return true if registered successfully.
        * {@hide}
        */
  public boolean registerThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException;
  /**
        * Unregister a previously-registered listener for thermal status.
        * @param listener the {@link android.os.IThermalStatusListener} to no longer be notified.
        * @return true if unregistered successfully.
        * {@hide}
        */
  public boolean unregisterThermalStatusListener(android.os.IThermalStatusListener listener) throws android.os.RemoteException;
  /**
        * Get current thermal status.
        * @return status defined in {@link android.os.Temperature}.
        * {@hide}
        */
  public int getCurrentThermalStatus() throws android.os.RemoteException;
  /**
        * Get current cooling devices.
        * @return list of {@link android.os.CoolingDevice}.
        * {@hide}
        */
  public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevices() throws android.os.RemoteException;
  /**
        * Get current cooling devices on given type.
        * @param type the cooling device type to query.
        * @return list of {@link android.os.CoolingDevice}.
        * {@hide}
        */
  public java.util.List<android.os.CoolingDevice> getCurrentCoolingDevicesWithType(int type) throws android.os.RemoteException;
}
