/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.admin;
/**
 * @hide
 */
public interface IDeviceAdminService extends android.os.IInterface
{
  /** Default implementation for IDeviceAdminService. */
  public static class Default implements android.app.admin.IDeviceAdminService
  {
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.admin.IDeviceAdminService
  {
    private static final java.lang.String DESCRIPTOR = "android.app.admin.IDeviceAdminService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.admin.IDeviceAdminService interface,
     * generating a proxy if needed.
     */
    public static android.app.admin.IDeviceAdminService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.admin.IDeviceAdminService))) {
        return ((android.app.admin.IDeviceAdminService)iin);
      }
      return new android.app.admin.IDeviceAdminService.Stub.Proxy(obj);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.admin.IDeviceAdminService
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
      public static android.app.admin.IDeviceAdminService sDefaultImpl;
    }
    public static boolean setDefaultImpl(android.app.admin.IDeviceAdminService impl) {
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
    public static android.app.admin.IDeviceAdminService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
}
