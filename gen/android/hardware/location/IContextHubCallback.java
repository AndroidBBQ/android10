/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * @hide
 */
public interface IContextHubCallback extends android.os.IInterface
{
  /** Default implementation for IContextHubCallback. */
  public static class Default implements android.hardware.location.IContextHubCallback
  {
    @Override public void onMessageReceipt(int hubId, int nanoAppId, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IContextHubCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IContextHubCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IContextHubCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IContextHubCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IContextHubCallback))) {
        return ((android.hardware.location.IContextHubCallback)iin);
      }
      return new android.hardware.location.IContextHubCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onMessageReceipt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.location.ContextHubMessage _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.location.ContextHubMessage.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onMessageReceipt(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IContextHubCallback
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
      @Override public void onMessageReceipt(int hubId, int nanoAppId, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(hubId);
          _data.writeInt(nanoAppId);
          if ((msg!=null)) {
            _data.writeInt(1);
            msg.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageReceipt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageReceipt(hubId, nanoAppId, msg);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.location.IContextHubCallback sDefaultImpl;
    }
    static final int TRANSACTION_onMessageReceipt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.location.IContextHubCallback impl) {
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
    public static android.hardware.location.IContextHubCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onMessageReceipt(int hubId, int nanoAppId, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException;
}
