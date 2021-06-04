/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.hdmi;
/**
 * Callback interface definition for TV to get informed of
 * the input change event coming from other device.
 *
 * @hide
 */
public interface IHdmiInputChangeListener extends android.os.IInterface
{
  /** Default implementation for IHdmiInputChangeListener. */
  public static class Default implements android.hardware.hdmi.IHdmiInputChangeListener
  {
    @Override public void onChanged(android.hardware.hdmi.HdmiDeviceInfo device) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.hdmi.IHdmiInputChangeListener
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.hdmi.IHdmiInputChangeListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.hdmi.IHdmiInputChangeListener interface,
     * generating a proxy if needed.
     */
    public static android.hardware.hdmi.IHdmiInputChangeListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.hdmi.IHdmiInputChangeListener))) {
        return ((android.hardware.hdmi.IHdmiInputChangeListener)iin);
      }
      return new android.hardware.hdmi.IHdmiInputChangeListener.Stub.Proxy(obj);
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
        case TRANSACTION_onChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.HdmiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.hdmi.HdmiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.hdmi.IHdmiInputChangeListener
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
      @Override public void onChanged(android.hardware.hdmi.HdmiDeviceInfo device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onChanged(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.hdmi.IHdmiInputChangeListener sDefaultImpl;
    }
    static final int TRANSACTION_onChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.hdmi.IHdmiInputChangeListener impl) {
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
    public static android.hardware.hdmi.IHdmiInputChangeListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onChanged(android.hardware.hdmi.HdmiDeviceInfo device) throws android.os.RemoteException;
}
