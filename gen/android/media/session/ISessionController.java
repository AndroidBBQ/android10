/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * Interface to MediaSessionRecord in the system.
 * @hide
 */
public interface ISessionController extends android.os.IInterface
{
  /** Default implementation for ISessionController. */
  public static class Default implements android.media.session.ISessionController
  {
    @Override public void sendCommand(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException
    {
    }
    @Override public boolean sendMediaButton(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.view.KeyEvent mediaButton) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void registerCallback(java.lang.String packageName, android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException
    {
    }
    @Override public void unregisterCallback(android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getPackageName() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getTag() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.Bundle getSessionInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.PendingIntent getLaunchPendingIntent() throws android.os.RemoteException
    {
      return null;
    }
    @Override public long getFlags() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public android.media.session.MediaController.PlaybackInfo getVolumeAttributes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void adjustVolume(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int direction, int flags) throws android.os.RemoteException
    {
    }
    @Override public void setVolumeTo(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int value, int flags) throws android.os.RemoteException
    {
    }
    // These commands are for the TransportControls

    @Override public void prepare(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void prepareFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void prepareFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void prepareFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void play(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void playFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void playFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void playFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void skipToQueueItem(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException
    {
    }
    @Override public void pause(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void stop(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void next(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void previous(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void fastForward(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void rewind(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void seekTo(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException
    {
    }
    @Override public void rate(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException
    {
    }
    @Override public void setPlaybackSpeed(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException
    {
    }
    @Override public void sendCustomAction(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException
    {
    }
    @Override public android.media.MediaMetadata getMetadata() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.media.session.PlaybackState getPlaybackState() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getQueue() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.CharSequence getQueueTitle() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.Bundle getExtras() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getRatingType() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ISessionController
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ISessionController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ISessionController interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ISessionController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ISessionController))) {
        return ((android.media.session.ISessionController)iin);
      }
      return new android.media.session.ISessionController.Stub.Proxy(obj);
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
        case TRANSACTION_sendCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.os.ResultReceiver _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.sendCommand(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendMediaButton:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.view.KeyEvent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.sendMediaButton(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          android.media.session.ISessionControllerCallback _arg0;
          _arg0 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackageName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getPackageName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getTag:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getTag();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSessionInfo:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _result = this.getSessionInfo();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getLaunchPendingIntent:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _result = this.getLaunchPendingIntent();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getFlags:
        {
          data.enforceInterface(descriptor);
          long _result = this.getFlags();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getVolumeAttributes:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaController.PlaybackInfo _result = this.getVolumeAttributes();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_adjustVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.session.ISessionControllerCallback _arg2;
          _arg2 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.adjustVolume(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVolumeTo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.session.ISessionControllerCallback _arg2;
          _arg2 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.setVolumeTo(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepare:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.prepare(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepareFromMediaId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.prepareFromMediaId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepareFromSearch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.prepareFromSearch(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_prepareFromUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.prepareFromUri(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_play:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.play(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_playFromMediaId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.playFromMediaId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_playFromSearch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.playFromSearch(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_playFromUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.playFromUri(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_skipToQueueItem:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          long _arg2;
          _arg2 = data.readLong();
          this.skipToQueueItem(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_pause:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.pause(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stop:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.stop(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_next:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.next(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_previous:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.previous(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_fastForward:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.fastForward(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_rewind:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.rewind(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_seekTo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          long _arg2;
          _arg2 = data.readLong();
          this.seekTo(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_rate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.media.Rating _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.Rating.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.rate(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPlaybackSpeed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          float _arg2;
          _arg2 = data.readFloat();
          this.setPlaybackSpeed(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendCustomAction:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionControllerCallback _arg1;
          _arg1 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.sendCustomAction(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMetadata:
        {
          data.enforceInterface(descriptor);
          android.media.MediaMetadata _result = this.getMetadata();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getPlaybackState:
        {
          data.enforceInterface(descriptor);
          android.media.session.PlaybackState _result = this.getPlaybackState();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getQueue:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _result = this.getQueue();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getQueueTitle:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _result = this.getQueueTitle();
          reply.writeNoException();
          if (_result!=null) {
            reply.writeInt(1);
            android.text.TextUtils.writeToParcel(_result, reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getExtras:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _result = this.getExtras();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getRatingType:
        {
          data.enforceInterface(descriptor);
          int _result = this.getRatingType();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.ISessionController
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
      @Override public void sendCommand(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(command);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((cb!=null)) {
            _data.writeInt(1);
            cb.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCommand(packageName, caller, command, args, cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean sendMediaButton(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.view.KeyEvent mediaButton) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((mediaButton!=null)) {
            _data.writeInt(1);
            mediaButton.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMediaButton, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendMediaButton(packageName, caller, mediaButton);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void registerCallback(java.lang.String packageName, android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(packageName, cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterCallback(android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getPackageName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageName();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getTag() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTag, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTag();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.Bundle getSessionInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSessionInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSessionInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.app.PendingIntent getLaunchPendingIntent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.PendingIntent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLaunchPendingIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLaunchPendingIntent();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.PendingIntent.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getFlags() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFlags, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFlags();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.media.session.MediaController.PlaybackInfo getVolumeAttributes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.session.MediaController.PlaybackInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVolumeAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVolumeAttributes();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.session.MediaController.PlaybackInfo.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void adjustVolume(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int direction, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeInt(direction);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adjustVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adjustVolume(packageName, opPackageName, caller, direction, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setVolumeTo(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int value, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeInt(value);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolumeTo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolumeTo(packageName, opPackageName, caller, value, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These commands are for the TransportControls

      @Override public void prepare(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepare, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepare(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void prepareFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(mediaId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareFromMediaId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareFromMediaId(packageName, caller, mediaId, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void prepareFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(string);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareFromSearch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareFromSearch(packageName, caller, string, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void prepareFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareFromUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareFromUri(packageName, caller, uri, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void play(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_play, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().play(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void playFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(mediaId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_playFromMediaId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playFromMediaId(packageName, caller, mediaId, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void playFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(string);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_playFromSearch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playFromSearch(packageName, caller, string, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void playFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_playFromUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playFromUri(packageName, caller, uri, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void skipToQueueItem(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeLong(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_skipToQueueItem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().skipToQueueItem(packageName, caller, id);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void pause(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_pause, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pause(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stop(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stop, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stop(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void next(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_next, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().next(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void previous(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_previous, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().previous(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void fastForward(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_fastForward, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().fastForward(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void rewind(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_rewind, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rewind(packageName, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void seekTo(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeLong(pos);
          boolean _status = mRemote.transact(Stub.TRANSACTION_seekTo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().seekTo(packageName, caller, pos);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void rate(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((rating!=null)) {
            _data.writeInt(1);
            rating.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_rate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rate(packageName, caller, rating);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPlaybackSpeed(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeFloat(speed);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPlaybackSpeed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPlaybackSpeed(packageName, caller, speed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendCustomAction(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(action);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCustomAction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCustomAction(packageName, caller, action, args);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.media.MediaMetadata getMetadata() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.MediaMetadata _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMetadata, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMetadata();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.MediaMetadata.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.media.session.PlaybackState getPlaybackState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.session.PlaybackState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPlaybackState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPlaybackState();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.session.PlaybackState.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.pm.ParceledListSlice getQueue() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getQueue, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getQueue();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.CharSequence getQueueTitle() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.CharSequence _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getQueueTitle, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getQueueTitle();
          }
          _reply.readException();
          if (0!=_reply.readInt()) {
            _result = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.Bundle getExtras() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getExtras();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getRatingType() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRatingType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRatingType();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.media.session.ISessionController sDefaultImpl;
    }
    static final int TRANSACTION_sendCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_sendMediaButton = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getPackageName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getTag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getSessionInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getLaunchPendingIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getFlags = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getVolumeAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_adjustVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setVolumeTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_prepare = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_prepareFromMediaId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_prepareFromSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_prepareFromUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_play = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_playFromMediaId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_playFromSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_playFromUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_skipToQueueItem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_pause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_stop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_next = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_previous = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_fastForward = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_rewind = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_seekTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_rate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_setPlaybackSpeed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_sendCustomAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getPlaybackState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getQueue = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_getQueueTitle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getRatingType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    public static boolean setDefaultImpl(android.media.session.ISessionController impl) {
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
    public static android.media.session.ISessionController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void sendCommand(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException;
  public boolean sendMediaButton(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.view.KeyEvent mediaButton) throws android.os.RemoteException;
  public void registerCallback(java.lang.String packageName, android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException;
  public void unregisterCallback(android.media.session.ISessionControllerCallback cb) throws android.os.RemoteException;
  public java.lang.String getPackageName() throws android.os.RemoteException;
  public java.lang.String getTag() throws android.os.RemoteException;
  public android.os.Bundle getSessionInfo() throws android.os.RemoteException;
  public android.app.PendingIntent getLaunchPendingIntent() throws android.os.RemoteException;
  public long getFlags() throws android.os.RemoteException;
  public android.media.session.MediaController.PlaybackInfo getVolumeAttributes() throws android.os.RemoteException;
  public void adjustVolume(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int direction, int flags) throws android.os.RemoteException;
  public void setVolumeTo(java.lang.String packageName, java.lang.String opPackageName, android.media.session.ISessionControllerCallback caller, int value, int flags) throws android.os.RemoteException;
  // These commands are for the TransportControls

  public void prepare(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void prepareFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException;
  public void prepareFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException;
  public void prepareFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException;
  public void play(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void playFromMediaId(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException;
  public void playFromSearch(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String string, android.os.Bundle extras) throws android.os.RemoteException;
  public void playFromUri(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException;
  public void skipToQueueItem(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException;
  public void pause(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void stop(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void next(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void previous(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void fastForward(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void rewind(java.lang.String packageName, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void seekTo(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException;
  public void rate(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException;
  public void setPlaybackSpeed(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException;
  public void sendCustomAction(java.lang.String packageName, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException;
  public android.media.MediaMetadata getMetadata() throws android.os.RemoteException;
  public android.media.session.PlaybackState getPlaybackState() throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getQueue() throws android.os.RemoteException;
  public java.lang.CharSequence getQueueTitle() throws android.os.RemoteException;
  public android.os.Bundle getExtras() throws android.os.RemoteException;
  public int getRatingType() throws android.os.RemoteException;
}
