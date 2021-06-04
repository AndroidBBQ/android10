/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * Interface to the TV input manager service.
 * @hide
 */
public interface ITvInputManager extends android.os.IInterface
{
  /** Default implementation for ITvInputManager. */
  public static class Default implements android.media.tv.ITvInputManager
  {
    @Override public java.util.List<android.media.tv.TvInputInfo> getTvInputList(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.media.tv.TvInputInfo getTvInputInfo(java.lang.String inputId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void updateTvInputInfo(android.media.tv.TvInputInfo inputInfo, int userId) throws android.os.RemoteException
    {
    }
    @Override public int getTvInputState(java.lang.String inputId, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.util.List<android.media.tv.TvContentRatingSystemInfo> getTvContentRatingSystemList(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException
    {
    }
    @Override public void unregisterCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean isParentalControlsEnabled(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setParentalControlsEnabled(boolean enabled, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean isRatingBlocked(java.lang.String rating, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<java.lang.String> getBlockedRatings(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException
    {
    }
    @Override public void createSession(android.media.tv.ITvInputClient client, java.lang.String inputId, boolean isRecordingSession, int seq, int userId) throws android.os.RemoteException
    {
    }
    @Override public void releaseSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setMainSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setSurface(android.os.IBinder sessionToken, android.view.Surface surface, int userId) throws android.os.RemoteException
    {
    }
    @Override public void dispatchSurfaceChanged(android.os.IBinder sessionToken, int format, int width, int height, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setVolume(android.os.IBinder sessionToken, float volume, int userId) throws android.os.RemoteException
    {
    }
    @Override public void tune(android.os.IBinder sessionToken, android.net.Uri channelUri, android.os.Bundle params, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setCaptionEnabled(android.os.IBinder sessionToken, boolean enabled, int userId) throws android.os.RemoteException
    {
    }
    @Override public void selectTrack(android.os.IBinder sessionToken, int type, java.lang.String trackId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void sendAppPrivateCommand(android.os.IBinder sessionToken, java.lang.String action, android.os.Bundle data, int userId) throws android.os.RemoteException
    {
    }
    @Override public void createOverlayView(android.os.IBinder sessionToken, android.os.IBinder windowToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException
    {
    }
    @Override public void relayoutOverlayView(android.os.IBinder sessionToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeOverlayView(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    @Override public void unblockContent(android.os.IBinder sessionToken, java.lang.String unblockedRating, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftPlay(android.os.IBinder sessionToken, android.net.Uri recordedProgramUri, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftPause(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftResume(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftSeekTo(android.os.IBinder sessionToken, long timeMs, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftSetPlaybackParams(android.os.IBinder sessionToken, android.media.PlaybackParams params, int userId) throws android.os.RemoteException
    {
    }
    @Override public void timeShiftEnablePositionTracking(android.os.IBinder sessionToken, boolean enable, int userId) throws android.os.RemoteException
    {
    }
    // For the recording session

    @Override public void startRecording(android.os.IBinder sessionToken, android.net.Uri programUri, int userId) throws android.os.RemoteException
    {
    }
    @Override public void stopRecording(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
    {
    }
    // For TV input hardware binding

    @Override public java.util.List<android.media.tv.TvInputHardwareInfo> getHardwareList() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.media.tv.ITvInputHardware acquireTvInputHardware(int deviceId, android.media.tv.ITvInputHardwareCallback callback, android.media.tv.TvInputInfo info, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void releaseTvInputHardware(int deviceId, android.media.tv.ITvInputHardware hardware, int userId) throws android.os.RemoteException
    {
    }
    // For TV input capturing

    @Override public java.util.List<android.media.tv.TvStreamConfig> getAvailableTvStreamConfigList(java.lang.String inputId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean captureFrame(java.lang.String inputId, android.view.Surface surface, android.media.tv.TvStreamConfig config, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isSingleSessionActive(int userId) throws android.os.RemoteException
    {
      return false;
    }
    // For DVB device binding

    @Override public java.util.List<android.media.tv.DvbDeviceInfo> getDvbDeviceList() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.ParcelFileDescriptor openDvbDevice(android.media.tv.DvbDeviceInfo info, int device) throws android.os.RemoteException
    {
      return null;
    }
    // For preview channels and programs

    @Override public void sendTvInputNotifyIntent(android.content.Intent intent, int userId) throws android.os.RemoteException
    {
    }
    @Override public void requestChannelBrowsable(android.net.Uri channelUri, int userId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputManager
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputManager interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputManager))) {
        return ((android.media.tv.ITvInputManager)iin);
      }
      return new android.media.tv.ITvInputManager.Stub.Proxy(obj);
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
        case TRANSACTION_getTvInputList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.media.tv.TvInputInfo> _result = this.getTvInputList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getTvInputInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.media.tv.TvInputInfo _result = this.getTvInputInfo(_arg0, _arg1);
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
        case TRANSACTION_updateTvInputInfo:
        {
          data.enforceInterface(descriptor);
          android.media.tv.TvInputInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.tv.TvInputInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.updateTvInputInfo(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTvInputState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getTvInputState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getTvContentRatingSystemList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.media.tv.TvContentRatingSystemInfo> _result = this.getTvContentRatingSystemList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputManagerCallback _arg0;
          _arg0 = android.media.tv.ITvInputManagerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.registerCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputManagerCallback _arg0;
          _arg0 = android.media.tv.ITvInputManagerCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.unregisterCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isParentalControlsEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isParentalControlsEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setParentalControlsEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setParentalControlsEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isRatingBlocked:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isRatingBlocked(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getBlockedRatings:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _result = this.getBlockedRatings(_arg0);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_addBlockedRating:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.addBlockedRating(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeBlockedRating:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.removeBlockedRating(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createSession:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputClient _arg0;
          _arg0 = android.media.tv.ITvInputClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.createSession(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.releaseSession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMainSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.setMainSession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSurface:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.view.Surface _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.setSurface(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dispatchSurfaceChanged:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.dispatchSurfaceChanged(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVolume:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          float _arg1;
          _arg1 = data.readFloat();
          int _arg2;
          _arg2 = data.readInt();
          this.setVolume(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_tune:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.tune(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCaptionEnabled:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.setCaptionEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_selectTrack:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.selectTrack(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendAppPrivateCommand:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.sendAppPrivateCommand(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createOverlayView:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.createOverlayView(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_relayoutOverlayView:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.relayoutOverlayView(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOverlayView:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.removeOverlayView(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unblockContent:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.unblockContent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftPlay:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.timeShiftPlay(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftPause:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.timeShiftPause(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftResume:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.timeShiftResume(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftSeekTo:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.timeShiftSeekTo(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftSetPlaybackParams:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.media.PlaybackParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.PlaybackParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.timeShiftSetPlaybackParams(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_timeShiftEnablePositionTracking:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.timeShiftEnablePositionTracking(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startRecording:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.startRecording(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopRecording:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.stopRecording(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getHardwareList:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.tv.TvInputHardwareInfo> _result = this.getHardwareList();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_acquireTvInputHardware:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.tv.ITvInputHardwareCallback _arg1;
          _arg1 = android.media.tv.ITvInputHardwareCallback.Stub.asInterface(data.readStrongBinder());
          android.media.tv.TvInputInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.tv.TvInputInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.media.tv.ITvInputHardware _result = this.acquireTvInputHardware(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_releaseTvInputHardware:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.tv.ITvInputHardware _arg1;
          _arg1 = android.media.tv.ITvInputHardware.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.releaseTvInputHardware(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAvailableTvStreamConfigList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.media.tv.TvStreamConfig> _result = this.getAvailableTvStreamConfigList(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_captureFrame:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.view.Surface _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.media.tv.TvStreamConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.tv.TvStreamConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.captureFrame(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSingleSessionActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isSingleSessionActive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getDvbDeviceList:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.tv.DvbDeviceInfo> _result = this.getDvbDeviceList();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_openDvbDevice:
        {
          data.enforceInterface(descriptor);
          android.media.tv.DvbDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.tv.DvbDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.os.ParcelFileDescriptor _result = this.openDvbDevice(_arg0, _arg1);
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
        case TRANSACTION_sendTvInputNotifyIntent:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.sendTvInputNotifyIntent(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestChannelBrowsable:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.requestChannelBrowsable(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputManager
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
      @Override public java.util.List<android.media.tv.TvInputInfo> getTvInputList(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.tv.TvInputInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTvInputList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTvInputList(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.tv.TvInputInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.media.tv.TvInputInfo getTvInputInfo(java.lang.String inputId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.tv.TvInputInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTvInputInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTvInputInfo(inputId, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.tv.TvInputInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void updateTvInputInfo(android.media.tv.TvInputInfo inputInfo, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((inputInfo!=null)) {
            _data.writeInt(1);
            inputInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateTvInputInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateTvInputInfo(inputInfo, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getTvInputState(java.lang.String inputId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTvInputState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTvInputState(inputId, userId);
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
      @Override public java.util.List<android.media.tv.TvContentRatingSystemInfo> getTvContentRatingSystemList(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.tv.TvContentRatingSystemInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTvContentRatingSystemList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTvContentRatingSystemList(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.tv.TvContentRatingSystemInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void registerCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(callback, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(callback, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isParentalControlsEnabled(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isParentalControlsEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isParentalControlsEnabled(userId);
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
      @Override public void setParentalControlsEnabled(boolean enabled, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setParentalControlsEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setParentalControlsEnabled(enabled, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isRatingBlocked(java.lang.String rating, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rating);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRatingBlocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRatingBlocked(rating, userId);
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
      @Override public java.util.List<java.lang.String> getBlockedRatings(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBlockedRatings, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBlockedRatings(userId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void addBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rating);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addBlockedRating, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addBlockedRating(rating, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rating);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeBlockedRating, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeBlockedRating(rating, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void createSession(android.media.tv.ITvInputClient client, java.lang.String inputId, boolean isRecordingSession, int seq, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(inputId);
          _data.writeInt(((isRecordingSession)?(1):(0)));
          _data.writeInt(seq);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createSession(client, inputId, isRecordingSession, seq, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releaseSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseSession(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setMainSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMainSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMainSession(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSurface(android.os.IBinder sessionToken, android.view.Surface surface, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSurface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSurface(sessionToken, surface, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dispatchSurfaceChanged(android.os.IBinder sessionToken, int format, int width, int height, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(format);
          _data.writeInt(width);
          _data.writeInt(height);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchSurfaceChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchSurfaceChanged(sessionToken, format, width, height, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setVolume(android.os.IBinder sessionToken, float volume, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeFloat(volume);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolume(sessionToken, volume, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void tune(android.os.IBinder sessionToken, android.net.Uri channelUri, android.os.Bundle params, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((channelUri!=null)) {
            _data.writeInt(1);
            channelUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_tune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tune(sessionToken, channelUri, params, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setCaptionEnabled(android.os.IBinder sessionToken, boolean enabled, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCaptionEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCaptionEnabled(sessionToken, enabled, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void selectTrack(android.os.IBinder sessionToken, int type, java.lang.String trackId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(type);
          _data.writeString(trackId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_selectTrack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().selectTrack(sessionToken, type, trackId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendAppPrivateCommand(android.os.IBinder sessionToken, java.lang.String action, android.os.Bundle data, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeString(action);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendAppPrivateCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendAppPrivateCommand(sessionToken, action, data, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void createOverlayView(android.os.IBinder sessionToken, android.os.IBinder windowToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeStrongBinder(windowToken);
          if ((frame!=null)) {
            _data.writeInt(1);
            frame.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createOverlayView, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createOverlayView(sessionToken, windowToken, frame, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void relayoutOverlayView(android.os.IBinder sessionToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((frame!=null)) {
            _data.writeInt(1);
            frame.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_relayoutOverlayView, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().relayoutOverlayView(sessionToken, frame, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOverlayView(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOverlayView, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOverlayView(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unblockContent(android.os.IBinder sessionToken, java.lang.String unblockedRating, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeString(unblockedRating);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unblockContent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unblockContent(sessionToken, unblockedRating, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftPlay(android.os.IBinder sessionToken, android.net.Uri recordedProgramUri, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((recordedProgramUri!=null)) {
            _data.writeInt(1);
            recordedProgramUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftPlay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftPlay(sessionToken, recordedProgramUri, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftPause(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftPause, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftPause(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftResume(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftResume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftResume(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftSeekTo(android.os.IBinder sessionToken, long timeMs, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeLong(timeMs);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftSeekTo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftSeekTo(sessionToken, timeMs, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftSetPlaybackParams(android.os.IBinder sessionToken, android.media.PlaybackParams params, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftSetPlaybackParams, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftSetPlaybackParams(sessionToken, params, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void timeShiftEnablePositionTracking(android.os.IBinder sessionToken, boolean enable, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_timeShiftEnablePositionTracking, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().timeShiftEnablePositionTracking(sessionToken, enable, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // For the recording session

      @Override public void startRecording(android.os.IBinder sessionToken, android.net.Uri programUri, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          if ((programUri!=null)) {
            _data.writeInt(1);
            programUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecording, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startRecording(sessionToken, programUri, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopRecording(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(sessionToken);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRecording, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopRecording(sessionToken, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // For TV input hardware binding

      @Override public java.util.List<android.media.tv.TvInputHardwareInfo> getHardwareList() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.tv.TvInputHardwareInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getHardwareList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getHardwareList();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.tv.TvInputHardwareInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.media.tv.ITvInputHardware acquireTvInputHardware(int deviceId, android.media.tv.ITvInputHardwareCallback callback, android.media.tv.TvInputInfo info, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.tv.ITvInputHardware _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireTvInputHardware, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().acquireTvInputHardware(deviceId, callback, info, userId);
          }
          _reply.readException();
          _result = android.media.tv.ITvInputHardware.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void releaseTvInputHardware(int deviceId, android.media.tv.ITvInputHardware hardware, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeStrongBinder((((hardware!=null))?(hardware.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseTvInputHardware, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseTvInputHardware(deviceId, hardware, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // For TV input capturing

      @Override public java.util.List<android.media.tv.TvStreamConfig> getAvailableTvStreamConfigList(java.lang.String inputId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.tv.TvStreamConfig> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableTvStreamConfigList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableTvStreamConfigList(inputId, userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.tv.TvStreamConfig.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean captureFrame(java.lang.String inputId, android.view.Surface surface, android.media.tv.TvStreamConfig config, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputId);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_captureFrame, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().captureFrame(inputId, surface, config, userId);
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
      @Override public boolean isSingleSessionActive(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSingleSessionActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSingleSessionActive(userId);
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
      // For DVB device binding

      @Override public java.util.List<android.media.tv.DvbDeviceInfo> getDvbDeviceList() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.tv.DvbDeviceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDvbDeviceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDvbDeviceList();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.tv.DvbDeviceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.ParcelFileDescriptor openDvbDevice(android.media.tv.DvbDeviceInfo info, int device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(device);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDvbDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openDvbDevice(info, device);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
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
      // For preview channels and programs

      @Override public void sendTvInputNotifyIntent(android.content.Intent intent, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendTvInputNotifyIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendTvInputNotifyIntent(intent, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestChannelBrowsable(android.net.Uri channelUri, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((channelUri!=null)) {
            _data.writeInt(1);
            channelUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestChannelBrowsable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestChannelBrowsable(channelUri, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputManager sDefaultImpl;
    }
    static final int TRANSACTION_getTvInputList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getTvInputInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updateTvInputInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getTvInputState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getTvContentRatingSystemList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isParentalControlsEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setParentalControlsEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isRatingBlocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getBlockedRatings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addBlockedRating = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_removeBlockedRating = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_createSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_releaseSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setMainSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setSurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_dispatchSurfaceChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_tune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setCaptionEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_selectTrack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_sendAppPrivateCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_createOverlayView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_relayoutOverlayView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_removeOverlayView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_unblockContent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_timeShiftPlay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_timeShiftPause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_timeShiftResume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_timeShiftSeekTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_timeShiftSetPlaybackParams = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_timeShiftEnablePositionTracking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_startRecording = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_stopRecording = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getHardwareList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_acquireTvInputHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_releaseTvInputHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_getAvailableTvStreamConfigList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_captureFrame = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_isSingleSessionActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getDvbDeviceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_openDvbDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_sendTvInputNotifyIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_requestChannelBrowsable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    public static boolean setDefaultImpl(android.media.tv.ITvInputManager impl) {
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
    public static android.media.tv.ITvInputManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.util.List<android.media.tv.TvInputInfo> getTvInputList(int userId) throws android.os.RemoteException;
  public android.media.tv.TvInputInfo getTvInputInfo(java.lang.String inputId, int userId) throws android.os.RemoteException;
  public void updateTvInputInfo(android.media.tv.TvInputInfo inputInfo, int userId) throws android.os.RemoteException;
  public int getTvInputState(java.lang.String inputId, int userId) throws android.os.RemoteException;
  public java.util.List<android.media.tv.TvContentRatingSystemInfo> getTvContentRatingSystemList(int userId) throws android.os.RemoteException;
  public void registerCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException;
  public void unregisterCallback(android.media.tv.ITvInputManagerCallback callback, int userId) throws android.os.RemoteException;
  public boolean isParentalControlsEnabled(int userId) throws android.os.RemoteException;
  public void setParentalControlsEnabled(boolean enabled, int userId) throws android.os.RemoteException;
  public boolean isRatingBlocked(java.lang.String rating, int userId) throws android.os.RemoteException;
  public java.util.List<java.lang.String> getBlockedRatings(int userId) throws android.os.RemoteException;
  public void addBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException;
  public void removeBlockedRating(java.lang.String rating, int userId) throws android.os.RemoteException;
  public void createSession(android.media.tv.ITvInputClient client, java.lang.String inputId, boolean isRecordingSession, int seq, int userId) throws android.os.RemoteException;
  public void releaseSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  public void setMainSession(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  public void setSurface(android.os.IBinder sessionToken, android.view.Surface surface, int userId) throws android.os.RemoteException;
  public void dispatchSurfaceChanged(android.os.IBinder sessionToken, int format, int width, int height, int userId) throws android.os.RemoteException;
  public void setVolume(android.os.IBinder sessionToken, float volume, int userId) throws android.os.RemoteException;
  public void tune(android.os.IBinder sessionToken, android.net.Uri channelUri, android.os.Bundle params, int userId) throws android.os.RemoteException;
  public void setCaptionEnabled(android.os.IBinder sessionToken, boolean enabled, int userId) throws android.os.RemoteException;
  public void selectTrack(android.os.IBinder sessionToken, int type, java.lang.String trackId, int userId) throws android.os.RemoteException;
  public void sendAppPrivateCommand(android.os.IBinder sessionToken, java.lang.String action, android.os.Bundle data, int userId) throws android.os.RemoteException;
  public void createOverlayView(android.os.IBinder sessionToken, android.os.IBinder windowToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException;
  public void relayoutOverlayView(android.os.IBinder sessionToken, android.graphics.Rect frame, int userId) throws android.os.RemoteException;
  public void removeOverlayView(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  public void unblockContent(android.os.IBinder sessionToken, java.lang.String unblockedRating, int userId) throws android.os.RemoteException;
  public void timeShiftPlay(android.os.IBinder sessionToken, android.net.Uri recordedProgramUri, int userId) throws android.os.RemoteException;
  public void timeShiftPause(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  public void timeShiftResume(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  public void timeShiftSeekTo(android.os.IBinder sessionToken, long timeMs, int userId) throws android.os.RemoteException;
  public void timeShiftSetPlaybackParams(android.os.IBinder sessionToken, android.media.PlaybackParams params, int userId) throws android.os.RemoteException;
  public void timeShiftEnablePositionTracking(android.os.IBinder sessionToken, boolean enable, int userId) throws android.os.RemoteException;
  // For the recording session

  public void startRecording(android.os.IBinder sessionToken, android.net.Uri programUri, int userId) throws android.os.RemoteException;
  public void stopRecording(android.os.IBinder sessionToken, int userId) throws android.os.RemoteException;
  // For TV input hardware binding

  public java.util.List<android.media.tv.TvInputHardwareInfo> getHardwareList() throws android.os.RemoteException;
  public android.media.tv.ITvInputHardware acquireTvInputHardware(int deviceId, android.media.tv.ITvInputHardwareCallback callback, android.media.tv.TvInputInfo info, int userId) throws android.os.RemoteException;
  public void releaseTvInputHardware(int deviceId, android.media.tv.ITvInputHardware hardware, int userId) throws android.os.RemoteException;
  // For TV input capturing

  public java.util.List<android.media.tv.TvStreamConfig> getAvailableTvStreamConfigList(java.lang.String inputId, int userId) throws android.os.RemoteException;
  public boolean captureFrame(java.lang.String inputId, android.view.Surface surface, android.media.tv.TvStreamConfig config, int userId) throws android.os.RemoteException;
  public boolean isSingleSessionActive(int userId) throws android.os.RemoteException;
  // For DVB device binding

  public java.util.List<android.media.tv.DvbDeviceInfo> getDvbDeviceList() throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor openDvbDevice(android.media.tv.DvbDeviceInfo info, int device) throws android.os.RemoteException;
  // For preview channels and programs

  public void sendTvInputNotifyIntent(android.content.Intent intent, int userId) throws android.os.RemoteException;
  public void requestChannelBrowsable(android.net.Uri channelUri, int userId) throws android.os.RemoteException;
}
