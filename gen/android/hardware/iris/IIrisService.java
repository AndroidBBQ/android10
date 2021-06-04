/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.iris;
/**
 * Communication channel from client to the iris service. These methods are all require the
 * MANAGE_BIOMETRIC signature permission.
 * @hide
 */
public interface IIrisService extends android.os.IInterface
{
  /** Default implementation for IIrisService. */
  public static class Default implements android.hardware.iris.IIrisService
  {
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.iris.IIrisService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.iris.IIrisService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.iris.IIrisService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.iris.IIrisService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.iris.IIrisService))) {
        return ((android.hardware.iris.IIrisService)iin);
      }
      return new android.hardware.iris.IIrisService.Stub.Proxy(obj);
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
    private static class Proxy implements android.hardware.iris.IIrisService
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
      public static android.hardware.iris.IIrisService sDefaultImpl;
    }
    public static boolean setDefaultImpl(android.hardware.iris.IIrisService impl) {
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
    public static android.hardware.iris.IIrisService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
}
