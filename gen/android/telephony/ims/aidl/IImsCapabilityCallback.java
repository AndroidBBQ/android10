/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See ImsFeature#CapabilityCallback for more information.
 * {@hide}
 */
public interface IImsCapabilityCallback extends android.os.IInterface
{
  /** Default implementation for IImsCapabilityCallback. */
  public static class Default implements android.telephony.ims.aidl.IImsCapabilityCallback
  {
    @Override public void onQueryCapabilityConfiguration(int capability, int radioTech, boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void onChangeCapabilityConfigurationError(int capability, int radioTech, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onCapabilitiesStatusChanged(int config) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsCapabilityCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsCapabilityCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsCapabilityCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsCapabilityCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsCapabilityCallback))) {
        return ((android.telephony.ims.aidl.IImsCapabilityCallback)iin);
      }
      return new android.telephony.ims.aidl.IImsCapabilityCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onQueryCapabilityConfiguration:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onQueryCapabilityConfiguration(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onChangeCapabilityConfigurationError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onChangeCapabilityConfigurationError(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onCapabilitiesStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onCapabilitiesStatusChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsCapabilityCallback
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
      @Override public void onQueryCapabilityConfiguration(int capability, int radioTech, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(capability);
          _data.writeInt(radioTech);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQueryCapabilityConfiguration, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQueryCapabilityConfiguration(capability, radioTech, enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onChangeCapabilityConfigurationError(int capability, int radioTech, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(capability);
          _data.writeInt(radioTech);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onChangeCapabilityConfigurationError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onChangeCapabilityConfigurationError(capability, radioTech, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCapabilitiesStatusChanged(int config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(config);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCapabilitiesStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCapabilitiesStatusChanged(config);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsCapabilityCallback sDefaultImpl;
    }
    static final int TRANSACTION_onQueryCapabilityConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onChangeCapabilityConfigurationError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onCapabilitiesStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsCapabilityCallback impl) {
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
    public static android.telephony.ims.aidl.IImsCapabilityCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onQueryCapabilityConfiguration(int capability, int radioTech, boolean enabled) throws android.os.RemoteException;
  public void onChangeCapabilityConfigurationError(int capability, int radioTech, int reason) throws android.os.RemoteException;
  public void onCapabilitiesStatusChanged(int config) throws android.os.RemoteException;
}
