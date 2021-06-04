/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/** @hide */
public interface IGeofenceHardwareMonitorCallback extends android.os.IInterface
{
  /** Default implementation for IGeofenceHardwareMonitorCallback. */
  public static class Default implements android.hardware.location.IGeofenceHardwareMonitorCallback
  {
    @Override public void onMonitoringSystemChange(android.hardware.location.GeofenceHardwareMonitorEvent event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IGeofenceHardwareMonitorCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IGeofenceHardwareMonitorCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IGeofenceHardwareMonitorCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IGeofenceHardwareMonitorCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IGeofenceHardwareMonitorCallback))) {
        return ((android.hardware.location.IGeofenceHardwareMonitorCallback)iin);
      }
      return new android.hardware.location.IGeofenceHardwareMonitorCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onMonitoringSystemChange:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.GeofenceHardwareMonitorEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.location.GeofenceHardwareMonitorEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onMonitoringSystemChange(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IGeofenceHardwareMonitorCallback
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
      @Override public void onMonitoringSystemChange(android.hardware.location.GeofenceHardwareMonitorEvent event) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMonitoringSystemChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMonitoringSystemChange(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.location.IGeofenceHardwareMonitorCallback sDefaultImpl;
    }
    static final int TRANSACTION_onMonitoringSystemChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.location.IGeofenceHardwareMonitorCallback impl) {
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
    public static android.hardware.location.IGeofenceHardwareMonitorCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onMonitoringSystemChange(android.hardware.location.GeofenceHardwareMonitorEvent event) throws android.os.RemoteException;
}
