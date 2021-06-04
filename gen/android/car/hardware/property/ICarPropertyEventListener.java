/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.hardware.property;
/**
 * Binder callback for CarPropertyEventCallback.
 * This is generated per each CarClient.
 * @hide
 */
public interface ICarPropertyEventListener extends android.os.IInterface
{
  /** Default implementation for ICarPropertyEventListener. */
  public static class Default implements android.car.hardware.property.ICarPropertyEventListener
  {
    /**
         * Called when an event is triggered in response to one of the calls (such as on tune) or
         * asynchronously (such as on announcement).
         */
    @Override public void onEvent(java.util.List<android.car.hardware.property.CarPropertyEvent> events) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.hardware.property.ICarPropertyEventListener
  {
    private static final java.lang.String DESCRIPTOR = "android.car.hardware.property.ICarPropertyEventListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.hardware.property.ICarPropertyEventListener interface,
     * generating a proxy if needed.
     */
    public static android.car.hardware.property.ICarPropertyEventListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.hardware.property.ICarPropertyEventListener))) {
        return ((android.car.hardware.property.ICarPropertyEventListener)iin);
      }
      return new android.car.hardware.property.ICarPropertyEventListener.Stub.Proxy(obj);
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
        case TRANSACTION_onEvent:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.hardware.property.CarPropertyEvent> _arg0;
          _arg0 = data.createTypedArrayList(android.car.hardware.property.CarPropertyEvent.CREATOR);
          this.onEvent(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.hardware.property.ICarPropertyEventListener
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
      /**
           * Called when an event is triggered in response to one of the calls (such as on tune) or
           * asynchronously (such as on announcement).
           */
      @Override public void onEvent(java.util.List<android.car.hardware.property.CarPropertyEvent> events) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(events);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEvent(events);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.hardware.property.ICarPropertyEventListener sDefaultImpl;
    }
    static final int TRANSACTION_onEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.hardware.property.ICarPropertyEventListener impl) {
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
    public static android.car.hardware.property.ICarPropertyEventListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when an event is triggered in response to one of the calls (such as on tune) or
       * asynchronously (such as on announcement).
       */
  public void onEvent(java.util.List<android.car.hardware.property.CarPropertyEvent> events) throws android.os.RemoteException;
}
