/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See ImsService#Listener for more information.
 * {@hide}
 */
public interface IImsServiceControllerListener extends android.os.IInterface
{
  /** Default implementation for IImsServiceControllerListener. */
  public static class Default implements android.telephony.ims.aidl.IImsServiceControllerListener
  {
    @Override public void onUpdateSupportedImsFeatures(android.telephony.ims.stub.ImsFeatureConfiguration c) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsServiceControllerListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsServiceControllerListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsServiceControllerListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsServiceControllerListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsServiceControllerListener))) {
        return ((android.telephony.ims.aidl.IImsServiceControllerListener)iin);
      }
      return new android.telephony.ims.aidl.IImsServiceControllerListener.Stub.Proxy(obj);
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
        case TRANSACTION_onUpdateSupportedImsFeatures:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.stub.ImsFeatureConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.stub.ImsFeatureConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onUpdateSupportedImsFeatures(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsServiceControllerListener
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
      @Override public void onUpdateSupportedImsFeatures(android.telephony.ims.stub.ImsFeatureConfiguration c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((c!=null)) {
            _data.writeInt(1);
            c.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUpdateSupportedImsFeatures, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUpdateSupportedImsFeatures(c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsServiceControllerListener sDefaultImpl;
    }
    static final int TRANSACTION_onUpdateSupportedImsFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsServiceControllerListener impl) {
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
    public static android.telephony.ims.aidl.IImsServiceControllerListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUpdateSupportedImsFeatures(android.telephony.ims.stub.ImsFeatureConfiguration c) throws android.os.RemoteException;
}
