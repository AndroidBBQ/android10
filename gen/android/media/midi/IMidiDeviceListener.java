/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.midi;
/** @hide */
public interface IMidiDeviceListener extends android.os.IInterface
{
  /** Default implementation for IMidiDeviceListener. */
  public static class Default implements android.media.midi.IMidiDeviceListener
  {
    @Override public void onDeviceAdded(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException
    {
    }
    @Override public void onDeviceRemoved(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException
    {
    }
    @Override public void onDeviceStatusChanged(android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.midi.IMidiDeviceListener
  {
    private static final java.lang.String DESCRIPTOR = "android.media.midi.IMidiDeviceListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.midi.IMidiDeviceListener interface,
     * generating a proxy if needed.
     */
    public static android.media.midi.IMidiDeviceListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.midi.IMidiDeviceListener))) {
        return ((android.media.midi.IMidiDeviceListener)iin);
      }
      return new android.media.midi.IMidiDeviceListener.Stub.Proxy(obj);
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
        case TRANSACTION_onDeviceAdded:
        {
          data.enforceInterface(descriptor);
          android.media.midi.MidiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeviceAdded(_arg0);
          return true;
        }
        case TRANSACTION_onDeviceRemoved:
        {
          data.enforceInterface(descriptor);
          android.media.midi.MidiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeviceRemoved(_arg0);
          return true;
        }
        case TRANSACTION_onDeviceStatusChanged:
        {
          data.enforceInterface(descriptor);
          android.media.midi.MidiDeviceStatus _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.midi.MidiDeviceStatus.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeviceStatusChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.midi.IMidiDeviceListener
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
      @Override public void onDeviceAdded(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceAdded(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeviceRemoved(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceRemoved(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeviceStatusChanged(android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((status!=null)) {
            _data.writeInt(1);
            status.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceStatusChanged(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.midi.IMidiDeviceListener sDefaultImpl;
    }
    static final int TRANSACTION_onDeviceAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDeviceRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onDeviceStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.media.midi.IMidiDeviceListener impl) {
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
    public static android.media.midi.IMidiDeviceListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDeviceAdded(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException;
  public void onDeviceRemoved(android.media.midi.MidiDeviceInfo device) throws android.os.RemoteException;
  public void onDeviceStatusChanged(android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException;
}
