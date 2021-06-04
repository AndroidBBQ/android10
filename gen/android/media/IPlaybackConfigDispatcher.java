/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the PlaybackActivityMonitor in AudioService to signal audio playback updates.
 *
 * {@hide}
 */
public interface IPlaybackConfigDispatcher extends android.os.IInterface
{
  /** Default implementation for IPlaybackConfigDispatcher. */
  public static class Default implements android.media.IPlaybackConfigDispatcher
  {
    @Override public void dispatchPlaybackConfigChange(java.util.List<android.media.AudioPlaybackConfiguration> configs, boolean flush) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IPlaybackConfigDispatcher
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IPlaybackConfigDispatcher";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IPlaybackConfigDispatcher interface,
     * generating a proxy if needed.
     */
    public static android.media.IPlaybackConfigDispatcher asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IPlaybackConfigDispatcher))) {
        return ((android.media.IPlaybackConfigDispatcher)iin);
      }
      return new android.media.IPlaybackConfigDispatcher.Stub.Proxy(obj);
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
        case TRANSACTION_dispatchPlaybackConfigChange:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.AudioPlaybackConfiguration> _arg0;
          _arg0 = data.createTypedArrayList(android.media.AudioPlaybackConfiguration.CREATOR);
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.dispatchPlaybackConfigChange(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IPlaybackConfigDispatcher
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
      @Override public void dispatchPlaybackConfigChange(java.util.List<android.media.AudioPlaybackConfiguration> configs, boolean flush) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(configs);
          _data.writeInt(((flush)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchPlaybackConfigChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchPlaybackConfigChange(configs, flush);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IPlaybackConfigDispatcher sDefaultImpl;
    }
    static final int TRANSACTION_dispatchPlaybackConfigChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.IPlaybackConfigDispatcher impl) {
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
    public static android.media.IPlaybackConfigDispatcher getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void dispatchPlaybackConfigChange(java.util.List<android.media.AudioPlaybackConfiguration> configs, boolean flush) throws android.os.RemoteException;
}
