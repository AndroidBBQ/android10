/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.content.pm;
/**
 * Implemented by CarAppBlockingPolicyService
 * @hide
 */
public interface ICarAppBlockingPolicy extends android.os.IInterface
{
  /** Default implementation for ICarAppBlockingPolicy. */
  public static class Default implements android.car.content.pm.ICarAppBlockingPolicy
  {
    @Override public void setAppBlockingPolicySetter(android.car.content.pm.ICarAppBlockingPolicySetter setter) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.content.pm.ICarAppBlockingPolicy
  {
    private static final java.lang.String DESCRIPTOR = "android.car.content.pm.ICarAppBlockingPolicy";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.content.pm.ICarAppBlockingPolicy interface,
     * generating a proxy if needed.
     */
    public static android.car.content.pm.ICarAppBlockingPolicy asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.content.pm.ICarAppBlockingPolicy))) {
        return ((android.car.content.pm.ICarAppBlockingPolicy)iin);
      }
      return new android.car.content.pm.ICarAppBlockingPolicy.Stub.Proxy(obj);
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
        case TRANSACTION_setAppBlockingPolicySetter:
        {
          data.enforceInterface(descriptor);
          android.car.content.pm.ICarAppBlockingPolicySetter _arg0;
          _arg0 = android.car.content.pm.ICarAppBlockingPolicySetter.Stub.asInterface(data.readStrongBinder());
          this.setAppBlockingPolicySetter(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.content.pm.ICarAppBlockingPolicy
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
      @Override public void setAppBlockingPolicySetter(android.car.content.pm.ICarAppBlockingPolicySetter setter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((setter!=null))?(setter.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppBlockingPolicySetter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppBlockingPolicySetter(setter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.content.pm.ICarAppBlockingPolicy sDefaultImpl;
    }
    static final int TRANSACTION_setAppBlockingPolicySetter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.content.pm.ICarAppBlockingPolicy impl) {
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
    public static android.car.content.pm.ICarAppBlockingPolicy getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setAppBlockingPolicySetter(android.car.content.pm.ICarAppBlockingPolicySetter setter) throws android.os.RemoteException;
}
