/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * TvInputService representing a physical port should connect to HAL through this interface.
 * Framework will take care of communication among system services including TvInputManagerService,
 * HdmiControlService, AudioService, etc.
 *
 * @hide
 */
public interface ITvInputHardware extends android.os.IInterface
{
  /** Default implementation for ITvInputHardware. */
  public static class Default implements android.media.tv.ITvInputHardware
  {
    /**
         * Make the input render on the surface according to the config. In case of HDMI, this will
         * trigger CEC commands for adjusting active HDMI source. Returns true on success.
         */
    @Override public boolean setSurface(android.view.Surface surface, android.media.tv.TvStreamConfig config) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set volume for this stream via AudioGain.
         */
    @Override public void setStreamVolume(float volume) throws android.os.RemoteException
    {
    }
    /**
         * Override default audio sink from audio policy. When override is on, it is
         * TvInputService's responsibility to adjust to audio configuration change
         * (for example, when the audio sink becomes unavailable or more desirable
         * audio sink is detected).
         *
         * @param audioType one of AudioManager.DEVICE_* values. When it's * DEVICE_NONE, override
         *        becomes off.
         * @param audioAddress audio address of the overriding device.
         * @param samplingRate desired sampling rate. Use default when it's 0.
         * @param channelMask desired channel mask. Use default when it's
         *        AudioFormat.CHANNEL_OUT_DEFAULT.
         * @param format desired format. Use default when it's AudioFormat.ENCODING_DEFAULT.
         */
    @Override public void overrideAudioSink(int audioType, java.lang.String audioAddress, int samplingRate, int channelMask, int format) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputHardware
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputHardware";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputHardware interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputHardware asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputHardware))) {
        return ((android.media.tv.ITvInputHardware)iin);
      }
      return new android.media.tv.ITvInputHardware.Stub.Proxy(obj);
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
        case TRANSACTION_setSurface:
        {
          data.enforceInterface(descriptor);
          android.view.Surface _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.tv.TvStreamConfig _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.tv.TvStreamConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.setSurface(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setStreamVolume:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setStreamVolume(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_overrideAudioSink:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.overrideAudioSink(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputHardware
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
      /**
           * Make the input render on the surface according to the config. In case of HDMI, this will
           * trigger CEC commands for adjusting active HDMI source. Returns true on success.
           */
      @Override public boolean setSurface(android.view.Surface surface, android.media.tv.TvStreamConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSurface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setSurface(surface, config);
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
      /**
           * Set volume for this stream via AudioGain.
           */
      @Override public void setStreamVolume(float volume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(volume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStreamVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStreamVolume(volume);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Override default audio sink from audio policy. When override is on, it is
           * TvInputService's responsibility to adjust to audio configuration change
           * (for example, when the audio sink becomes unavailable or more desirable
           * audio sink is detected).
           *
           * @param audioType one of AudioManager.DEVICE_* values. When it's * DEVICE_NONE, override
           *        becomes off.
           * @param audioAddress audio address of the overriding device.
           * @param samplingRate desired sampling rate. Use default when it's 0.
           * @param channelMask desired channel mask. Use default when it's
           *        AudioFormat.CHANNEL_OUT_DEFAULT.
           * @param format desired format. Use default when it's AudioFormat.ENCODING_DEFAULT.
           */
      @Override public void overrideAudioSink(int audioType, java.lang.String audioAddress, int samplingRate, int channelMask, int format) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(audioType);
          _data.writeString(audioAddress);
          _data.writeInt(samplingRate);
          _data.writeInt(channelMask);
          _data.writeInt(format);
          boolean _status = mRemote.transact(Stub.TRANSACTION_overrideAudioSink, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().overrideAudioSink(audioType, audioAddress, samplingRate, channelMask, format);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputHardware sDefaultImpl;
    }
    static final int TRANSACTION_setSurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setStreamVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_overrideAudioSink = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.media.tv.ITvInputHardware impl) {
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
    public static android.media.tv.ITvInputHardware getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Make the input render on the surface according to the config. In case of HDMI, this will
       * trigger CEC commands for adjusting active HDMI source. Returns true on success.
       */
  public boolean setSurface(android.view.Surface surface, android.media.tv.TvStreamConfig config) throws android.os.RemoteException;
  /**
       * Set volume for this stream via AudioGain.
       */
  public void setStreamVolume(float volume) throws android.os.RemoteException;
  /**
       * Override default audio sink from audio policy. When override is on, it is
       * TvInputService's responsibility to adjust to audio configuration change
       * (for example, when the audio sink becomes unavailable or more desirable
       * audio sink is detected).
       *
       * @param audioType one of AudioManager.DEVICE_* values. When it's * DEVICE_NONE, override
       *        becomes off.
       * @param audioAddress audio address of the overriding device.
       * @param samplingRate desired sampling rate. Use default when it's 0.
       * @param channelMask desired channel mask. Use default when it's
       *        AudioFormat.CHANNEL_OUT_DEFAULT.
       * @param format desired format. Use default when it's AudioFormat.ENCODING_DEFAULT.
       */
  public void overrideAudioSink(int audioType, java.lang.String audioAddress, int samplingRate, int channelMask, int format) throws android.os.RemoteException;
}
