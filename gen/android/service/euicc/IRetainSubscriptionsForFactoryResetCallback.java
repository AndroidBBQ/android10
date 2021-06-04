/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.euicc;
/** @hide */
public interface IRetainSubscriptionsForFactoryResetCallback extends android.os.IInterface
{
  /** Default implementation for IRetainSubscriptionsForFactoryResetCallback. */
  public static class Default implements android.service.euicc.IRetainSubscriptionsForFactoryResetCallback
  {
    @Override public void onComplete(int result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.euicc.IRetainSubscriptionsForFactoryResetCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.euicc.IRetainSubscriptionsForFactoryResetCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.euicc.IRetainSubscriptionsForFactoryResetCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.euicc.IRetainSubscriptionsForFactoryResetCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.euicc.IRetainSubscriptionsForFactoryResetCallback))) {
        return ((android.service.euicc.IRetainSubscriptionsForFactoryResetCallback)iin);
      }
      return new android.service.euicc.IRetainSubscriptionsForFactoryResetCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onComplete(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.euicc.IRetainSubscriptionsForFactoryResetCallback
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
      @Override public void onComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.euicc.IRetainSubscriptionsForFactoryResetCallback sDefaultImpl;
    }
    static final int TRANSACTION_onComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.euicc.IRetainSubscriptionsForFactoryResetCallback impl) {
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
    public static android.service.euicc.IRetainSubscriptionsForFactoryResetCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/euicc/IRetainSubscriptionsForFactoryResetCallback.aidl:21:1:21:25")
  public void onComplete(int result) throws android.os.RemoteException;
}
