/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the AudioService to signal audio server state changes
 *
 * {@hide}
 */
public interface IAudioServerStateDispatcher extends android.os.IInterface
{
  /** Default implementation for IAudioServerStateDispatcher. */
  public static class Default implements android.media.IAudioServerStateDispatcher
  {
    @Override public void dispatchAudioServerStateChange(boolean state) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IAudioServerStateDispatcher
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IAudioServerStateDispatcher";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IAudioServerStateDispatcher interface,
     * generating a proxy if needed.
     */
    public static android.media.IAudioServerStateDispatcher asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IAudioServerStateDispatcher))) {
        return ((android.media.IAudioServerStateDispatcher)iin);
      }
      return new android.media.IAudioServerStateDispatcher.Stub.Proxy(obj);
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
        case TRANSACTION_dispatchAudioServerStateChange:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.dispatchAudioServerStateChange(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IAudioServerStateDispatcher
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
      @Override public void dispatchAudioServerStateChange(boolean state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((state)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchAudioServerStateChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchAudioServerStateChange(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IAudioServerStateDispatcher sDefaultImpl;
    }
    static final int TRANSACTION_dispatchAudioServerStateChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.IAudioServerStateDispatcher impl) {
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
    public static android.media.IAudioServerStateDispatcher getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void dispatchAudioServerStateChange(boolean state) throws android.os.RemoteException;
}
