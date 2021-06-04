/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * An interface for location providers implementing the Geofencing service
 *
 * {@hide}
 */
public interface IGeofenceProvider extends android.os.IInterface
{
  /** Default implementation for IGeofenceProvider. */
  public static class Default implements android.location.IGeofenceProvider
  {
    @Override public void setGeofenceHardware(android.hardware.location.IGeofenceHardware proxy) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.IGeofenceProvider
  {
    private static final java.lang.String DESCRIPTOR = "android.location.IGeofenceProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.IGeofenceProvider interface,
     * generating a proxy if needed.
     */
    public static android.location.IGeofenceProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.IGeofenceProvider))) {
        return ((android.location.IGeofenceProvider)iin);
      }
      return new android.location.IGeofenceProvider.Stub.Proxy(obj);
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
        case TRANSACTION_setGeofenceHardware:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.IGeofenceHardware _arg0;
          _arg0 = android.hardware.location.IGeofenceHardware.Stub.asInterface(data.readStrongBinder());
          this.setGeofenceHardware(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.IGeofenceProvider
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
      @Override public void setGeofenceHardware(android.hardware.location.IGeofenceHardware proxy) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((proxy!=null))?(proxy.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGeofenceHardware, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGeofenceHardware(proxy);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.location.IGeofenceProvider sDefaultImpl;
    }
    static final int TRANSACTION_setGeofenceHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.location.IGeofenceProvider impl) {
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
    public static android.location.IGeofenceProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/android/location/IGeofenceProvider.aidl:27:1:27:25")
  public void setGeofenceHardware(android.hardware.location.IGeofenceHardware proxy) throws android.os.RemoteException;
}
