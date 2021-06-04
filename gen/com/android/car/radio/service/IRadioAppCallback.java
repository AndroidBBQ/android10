/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.radio.service;
/**
 * Watches current program changes.
 */
public interface IRadioAppCallback extends android.os.IInterface
{
  /** Default implementation for IRadioAppCallback. */
  public static class Default implements com.android.car.radio.service.IRadioAppCallback
  {
    /**
         * Called when hardware error has occured.
         *
         * Client must unbind from the {@link RadioAppService} after getting this callback.
         */
    @Override public void onHardwareError() throws android.os.RemoteException
    {
    }
    /**
         * Called when current program details changes.
         *
         * This might happen as a result of tuning to a different program or just metadata change.
         *
         * @param info Current program info
         */
    @Override public void onCurrentProgramChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException
    {
    }
    /**
         * Called when playback state (play/pause) changes.
         *
         * @param state New playback state
         */
    @Override public void onPlaybackStateChanged(int state) throws android.os.RemoteException
    {
    }
    /**
         * Called when program list changes.
         *
         * @param New program list
         */
    @Override public void onProgramListChanged(java.util.List<android.hardware.radio.RadioManager.ProgramInfo> plist) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.radio.service.IRadioAppCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.radio.service.IRadioAppCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.radio.service.IRadioAppCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.radio.service.IRadioAppCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.radio.service.IRadioAppCallback))) {
        return ((com.android.car.radio.service.IRadioAppCallback)iin);
      }
      return new com.android.car.radio.service.IRadioAppCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onHardwareError:
        {
          data.enforceInterface(descriptor);
          this.onHardwareError();
          return true;
        }
        case TRANSACTION_onCurrentProgramChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.RadioManager.ProgramInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.RadioManager.ProgramInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCurrentProgramChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPlaybackStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onPlaybackStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onProgramListChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.radio.RadioManager.ProgramInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.hardware.radio.RadioManager.ProgramInfo.CREATOR);
          this.onProgramListChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.radio.service.IRadioAppCallback
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
           * Called when hardware error has occured.
           *
           * Client must unbind from the {@link RadioAppService} after getting this callback.
           */
      @Override public void onHardwareError() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHardwareError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHardwareError();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when current program details changes.
           *
           * This might happen as a result of tuning to a different program or just metadata change.
           *
           * @param info Current program info
           */
      @Override public void onCurrentProgramChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCurrentProgramChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCurrentProgramChanged(info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when playback state (play/pause) changes.
           *
           * @param state New playback state
           */
      @Override public void onPlaybackStateChanged(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
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
      /**
           * Called when program list changes.
           *
           * @param New program list
           */
      @Override public void onProgramListChanged(java.util.List<android.hardware.radio.RadioManager.ProgramInfo> plist) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(plist);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgramListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgramListChanged(plist);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.radio.service.IRadioAppCallback sDefaultImpl;
    }
    static final int TRANSACTION_onHardwareError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onCurrentProgramChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onPlaybackStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onProgramListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.car.radio.service.IRadioAppCallback impl) {
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
    public static com.android.car.radio.service.IRadioAppCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when hardware error has occured.
       *
       * Client must unbind from the {@link RadioAppService} after getting this callback.
       */
  public void onHardwareError() throws android.os.RemoteException;
  /**
       * Called when current program details changes.
       *
       * This might happen as a result of tuning to a different program or just metadata change.
       *
       * @param info Current program info
       */
  public void onCurrentProgramChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException;
  /**
       * Called when playback state (play/pause) changes.
       *
       * @param state New playback state
       */
  public void onPlaybackStateChanged(int state) throws android.os.RemoteException;
  /**
       * Called when program list changes.
       *
       * @param New program list
       */
  public void onProgramListChanged(java.util.List<android.hardware.radio.RadioManager.ProgramInfo> plist) throws android.os.RemoteException;
}
