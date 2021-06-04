/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Interface for observing discovered printers by a discovery session.
 *
 * @hide
 */
public interface IPrinterDiscoveryObserver extends android.os.IInterface
{
  /** Default implementation for IPrinterDiscoveryObserver. */
  public static class Default implements android.print.IPrinterDiscoveryObserver
  {
    @Override public void onPrintersAdded(android.content.pm.ParceledListSlice printers) throws android.os.RemoteException
    {
    }
    @Override public void onPrintersRemoved(android.content.pm.ParceledListSlice printerIds) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IPrinterDiscoveryObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IPrinterDiscoveryObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IPrinterDiscoveryObserver interface,
     * generating a proxy if needed.
     */
    public static android.print.IPrinterDiscoveryObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IPrinterDiscoveryObserver))) {
        return ((android.print.IPrinterDiscoveryObserver)iin);
      }
      return new android.print.IPrinterDiscoveryObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onPrintersAdded:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPrintersAdded(_arg0);
          return true;
        }
        case TRANSACTION_onPrintersRemoved:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPrintersRemoved(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IPrinterDiscoveryObserver
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
      @Override public void onPrintersAdded(android.content.pm.ParceledListSlice printers) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printers!=null)) {
            _data.writeInt(1);
            printers.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrintersAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrintersAdded(printers);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrintersRemoved(android.content.pm.ParceledListSlice printerIds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerIds!=null)) {
            _data.writeInt(1);
            printerIds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrintersRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrintersRemoved(printerIds);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.IPrinterDiscoveryObserver sDefaultImpl;
    }
    static final int TRANSACTION_onPrintersAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onPrintersRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.print.IPrinterDiscoveryObserver impl) {
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
    public static android.print.IPrinterDiscoveryObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onPrintersAdded(android.content.pm.ParceledListSlice printers) throws android.os.RemoteException;
  public void onPrintersRemoved(android.content.pm.ParceledListSlice printerIds) throws android.os.RemoteException;
}
