/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.se.omapi;
/**
 * Interface to receive call-backs when the service is connected.
 * @hide
 */
public interface ISecureElementListener extends android.os.IInterface
{
  /** Default implementation for ISecureElementListener. */
  public static class Default implements android.se.omapi.ISecureElementListener
  {
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.se.omapi.ISecureElementListener
  {
    private static final java.lang.String DESCRIPTOR = "android.se.omapi.ISecureElementListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.se.omapi.ISecureElementListener interface,
     * generating a proxy if needed.
     */
    public static android.se.omapi.ISecureElementListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.se.omapi.ISecureElementListener))) {
        return ((android.se.omapi.ISecureElementListener)iin);
      }
      return new android.se.omapi.ISecureElementListener.Stub.Proxy(obj);
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
    private static class Proxy implements android.se.omapi.ISecureElementListener
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
      public static android.se.omapi.ISecureElementListener sDefaultImpl;
    }
    public static boolean setDefaultImpl(android.se.omapi.ISecureElementListener impl) {
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
    public static android.se.omapi.ISecureElementListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
}
