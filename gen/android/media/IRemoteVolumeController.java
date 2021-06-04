/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the MediaSessionService to report interesting events on remote playback
 * to a volume control dialog. See also IVolumeController for the AudioService half.
 * TODO add in better support for multiple remote sessions.
 * @hide
 */
public interface IRemoteVolumeController extends android.os.IInterface
{
  /** Default implementation for IRemoteVolumeController. */
  public static class Default implements android.media.IRemoteVolumeController
  {
    @Override public void remoteVolumeChanged(android.media.session.MediaSession.Token sessionToken, int flags) throws android.os.RemoteException
    {
    }
    // sets the default session to use with the slider, replaces remoteSliderVisibility
    // on IVolumeController

    @Override public void updateRemoteController(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IRemoteVolumeController
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IRemoteVolumeController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IRemoteVolumeController interface,
     * generating a proxy if needed.
     */
    public static android.media.IRemoteVolumeController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IRemoteVolumeController))) {
        return ((android.media.IRemoteVolumeController)iin);
      }
      return new android.media.IRemoteVolumeController.Stub.Proxy(obj);
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
        case TRANSACTION_remoteVolumeChanged:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaSession.Token _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.remoteVolumeChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_updateRemoteController:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaSession.Token _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updateRemoteController(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IRemoteVolumeController
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
      @Override public void remoteVolumeChanged(android.media.session.MediaSession.Token sessionToken, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_remoteVolumeChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remoteVolumeChanged(sessionToken, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // sets the default session to use with the slider, replaces remoteSliderVisibility
      // on IVolumeController

      @Override public void updateRemoteController(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateRemoteController, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateRemoteController(sessionToken);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IRemoteVolumeController sDefaultImpl;
    }
    static final int TRANSACTION_remoteVolumeChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateRemoteController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.media.IRemoteVolumeController impl) {
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
    public static android.media.IRemoteVolumeController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void remoteVolumeChanged(android.media.session.MediaSession.Token sessionToken, int flags) throws android.os.RemoteException;
  // sets the default session to use with the slider, replaces remoteSliderVisibility
  // on IVolumeController

  public void updateRemoteController(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException;
}
