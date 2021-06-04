/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.radio.service;
/**
 * An interface to the backend Radio app's service.
 */
public interface IRadioAppService extends android.os.IInterface
{
  /** Default implementation for IRadioAppService. */
  public static class Default implements com.android.car.radio.service.IRadioAppService
  {
    /**
         * Adds {@link RadioAppService} callback.
         *
         * Triggers state updates on newly added callback.
         */
    @Override public void addCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Removes {@link RadioAppService} callback.
         */
    @Override public void removeCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Tunes to a given program.
         */
    @Override public void tune(android.hardware.radio.ProgramSelector sel, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Seeks forward or backwards.
         */
    @Override public void seek(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Steps forward or backwards.
         */
    @Override public void step(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Skips forward or backwards; the meaning of "skip" is defined by setSkipMode().
         */
    @Override public void skip(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
    {
    }
    /**
          * Sets the behavior of skip()
          *
          * @param mode must be a valid SkipMode enum value.
          */
    @Override public void setSkipMode(int mode) throws android.os.RemoteException
    {
    }
    /**
         * Mutes or resumes audio.
         *
         * @param muted {@code true} to mute, {@code false} to resume audio.
         */
    @Override public void setMuted(boolean muted) throws android.os.RemoteException
    {
    }
    /**
         * Tune to a default channel of a given program type (band).
         *
         * Usually, this means tuning to the recently listened program of a given band.
         *
         * @param band Program type to switch to
         */
    @Override public void switchBand(com.android.car.radio.bands.ProgramType band) throws android.os.RemoteException
    {
    }
    /**
         * States whether program list is supported on current device or not.
         *
         * @return {@code true} if the program list is supported, {@code false} otherwise.
         */
    @Override public boolean isProgramListSupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns current region config (like frequency ranges for AM/FM).
         */
    @Override public com.android.car.radio.bands.RegionConfig getRegionConfig() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.radio.service.IRadioAppService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.radio.service.IRadioAppService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.radio.service.IRadioAppService interface,
     * generating a proxy if needed.
     */
    public static com.android.car.radio.service.IRadioAppService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.radio.service.IRadioAppService))) {
        return ((com.android.car.radio.service.IRadioAppService)iin);
      }
      return new com.android.car.radio.service.IRadioAppService.Stub.Proxy(obj);
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
        case TRANSACTION_addCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.radio.service.IRadioAppCallback _arg0;
          _arg0 = com.android.car.radio.service.IRadioAppCallback.Stub.asInterface(data.readStrongBinder());
          this.addCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.radio.service.IRadioAppCallback _arg0;
          _arg0 = com.android.car.radio.service.IRadioAppCallback.Stub.asInterface(data.readStrongBinder());
          this.removeCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_tune:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.ProgramSelector _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.ProgramSelector.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          com.android.car.radio.service.ITuneCallback _arg1;
          _arg1 = com.android.car.radio.service.ITuneCallback.Stub.asInterface(data.readStrongBinder());
          this.tune(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_seek:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          com.android.car.radio.service.ITuneCallback _arg1;
          _arg1 = com.android.car.radio.service.ITuneCallback.Stub.asInterface(data.readStrongBinder());
          this.seek(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_step:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          com.android.car.radio.service.ITuneCallback _arg1;
          _arg1 = com.android.car.radio.service.ITuneCallback.Stub.asInterface(data.readStrongBinder());
          this.step(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_skip:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          com.android.car.radio.service.ITuneCallback _arg1;
          _arg1 = com.android.car.radio.service.ITuneCallback.Stub.asInterface(data.readStrongBinder());
          this.skip(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSkipMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setSkipMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMuted:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setMuted(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_switchBand:
        {
          data.enforceInterface(descriptor);
          com.android.car.radio.bands.ProgramType _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.radio.bands.ProgramType.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.switchBand(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isProgramListSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isProgramListSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getRegionConfig:
        {
          data.enforceInterface(descriptor);
          com.android.car.radio.bands.RegionConfig _result = this.getRegionConfig();
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.radio.service.IRadioAppService
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
           * Adds {@link RadioAppService} callback.
           *
           * Triggers state updates on newly added callback.
           */
      @Override public void addCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addCallback(callback);
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
           * Removes {@link RadioAppService} callback.
           */
      @Override public void removeCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeCallback(callback);
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
           * Tunes to a given program.
           */
      @Override public void tune(android.hardware.radio.ProgramSelector sel, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sel!=null)) {
            _data.writeInt(1);
            sel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_tune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tune(sel, callback);
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
           * Seeks forward or backwards.
           */
      @Override public void seek(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((forward)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_seek, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().seek(forward, callback);
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
           * Steps forward or backwards.
           */
      @Override public void step(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((forward)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_step, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().step(forward, callback);
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
           * Skips forward or backwards; the meaning of "skip" is defined by setSkipMode().
           */
      @Override public void skip(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((forward)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_skip, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().skip(forward, callback);
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
            * Sets the behavior of skip()
            *
            * @param mode must be a valid SkipMode enum value.
            */
      @Override public void setSkipMode(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSkipMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSkipMode(mode);
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
           * Mutes or resumes audio.
           *
           * @param muted {@code true} to mute, {@code false} to resume audio.
           */
      @Override public void setMuted(boolean muted) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((muted)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMuted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMuted(muted);
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
           * Tune to a default channel of a given program type (band).
           *
           * Usually, this means tuning to the recently listened program of a given band.
           *
           * @param band Program type to switch to
           */
      @Override public void switchBand(com.android.car.radio.bands.ProgramType band) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((band!=null)) {
            _data.writeInt(1);
            band.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchBand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().switchBand(band);
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
           * States whether program list is supported on current device or not.
           *
           * @return {@code true} if the program list is supported, {@code false} otherwise.
           */
      @Override public boolean isProgramListSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isProgramListSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isProgramListSupported();
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
           * Returns current region config (like frequency ranges for AM/FM).
           */
      @Override public com.android.car.radio.bands.RegionConfig getRegionConfig() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.car.radio.bands.RegionConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRegionConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRegionConfig();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.car.radio.bands.RegionConfig.CREATOR.createFromParcel(_reply);
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
      public static com.android.car.radio.service.IRadioAppService sDefaultImpl;
    }
    static final int TRANSACTION_addCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_tune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_seek = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_step = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_skip = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setSkipMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setMuted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_switchBand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isProgramListSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getRegionConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(com.android.car.radio.service.IRadioAppService impl) {
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
    public static com.android.car.radio.service.IRadioAppService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Adds {@link RadioAppService} callback.
       *
       * Triggers state updates on newly added callback.
       */
  public void addCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException;
  /**
       * Removes {@link RadioAppService} callback.
       */
  public void removeCallback(com.android.car.radio.service.IRadioAppCallback callback) throws android.os.RemoteException;
  /**
       * Tunes to a given program.
       */
  public void tune(android.hardware.radio.ProgramSelector sel, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException;
  /**
       * Seeks forward or backwards.
       */
  public void seek(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException;
  /**
       * Steps forward or backwards.
       */
  public void step(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException;
  /**
       * Skips forward or backwards; the meaning of "skip" is defined by setSkipMode().
       */
  public void skip(boolean forward, com.android.car.radio.service.ITuneCallback callback) throws android.os.RemoteException;
  /**
        * Sets the behavior of skip()
        *
        * @param mode must be a valid SkipMode enum value.
        */
  public void setSkipMode(int mode) throws android.os.RemoteException;
  /**
       * Mutes or resumes audio.
       *
       * @param muted {@code true} to mute, {@code false} to resume audio.
       */
  public void setMuted(boolean muted) throws android.os.RemoteException;
  /**
       * Tune to a default channel of a given program type (band).
       *
       * Usually, this means tuning to the recently listened program of a given band.
       *
       * @param band Program type to switch to
       */
  public void switchBand(com.android.car.radio.bands.ProgramType band) throws android.os.RemoteException;
  /**
       * States whether program list is supported on current device or not.
       *
       * @return {@code true} if the program list is supported, {@code false} otherwise.
       */
  public boolean isProgramListSupported() throws android.os.RemoteException;
  /**
       * Returns current region config (like frequency ranges for AM/FM).
       */
  public com.android.car.radio.bands.RegionConfig getRegionConfig() throws android.os.RemoteException;
}
