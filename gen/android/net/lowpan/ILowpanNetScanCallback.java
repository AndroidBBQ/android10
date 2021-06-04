/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.lowpan;
/** {@hide} */
public interface ILowpanNetScanCallback extends android.os.IInterface
{
  /** Default implementation for ILowpanNetScanCallback. */
  public static class Default implements android.net.lowpan.ILowpanNetScanCallback
  {
    @Override public void onNetScanBeacon(android.net.lowpan.LowpanBeaconInfo beacon) throws android.os.RemoteException
    {
    }
    @Override public void onNetScanFinished() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.lowpan.ILowpanNetScanCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.lowpan.ILowpanNetScanCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.lowpan.ILowpanNetScanCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.lowpan.ILowpanNetScanCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.lowpan.ILowpanNetScanCallback))) {
        return ((android.net.lowpan.ILowpanNetScanCallback)iin);
      }
      return new android.net.lowpan.ILowpanNetScanCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onNetScanBeacon:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanBeaconInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.lowpan.LowpanBeaconInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onNetScanBeacon(_arg0);
          return true;
        }
        case TRANSACTION_onNetScanFinished:
        {
          data.enforceInterface(descriptor);
          this.onNetScanFinished();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.lowpan.ILowpanNetScanCallback
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
      @Override public void onNetScanBeacon(android.net.lowpan.LowpanBeaconInfo beacon) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((beacon!=null)) {
            _data.writeInt(1);
            beacon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNetScanBeacon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNetScanBeacon(beacon);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNetScanFinished() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNetScanFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNetScanFinished();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.lowpan.ILowpanNetScanCallback sDefaultImpl;
    }
    static final int TRANSACTION_onNetScanBeacon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onNetScanFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.lowpan.ILowpanNetScanCallback impl) {
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
    public static android.net.lowpan.ILowpanNetScanCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onNetScanBeacon(android.net.lowpan.LowpanBeaconInfo beacon) throws android.os.RemoteException;
  public void onNetScanFinished() throws android.os.RemoteException;
}
