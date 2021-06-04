/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/**
 * Binder interface for {@link android.car.CarProjectionManager}.
 * Check {@link android.car.CarProjectionManager} APIs for expected behavior of each calls.
 *
 * @hide
 */
public interface ICarProjection extends android.os.IInterface
{
  /** Default implementation for ICarProjection. */
  public static class Default implements android.car.ICarProjection
  {
    /**
         * Registers projection runner on projection start with projection service
         * to create reverse binding.
         */
    @Override public void registerProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException
    {
    }
    /**
         * Unregisters projection runner on projection stop with projection service to create
         * reverse binding.
         */
    @Override public void unregisterProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException
    {
    }
    /**
         * Registers projection key event handler.
         * Re-registering same event handler with different events will cause only events to update.
         */
    @Override public void registerKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler, byte[] eventMask) throws android.os.RemoteException
    {
    }
    /**
         * Unregisters projection key event handler.
         */
    @Override public void unregisterKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler) throws android.os.RemoteException
    {
    }
    /**
         * Starts Wi-Fi access point if it hasn't been started yet for wireless projection and returns
         * WiFiConfiguration object with access point details.
         */
    @Override public void startProjectionAccessPoint(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    /**
         * Stops previously requested Wi-Fi access point.
         */
    @Override public void stopProjectionAccessPoint(android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    /** Disconnect a Bluetooth profile, and prevent it from reconnecting. */
    @Override public boolean requestBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    /** Undo the effects of requestBluetoothProfileInhibit. */
    @Override public boolean releaseBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    /** Reports projection status for a given projection receiver app. */
    @Override public void updateProjectionStatus(android.car.projection.ProjectionStatus status, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    /** Registers projection status listener */
    @Override public void registerProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException
    {
    }
    /** Unregister projection status listener */
    @Override public void unregisterProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Returns options for projection receiver app that can be un-packed using
         * {@link android.car.projection.ProjectionOptions} class.
         */
    @Override public android.os.Bundle getProjectionOptions() throws android.os.RemoteException
    {
      return null;
    }
    /** Returns a list of available Wi-Fi channels */
    @Override public int[] getAvailableWifiChannels(int band) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICarProjection
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICarProjection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICarProjection interface,
     * generating a proxy if needed.
     */
    public static android.car.ICarProjection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICarProjection))) {
        return ((android.car.ICarProjection)iin);
      }
      return new android.car.ICarProjection.Stub.Proxy(obj);
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
        case TRANSACTION_registerProjectionRunner:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.registerProjectionRunner(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterProjectionRunner:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.unregisterProjectionRunner(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerKeyEventHandler:
        {
          data.enforceInterface(descriptor);
          android.car.ICarProjectionKeyEventHandler _arg0;
          _arg0 = android.car.ICarProjectionKeyEventHandler.Stub.asInterface(data.readStrongBinder());
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.registerKeyEventHandler(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterKeyEventHandler:
        {
          data.enforceInterface(descriptor);
          android.car.ICarProjectionKeyEventHandler _arg0;
          _arg0 = android.car.ICarProjectionKeyEventHandler.Stub.asInterface(data.readStrongBinder());
          this.unregisterKeyEventHandler(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startProjectionAccessPoint:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.startProjectionAccessPoint(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopProjectionAccessPoint:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.stopProjectionAccessPoint(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestBluetoothProfileInhibit:
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
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          boolean _result = this.requestBluetoothProfileInhibit(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_releaseBluetoothProfileInhibit:
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
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          boolean _result = this.releaseBluetoothProfileInhibit(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateProjectionStatus:
        {
          data.enforceInterface(descriptor);
          android.car.projection.ProjectionStatus _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.projection.ProjectionStatus.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.updateProjectionStatus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerProjectionStatusListener:
        {
          data.enforceInterface(descriptor);
          android.car.ICarProjectionStatusListener _arg0;
          _arg0 = android.car.ICarProjectionStatusListener.Stub.asInterface(data.readStrongBinder());
          this.registerProjectionStatusListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterProjectionStatusListener:
        {
          data.enforceInterface(descriptor);
          android.car.ICarProjectionStatusListener _arg0;
          _arg0 = android.car.ICarProjectionStatusListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterProjectionStatusListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProjectionOptions:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _result = this.getProjectionOptions();
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
        case TRANSACTION_getAvailableWifiChannels:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _result = this.getAvailableWifiChannels(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICarProjection
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
           * Registers projection runner on projection start with projection service
           * to create reverse binding.
           */
      @Override public void registerProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((serviceIntent!=null)) {
            _data.writeInt(1);
            serviceIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerProjectionRunner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerProjectionRunner(serviceIntent);
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
           * Unregisters projection runner on projection stop with projection service to create
           * reverse binding.
           */
      @Override public void unregisterProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((serviceIntent!=null)) {
            _data.writeInt(1);
            serviceIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterProjectionRunner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterProjectionRunner(serviceIntent);
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
           * Registers projection key event handler.
           * Re-registering same event handler with different events will cause only events to update.
           */
      @Override public void registerKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler, byte[] eventMask) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((eventHandler!=null))?(eventHandler.asBinder()):(null)));
          _data.writeByteArray(eventMask);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerKeyEventHandler, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerKeyEventHandler(eventHandler, eventMask);
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
           * Unregisters projection key event handler.
           */
      @Override public void unregisterKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((eventHandler!=null))?(eventHandler.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterKeyEventHandler, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterKeyEventHandler(eventHandler);
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
           * Starts Wi-Fi access point if it hasn't been started yet for wireless projection and returns
           * WiFiConfiguration object with access point details.
           */
      @Override public void startProjectionAccessPoint(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startProjectionAccessPoint, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startProjectionAccessPoint(messenger, binder);
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
           * Stops previously requested Wi-Fi access point.
           */
      @Override public void stopProjectionAccessPoint(android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopProjectionAccessPoint, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopProjectionAccessPoint(binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Disconnect a Bluetooth profile, and prevent it from reconnecting. */
      @Override public boolean requestBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(profile);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBluetoothProfileInhibit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestBluetoothProfileInhibit(device, profile, token);
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
      /** Undo the effects of requestBluetoothProfileInhibit. */
      @Override public boolean releaseBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(profile);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseBluetoothProfileInhibit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().releaseBluetoothProfileInhibit(device, profile, token);
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
      /** Reports projection status for a given projection receiver app. */
      @Override public void updateProjectionStatus(android.car.projection.ProjectionStatus status, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((status!=null)) {
            _data.writeInt(1);
            status.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateProjectionStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateProjectionStatus(status, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Registers projection status listener */
      @Override public void registerProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerProjectionStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerProjectionStatusListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Unregister projection status listener */
      @Override public void unregisterProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterProjectionStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterProjectionStatusListener(listener);
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
           * Returns options for projection receiver app that can be un-packed using
           * {@link android.car.projection.ProjectionOptions} class.
           */
      @Override public android.os.Bundle getProjectionOptions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProjectionOptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProjectionOptions();
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
      /** Returns a list of available Wi-Fi channels */
      @Override public int[] getAvailableWifiChannels(int band) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(band);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableWifiChannels, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableWifiChannels(band);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.car.ICarProjection sDefaultImpl;
    }
    static final int TRANSACTION_registerProjectionRunner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterProjectionRunner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registerKeyEventHandler = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_unregisterKeyEventHandler = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startProjectionAccessPoint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_stopProjectionAccessPoint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_requestBluetoothProfileInhibit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_releaseBluetoothProfileInhibit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_updateProjectionStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_registerProjectionStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_unregisterProjectionStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getProjectionOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getAvailableWifiChannels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.car.ICarProjection impl) {
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
    public static android.car.ICarProjection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Registers projection runner on projection start with projection service
       * to create reverse binding.
       */
  public void registerProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException;
  /**
       * Unregisters projection runner on projection stop with projection service to create
       * reverse binding.
       */
  public void unregisterProjectionRunner(android.content.Intent serviceIntent) throws android.os.RemoteException;
  /**
       * Registers projection key event handler.
       * Re-registering same event handler with different events will cause only events to update.
       */
  public void registerKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler, byte[] eventMask) throws android.os.RemoteException;
  /**
       * Unregisters projection key event handler.
       */
  public void unregisterKeyEventHandler(android.car.ICarProjectionKeyEventHandler eventHandler) throws android.os.RemoteException;
  /**
       * Starts Wi-Fi access point if it hasn't been started yet for wireless projection and returns
       * WiFiConfiguration object with access point details.
       */
  public void startProjectionAccessPoint(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException;
  /**
       * Stops previously requested Wi-Fi access point.
       */
  public void stopProjectionAccessPoint(android.os.IBinder binder) throws android.os.RemoteException;
  /** Disconnect a Bluetooth profile, and prevent it from reconnecting. */
  public boolean requestBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException;
  /** Undo the effects of requestBluetoothProfileInhibit. */
  public boolean releaseBluetoothProfileInhibit(android.bluetooth.BluetoothDevice device, int profile, android.os.IBinder token) throws android.os.RemoteException;
  /** Reports projection status for a given projection receiver app. */
  public void updateProjectionStatus(android.car.projection.ProjectionStatus status, android.os.IBinder token) throws android.os.RemoteException;
  /** Registers projection status listener */
  public void registerProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException;
  /** Unregister projection status listener */
  public void unregisterProjectionStatusListener(android.car.ICarProjectionStatusListener listener) throws android.os.RemoteException;
  /**
       * Returns options for projection receiver app that can be un-packed using
       * {@link android.car.projection.ProjectionOptions} class.
       */
  public android.os.Bundle getProjectionOptions() throws android.os.RemoteException;
  /** Returns a list of available Wi-Fi channels */
  public int[] getAvailableWifiChannels(int band) throws android.os.RemoteException;
}
