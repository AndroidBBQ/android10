/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.diagnostic;
/** @hide */
public interface ICarDiagnostic extends android.os.IInterface
{
  /** Default implementation for ICarDiagnostic. */
  public static class Default implements android.car.diagnostic.ICarDiagnostic
  {
    /**
         * Register a callback (or update registration) for diagnostic events.
         */
    @Override public boolean registerOrUpdateDiagnosticListener(int frameType, int rate, android.car.diagnostic.ICarDiagnosticEventListener listener) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the value for the most recent live frame data available.
         */
    @Override public android.car.diagnostic.CarDiagnosticEvent getLatestLiveFrame() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the list of timestamps for which there exist a freeze frame stored.
         */
    @Override public long[] getFreezeFrameTimestamps() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the value for the freeze frame stored given a timestamp.
         */
    @Override public android.car.diagnostic.CarDiagnosticEvent getFreezeFrame(long timestamp) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Erase freeze frames given timestamps (or all, if no timestamps).
         */
    @Override public boolean clearFreezeFrames(long[] timestamps) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Stop receiving diagnostic events for a given callback.
         */
    @Override public void unregisterDiagnosticListener(int frameType, android.car.diagnostic.ICarDiagnosticEventListener callback) throws android.os.RemoteException
    {
    }
    /**
         * Returns whether the underlying HAL supports live frames.
         */
    @Override public boolean isLiveFrameSupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns whether the underlying HAL supports sending notifications to
         * registered listeners when new freeze frames happen.
         */
    @Override public boolean isFreezeFrameNotificationSupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns whether the underlying HAL supports retrieving freeze frames
         * stored in vehicle memory using timestamp.
         */
    @Override public boolean isGetFreezeFrameSupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns whether the underlying HAL supports clearing freeze frames.
         */
    @Override public boolean isClearFreezeFramesSupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns whether the underlying HAL supports clearing specific freeze frames specified
         * by means of their timestamps.
         */
    @Override public boolean isSelectiveClearFreezeFramesSupported() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.diagnostic.ICarDiagnostic
  {
    private static final java.lang.String DESCRIPTOR = "android.car.diagnostic.ICarDiagnostic";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.diagnostic.ICarDiagnostic interface,
     * generating a proxy if needed.
     */
    public static android.car.diagnostic.ICarDiagnostic asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.diagnostic.ICarDiagnostic))) {
        return ((android.car.diagnostic.ICarDiagnostic)iin);
      }
      return new android.car.diagnostic.ICarDiagnostic.Stub.Proxy(obj);
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
        case TRANSACTION_registerOrUpdateDiagnosticListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.car.diagnostic.ICarDiagnosticEventListener _arg2;
          _arg2 = android.car.diagnostic.ICarDiagnosticEventListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerOrUpdateDiagnosticListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLatestLiveFrame:
        {
          data.enforceInterface(descriptor);
          android.car.diagnostic.CarDiagnosticEvent _result = this.getLatestLiveFrame();
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
        case TRANSACTION_getFreezeFrameTimestamps:
        {
          data.enforceInterface(descriptor);
          long[] _result = this.getFreezeFrameTimestamps();
          reply.writeNoException();
          reply.writeLongArray(_result);
          return true;
        }
        case TRANSACTION_getFreezeFrame:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.car.diagnostic.CarDiagnosticEvent _result = this.getFreezeFrame(_arg0);
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
        case TRANSACTION_clearFreezeFrames:
        {
          data.enforceInterface(descriptor);
          long[] _arg0;
          _arg0 = data.createLongArray();
          boolean _result = this.clearFreezeFrames(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterDiagnosticListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.car.diagnostic.ICarDiagnosticEventListener _arg1;
          _arg1 = android.car.diagnostic.ICarDiagnosticEventListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterDiagnosticListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isLiveFrameSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isLiveFrameSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isFreezeFrameNotificationSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isFreezeFrameNotificationSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isGetFreezeFrameSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isGetFreezeFrameSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isClearFreezeFramesSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isClearFreezeFramesSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSelectiveClearFreezeFramesSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSelectiveClearFreezeFramesSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.diagnostic.ICarDiagnostic
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
           * Register a callback (or update registration) for diagnostic events.
           */
      @Override public boolean registerOrUpdateDiagnosticListener(int frameType, int rate, android.car.diagnostic.ICarDiagnosticEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(frameType);
          _data.writeInt(rate);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerOrUpdateDiagnosticListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerOrUpdateDiagnosticListener(frameType, rate, listener);
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
           * Get the value for the most recent live frame data available.
           */
      @Override public android.car.diagnostic.CarDiagnosticEvent getLatestLiveFrame() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.diagnostic.CarDiagnosticEvent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLatestLiveFrame, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLatestLiveFrame();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.diagnostic.CarDiagnosticEvent.CREATOR.createFromParcel(_reply);
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
      /**
           * Get the list of timestamps for which there exist a freeze frame stored.
           */
      @Override public long[] getFreezeFrameTimestamps() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFreezeFrameTimestamps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFreezeFrameTimestamps();
          }
          _reply.readException();
          _result = _reply.createLongArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the value for the freeze frame stored given a timestamp.
           */
      @Override public android.car.diagnostic.CarDiagnosticEvent getFreezeFrame(long timestamp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.diagnostic.CarDiagnosticEvent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFreezeFrame, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFreezeFrame(timestamp);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.diagnostic.CarDiagnosticEvent.CREATOR.createFromParcel(_reply);
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
      /**
           * Erase freeze frames given timestamps (or all, if no timestamps).
           */
      @Override public boolean clearFreezeFrames(long[] timestamps) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLongArray(timestamps);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearFreezeFrames, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearFreezeFrames(timestamps);
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
           * Stop receiving diagnostic events for a given callback.
           */
      @Override public void unregisterDiagnosticListener(int frameType, android.car.diagnostic.ICarDiagnosticEventListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(frameType);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDiagnosticListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDiagnosticListener(frameType, callback);
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
           * Returns whether the underlying HAL supports live frames.
           */
      @Override public boolean isLiveFrameSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isLiveFrameSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isLiveFrameSupported();
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
           * Returns whether the underlying HAL supports sending notifications to
           * registered listeners when new freeze frames happen.
           */
      @Override public boolean isFreezeFrameNotificationSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFreezeFrameNotificationSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isFreezeFrameNotificationSupported();
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
           * Returns whether the underlying HAL supports retrieving freeze frames
           * stored in vehicle memory using timestamp.
           */
      @Override public boolean isGetFreezeFrameSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isGetFreezeFrameSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isGetFreezeFrameSupported();
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
           * Returns whether the underlying HAL supports clearing freeze frames.
           */
      @Override public boolean isClearFreezeFramesSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isClearFreezeFramesSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isClearFreezeFramesSupported();
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
           * Returns whether the underlying HAL supports clearing specific freeze frames specified
           * by means of their timestamps.
           */
      @Override public boolean isSelectiveClearFreezeFramesSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSelectiveClearFreezeFramesSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSelectiveClearFreezeFramesSupported();
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
      public static android.car.diagnostic.ICarDiagnostic sDefaultImpl;
    }
    static final int TRANSACTION_registerOrUpdateDiagnosticListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getLatestLiveFrame = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getFreezeFrameTimestamps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getFreezeFrame = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_clearFreezeFrames = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_unregisterDiagnosticListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isLiveFrameSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_isFreezeFrameNotificationSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isGetFreezeFrameSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isClearFreezeFramesSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_isSelectiveClearFreezeFramesSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(android.car.diagnostic.ICarDiagnostic impl) {
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
    public static android.car.diagnostic.ICarDiagnostic getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Register a callback (or update registration) for diagnostic events.
       */
  public boolean registerOrUpdateDiagnosticListener(int frameType, int rate, android.car.diagnostic.ICarDiagnosticEventListener listener) throws android.os.RemoteException;
  /**
       * Get the value for the most recent live frame data available.
       */
  public android.car.diagnostic.CarDiagnosticEvent getLatestLiveFrame() throws android.os.RemoteException;
  /**
       * Get the list of timestamps for which there exist a freeze frame stored.
       */
  public long[] getFreezeFrameTimestamps() throws android.os.RemoteException;
  /**
       * Get the value for the freeze frame stored given a timestamp.
       */
  public android.car.diagnostic.CarDiagnosticEvent getFreezeFrame(long timestamp) throws android.os.RemoteException;
  /**
       * Erase freeze frames given timestamps (or all, if no timestamps).
       */
  public boolean clearFreezeFrames(long[] timestamps) throws android.os.RemoteException;
  /**
       * Stop receiving diagnostic events for a given callback.
       */
  public void unregisterDiagnosticListener(int frameType, android.car.diagnostic.ICarDiagnosticEventListener callback) throws android.os.RemoteException;
  /**
       * Returns whether the underlying HAL supports live frames.
       */
  public boolean isLiveFrameSupported() throws android.os.RemoteException;
  /**
       * Returns whether the underlying HAL supports sending notifications to
       * registered listeners when new freeze frames happen.
       */
  public boolean isFreezeFrameNotificationSupported() throws android.os.RemoteException;
  /**
       * Returns whether the underlying HAL supports retrieving freeze frames
       * stored in vehicle memory using timestamp.
       */
  public boolean isGetFreezeFrameSupported() throws android.os.RemoteException;
  /**
       * Returns whether the underlying HAL supports clearing freeze frames.
       */
  public boolean isClearFreezeFramesSupported() throws android.os.RemoteException;
  /**
       * Returns whether the underlying HAL supports clearing specific freeze frames specified
       * by means of their timestamps.
       */
  public boolean isSelectiveClearFreezeFramesSupported() throws android.os.RemoteException;
}
