/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/** @hide */
public interface IGeofenceHardware extends android.os.IInterface
{
  /** Default implementation for IGeofenceHardware. */
  public static class Default implements android.hardware.location.IGeofenceHardware
  {
    @Override public void setGpsGeofenceHardware(android.location.IGpsGeofenceHardware service) throws android.os.RemoteException
    {
    }
    @Override public void setFusedGeofenceHardware(android.location.IFusedGeofenceHardware service) throws android.os.RemoteException
    {
    }
    @Override public int[] getMonitoringTypes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getStatusOfMonitoringType(int monitoringType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean addCircularFence(int monitoringType, android.hardware.location.GeofenceHardwareRequestParcelable request, android.hardware.location.IGeofenceHardwareCallback callback) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeGeofence(int id, int monitoringType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean pauseGeofence(int id, int monitoringType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean resumeGeofence(int id, int monitoringType, int monitorTransitions) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean registerForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean unregisterForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IGeofenceHardware
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IGeofenceHardware";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IGeofenceHardware interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IGeofenceHardware asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IGeofenceHardware))) {
        return ((android.hardware.location.IGeofenceHardware)iin);
      }
      return new android.hardware.location.IGeofenceHardware.Stub.Proxy(obj);
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
        case TRANSACTION_setGpsGeofenceHardware:
        {
          data.enforceInterface(descriptor);
          android.location.IGpsGeofenceHardware _arg0;
          _arg0 = android.location.IGpsGeofenceHardware.Stub.asInterface(data.readStrongBinder());
          this.setGpsGeofenceHardware(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFusedGeofenceHardware:
        {
          data.enforceInterface(descriptor);
          android.location.IFusedGeofenceHardware _arg0;
          _arg0 = android.location.IFusedGeofenceHardware.Stub.asInterface(data.readStrongBinder());
          this.setFusedGeofenceHardware(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMonitoringTypes:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getMonitoringTypes();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getStatusOfMonitoringType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getStatusOfMonitoringType(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addCircularFence:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.GeofenceHardwareRequestParcelable _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.location.GeofenceHardwareRequestParcelable.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.hardware.location.IGeofenceHardwareCallback _arg2;
          _arg2 = android.hardware.location.IGeofenceHardwareCallback.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.addCircularFence(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeGeofence:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.removeGeofence(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_pauseGeofence:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.pauseGeofence(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_resumeGeofence:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.resumeGeofence(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerForMonitorStateChangeCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IGeofenceHardwareMonitorCallback _arg1;
          _arg1 = android.hardware.location.IGeofenceHardwareMonitorCallback.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerForMonitorStateChangeCallback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterForMonitorStateChangeCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IGeofenceHardwareMonitorCallback _arg1;
          _arg1 = android.hardware.location.IGeofenceHardwareMonitorCallback.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unregisterForMonitorStateChangeCallback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IGeofenceHardware
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
      @Override public void setGpsGeofenceHardware(android.location.IGpsGeofenceHardware service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGpsGeofenceHardware, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGpsGeofenceHardware(service);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFusedGeofenceHardware(android.location.IFusedGeofenceHardware service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFusedGeofenceHardware, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFusedGeofenceHardware(service);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getMonitoringTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMonitoringTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMonitoringTypes();
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
      @Override public int getStatusOfMonitoringType(int monitoringType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(monitoringType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStatusOfMonitoringType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStatusOfMonitoringType(monitoringType);
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
      @Override public boolean addCircularFence(int monitoringType, android.hardware.location.GeofenceHardwareRequestParcelable request, android.hardware.location.IGeofenceHardwareCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(monitoringType);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCircularFence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addCircularFence(monitoringType, request, callback);
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
      @Override public boolean removeGeofence(int id, int monitoringType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          _data.writeInt(monitoringType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeGeofence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeGeofence(id, monitoringType);
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
      @Override public boolean pauseGeofence(int id, int monitoringType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          _data.writeInt(monitoringType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pauseGeofence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().pauseGeofence(id, monitoringType);
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
      @Override public boolean resumeGeofence(int id, int monitoringType, int monitorTransitions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          _data.writeInt(monitoringType);
          _data.writeInt(monitorTransitions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resumeGeofence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().resumeGeofence(id, monitoringType, monitorTransitions);
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
      @Override public boolean registerForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(monitoringType);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerForMonitorStateChangeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerForMonitorStateChangeCallback(monitoringType, callback);
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
      @Override public boolean unregisterForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(monitoringType);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterForMonitorStateChangeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unregisterForMonitorStateChangeCallback(monitoringType, callback);
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
      public static android.hardware.location.IGeofenceHardware sDefaultImpl;
    }
    static final int TRANSACTION_setGpsGeofenceHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setFusedGeofenceHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getMonitoringTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getStatusOfMonitoringType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addCircularFence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeGeofence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_pauseGeofence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_resumeGeofence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_registerForMonitorStateChangeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_unregisterForMonitorStateChangeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.hardware.location.IGeofenceHardware impl) {
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
    public static android.hardware.location.IGeofenceHardware getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setGpsGeofenceHardware(android.location.IGpsGeofenceHardware service) throws android.os.RemoteException;
  public void setFusedGeofenceHardware(android.location.IFusedGeofenceHardware service) throws android.os.RemoteException;
  public int[] getMonitoringTypes() throws android.os.RemoteException;
  public int getStatusOfMonitoringType(int monitoringType) throws android.os.RemoteException;
  public boolean addCircularFence(int monitoringType, android.hardware.location.GeofenceHardwareRequestParcelable request, android.hardware.location.IGeofenceHardwareCallback callback) throws android.os.RemoteException;
  public boolean removeGeofence(int id, int monitoringType) throws android.os.RemoteException;
  public boolean pauseGeofence(int id, int monitoringType) throws android.os.RemoteException;
  public boolean resumeGeofence(int id, int monitoringType, int monitorTransitions) throws android.os.RemoteException;
  public boolean registerForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException;
  public boolean unregisterForMonitorStateChangeCallback(int monitoringType, android.hardware.location.IGeofenceHardwareMonitorCallback callback) throws android.os.RemoteException;
}
