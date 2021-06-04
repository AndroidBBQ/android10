/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.euicc;
/** @hide */
public interface IGetEuiccInfoCallback extends android.os.IInterface
{
  /** Default implementation for IGetEuiccInfoCallback. */
  public static class Default implements android.service.euicc.IGetEuiccInfoCallback
  {
    @Override public void onSuccess(android.telephony.euicc.EuiccInfo euiccInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.euicc.IGetEuiccInfoCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.euicc.IGetEuiccInfoCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.euicc.IGetEuiccInfoCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.euicc.IGetEuiccInfoCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.euicc.IGetEuiccInfoCallback))) {
        return ((android.service.euicc.IGetEuiccInfoCallback)iin);
      }
      return new android.service.euicc.IGetEuiccInfoCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSuccess:
        {
          data.enforceInterface(descriptor);
          android.telephony.euicc.EuiccInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.euicc.EuiccInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSuccess(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.euicc.IGetEuiccInfoCallback
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
      @Override public void onSuccess(android.telephony.euicc.EuiccInfo euiccInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((euiccInfo!=null)) {
            _data.writeInt(1);
            euiccInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccess(euiccInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.euicc.IGetEuiccInfoCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.euicc.IGetEuiccInfoCallback impl) {
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
    public static android.service.euicc.IGetEuiccInfoCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/euicc/IGetEuiccInfoCallback.aidl:23:1:23:25")
  public void onSuccess(android.telephony.euicc.EuiccInfo euiccInfo) throws android.os.RemoteException;
}
