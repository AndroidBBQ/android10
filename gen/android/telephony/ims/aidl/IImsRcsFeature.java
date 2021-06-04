/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See RcsFeature for more information.
 * {@hide}
 */
public interface IImsRcsFeature extends android.os.IInterface
{
  /** Default implementation for IImsRcsFeature. */
  public static class Default implements android.telephony.ims.aidl.IImsRcsFeature
  {
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsRcsFeature
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsRcsFeature";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsRcsFeature interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsRcsFeature asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsRcsFeature))) {
        return ((android.telephony.ims.aidl.IImsRcsFeature)iin);
      }
      return new android.telephony.ims.aidl.IImsRcsFeature.Stub.Proxy(obj);
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
    private static class Proxy implements android.telephony.ims.aidl.IImsRcsFeature
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
      public static android.telephony.ims.aidl.IImsRcsFeature sDefaultImpl;
    }
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsRcsFeature impl) {
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
    public static android.telephony.ims.aidl.IImsRcsFeature getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
}
