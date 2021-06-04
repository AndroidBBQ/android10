/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/** @hide */
public interface IGeofenceHardwareCallback extends android.os.IInterface
{
  /** Default implementation for IGeofenceHardwareCallback. */
  public static class Default implements android.hardware.location.IGeofenceHardwareCallback
  {
    @Override public void onGeofenceTransition(int geofenceId, int transition, android.location.Location location, long timestamp, int monitoringType) throws android.os.RemoteException
    {
    }
    @Override public void onGeofenceAdd(int geofenceId, int status) throws android.os.RemoteException
    {
    }
    @Override public void onGeofenceRemove(int geofenceId, int status) throws android.os.RemoteException
    {
    }
    @Override public void onGeofencePause(int geofenceId, int status) throws android.os.RemoteException
    {
    }
    @Override public void onGeofenceResume(int geofenceId, int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IGeofenceHardwareCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IGeofenceHardwareCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IGeofenceHardwareCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IGeofenceHardwareCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IGeofenceHardwareCallback))) {
        return ((android.hardware.location.IGeofenceHardwareCallback)iin);
      }
      return new android.hardware.location.IGeofenceHardwareCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onGeofenceTransition:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.location.Location _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.location.Location.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          long _arg3;
          _arg3 = data.readLong();
          int _arg4;
          _arg4 = data.readInt();
          this.onGeofenceTransition(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onGeofenceAdd:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGeofenceAdd(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGeofenceRemove:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGeofenceRemove(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGeofencePause:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGeofencePause(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGeofenceResume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGeofenceResume(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IGeofenceHardwareCallback
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
      @Override public void onGeofenceTransition(int geofenceId, int transition, android.location.Location location, long timestamp, int monitoringType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(geofenceId);
          _data.writeInt(transition);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(timestamp);
          _data.writeInt(monitoringType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGeofenceTransition, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGeofenceTransition(geofenceId, transition, location, timestamp, monitoringType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGeofenceAdd(int geofenceId, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(geofenceId);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGeofenceAdd, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGeofenceAdd(geofenceId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGeofenceRemove(int geofenceId, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(geofenceId);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGeofenceRemove, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGeofenceRemove(geofenceId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGeofencePause(int geofenceId, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(geofenceId);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGeofencePause, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGeofencePause(geofenceId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGeofenceResume(int geofenceId, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(geofenceId);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGeofenceResume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGeofenceResume(geofenceId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.location.IGeofenceHardwareCallback sDefaultImpl;
    }
    static final int TRANSACTION_onGeofenceTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGeofenceAdd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onGeofenceRemove = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onGeofencePause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onGeofenceResume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.hardware.location.IGeofenceHardwareCallback impl) {
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
    public static android.hardware.location.IGeofenceHardwareCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGeofenceTransition(int geofenceId, int transition, android.location.Location location, long timestamp, int monitoringType) throws android.os.RemoteException;
  public void onGeofenceAdd(int geofenceId, int status) throws android.os.RemoteException;
  public void onGeofenceRemove(int geofenceId, int status) throws android.os.RemoteException;
  public void onGeofencePause(int geofenceId, int status) throws android.os.RemoteException;
  public void onGeofenceResume(int geofenceId, int status) throws android.os.RemoteException;
}
