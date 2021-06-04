/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * @hide
 */
public interface ISessionControllerCallback extends android.os.IInterface
{
  /** Default implementation for ISessionControllerCallback. */
  public static class Default implements android.media.session.ISessionControllerCallback
  {
    @Override public void onEvent(java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onSessionDestroyed() throws android.os.RemoteException
    {
    }
    // These callbacks are for the TransportController

    @Override public void onPlaybackStateChanged(android.media.session.PlaybackState state) throws android.os.RemoteException
    {
    }
    @Override public void onMetadataChanged(android.media.MediaMetadata metadata) throws android.os.RemoteException
    {
    }
    @Override public void onQueueChanged(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException
    {
    }
    @Override public void onQueueTitleChanged(java.lang.CharSequence title) throws android.os.RemoteException
    {
    }
    @Override public void onExtrasChanged(android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onVolumeInfoChanged(android.media.session.MediaController.PlaybackInfo info) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ISessionControllerCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ISessionControllerCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ISessionControllerCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ISessionControllerCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ISessionControllerCallback))) {
        return ((android.media.session.ISessionControllerCallback)iin);
      }
      return new android.media.session.ISessionControllerCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSessionDestroyed:
        {
          data.enforceInterface(descriptor);
          this.onSessionDestroyed();
          return true;
        }
        case TRANSACTION_onPlaybackStateChanged:
        {
          data.enforceInterface(descriptor);
          android.media.session.PlaybackState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.PlaybackState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPlaybackStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onMetadataChanged:
        {
          data.enforceInterface(descriptor);
          android.media.MediaMetadata _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.MediaMetadata.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onMetadataChanged(_arg0);
          return true;
        }
        case TRANSACTION_onQueueChanged:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onQueueChanged(_arg0);
          return true;
        }
        case TRANSACTION_onQueueTitleChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onQueueTitleChanged(_arg0);
          return true;
        }
        case TRANSACTION_onExtrasChanged:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onExtrasChanged(_arg0);
          return true;
        }
        case TRANSACTION_onVolumeInfoChanged:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaController.PlaybackInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.MediaController.PlaybackInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onVolumeInfoChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.ISessionControllerCallback
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
      @Override public void onEvent(java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(event);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEvent(event, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionDestroyed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionDestroyed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionDestroyed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // These callbacks are for the TransportController

      @Override public void onPlaybackStateChanged(android.media.session.PlaybackState state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPlaybackStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPlaybackStateChanged(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMetadataChanged(android.media.MediaMetadata metadata) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((metadata!=null)) {
            _data.writeInt(1);
            metadata.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMetadataChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMetadataChanged(metadata);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onQueueChanged(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queue!=null)) {
            _data.writeInt(1);
            queue.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQueueChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQueueChanged(queue);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onQueueTitleChanged(java.lang.CharSequence title) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (title!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(title, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQueueTitleChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQueueTitleChanged(title);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onExtrasChanged(android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onExtrasChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onExtrasChanged(extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVolumeInfoChanged(android.media.session.MediaController.PlaybackInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVolumeInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVolumeInfoChanged(info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.session.ISessionControllerCallback sDefaultImpl;
    }
    static final int TRANSACTION_onEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSessionDestroyed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onPlaybackStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onMetadataChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onQueueChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onQueueTitleChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onExtrasChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onVolumeInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.media.session.ISessionControllerCallback impl) {
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
    public static android.media.session.ISessionControllerCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onEvent(java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException;
  public void onSessionDestroyed() throws android.os.RemoteException;
  // These callbacks are for the TransportController

  public void onPlaybackStateChanged(android.media.session.PlaybackState state) throws android.os.RemoteException;
  public void onMetadataChanged(android.media.MediaMetadata metadata) throws android.os.RemoteException;
  public void onQueueChanged(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException;
  public void onQueueTitleChanged(java.lang.CharSequence title) throws android.os.RemoteException;
  public void onExtrasChanged(android.os.Bundle extras) throws android.os.RemoteException;
  public void onVolumeInfoChanged(android.media.session.MediaController.PlaybackInfo info) throws android.os.RemoteException;
}
