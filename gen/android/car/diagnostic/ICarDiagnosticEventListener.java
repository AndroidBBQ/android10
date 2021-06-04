/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.diagnostic;
/**
 * @hide
 */
public interface ICarDiagnosticEventListener extends android.os.IInterface
{
  /** Default implementation for ICarDiagnosticEventListener. */
  public static class Default implements android.car.diagnostic.ICarDiagnosticEventListener
  {
    @Override public void onDiagnosticEvents(java.util.List<android.car.diagnostic.CarDiagnosticEvent> events) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.diagnostic.ICarDiagnosticEventListener
  {
    private static final java.lang.String DESCRIPTOR = "android.car.diagnostic.ICarDiagnosticEventListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.diagnostic.ICarDiagnosticEventListener interface,
     * generating a proxy if needed.
     */
    public static android.car.diagnostic.ICarDiagnosticEventListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.diagnostic.ICarDiagnosticEventListener))) {
        return ((android.car.diagnostic.ICarDiagnosticEventListener)iin);
      }
      return new android.car.diagnostic.ICarDiagnosticEventListener.Stub.Proxy(obj);
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
        case TRANSACTION_onDiagnosticEvents:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.diagnostic.CarDiagnosticEvent> _arg0;
          _arg0 = data.createTypedArrayList(android.car.diagnostic.CarDiagnosticEvent.CREATOR);
          this.onDiagnosticEvents(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.diagnostic.ICarDiagnosticEventListener
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
      @Override public void onDiagnosticEvents(java.util.List<android.car.diagnostic.CarDiagnosticEvent> events) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(events);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDiagnosticEvents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDiagnosticEvents(events);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.diagnostic.ICarDiagnosticEventListener sDefaultImpl;
    }
    static final int TRANSACTION_onDiagnosticEvents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.diagnostic.ICarDiagnosticEventListener impl) {
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
    public static android.car.diagnostic.ICarDiagnosticEventListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDiagnosticEvents(java.util.List<android.car.diagnostic.CarDiagnosticEvent> events) throws android.os.RemoteException;
}
