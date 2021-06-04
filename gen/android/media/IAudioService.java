/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * {@hide}
 */
public interface IAudioService extends android.os.IInterface
{
  /** Default implementation for IAudioService. */
  public static class Default implements android.media.IAudioService
  {
    // C++ and Java methods below.
    // WARNING: When methods are inserted or deleted in this section, the transaction IDs in
    // frameworks/native/include/audiomanager/IAudioManager.h must be updated to match the order
    // in this file.
    //
    // When a method's argument list is changed, BpAudioManager's corresponding serialization code
    // (if any) in frameworks/native/services/audiomanager/IAudioManager.cpp must be updated.

    @Override public int trackPlayer(android.media.PlayerBase.PlayerIdCard pic) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void playerAttributes(int piid, android.media.AudioAttributes attr) throws android.os.RemoteException
    {
    }
    @Override public void playerEvent(int piid, int event) throws android.os.RemoteException
    {
    }
    @Override public void releasePlayer(int piid) throws android.os.RemoteException
    {
    }
    @Override public int trackRecorder(android.os.IBinder recorder) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void recorderEvent(int riid, int event) throws android.os.RemoteException
    {
    }
    @Override public void releaseRecorder(int riid) throws android.os.RemoteException
    {
    }
    // Java-only methods below.

    @Override public void adjustSuggestedStreamVolume(int direction, int suggestedStreamType, int flags, java.lang.String callingPackage, java.lang.String caller) throws android.os.RemoteException
    {
    }
    @Override public void adjustStreamVolume(int streamType, int direction, int flags, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void setStreamVolume(int streamType, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public boolean isStreamMute(int streamType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void forceRemoteSubmixFullVolume(boolean startForcing, android.os.IBinder cb) throws android.os.RemoteException
    {
    }
    @Override public boolean isMasterMute() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setMasterMute(boolean mute, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
    }
    @Override public int getStreamVolume(int streamType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getStreamMinVolume(int streamType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getStreamMaxVolume(int streamType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.util.List<android.media.audiopolicy.AudioVolumeGroup> getAudioVolumeGroups() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setVolumeIndexForAttributes(android.media.AudioAttributes aa, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getMaxVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getMinVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getLastAudibleStreamVolume(int streamType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.util.List<android.media.audiopolicy.AudioProductStrategy> getAudioProductStrategies() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setMicrophoneMute(boolean on, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setRingerModeExternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException
    {
    }
    @Override public void setRingerModeInternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException
    {
    }
    @Override public int getRingerModeExternal() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getRingerModeInternal() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isValidRingerMode(int ringerMode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setVibrateSetting(int vibrateType, int vibrateSetting) throws android.os.RemoteException
    {
    }
    @Override public int getVibrateSetting(int vibrateType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean shouldVibrate(int vibrateType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setMode(int mode, android.os.IBinder cb, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getMode() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void playSoundEffect(int effectType) throws android.os.RemoteException
    {
    }
    @Override public void playSoundEffectVolume(int effectType, float volume) throws android.os.RemoteException
    {
    }
    @Override public boolean loadSoundEffects() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void unloadSoundEffects() throws android.os.RemoteException
    {
    }
    @Override public void reloadAudioSettings() throws android.os.RemoteException
    {
    }
    @Override public void avrcpSupportsAbsoluteVolume(java.lang.String address, boolean support) throws android.os.RemoteException
    {
    }
    @Override public void setSpeakerphoneOn(boolean on) throws android.os.RemoteException
    {
    }
    @Override public boolean isSpeakerphoneOn() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setBluetoothScoOn(boolean on) throws android.os.RemoteException
    {
    }
    @Override public boolean isBluetoothScoOn() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setBluetoothA2dpOn(boolean on) throws android.os.RemoteException
    {
    }
    @Override public boolean isBluetoothA2dpOn() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int requestAudioFocus(android.media.AudioAttributes aa, int durationHint, android.os.IBinder cb, android.media.IAudioFocusDispatcher fd, java.lang.String clientId, java.lang.String callingPackageName, int flags, android.media.audiopolicy.IAudioPolicyCallback pcb, int sdk) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int abandonAudioFocus(android.media.IAudioFocusDispatcher fd, java.lang.String clientId, android.media.AudioAttributes aa, java.lang.String callingPackageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void unregisterAudioFocusClient(java.lang.String clientId) throws android.os.RemoteException
    {
    }
    @Override public int getCurrentAudioFocus() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void startBluetoothSco(android.os.IBinder cb, int targetSdkVersion) throws android.os.RemoteException
    {
    }
    @Override public void startBluetoothScoVirtualCall(android.os.IBinder cb) throws android.os.RemoteException
    {
    }
    @Override public void stopBluetoothSco(android.os.IBinder cb) throws android.os.RemoteException
    {
    }
    @Override public void forceVolumeControlStream(int streamType, android.os.IBinder cb) throws android.os.RemoteException
    {
    }
    @Override public void setRingtonePlayer(android.media.IRingtonePlayer player) throws android.os.RemoteException
    {
    }
    @Override public android.media.IRingtonePlayer getRingtonePlayer() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getUiSoundsStreamType() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setWiredDeviceConnectionState(int type, int state, java.lang.String address, java.lang.String name, java.lang.String caller) throws android.os.RemoteException
    {
    }
    @Override public void handleBluetoothA2dpDeviceConfigChange(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
    }
    @Override public android.media.AudioRoutesInfo startWatchingRoutes(android.media.IAudioRoutesObserver observer) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isCameraSoundForced() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setVolumeController(android.media.IVolumeController controller) throws android.os.RemoteException
    {
    }
    @Override public void notifyVolumeControllerVisible(android.media.IVolumeController controller, boolean visible) throws android.os.RemoteException
    {
    }
    @Override public boolean isStreamAffectedByRingerMode(int streamType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isStreamAffectedByMute(int streamType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void disableSafeMediaVolume(java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int setHdmiSystemAudioSupported(boolean on) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isHdmiSystemAudioSupported() throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String registerAudioPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb, boolean hasFocusListener, boolean isFocusPolicy, boolean isTestFocusPolicy, boolean isVolumeController, android.media.projection.IMediaProjection projection) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void unregisterAudioPolicyAsync(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
    }
    @Override public void unregisterAudioPolicy(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
    }
    @Override public int addMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int removeMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int setFocusPropertiesForPolicy(int duckingBehavior, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setVolumePolicy(android.media.VolumePolicy policy) throws android.os.RemoteException
    {
    }
    @Override public boolean hasRegisteredDynamicPolicy() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void registerRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException
    {
    }
    @Override public void unregisterRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.media.AudioRecordingConfiguration> getActiveRecordingConfigurations() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException
    {
    }
    @Override public void unregisterPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.media.AudioPlaybackConfiguration> getActivePlaybackConfigurations() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void disableRingtoneSync(int userId) throws android.os.RemoteException
    {
    }
    @Override public int getFocusRampTimeMs(int focusGain, android.media.AudioAttributes attr) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int dispatchFocusChange(android.media.AudioFocusInfo afi, int focusChange, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void playerHasOpPlayAudio(int piid, boolean hasOpPlayAudio) throws android.os.RemoteException
    {
    }
    @Override public void setBluetoothHearingAidDeviceConnectionState(android.bluetooth.BluetoothDevice device, int state, boolean suppressNoisyIntent, int musicDevice) throws android.os.RemoteException
    {
    }
    @Override public void setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent(android.bluetooth.BluetoothDevice device, int state, int profile, boolean suppressNoisyIntent, int a2dpVolume) throws android.os.RemoteException
    {
    }
    @Override public void setFocusRequestResultFromExtPolicy(android.media.AudioFocusInfo afi, int requestResult, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
    {
    }
    @Override public void registerAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException
    {
    }
    @Override public void unregisterAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException
    {
    }
    @Override public boolean isAudioServerRunning() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int setUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid, int[] deviceTypes, java.lang.String[] deviceAddresses) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int removeUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean hasHapticChannels(android.net.Uri uri) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int setAllowedCapturePolicy(int capturePolicy) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getAllowedCapturePolicy() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IAudioService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IAudioService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IAudioService interface,
     * generating a proxy if needed.
     */
    public static android.media.IAudioService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IAudioService))) {
        return ((android.media.IAudioService)iin);
      }
      return new android.media.IAudioService.Stub.Proxy(obj);
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
        case TRANSACTION_trackPlayer:
        {
          data.enforceInterface(descriptor);
          android.media.PlayerBase.PlayerIdCard _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.PlayerBase.PlayerIdCard.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.trackPlayer(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_playerAttributes:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.AudioAttributes _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.playerAttributes(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_playerEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.playerEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_releasePlayer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.releasePlayer(_arg0);
          return true;
        }
        case TRANSACTION_trackRecorder:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.trackRecorder(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_recorderEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.recorderEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_releaseRecorder:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.releaseRecorder(_arg0);
          return true;
        }
        case TRANSACTION_adjustSuggestedStreamVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.adjustSuggestedStreamVolume(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_adjustStreamVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.adjustStreamVolume(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setStreamVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setStreamVolume(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isStreamMute:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isStreamMute(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_forceRemoteSubmixFullVolume:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.forceRemoteSubmixFullVolume(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isMasterMute:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isMasterMute();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setMasterMute:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.setMasterMute(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getStreamVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getStreamVolume(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getStreamMinVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getStreamMinVolume(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getStreamMaxVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getStreamMaxVolume(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAudioVolumeGroups:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.audiopolicy.AudioVolumeGroup> _result = this.getAudioVolumeGroups();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setVolumeIndexForAttributes:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setVolumeIndexForAttributes(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVolumeIndexForAttributes:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getVolumeIndexForAttributes(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMaxVolumeIndexForAttributes:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getMaxVolumeIndexForAttributes(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMinVolumeIndexForAttributes:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getMinVolumeIndexForAttributes(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getLastAudibleStreamVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getLastAudibleStreamVolume(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAudioProductStrategies:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.audiopolicy.AudioProductStrategy> _result = this.getAudioProductStrategies();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setMicrophoneMute:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.setMicrophoneMute(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRingerModeExternal:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setRingerModeExternal(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRingerModeInternal:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setRingerModeInternal(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRingerModeExternal:
        {
          data.enforceInterface(descriptor);
          int _result = this.getRingerModeExternal();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getRingerModeInternal:
        {
          data.enforceInterface(descriptor);
          int _result = this.getRingerModeInternal();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isValidRingerMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isValidRingerMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVibrateSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setVibrateSetting(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVibrateSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getVibrateSetting(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_shouldVibrate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.shouldVibrate(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_playSoundEffect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.playSoundEffect(_arg0);
          return true;
        }
        case TRANSACTION_playSoundEffectVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _arg1;
          _arg1 = data.readFloat();
          this.playSoundEffectVolume(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_loadSoundEffects:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.loadSoundEffects();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unloadSoundEffects:
        {
          data.enforceInterface(descriptor);
          this.unloadSoundEffects();
          return true;
        }
        case TRANSACTION_reloadAudioSettings:
        {
          data.enforceInterface(descriptor);
          this.reloadAudioSettings();
          return true;
        }
        case TRANSACTION_avrcpSupportsAbsoluteVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.avrcpSupportsAbsoluteVolume(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setSpeakerphoneOn:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSpeakerphoneOn(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isSpeakerphoneOn:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSpeakerphoneOn();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setBluetoothScoOn:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setBluetoothScoOn(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBluetoothScoOn:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isBluetoothScoOn();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setBluetoothA2dpOn:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setBluetoothA2dpOn(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBluetoothA2dpOn:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isBluetoothA2dpOn();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestAudioFocus:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          android.media.IAudioFocusDispatcher _arg3;
          _arg3 = android.media.IAudioFocusDispatcher.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          java.lang.String _arg5;
          _arg5 = data.readString();
          int _arg6;
          _arg6 = data.readInt();
          android.media.audiopolicy.IAudioPolicyCallback _arg7;
          _arg7 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _arg8;
          _arg8 = data.readInt();
          int _result = this.requestAudioFocus(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_abandonAudioFocus:
        {
          data.enforceInterface(descriptor);
          android.media.IAudioFocusDispatcher _arg0;
          _arg0 = android.media.IAudioFocusDispatcher.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.AudioAttributes _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _result = this.abandonAudioFocus(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unregisterAudioFocusClient:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.unregisterAudioFocusClient(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentAudioFocus:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCurrentAudioFocus();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startBluetoothSco:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.startBluetoothSco(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startBluetoothScoVirtualCall:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.startBluetoothScoVirtualCall(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopBluetoothSco:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.stopBluetoothSco(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forceVolumeControlStream:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.forceVolumeControlStream(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRingtonePlayer:
        {
          data.enforceInterface(descriptor);
          android.media.IRingtonePlayer _arg0;
          _arg0 = android.media.IRingtonePlayer.Stub.asInterface(data.readStrongBinder());
          this.setRingtonePlayer(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRingtonePlayer:
        {
          data.enforceInterface(descriptor);
          android.media.IRingtonePlayer _result = this.getRingtonePlayer();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getUiSoundsStreamType:
        {
          data.enforceInterface(descriptor);
          int _result = this.getUiSoundsStreamType();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setWiredDeviceConnectionState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.setWiredDeviceConnectionState(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handleBluetoothA2dpDeviceConfigChange:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.handleBluetoothA2dpDeviceConfigChange(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWatchingRoutes:
        {
          data.enforceInterface(descriptor);
          android.media.IAudioRoutesObserver _arg0;
          _arg0 = android.media.IAudioRoutesObserver.Stub.asInterface(data.readStrongBinder());
          android.media.AudioRoutesInfo _result = this.startWatchingRoutes(_arg0);
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
        case TRANSACTION_isCameraSoundForced:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isCameraSoundForced();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVolumeController:
        {
          data.enforceInterface(descriptor);
          android.media.IVolumeController _arg0;
          _arg0 = android.media.IVolumeController.Stub.asInterface(data.readStrongBinder());
          this.setVolumeController(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyVolumeControllerVisible:
        {
          data.enforceInterface(descriptor);
          android.media.IVolumeController _arg0;
          _arg0 = android.media.IVolumeController.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.notifyVolumeControllerVisible(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isStreamAffectedByRingerMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isStreamAffectedByRingerMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isStreamAffectedByMute:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isStreamAffectedByMute(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableSafeMediaVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.disableSafeMediaVolume(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setHdmiSystemAudioSupported:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _result = this.setHdmiSystemAudioSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isHdmiSystemAudioSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isHdmiSystemAudioSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerAudioPolicy:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.AudioPolicyConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.audiopolicy.AudioPolicyConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.audiopolicy.IAudioPolicyCallback _arg1;
          _arg1 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          android.media.projection.IMediaProjection _arg6;
          _arg6 = android.media.projection.IMediaProjection.Stub.asInterface(data.readStrongBinder());
          java.lang.String _result = this.registerAudioPolicy(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_unregisterAudioPolicyAsync:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.IAudioPolicyCallback _arg0;
          _arg0 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterAudioPolicyAsync(_arg0);
          return true;
        }
        case TRANSACTION_unregisterAudioPolicy:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.IAudioPolicyCallback _arg0;
          _arg0 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterAudioPolicy(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addMixForPolicy:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.AudioPolicyConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.audiopolicy.AudioPolicyConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.audiopolicy.IAudioPolicyCallback _arg1;
          _arg1 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.addMixForPolicy(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeMixForPolicy:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.AudioPolicyConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.audiopolicy.AudioPolicyConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.audiopolicy.IAudioPolicyCallback _arg1;
          _arg1 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.removeMixForPolicy(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFocusPropertiesForPolicy:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.audiopolicy.IAudioPolicyCallback _arg1;
          _arg1 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.setFocusPropertiesForPolicy(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setVolumePolicy:
        {
          data.enforceInterface(descriptor);
          android.media.VolumePolicy _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.VolumePolicy.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setVolumePolicy(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasRegisteredDynamicPolicy:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasRegisteredDynamicPolicy();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerRecordingCallback:
        {
          data.enforceInterface(descriptor);
          android.media.IRecordingConfigDispatcher _arg0;
          _arg0 = android.media.IRecordingConfigDispatcher.Stub.asInterface(data.readStrongBinder());
          this.registerRecordingCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterRecordingCallback:
        {
          data.enforceInterface(descriptor);
          android.media.IRecordingConfigDispatcher _arg0;
          _arg0 = android.media.IRecordingConfigDispatcher.Stub.asInterface(data.readStrongBinder());
          this.unregisterRecordingCallback(_arg0);
          return true;
        }
        case TRANSACTION_getActiveRecordingConfigurations:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.AudioRecordingConfiguration> _result = this.getActiveRecordingConfigurations();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerPlaybackCallback:
        {
          data.enforceInterface(descriptor);
          android.media.IPlaybackConfigDispatcher _arg0;
          _arg0 = android.media.IPlaybackConfigDispatcher.Stub.asInterface(data.readStrongBinder());
          this.registerPlaybackCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterPlaybackCallback:
        {
          data.enforceInterface(descriptor);
          android.media.IPlaybackConfigDispatcher _arg0;
          _arg0 = android.media.IPlaybackConfigDispatcher.Stub.asInterface(data.readStrongBinder());
          this.unregisterPlaybackCallback(_arg0);
          return true;
        }
        case TRANSACTION_getActivePlaybackConfigurations:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.AudioPlaybackConfiguration> _result = this.getActivePlaybackConfigurations();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_disableRingtoneSync:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableRingtoneSync(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFocusRampTimeMs:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.AudioAttributes _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.getFocusRampTimeMs(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_dispatchFocusChange:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.media.audiopolicy.IAudioPolicyCallback _arg2;
          _arg2 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.dispatchFocusChange(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_playerHasOpPlayAudio:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.playerHasOpPlayAudio(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setBluetoothHearingAidDeviceConnectionState:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.setBluetoothHearingAidDeviceConnectionState(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          int _arg4;
          _arg4 = data.readInt();
          this.setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFocusRequestResultFromExtPolicy:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.media.audiopolicy.IAudioPolicyCallback _arg2;
          _arg2 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          this.setFocusRequestResultFromExtPolicy(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_registerAudioServerStateDispatcher:
        {
          data.enforceInterface(descriptor);
          android.media.IAudioServerStateDispatcher _arg0;
          _arg0 = android.media.IAudioServerStateDispatcher.Stub.asInterface(data.readStrongBinder());
          this.registerAudioServerStateDispatcher(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterAudioServerStateDispatcher:
        {
          data.enforceInterface(descriptor);
          android.media.IAudioServerStateDispatcher _arg0;
          _arg0 = android.media.IAudioServerStateDispatcher.Stub.asInterface(data.readStrongBinder());
          this.unregisterAudioServerStateDispatcher(_arg0);
          return true;
        }
        case TRANSACTION_isAudioServerRunning:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isAudioServerRunning();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setUidDeviceAffinity:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.IAudioPolicyCallback _arg0;
          _arg0 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int[] _arg2;
          _arg2 = data.createIntArray();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          int _result = this.setUidDeviceAffinity(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeUidDeviceAffinity:
        {
          data.enforceInterface(descriptor);
          android.media.audiopolicy.IAudioPolicyCallback _arg0;
          _arg0 = android.media.audiopolicy.IAudioPolicyCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.removeUidDeviceAffinity(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasHapticChannels:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.hasHapticChannels(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAllowedCapturePolicy:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.setAllowedCapturePolicy(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAllowedCapturePolicy:
        {
          data.enforceInterface(descriptor);
          int _result = this.getAllowedCapturePolicy();
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
    private static class Proxy implements android.media.IAudioService
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
      // C++ and Java methods below.
      // WARNING: When methods are inserted or deleted in this section, the transaction IDs in
      // frameworks/native/include/audiomanager/IAudioManager.h must be updated to match the order
      // in this file.
      //
      // When a method's argument list is changed, BpAudioManager's corresponding serialization code
      // (if any) in frameworks/native/services/audiomanager/IAudioManager.cpp must be updated.

      @Override public int trackPlayer(android.media.PlayerBase.PlayerIdCard pic) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pic!=null)) {
            _data.writeInt(1);
            pic.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_trackPlayer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().trackPlayer(pic);
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
      @Override public void playerAttributes(int piid, android.media.AudioAttributes attr) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(piid);
          if ((attr!=null)) {
            _data.writeInt(1);
            attr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_playerAttributes, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playerAttributes(piid, attr);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void playerEvent(int piid, int event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(piid);
          _data.writeInt(event);
          boolean _status = mRemote.transact(Stub.TRANSACTION_playerEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playerEvent(piid, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void releasePlayer(int piid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(piid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releasePlayer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releasePlayer(piid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public int trackRecorder(android.os.IBinder recorder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(recorder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_trackRecorder, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().trackRecorder(recorder);
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
      @Override public void recorderEvent(int riid, int event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(riid);
          _data.writeInt(event);
          boolean _status = mRemote.transact(Stub.TRANSACTION_recorderEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().recorderEvent(riid, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void releaseRecorder(int riid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(riid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseRecorder, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseRecorder(riid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Java-only methods below.

      @Override public void adjustSuggestedStreamVolume(int direction, int suggestedStreamType, int flags, java.lang.String callingPackage, java.lang.String caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(direction);
          _data.writeInt(suggestedStreamType);
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          _data.writeString(caller);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adjustSuggestedStreamVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adjustSuggestedStreamVolume(direction, suggestedStreamType, flags, callingPackage, caller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void adjustStreamVolume(int streamType, int direction, int flags, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          _data.writeInt(direction);
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adjustStreamVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adjustStreamVolume(streamType, direction, flags, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setStreamVolume(int streamType, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          _data.writeInt(index);
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStreamVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStreamVolume(streamType, index, flags, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isStreamMute(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isStreamMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isStreamMute(streamType);
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
      @Override public void forceRemoteSubmixFullVolume(boolean startForcing, android.os.IBinder cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((startForcing)?(1):(0)));
          _data.writeStrongBinder(cb);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceRemoteSubmixFullVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceRemoteSubmixFullVolume(startForcing, cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isMasterMute() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMasterMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMasterMute();
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
      @Override public void setMasterMute(boolean mute, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((mute)?(1):(0)));
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMasterMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMasterMute(mute, flags, callingPackage, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getStreamVolume(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStreamVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStreamVolume(streamType);
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
      @Override public int getStreamMinVolume(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStreamMinVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStreamMinVolume(streamType);
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
      @Override public int getStreamMaxVolume(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStreamMaxVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStreamMaxVolume(streamType);
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
      @Override public java.util.List<android.media.audiopolicy.AudioVolumeGroup> getAudioVolumeGroups() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.audiopolicy.AudioVolumeGroup> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAudioVolumeGroups, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAudioVolumeGroups();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.audiopolicy.AudioVolumeGroup.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setVolumeIndexForAttributes(android.media.AudioAttributes aa, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(index);
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolumeIndexForAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolumeIndexForAttributes(aa, index, flags, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVolumeIndexForAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVolumeIndexForAttributes(aa);
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
      @Override public int getMaxVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMaxVolumeIndexForAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMaxVolumeIndexForAttributes(aa);
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
      @Override public int getMinVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMinVolumeIndexForAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMinVolumeIndexForAttributes(aa);
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
      @Override public int getLastAudibleStreamVolume(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastAudibleStreamVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastAudibleStreamVolume(streamType);
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
      @Override public java.util.List<android.media.audiopolicy.AudioProductStrategy> getAudioProductStrategies() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.audiopolicy.AudioProductStrategy> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAudioProductStrategies, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAudioProductStrategies();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.audiopolicy.AudioProductStrategy.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setMicrophoneMute(boolean on, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMicrophoneMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMicrophoneMute(on, callingPackage, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRingerModeExternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(ringerMode);
          _data.writeString(caller);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRingerModeExternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRingerModeExternal(ringerMode, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRingerModeInternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(ringerMode);
          _data.writeString(caller);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRingerModeInternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRingerModeInternal(ringerMode, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getRingerModeExternal() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRingerModeExternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRingerModeExternal();
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
      @Override public int getRingerModeInternal() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRingerModeInternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRingerModeInternal();
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
      @Override public boolean isValidRingerMode(int ringerMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(ringerMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isValidRingerMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isValidRingerMode(ringerMode);
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
      @Override public void setVibrateSetting(int vibrateType, int vibrateSetting) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(vibrateType);
          _data.writeInt(vibrateSetting);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVibrateSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVibrateSetting(vibrateType, vibrateSetting);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getVibrateSetting(int vibrateType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(vibrateType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVibrateSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVibrateSetting(vibrateType);
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
      @Override public boolean shouldVibrate(int vibrateType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(vibrateType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldVibrate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldVibrate(vibrateType);
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
      @Override public void setMode(int mode, android.os.IBinder cb, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          _data.writeStrongBinder(cb);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMode(mode, cb, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMode();
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
      @Override public void playSoundEffect(int effectType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(effectType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_playSoundEffect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playSoundEffect(effectType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void playSoundEffectVolume(int effectType, float volume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(effectType);
          _data.writeFloat(volume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_playSoundEffectVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playSoundEffectVolume(effectType, volume);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public boolean loadSoundEffects() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadSoundEffects, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().loadSoundEffects();
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
      @Override public void unloadSoundEffects() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unloadSoundEffects, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unloadSoundEffects();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void reloadAudioSettings() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reloadAudioSettings, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reloadAudioSettings();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void avrcpSupportsAbsoluteVolume(java.lang.String address, boolean support) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          _data.writeInt(((support)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_avrcpSupportsAbsoluteVolume, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().avrcpSupportsAbsoluteVolume(address, support);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setSpeakerphoneOn(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSpeakerphoneOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSpeakerphoneOn(on);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isSpeakerphoneOn() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSpeakerphoneOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSpeakerphoneOn();
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
      @Override public void setBluetoothScoOn(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBluetoothScoOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBluetoothScoOn(on);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isBluetoothScoOn() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBluetoothScoOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBluetoothScoOn();
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
      @Override public void setBluetoothA2dpOn(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBluetoothA2dpOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBluetoothA2dpOn(on);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isBluetoothA2dpOn() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBluetoothA2dpOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBluetoothA2dpOn();
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
      @Override public int requestAudioFocus(android.media.AudioAttributes aa, int durationHint, android.os.IBinder cb, android.media.IAudioFocusDispatcher fd, java.lang.String clientId, java.lang.String callingPackageName, int flags, android.media.audiopolicy.IAudioPolicyCallback pcb, int sdk) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(durationHint);
          _data.writeStrongBinder(cb);
          _data.writeStrongBinder((((fd!=null))?(fd.asBinder()):(null)));
          _data.writeString(clientId);
          _data.writeString(callingPackageName);
          _data.writeInt(flags);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          _data.writeInt(sdk);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAudioFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestAudioFocus(aa, durationHint, cb, fd, clientId, callingPackageName, flags, pcb, sdk);
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
      @Override public int abandonAudioFocus(android.media.IAudioFocusDispatcher fd, java.lang.String clientId, android.media.AudioAttributes aa, java.lang.String callingPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((fd!=null))?(fd.asBinder()):(null)));
          _data.writeString(clientId);
          if ((aa!=null)) {
            _data.writeInt(1);
            aa.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_abandonAudioFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().abandonAudioFocus(fd, clientId, aa, callingPackageName);
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
      @Override public void unregisterAudioFocusClient(java.lang.String clientId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(clientId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAudioFocusClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAudioFocusClient(clientId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getCurrentAudioFocus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentAudioFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentAudioFocus();
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
      @Override public void startBluetoothSco(android.os.IBinder cb, int targetSdkVersion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(cb);
          _data.writeInt(targetSdkVersion);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startBluetoothSco, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startBluetoothSco(cb, targetSdkVersion);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startBluetoothScoVirtualCall(android.os.IBinder cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(cb);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startBluetoothScoVirtualCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startBluetoothScoVirtualCall(cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopBluetoothSco(android.os.IBinder cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(cb);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopBluetoothSco, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopBluetoothSco(cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void forceVolumeControlStream(int streamType, android.os.IBinder cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          _data.writeStrongBinder(cb);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceVolumeControlStream, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceVolumeControlStream(streamType, cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRingtonePlayer(android.media.IRingtonePlayer player) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((player!=null))?(player.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRingtonePlayer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRingtonePlayer(player);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.media.IRingtonePlayer getRingtonePlayer() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.IRingtonePlayer _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRingtonePlayer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRingtonePlayer();
          }
          _reply.readException();
          _result = android.media.IRingtonePlayer.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getUiSoundsStreamType() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUiSoundsStreamType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUiSoundsStreamType();
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
      @Override public void setWiredDeviceConnectionState(int type, int state, java.lang.String address, java.lang.String name, java.lang.String caller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          _data.writeInt(state);
          _data.writeString(address);
          _data.writeString(name);
          _data.writeString(caller);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWiredDeviceConnectionState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWiredDeviceConnectionState(type, state, address, name, caller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void handleBluetoothA2dpDeviceConfigChange(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleBluetoothA2dpDeviceConfigChange, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleBluetoothA2dpDeviceConfigChange(device);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.media.AudioRoutesInfo startWatchingRoutes(android.media.IAudioRoutesObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.AudioRoutesInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchingRoutes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startWatchingRoutes(observer);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.AudioRoutesInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean isCameraSoundForced() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCameraSoundForced, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isCameraSoundForced();
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
      @Override public void setVolumeController(android.media.IVolumeController controller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((controller!=null))?(controller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolumeController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolumeController(controller);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyVolumeControllerVisible(android.media.IVolumeController controller, boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((controller!=null))?(controller.asBinder()):(null)));
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyVolumeControllerVisible, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyVolumeControllerVisible(controller, visible);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isStreamAffectedByRingerMode(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isStreamAffectedByRingerMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isStreamAffectedByRingerMode(streamType);
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
      @Override public boolean isStreamAffectedByMute(int streamType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(streamType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isStreamAffectedByMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isStreamAffectedByMute(streamType);
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
      @Override public void disableSafeMediaVolume(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableSafeMediaVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableSafeMediaVolume(callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int setHdmiSystemAudioSupported(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHdmiSystemAudioSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setHdmiSystemAudioSupported(on);
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
      @Override public boolean isHdmiSystemAudioSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isHdmiSystemAudioSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isHdmiSystemAudioSupported();
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
      @Override public java.lang.String registerAudioPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb, boolean hasFocusListener, boolean isFocusPolicy, boolean isTestFocusPolicy, boolean isVolumeController, android.media.projection.IMediaProjection projection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((policyConfig!=null)) {
            _data.writeInt(1);
            policyConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          _data.writeInt(((hasFocusListener)?(1):(0)));
          _data.writeInt(((isFocusPolicy)?(1):(0)));
          _data.writeInt(((isTestFocusPolicy)?(1):(0)));
          _data.writeInt(((isVolumeController)?(1):(0)));
          _data.writeStrongBinder((((projection!=null))?(projection.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAudioPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerAudioPolicy(policyConfig, pcb, hasFocusListener, isFocusPolicy, isTestFocusPolicy, isVolumeController, projection);
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
      @Override public void unregisterAudioPolicyAsync(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAudioPolicyAsync, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAudioPolicyAsync(pcb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterAudioPolicy(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAudioPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAudioPolicy(pcb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int addMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((policyConfig!=null)) {
            _data.writeInt(1);
            policyConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addMixForPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addMixForPolicy(policyConfig, pcb);
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
      @Override public int removeMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((policyConfig!=null)) {
            _data.writeInt(1);
            policyConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeMixForPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeMixForPolicy(policyConfig, pcb);
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
      @Override public int setFocusPropertiesForPolicy(int duckingBehavior, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(duckingBehavior);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFocusPropertiesForPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setFocusPropertiesForPolicy(duckingBehavior, pcb);
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
      @Override public void setVolumePolicy(android.media.VolumePolicy policy) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((policy!=null)) {
            _data.writeInt(1);
            policy.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVolumePolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVolumePolicy(policy);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean hasRegisteredDynamicPolicy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasRegisteredDynamicPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasRegisteredDynamicPolicy();
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
      @Override public void registerRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((rcdb!=null))?(rcdb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerRecordingCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerRecordingCallback(rcdb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((rcdb!=null))?(rcdb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterRecordingCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterRecordingCallback(rcdb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public java.util.List<android.media.AudioRecordingConfiguration> getActiveRecordingConfigurations() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.AudioRecordingConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveRecordingConfigurations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveRecordingConfigurations();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.AudioRecordingConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void registerPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcdb!=null))?(pcdb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPlaybackCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPlaybackCallback(pcdb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcdb!=null))?(pcdb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterPlaybackCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterPlaybackCallback(pcdb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public java.util.List<android.media.AudioPlaybackConfiguration> getActivePlaybackConfigurations() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.AudioPlaybackConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivePlaybackConfigurations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivePlaybackConfigurations();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.AudioPlaybackConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void disableRingtoneSync(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableRingtoneSync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableRingtoneSync(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getFocusRampTimeMs(int focusGain, android.media.AudioAttributes attr) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(focusGain);
          if ((attr!=null)) {
            _data.writeInt(1);
            attr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFocusRampTimeMs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFocusRampTimeMs(focusGain, attr);
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
      @Override public int dispatchFocusChange(android.media.AudioFocusInfo afi, int focusChange, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(focusChange);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchFocusChange, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().dispatchFocusChange(afi, focusChange, pcb);
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
      @Override public void playerHasOpPlayAudio(int piid, boolean hasOpPlayAudio) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(piid);
          _data.writeInt(((hasOpPlayAudio)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_playerHasOpPlayAudio, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playerHasOpPlayAudio(piid, hasOpPlayAudio);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setBluetoothHearingAidDeviceConnectionState(android.bluetooth.BluetoothDevice device, int state, boolean suppressNoisyIntent, int musicDevice) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(state);
          _data.writeInt(((suppressNoisyIntent)?(1):(0)));
          _data.writeInt(musicDevice);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBluetoothHearingAidDeviceConnectionState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBluetoothHearingAidDeviceConnectionState(device, state, suppressNoisyIntent, musicDevice);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent(android.bluetooth.BluetoothDevice device, int state, int profile, boolean suppressNoisyIntent, int a2dpVolume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(state);
          _data.writeInt(profile);
          _data.writeInt(((suppressNoisyIntent)?(1):(0)));
          _data.writeInt(a2dpVolume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent(device, state, profile, suppressNoisyIntent, a2dpVolume);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFocusRequestResultFromExtPolicy(android.media.AudioFocusInfo afi, int requestResult, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(requestResult);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFocusRequestResultFromExtPolicy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFocusRequestResultFromExtPolicy(afi, requestResult, pcb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void registerAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((asd!=null))?(asd.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAudioServerStateDispatcher, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerAudioServerStateDispatcher(asd);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((asd!=null))?(asd.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAudioServerStateDispatcher, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAudioServerStateDispatcher(asd);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public boolean isAudioServerRunning() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAudioServerRunning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAudioServerRunning();
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
      @Override public int setUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid, int[] deviceTypes, java.lang.String[] deviceAddresses) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          _data.writeInt(uid);
          _data.writeIntArray(deviceTypes);
          _data.writeStringArray(deviceAddresses);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUidDeviceAffinity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setUidDeviceAffinity(pcb, uid, deviceTypes, deviceAddresses);
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
      @Override public int removeUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((pcb!=null))?(pcb.asBinder()):(null)));
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeUidDeviceAffinity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeUidDeviceAffinity(pcb, uid);
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
      @Override public boolean hasHapticChannels(android.net.Uri uri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasHapticChannels, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasHapticChannels(uri);
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
      @Override public int setAllowedCapturePolicy(int capturePolicy) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(capturePolicy);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAllowedCapturePolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAllowedCapturePolicy(capturePolicy);
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
      @Override public int getAllowedCapturePolicy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllowedCapturePolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllowedCapturePolicy();
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
      public static android.media.IAudioService sDefaultImpl;
    }
    static final int TRANSACTION_trackPlayer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_playerAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_playerEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_releasePlayer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_trackRecorder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_recorderEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_releaseRecorder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_adjustSuggestedStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_adjustStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isStreamMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_forceRemoteSubmixFullVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_isMasterMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setMasterMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getStreamMinVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getStreamMaxVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getAudioVolumeGroups = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setVolumeIndexForAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getVolumeIndexForAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getMaxVolumeIndexForAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getMinVolumeIndexForAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getLastAudibleStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_getAudioProductStrategies = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_setMicrophoneMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setRingerModeExternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_setRingerModeInternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getRingerModeExternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getRingerModeInternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_isValidRingerMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_setVibrateSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getVibrateSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_shouldVibrate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_setMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_getMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_playSoundEffect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_playSoundEffectVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_loadSoundEffects = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_unloadSoundEffects = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_reloadAudioSettings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_avrcpSupportsAbsoluteVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_setSpeakerphoneOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_isSpeakerphoneOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_setBluetoothScoOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_isBluetoothScoOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_setBluetoothA2dpOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_isBluetoothA2dpOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_requestAudioFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_abandonAudioFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_unregisterAudioFocusClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_getCurrentAudioFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_startBluetoothSco = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_startBluetoothScoVirtualCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_stopBluetoothSco = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_forceVolumeControlStream = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_setRingtonePlayer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_getRingtonePlayer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_getUiSoundsStreamType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_setWiredDeviceConnectionState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_handleBluetoothA2dpDeviceConfigChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_startWatchingRoutes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_isCameraSoundForced = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_setVolumeController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_notifyVolumeControllerVisible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_isStreamAffectedByRingerMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_isStreamAffectedByMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_disableSafeMediaVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_setHdmiSystemAudioSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_isHdmiSystemAudioSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_registerAudioPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_unregisterAudioPolicyAsync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_unregisterAudioPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_addMixForPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_removeMixForPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_setFocusPropertiesForPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_setVolumePolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_hasRegisteredDynamicPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_registerRecordingCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_unregisterRecordingCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_getActiveRecordingConfigurations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_registerPlaybackCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_unregisterPlaybackCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_getActivePlaybackConfigurations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_disableRingtoneSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_getFocusRampTimeMs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_dispatchFocusChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_playerHasOpPlayAudio = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_setBluetoothHearingAidDeviceConnectionState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_setFocusRequestResultFromExtPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    static final int TRANSACTION_registerAudioServerStateDispatcher = (android.os.IBinder.FIRST_CALL_TRANSACTION + 90);
    static final int TRANSACTION_unregisterAudioServerStateDispatcher = (android.os.IBinder.FIRST_CALL_TRANSACTION + 91);
    static final int TRANSACTION_isAudioServerRunning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 92);
    static final int TRANSACTION_setUidDeviceAffinity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 93);
    static final int TRANSACTION_removeUidDeviceAffinity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 94);
    static final int TRANSACTION_hasHapticChannels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 95);
    static final int TRANSACTION_setAllowedCapturePolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 96);
    static final int TRANSACTION_getAllowedCapturePolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 97);
    public static boolean setDefaultImpl(android.media.IAudioService impl) {
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
    public static android.media.IAudioService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // C++ and Java methods below.
  // WARNING: When methods are inserted or deleted in this section, the transaction IDs in
  // frameworks/native/include/audiomanager/IAudioManager.h must be updated to match the order
  // in this file.
  //
  // When a method's argument list is changed, BpAudioManager's corresponding serialization code
  // (if any) in frameworks/native/services/audiomanager/IAudioManager.cpp must be updated.

  public int trackPlayer(android.media.PlayerBase.PlayerIdCard pic) throws android.os.RemoteException;
  public void playerAttributes(int piid, android.media.AudioAttributes attr) throws android.os.RemoteException;
  public void playerEvent(int piid, int event) throws android.os.RemoteException;
  public void releasePlayer(int piid) throws android.os.RemoteException;
  public int trackRecorder(android.os.IBinder recorder) throws android.os.RemoteException;
  public void recorderEvent(int riid, int event) throws android.os.RemoteException;
  public void releaseRecorder(int riid) throws android.os.RemoteException;
  // Java-only methods below.

  public void adjustSuggestedStreamVolume(int direction, int suggestedStreamType, int flags, java.lang.String callingPackage, java.lang.String caller) throws android.os.RemoteException;
  public void adjustStreamVolume(int streamType, int direction, int flags, java.lang.String callingPackage) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IAudioService.aidl:76:1:76:25")
  public void setStreamVolume(int streamType, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean isStreamMute(int streamType) throws android.os.RemoteException;
  public void forceRemoteSubmixFullVolume(boolean startForcing, android.os.IBinder cb) throws android.os.RemoteException;
  public boolean isMasterMute() throws android.os.RemoteException;
  public void setMasterMute(boolean mute, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IAudioService.aidl:87:1:87:25")
  public int getStreamVolume(int streamType) throws android.os.RemoteException;
  public int getStreamMinVolume(int streamType) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IAudioService.aidl:92:1:92:25")
  public int getStreamMaxVolume(int streamType) throws android.os.RemoteException;
  public java.util.List<android.media.audiopolicy.AudioVolumeGroup> getAudioVolumeGroups() throws android.os.RemoteException;
  public void setVolumeIndexForAttributes(android.media.AudioAttributes aa, int index, int flags, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException;
  public int getMaxVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException;
  public int getMinVolumeIndexForAttributes(android.media.AudioAttributes aa) throws android.os.RemoteException;
  public int getLastAudibleStreamVolume(int streamType) throws android.os.RemoteException;
  public java.util.List<android.media.audiopolicy.AudioProductStrategy> getAudioProductStrategies() throws android.os.RemoteException;
  public void setMicrophoneMute(boolean on, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public void setRingerModeExternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException;
  public void setRingerModeInternal(int ringerMode, java.lang.String caller) throws android.os.RemoteException;
  public int getRingerModeExternal() throws android.os.RemoteException;
  public int getRingerModeInternal() throws android.os.RemoteException;
  public boolean isValidRingerMode(int ringerMode) throws android.os.RemoteException;
  public void setVibrateSetting(int vibrateType, int vibrateSetting) throws android.os.RemoteException;
  public int getVibrateSetting(int vibrateType) throws android.os.RemoteException;
  public boolean shouldVibrate(int vibrateType) throws android.os.RemoteException;
  public void setMode(int mode, android.os.IBinder cb, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getMode() throws android.os.RemoteException;
  public void playSoundEffect(int effectType) throws android.os.RemoteException;
  public void playSoundEffectVolume(int effectType, float volume) throws android.os.RemoteException;
  public boolean loadSoundEffects() throws android.os.RemoteException;
  public void unloadSoundEffects() throws android.os.RemoteException;
  public void reloadAudioSettings() throws android.os.RemoteException;
  public void avrcpSupportsAbsoluteVolume(java.lang.String address, boolean support) throws android.os.RemoteException;
  public void setSpeakerphoneOn(boolean on) throws android.os.RemoteException;
  public boolean isSpeakerphoneOn() throws android.os.RemoteException;
  public void setBluetoothScoOn(boolean on) throws android.os.RemoteException;
  public boolean isBluetoothScoOn() throws android.os.RemoteException;
  public void setBluetoothA2dpOn(boolean on) throws android.os.RemoteException;
  public boolean isBluetoothA2dpOn() throws android.os.RemoteException;
  public int requestAudioFocus(android.media.AudioAttributes aa, int durationHint, android.os.IBinder cb, android.media.IAudioFocusDispatcher fd, java.lang.String clientId, java.lang.String callingPackageName, int flags, android.media.audiopolicy.IAudioPolicyCallback pcb, int sdk) throws android.os.RemoteException;
  public int abandonAudioFocus(android.media.IAudioFocusDispatcher fd, java.lang.String clientId, android.media.AudioAttributes aa, java.lang.String callingPackageName) throws android.os.RemoteException;
  public void unregisterAudioFocusClient(java.lang.String clientId) throws android.os.RemoteException;
  public int getCurrentAudioFocus() throws android.os.RemoteException;
  public void startBluetoothSco(android.os.IBinder cb, int targetSdkVersion) throws android.os.RemoteException;
  public void startBluetoothScoVirtualCall(android.os.IBinder cb) throws android.os.RemoteException;
  public void stopBluetoothSco(android.os.IBinder cb) throws android.os.RemoteException;
  public void forceVolumeControlStream(int streamType, android.os.IBinder cb) throws android.os.RemoteException;
  public void setRingtonePlayer(android.media.IRingtonePlayer player) throws android.os.RemoteException;
  public android.media.IRingtonePlayer getRingtonePlayer() throws android.os.RemoteException;
  public int getUiSoundsStreamType() throws android.os.RemoteException;
  public void setWiredDeviceConnectionState(int type, int state, java.lang.String address, java.lang.String name, java.lang.String caller) throws android.os.RemoteException;
  public void handleBluetoothA2dpDeviceConfigChange(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IAudioService.aidl:181:1:181:25")
  public android.media.AudioRoutesInfo startWatchingRoutes(android.media.IAudioRoutesObserver observer) throws android.os.RemoteException;
  public boolean isCameraSoundForced() throws android.os.RemoteException;
  public void setVolumeController(android.media.IVolumeController controller) throws android.os.RemoteException;
  public void notifyVolumeControllerVisible(android.media.IVolumeController controller, boolean visible) throws android.os.RemoteException;
  public boolean isStreamAffectedByRingerMode(int streamType) throws android.os.RemoteException;
  public boolean isStreamAffectedByMute(int streamType) throws android.os.RemoteException;
  public void disableSafeMediaVolume(java.lang.String callingPackage) throws android.os.RemoteException;
  public int setHdmiSystemAudioSupported(boolean on) throws android.os.RemoteException;
  public boolean isHdmiSystemAudioSupported() throws android.os.RemoteException;
  public java.lang.String registerAudioPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb, boolean hasFocusListener, boolean isFocusPolicy, boolean isTestFocusPolicy, boolean isVolumeController, android.media.projection.IMediaProjection projection) throws android.os.RemoteException;
  public void unregisterAudioPolicyAsync(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public void unregisterAudioPolicy(android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public int addMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public int removeMixForPolicy(android.media.audiopolicy.AudioPolicyConfig policyConfig, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public int setFocusPropertiesForPolicy(int duckingBehavior, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public void setVolumePolicy(android.media.VolumePolicy policy) throws android.os.RemoteException;
  public boolean hasRegisteredDynamicPolicy() throws android.os.RemoteException;
  public void registerRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException;
  public void unregisterRecordingCallback(android.media.IRecordingConfigDispatcher rcdb) throws android.os.RemoteException;
  public java.util.List<android.media.AudioRecordingConfiguration> getActiveRecordingConfigurations() throws android.os.RemoteException;
  public void registerPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException;
  public void unregisterPlaybackCallback(android.media.IPlaybackConfigDispatcher pcdb) throws android.os.RemoteException;
  public java.util.List<android.media.AudioPlaybackConfiguration> getActivePlaybackConfigurations() throws android.os.RemoteException;
  public void disableRingtoneSync(int userId) throws android.os.RemoteException;
  public int getFocusRampTimeMs(int focusGain, android.media.AudioAttributes attr) throws android.os.RemoteException;
  public int dispatchFocusChange(android.media.AudioFocusInfo afi, int focusChange, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public void playerHasOpPlayAudio(int piid, boolean hasOpPlayAudio) throws android.os.RemoteException;
  public void setBluetoothHearingAidDeviceConnectionState(android.bluetooth.BluetoothDevice device, int state, boolean suppressNoisyIntent, int musicDevice) throws android.os.RemoteException;
  public void setBluetoothA2dpDeviceConnectionStateSuppressNoisyIntent(android.bluetooth.BluetoothDevice device, int state, int profile, boolean suppressNoisyIntent, int a2dpVolume) throws android.os.RemoteException;
  public void setFocusRequestResultFromExtPolicy(android.media.AudioFocusInfo afi, int requestResult, android.media.audiopolicy.IAudioPolicyCallback pcb) throws android.os.RemoteException;
  public void registerAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException;
  public void unregisterAudioServerStateDispatcher(android.media.IAudioServerStateDispatcher asd) throws android.os.RemoteException;
  public boolean isAudioServerRunning() throws android.os.RemoteException;
  public int setUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid, int[] deviceTypes, java.lang.String[] deviceAddresses) throws android.os.RemoteException;
  public int removeUidDeviceAffinity(android.media.audiopolicy.IAudioPolicyCallback pcb, int uid) throws android.os.RemoteException;
  public boolean hasHapticChannels(android.net.Uri uri) throws android.os.RemoteException;
  public int setAllowedCapturePolicy(int capturePolicy) throws android.os.RemoteException;
  public int getAllowedCapturePolicy() throws android.os.RemoteException;
}
