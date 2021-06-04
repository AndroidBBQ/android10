/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.data;
/**
 * The qualified networks service call back interface
 * @hide
 */
public interface IQualifiedNetworksServiceCallback extends android.os.IInterface
{
  /** Default implementation for IQualifiedNetworksServiceCallback. */
  public static class Default implements android.telephony.data.IQualifiedNetworksServiceCallback
  {
    @Override public void onQualifiedNetworkTypesChanged(int apnTypes, int[] qualifiedNetworkTypes) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.data.IQualifiedNetworksServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.data.IQualifiedNetworksServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.data.IQualifiedNetworksServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.data.IQualifiedNetworksServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.data.IQualifiedNetworksServiceCallback))) {
        return ((android.telephony.data.IQualifiedNetworksServiceCallback)iin);
      }
      return new android.telephony.data.IQualifiedNetworksServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onQualifiedNetworkTypesChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          this.onQualifiedNetworkTypesChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.data.IQualifiedNetworksServiceCallback
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
      @Override public void onQualifiedNetworkTypesChanged(int apnTypes, int[] qualifiedNetworkTypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(apnTypes);
          _data.writeIntArray(qualifiedNetworkTypes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQualifiedNetworkTypesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQualifiedNetworkTypesChanged(apnTypes, qualifiedNetworkTypes);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.data.IQualifiedNetworksServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onQualifiedNetworkTypesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.telephony.data.IQualifiedNetworksServiceCallback impl) {
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
    public static android.telephony.data.IQualifiedNetworksServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onQualifiedNetworkTypesChanged(int apnTypes, int[] qualifiedNetworkTypes) throws android.os.RemoteException;
}
