/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * The communication channel by which an external system that wants to control the system
 * vibrator can notify the vibrator subsystem.
 *
 * Some vibrators can be driven via multiple paths (e.g. as an audio channel) in addition to
 * the usual interface, but we typically only want one vibration at a time playing because they
 * don't mix well. In order to synchronize the two places where vibration might be controlled,
 * we provide this interface so the vibrator subsystem has a chance to:
 *
 * 1) Decide whether the current vibration should play based on the current system policy.
 * 2) Stop any currently on-going vibrations.
 * {@hide}
 */
public interface IExternalVibratorService extends android.os.IInterface
{
  /** Default implementation for IExternalVibratorService. */
  public static class Default implements android.os.IExternalVibratorService
  {
    /**
         * A method called by the external system to start a vibration.
         *
         * If this returns {@code SCALE_MUTE}, then the vibration should <em>not</em> play. If this
         * returns any other scale level, then any currently playing vibration controlled by the
         * requesting system must be muted and this vibration can begin playback.
         *
         * Note that the IExternalVibratorService implementation will not call mute on any currently
         * playing external vibrations in order to avoid re-entrancy with the system on the other side.
         *
         * @param vibration An ExternalVibration
         *
         * @return {@code SCALE_MUTE} if the external vibration should not play, and any other scale
         *         level if it should.
         */
    @Override public int onExternalVibrationStart(android.os.ExternalVibration vib) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * A method called by the external system when a vibration no longer wants to play.
         */
    @Override public void onExternalVibrationStop(android.os.ExternalVibration vib) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IExternalVibratorService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IExternalVibratorService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IExternalVibratorService interface,
     * generating a proxy if needed.
     */
    public static android.os.IExternalVibratorService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IExternalVibratorService))) {
        return ((android.os.IExternalVibratorService)iin);
      }
      return new android.os.IExternalVibratorService.Stub.Proxy(obj);
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
        case TRANSACTION_onExternalVibrationStart:
        {
          data.enforceInterface(descriptor);
          android.os.ExternalVibration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ExternalVibration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.onExternalVibrationStart(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_onExternalVibrationStop:
        {
          data.enforceInterface(descriptor);
          android.os.ExternalVibration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ExternalVibration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onExternalVibrationStop(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IExternalVibratorService
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
           * A method called by the external system to start a vibration.
           *
           * If this returns {@code SCALE_MUTE}, then the vibration should <em>not</em> play. If this
           * returns any other scale level, then any currently playing vibration controlled by the
           * requesting system must be muted and this vibration can begin playback.
           *
           * Note that the IExternalVibratorService implementation will not call mute on any currently
           * playing external vibrations in order to avoid re-entrancy with the system on the other side.
           *
           * @param vibration An ExternalVibration
           *
           * @return {@code SCALE_MUTE} if the external vibration should not play, and any other scale
           *         level if it should.
           */
      @Override public int onExternalVibrationStart(android.os.ExternalVibration vib) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((vib!=null)) {
            _data.writeInt(1);
            vib.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onExternalVibrationStart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().onExternalVibrationStart(vib);
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
      /**
           * A method called by the external system when a vibration no longer wants to play.
           */
      @Override public void onExternalVibrationStop(android.os.ExternalVibration vib) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((vib!=null)) {
            _data.writeInt(1);
            vib.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onExternalVibrationStop, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onExternalVibrationStop(vib);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IExternalVibratorService sDefaultImpl;
    }
    static final int TRANSACTION_onExternalVibrationStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onExternalVibrationStop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.IExternalVibratorService impl) {
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
    public static android.os.IExternalVibratorService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public static final int SCALE_MUTE = -100;
  public static final int SCALE_VERY_LOW = -2;
  public static final int SCALE_LOW = -1;
  public static final int SCALE_NONE = 0;
  public static final int SCALE_HIGH = 1;
  public static final int SCALE_VERY_HIGH = 2;
  /**
       * A method called by the external system to start a vibration.
       *
       * If this returns {@code SCALE_MUTE}, then the vibration should <em>not</em> play. If this
       * returns any other scale level, then any currently playing vibration controlled by the
       * requesting system must be muted and this vibration can begin playback.
       *
       * Note that the IExternalVibratorService implementation will not call mute on any currently
       * playing external vibrations in order to avoid re-entrancy with the system on the other side.
       *
       * @param vibration An ExternalVibration
       *
       * @return {@code SCALE_MUTE} if the external vibration should not play, and any other scale
       *         level if it should.
       */
  public int onExternalVibrationStart(android.os.ExternalVibration vib) throws android.os.RemoteException;
  /**
       * A method called by the external system when a vibration no longer wants to play.
       */
  public void onExternalVibrationStop(android.os.ExternalVibration vib) throws android.os.RemoteException;
}
