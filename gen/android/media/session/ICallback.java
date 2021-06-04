/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * @hide
 */
public interface ICallback extends android.os.IInterface
{
  /** Default implementation for ICallback. */
  public static class Default implements android.media.session.ICallback
  {
    @Override public void onMediaKeyEventDispatchedToMediaSession(android.view.KeyEvent event, android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
    {
    }
    @Override public void onMediaKeyEventDispatchedToMediaButtonReceiver(android.view.KeyEvent event, android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException
    {
    }
    @Override public void onAddressedPlayerChangedToMediaSession(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
    {
    }
    @Override public void onAddressedPlayerChangedToMediaButtonReceiver(android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ICallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ICallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ICallback interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ICallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ICallback))) {
        return ((android.media.session.ICallback)iin);
      }
      return new android.media.session.ICallback.Stub.Proxy(obj);
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
        case TRANSACTION_onMediaKeyEventDispatchedToMediaSession:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.session.MediaSession.Token _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onMediaKeyEventDispatchedToMediaSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMediaKeyEventDispatchedToMediaButtonReceiver:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onMediaKeyEventDispatchedToMediaButtonReceiver(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onAddressedPlayerChangedToMediaSession:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaSession.Token _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAddressedPlayerChangedToMediaSession(_arg0);
          return true;
        }
        case TRANSACTION_onAddressedPlayerChangedToMediaButtonReceiver:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAddressedPlayerChangedToMediaButtonReceiver(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.ICallback
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
      @Override public void onMediaKeyEventDispatchedToMediaSession(android.view.KeyEvent event, android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMediaKeyEventDispatchedToMediaSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMediaKeyEventDispatchedToMediaSession(event, sessionToken);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMediaKeyEventDispatchedToMediaButtonReceiver(android.view.KeyEvent event, android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((mediaButtonReceiver!=null)) {
            _data.writeInt(1);
            mediaButtonReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMediaKeyEventDispatchedToMediaButtonReceiver, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMediaKeyEventDispatchedToMediaButtonReceiver(event, mediaButtonReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAddressedPlayerChangedToMediaSession(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAddressedPlayerChangedToMediaSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAddressedPlayerChangedToMediaSession(sessionToken);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAddressedPlayerChangedToMediaButtonReceiver(android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((mediaButtonReceiver!=null)) {
            _data.writeInt(1);
            mediaButtonReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAddressedPlayerChangedToMediaButtonReceiver, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAddressedPlayerChangedToMediaButtonReceiver(mediaButtonReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.session.ICallback sDefaultImpl;
    }
    static final int TRANSACTION_onMediaKeyEventDispatchedToMediaSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMediaKeyEventDispatchedToMediaButtonReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAddressedPlayerChangedToMediaSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onAddressedPlayerChangedToMediaButtonReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.media.session.ICallback impl) {
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
    public static android.media.session.ICallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onMediaKeyEventDispatchedToMediaSession(android.view.KeyEvent event, android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException;
  public void onMediaKeyEventDispatchedToMediaButtonReceiver(android.view.KeyEvent event, android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException;
  public void onAddressedPlayerChangedToMediaSession(android.media.session.MediaSession.Token sessionToken) throws android.os.RemoteException;
  public void onAddressedPlayerChangedToMediaButtonReceiver(android.content.ComponentName mediaButtonReceiver) throws android.os.RemoteException;
}
