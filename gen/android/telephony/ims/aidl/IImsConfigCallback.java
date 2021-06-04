/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * Provides callback interface for ImsConfig when a value has changed.
 *
 * {@hide}
 */
public interface IImsConfigCallback extends android.os.IInterface
{
  /** Default implementation for IImsConfigCallback. */
  public static class Default implements android.telephony.ims.aidl.IImsConfigCallback
  {
    @Override public void onIntConfigChanged(int item, int value) throws android.os.RemoteException
    {
    }
    @Override public void onStringConfigChanged(int item, java.lang.String value) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsConfigCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsConfigCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsConfigCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsConfigCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsConfigCallback))) {
        return ((android.telephony.ims.aidl.IImsConfigCallback)iin);
      }
      return new android.telephony.ims.aidl.IImsConfigCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onIntConfigChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onIntConfigChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onStringConfigChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onStringConfigChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsConfigCallback
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
      @Override public void onIntConfigChanged(int item, int value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          _data.writeInt(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onIntConfigChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onIntConfigChanged(item, value);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStringConfigChanged(int item, java.lang.String value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(item);
          _data.writeString(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStringConfigChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStringConfigChanged(item, value);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsConfigCallback sDefaultImpl;
    }
    static final int TRANSACTION_onIntConfigChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onStringConfigChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsConfigCallback impl) {
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
    public static android.telephony.ims.aidl.IImsConfigCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onIntConfigChanged(int item, int value) throws android.os.RemoteException;
  public void onStringConfigChanged(int item, java.lang.String value) throws android.os.RemoteException;
}
