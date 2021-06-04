/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * Helper interface for ITvInputService to allow the service to notify the
 * TvInputManagerService.
 * @hide
 */
public interface ITvInputServiceCallback extends android.os.IInterface
{
  /** Default implementation for ITvInputServiceCallback. */
  public static class Default implements android.media.tv.ITvInputServiceCallback
  {
    @Override public void addHardwareInput(int deviceId, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException
    {
    }
    @Override public void addHdmiInput(int id, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException
    {
    }
    @Override public void removeHardwareInput(java.lang.String inputId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputServiceCallback))) {
        return ((android.media.tv.ITvInputServiceCallback)iin);
      }
      return new android.media.tv.ITvInputServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_addHardwareInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.tv.TvInputInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.tv.TvInputInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addHardwareInput(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addHdmiInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.tv.TvInputInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.tv.TvInputInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addHdmiInput(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeHardwareInput:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeHardwareInput(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputServiceCallback
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
      @Override public void addHardwareInput(int deviceId, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          if ((inputInfo!=null)) {
            _data.writeInt(1);
            inputInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addHardwareInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addHardwareInput(deviceId, inputInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addHdmiInput(int id, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          if ((inputInfo!=null)) {
            _data.writeInt(1);
            inputInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addHdmiInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addHdmiInput(id, inputInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeHardwareInput(java.lang.String inputId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeHardwareInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeHardwareInput(inputId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_addHardwareInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addHdmiInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeHardwareInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.media.tv.ITvInputServiceCallback impl) {
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
    public static android.media.tv.ITvInputServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addHardwareInput(int deviceId, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException;
  public void addHdmiInput(int id, android.media.tv.TvInputInfo inputInfo) throws android.os.RemoteException;
  public void removeHardwareInput(java.lang.String inputId) throws android.os.RemoteException;
}
