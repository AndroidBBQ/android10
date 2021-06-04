/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * {@hide}
 */
public interface IGnssMeasurementsListener extends android.os.IInterface
{
  /** Default implementation for IGnssMeasurementsListener. */
  public static class Default implements android.location.IGnssMeasurementsListener
  {
    @Override public void onGnssMeasurementsReceived(android.location.GnssMeasurementsEvent event) throws android.os.RemoteException
    {
    }
    @Override public void onStatusChanged(int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.IGnssMeasurementsListener
  {
    private static final java.lang.String DESCRIPTOR = "android.location.IGnssMeasurementsListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.IGnssMeasurementsListener interface,
     * generating a proxy if needed.
     */
    public static android.location.IGnssMeasurementsListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.IGnssMeasurementsListener))) {
        return ((android.location.IGnssMeasurementsListener)iin);
      }
      return new android.location.IGnssMeasurementsListener.Stub.Proxy(obj);
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
        case TRANSACTION_onGnssMeasurementsReceived:
        {
          data.enforceInterface(descriptor);
          android.location.GnssMeasurementsEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.GnssMeasurementsEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onGnssMeasurementsReceived(_arg0);
          return true;
        }
        case TRANSACTION_onStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onStatusChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.IGnssMeasurementsListener
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
      @Override public void onGnssMeasurementsReceived(android.location.GnssMeasurementsEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGnssMeasurementsReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGnssMeasurementsReceived(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStatusChanged(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStatusChanged(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.location.IGnssMeasurementsListener sDefaultImpl;
    }
    static final int TRANSACTION_onGnssMeasurementsReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.location.IGnssMeasurementsListener impl) {
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
    public static android.location.IGnssMeasurementsListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGnssMeasurementsReceived(android.location.GnssMeasurementsEvent event) throws android.os.RemoteException;
  public void onStatusChanged(int status) throws android.os.RemoteException;
}
