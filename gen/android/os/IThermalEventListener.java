/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * Listener for thermal events.
 * {@hide}
 */
public interface IThermalEventListener extends android.os.IInterface
{
  /** Default implementation for IThermalEventListener. */
  public static class Default implements android.os.IThermalEventListener
  {
    /**
         * Called when a thermal throttling start/stop event is received.
         * @param temperature the temperature at which the event was generated.
         */
    @Override public void notifyThrottling(android.os.Temperature temperature) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IThermalEventListener
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IThermalEventListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IThermalEventListener interface,
     * generating a proxy if needed.
     */
    public static android.os.IThermalEventListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IThermalEventListener))) {
        return ((android.os.IThermalEventListener)iin);
      }
      return new android.os.IThermalEventListener.Stub.Proxy(obj);
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
        case TRANSACTION_notifyThrottling:
        {
          data.enforceInterface(descriptor);
          android.os.Temperature _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Temperature.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyThrottling(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IThermalEventListener
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
           * Called when a thermal throttling start/stop event is received.
           * @param temperature the temperature at which the event was generated.
           */
      @Override public void notifyThrottling(android.os.Temperature temperature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((temperature!=null)) {
            _data.writeInt(1);
            temperature.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyThrottling, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyThrottling(temperature);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.IThermalEventListener sDefaultImpl;
    }
    static final int TRANSACTION_notifyThrottling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.os.IThermalEventListener impl) {
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
    public static android.os.IThermalEventListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when a thermal throttling start/stop event is received.
       * @param temperature the temperature at which the event was generated.
       */
  public void notifyThrottling(android.os.Temperature temperature) throws android.os.RemoteException;
}
