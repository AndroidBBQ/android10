/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the AudioService to report changes in available audio routes.
 * @hide
 */
public interface IAudioRoutesObserver extends android.os.IInterface
{
  /** Default implementation for IAudioRoutesObserver. */
  public static class Default implements android.media.IAudioRoutesObserver
  {
    @Override public void dispatchAudioRoutesChanged(android.media.AudioRoutesInfo newRoutes) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IAudioRoutesObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IAudioRoutesObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IAudioRoutesObserver interface,
     * generating a proxy if needed.
     */
    public static android.media.IAudioRoutesObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IAudioRoutesObserver))) {
        return ((android.media.IAudioRoutesObserver)iin);
      }
      return new android.media.IAudioRoutesObserver.Stub.Proxy(obj);
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
        case TRANSACTION_dispatchAudioRoutesChanged:
        {
          data.enforceInterface(descriptor);
          android.media.AudioRoutesInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioRoutesInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.dispatchAudioRoutesChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IAudioRoutesObserver
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
      @Override public void dispatchAudioRoutesChanged(android.media.AudioRoutesInfo newRoutes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((newRoutes!=null)) {
            _data.writeInt(1);
            newRoutes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchAudioRoutesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchAudioRoutesChanged(newRoutes);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IAudioRoutesObserver sDefaultImpl;
    }
    static final int TRANSACTION_dispatchAudioRoutesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.IAudioRoutesObserver impl) {
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
    public static android.media.IAudioRoutesObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void dispatchAudioRoutesChanged(android.media.AudioRoutesInfo newRoutes) throws android.os.RemoteException;
}
