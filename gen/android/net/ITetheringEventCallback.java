/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Callback class for receiving tethering changed events
 * @hide
 */
public interface ITetheringEventCallback extends android.os.IInterface
{
  /** Default implementation for ITetheringEventCallback. */
  public static class Default implements android.net.ITetheringEventCallback
  {
    @Override public void onUpstreamChanged(android.net.Network network) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ITetheringEventCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ITetheringEventCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ITetheringEventCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.ITetheringEventCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ITetheringEventCallback))) {
        return ((android.net.ITetheringEventCallback)iin);
      }
      return new android.net.ITetheringEventCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onUpstreamChanged:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onUpstreamChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ITetheringEventCallback
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
      @Override public void onUpstreamChanged(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUpstreamChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUpstreamChanged(network);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.ITetheringEventCallback sDefaultImpl;
    }
    static final int TRANSACTION_onUpstreamChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.net.ITetheringEventCallback impl) {
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
    public static android.net.ITetheringEventCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUpstreamChanged(android.net.Network network) throws android.os.RemoteException;
}
