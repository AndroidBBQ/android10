/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.lowpan;
/** {@hide} */
public interface ILowpanEnergyScanCallback extends android.os.IInterface
{
  /** Default implementation for ILowpanEnergyScanCallback. */
  public static class Default implements android.net.lowpan.ILowpanEnergyScanCallback
  {
    @Override public void onEnergyScanResult(int channel, int rssi) throws android.os.RemoteException
    {
    }
    @Override public void onEnergyScanFinished() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.lowpan.ILowpanEnergyScanCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.lowpan.ILowpanEnergyScanCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.lowpan.ILowpanEnergyScanCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.lowpan.ILowpanEnergyScanCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.lowpan.ILowpanEnergyScanCallback))) {
        return ((android.net.lowpan.ILowpanEnergyScanCallback)iin);
      }
      return new android.net.lowpan.ILowpanEnergyScanCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onEnergyScanResult:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onEnergyScanResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onEnergyScanFinished:
        {
          data.enforceInterface(descriptor);
          this.onEnergyScanFinished();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.lowpan.ILowpanEnergyScanCallback
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
      @Override public void onEnergyScanResult(int channel, int rssi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(channel);
          _data.writeInt(rssi);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnergyScanResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnergyScanResult(channel, rssi);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onEnergyScanFinished() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnergyScanFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnergyScanFinished();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.lowpan.ILowpanEnergyScanCallback sDefaultImpl;
    }
    static final int TRANSACTION_onEnergyScanResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onEnergyScanFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.lowpan.ILowpanEnergyScanCallback impl) {
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
    public static android.net.lowpan.ILowpanEnergyScanCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onEnergyScanResult(int channel, int rssi) throws android.os.RemoteException;
  public void onEnergyScanFinished() throws android.os.RemoteException;
}
