/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * @hide
 */
public interface ITvInputHardwareCallback extends android.os.IInterface
{
  /** Default implementation for ITvInputHardwareCallback. */
  public static class Default implements android.media.tv.ITvInputHardwareCallback
  {
    @Override public void onReleased() throws android.os.RemoteException
    {
    }
    @Override public void onStreamConfigChanged(android.media.tv.TvStreamConfig[] configs) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputHardwareCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputHardwareCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputHardwareCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputHardwareCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputHardwareCallback))) {
        return ((android.media.tv.ITvInputHardwareCallback)iin);
      }
      return new android.media.tv.ITvInputHardwareCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onReleased:
        {
          data.enforceInterface(descriptor);
          this.onReleased();
          return true;
        }
        case TRANSACTION_onStreamConfigChanged:
        {
          data.enforceInterface(descriptor);
          android.media.tv.TvStreamConfig[] _arg0;
          _arg0 = data.createTypedArray(android.media.tv.TvStreamConfig.CREATOR);
          this.onStreamConfigChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputHardwareCallback
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
      @Override public void onReleased() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReleased, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReleased();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStreamConfigChanged(android.media.tv.TvStreamConfig[] configs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(configs, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStreamConfigChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStreamConfigChanged(configs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputHardwareCallback sDefaultImpl;
    }
    static final int TRANSACTION_onReleased = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onStreamConfigChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.media.tv.ITvInputHardwareCallback impl) {
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
    public static android.media.tv.ITvInputHardwareCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onReleased() throws android.os.RemoteException;
  public void onStreamConfigChanged(android.media.tv.TvStreamConfig[] configs) throws android.os.RemoteException;
}
