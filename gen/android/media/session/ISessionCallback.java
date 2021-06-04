/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * @hide
 */
public interface ISessionCallback extends android.os.IInterface
{
  /** Default implementation for ISessionCallback. */
  public static class Default implements android.media.session.ISessionCallback
  {
    @Override public void onCommand(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException
    {
    }
    @Override public void onMediaButton(java.lang.String packageName, int pid, int uid, android.content.Intent mediaButtonIntent, int sequenceNumber, android.os.ResultReceiver cb) throws android.os.RemoteException
    {
    }
    @Override public void onMediaButtonFromController(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.content.Intent mediaButtonIntent) throws android.os.RemoteException
    {
    }
    // These callbacks are for the TransportControls

    @Override public void onPrepare(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onPrepareFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onPrepareFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onPrepareFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onPlay(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onPlayFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onPlayFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onPlayFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onSkipToTrack(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException
    {
    }
    @Override public void onPause(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onStop(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onNext(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onPrevious(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onFastForward(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onRewind(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
    {
    }
    @Override public void onSeekTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException
    {
    }
    @Override public void onRate(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException
    {
    }
    @Override public void onSetPlaybackSpeed(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException
    {
    }
    @Override public void onCustomAction(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException
    {
    }
    // These callbacks are for volume handling

    @Override public void onAdjustVolume(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int direction) throws android.os.RemoteException
    {
    }
    @Override public void onSetVolumeTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int value) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ISessionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ISessionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ISessionCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ISessionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ISessionCallback))) {
        return ((android.media.session.ISessionCallback)iin);
      }
      return new android.media.session.ISessionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          android.os.ResultReceiver _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.onCommand(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_onMediaButton:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.content.Intent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          android.os.ResultReceiver _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onMediaButton(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onMediaButtonFromController:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.onMediaButtonFromController(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onPrepare:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onPrepare(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onPrepareFromMediaId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPrepareFromMediaId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onPrepareFromSearch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPrepareFromSearch(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onPrepareFromUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.net.Uri _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPrepareFromUri(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onPlay:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onPlay(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onPlayFromMediaId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPlayFromMediaId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onPlayFromSearch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPlayFromSearch(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onPlayFromUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.net.Uri _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onPlayFromUri(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onSkipToTrack:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          long _arg4;
          _arg4 = data.readLong();
          this.onSkipToTrack(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onPause:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onPause(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onStop:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onStop(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onNext:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onNext(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onPrevious:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onPrevious(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onFastForward:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onFastForward(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onRewind:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          this.onRewind(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onSeekTo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          long _arg4;
          _arg4 = data.readLong();
          this.onSeekTo(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onRate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          android.media.Rating _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.media.Rating.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.onRate(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onSetPlaybackSpeed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          float _arg4;
          _arg4 = data.readFloat();
          this.onSetPlaybackSpeed(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onCustomAction:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.onCustomAction(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onAdjustVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          this.onAdjustVolume(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onSetVolumeTo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.media.session.ISessionControllerCallback _arg3;
          _arg3 = android.media.session.ISessionControllerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          this.onSetVolumeTo(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.ISessionCallback
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
      @Override public void onCommand(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCommand(packageName, pid, uid, caller, command, args, cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMediaButton(java.lang.String packageName, int pid, int uid, android.content.Intent mediaButtonIntent, int sequenceNumber, android.os.ResultReceiver cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          if ((mediaButtonIntent!=null)) {
            _data.writeInt(1);
            mediaButtonIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sequenceNumber);
          if ((cb!=null)) {
            _data.writeInt(1);
            cb.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMediaButton, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMediaButton(packageName, pid, uid, mediaButtonIntent, sequenceNumber, cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMediaButtonFromController(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.content.Intent mediaButtonIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((mediaButtonIntent!=null)) {
            _data.writeInt(1);
            mediaButtonIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMediaButtonFromController, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMediaButtonFromController(packageName, pid, uid, caller, mediaButtonIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // These callbacks are for the TransportControls

      @Override public void onPrepare(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrepare, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrepare(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrepareFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(mediaId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrepareFromMediaId, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrepareFromMediaId(packageName, pid, uid, caller, mediaId, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrepareFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(query);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrepareFromSearch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrepareFromSearch(packageName, pid, uid, caller, query, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrepareFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrepareFromUri, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrepareFromUri(packageName, pid, uid, caller, uri, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPlay(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPlay, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPlay(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPlayFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(mediaId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPlayFromMediaId, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPlayFromMediaId(packageName, pid, uid, caller, mediaId, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPlayFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(query);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPlayFromSearch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPlayFromSearch(packageName, pid, uid, caller, query, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPlayFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPlayFromUri, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPlayFromUri(packageName, pid, uid, caller, uri, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSkipToTrack(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeLong(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSkipToTrack, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSkipToTrack(packageName, pid, uid, caller, id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPause(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPause, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPause(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStop(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStop, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStop(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNext(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNext, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNext(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrevious(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrevious, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrevious(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFastForward(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFastForward, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFastForward(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRewind(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRewind, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRewind(packageName, pid, uid, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSeekTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeLong(pos);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSeekTo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSeekTo(packageName, pid, uid, caller, pos);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRate(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((rating!=null)) {
            _data.writeInt(1);
            rating.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRate(packageName, pid, uid, caller, rating);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetPlaybackSpeed(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeFloat(speed);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetPlaybackSpeed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetPlaybackSpeed(packageName, pid, uid, caller, speed);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCustomAction(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(action);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCustomAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCustomAction(packageName, pid, uid, caller, action, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // These callbacks are for volume handling

      @Override public void onAdjustVolume(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int direction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeInt(direction);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAdjustVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAdjustVolume(packageName, pid, uid, caller, direction);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetVolumeTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeInt(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetVolumeTo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetVolumeTo(packageName, pid, uid, caller, value);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.session.ISessionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMediaButton = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onMediaButtonFromController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onPrepare = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onPrepareFromMediaId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onPrepareFromSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onPrepareFromUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onPlay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onPlayFromMediaId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onPlayFromSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onPlayFromUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onSkipToTrack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onPause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onStop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onNext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onPrevious = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_onFastForward = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_onRewind = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_onSeekTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_onRate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_onSetPlaybackSpeed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_onCustomAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_onAdjustVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_onSetVolumeTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    public static boolean setDefaultImpl(android.media.session.ISessionCallback impl) {
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
    public static android.media.session.ISessionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCommand(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String command, android.os.Bundle args, android.os.ResultReceiver cb) throws android.os.RemoteException;
  public void onMediaButton(java.lang.String packageName, int pid, int uid, android.content.Intent mediaButtonIntent, int sequenceNumber, android.os.ResultReceiver cb) throws android.os.RemoteException;
  public void onMediaButtonFromController(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.content.Intent mediaButtonIntent) throws android.os.RemoteException;
  // These callbacks are for the TransportControls

  public void onPrepare(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onPrepareFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException;
  public void onPrepareFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException;
  public void onPrepareFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException;
  public void onPlay(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onPlayFromMediaId(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String mediaId, android.os.Bundle extras) throws android.os.RemoteException;
  public void onPlayFromSearch(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String query, android.os.Bundle extras) throws android.os.RemoteException;
  public void onPlayFromUri(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.net.Uri uri, android.os.Bundle extras) throws android.os.RemoteException;
  public void onSkipToTrack(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long id) throws android.os.RemoteException;
  public void onPause(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onStop(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onNext(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onPrevious(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onFastForward(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onRewind(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller) throws android.os.RemoteException;
  public void onSeekTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, long pos) throws android.os.RemoteException;
  public void onRate(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, android.media.Rating rating) throws android.os.RemoteException;
  public void onSetPlaybackSpeed(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, float speed) throws android.os.RemoteException;
  public void onCustomAction(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, java.lang.String action, android.os.Bundle args) throws android.os.RemoteException;
  // These callbacks are for volume handling

  public void onAdjustVolume(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int direction) throws android.os.RemoteException;
  public void onSetVolumeTo(java.lang.String packageName, int pid, int uid, android.media.session.ISessionControllerCallback caller, int value) throws android.os.RemoteException;
}
