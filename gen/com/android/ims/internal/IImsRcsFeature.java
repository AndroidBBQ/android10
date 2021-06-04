/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * See RcsFeature for more information.
 * {@hide}
 */
public interface IImsRcsFeature extends android.os.IInterface
{
  /** Default implementation for IImsRcsFeature. */
  public static class Default implements com.android.ims.internal.IImsRcsFeature
  {
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsRcsFeature
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsRcsFeature";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsRcsFeature interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsRcsFeature asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsRcsFeature))) {
        return ((com.android.ims.internal.IImsRcsFeature)iin);
      }
      return new com.android.ims.internal.IImsRcsFeature.Stub.Proxy(obj);
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
    private static class Proxy implements com.android.ims.internal.IImsRcsFeature
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
      public static com.android.ims.internal.IImsRcsFeature sDefaultImpl;
    }
    public static boolean setDefaultImpl(com.android.ims.internal.IImsRcsFeature impl) {
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
    public static com.android.ims.internal.IImsRcsFeature getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
}
