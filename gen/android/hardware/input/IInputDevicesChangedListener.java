/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.input;
/** @hide */
public interface IInputDevicesChangedListener extends android.os.IInterface
{
  /** Default implementation for IInputDevicesChangedListener. */
  public static class Default implements android.hardware.input.IInputDevicesChangedListener
  {
    /* Called when input devices changed, such as a device being added,
         * removed or changing configuration.
         *
         * The parameter is an array of pairs (deviceId, generation) indicating the current
         * device id and generation of all input devices.  The client can determine what
         * has happened by comparing the result to its prior observations.
         */
    @Override public void onInputDevicesChanged(int[] deviceIdAndGeneration) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.input.IInputDevicesChangedListener
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.input.IInputDevicesChangedListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.input.IInputDevicesChangedListener interface,
     * generating a proxy if needed.
     */
    public static android.hardware.input.IInputDevicesChangedListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.input.IInputDevicesChangedListener))) {
        return ((android.hardware.input.IInputDevicesChangedListener)iin);
      }
      return new android.hardware.input.IInputDevicesChangedListener.Stub.Proxy(obj);
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
        case TRANSACTION_onInputDevicesChanged:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          this.onInputDevicesChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.input.IInputDevicesChangedListener
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
      /* Called when input devices changed, such as a device being added,
           * removed or changing configuration.
           *
           * The parameter is an array of pairs (deviceId, generation) indicating the current
           * device id and generation of all input devices.  The client can determine what
           * has happened by comparing the result to its prior observations.
           */
      @Override public void onInputDevicesChanged(int[] deviceIdAndGeneration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(deviceIdAndGeneration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInputDevicesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInputDevicesChanged(deviceIdAndGeneration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.input.IInputDevicesChangedListener sDefaultImpl;
    }
    static final int TRANSACTION_onInputDevicesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.input.IInputDevicesChangedListener impl) {
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
    public static android.hardware.input.IInputDevicesChangedListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /* Called when input devices changed, such as a device being added,
       * removed or changing configuration.
       *
       * The parameter is an array of pairs (deviceId, generation) indicating the current
       * device id and generation of all input devices.  The client can determine what
       * has happened by comparing the result to its prior observations.
       */
  public void onInputDevicesChanged(int[] deviceIdAndGeneration) throws android.os.RemoteException;
}
