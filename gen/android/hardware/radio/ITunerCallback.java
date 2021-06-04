/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.radio;
/** {@hide} */
public interface ITunerCallback extends android.os.IInterface
{
  /** Default implementation for ITunerCallback. */
  public static class Default implements android.hardware.radio.ITunerCallback
  {
    @Override public void onError(int status) throws android.os.RemoteException
    {
    }
    @Override public void onTuneFailed(int result, android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException
    {
    }
    @Override public void onConfigurationChanged(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException
    {
    }
    @Override public void onCurrentProgramInfoChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException
    {
    }
    @Override public void onTrafficAnnouncement(boolean active) throws android.os.RemoteException
    {
    }
    @Override public void onEmergencyAnnouncement(boolean active) throws android.os.RemoteException
    {
    }
    @Override public void onAntennaState(boolean connected) throws android.os.RemoteException
    {
    }
    @Override public void onBackgroundScanAvailabilityChange(boolean isAvailable) throws android.os.RemoteException
    {
    }
    @Override public void onBackgroundScanComplete() throws android.os.RemoteException
    {
    }
    @Override public void onProgramListChanged() throws android.os.RemoteException
    {
    }
    @Override public void onProgramListUpdated(android.hardware.radio.ProgramList.Chunk chunk) throws android.os.RemoteException
    {
    }
    /**
         * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
         */
    @Override public void onParametersUpdated(java.util.Map parameters) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.radio.ITunerCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.radio.ITunerCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.radio.ITunerCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.radio.ITunerCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.radio.ITunerCallback))) {
        return ((android.hardware.radio.ITunerCallback)iin);
      }
      return new android.hardware.radio.ITunerCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onError(_arg0);
          return true;
        }
        case TRANSACTION_onTuneFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.radio.ProgramSelector _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.radio.ProgramSelector.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onTuneFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onConfigurationChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.RadioManager.BandConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.RadioManager.BandConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onConfigurationChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCurrentProgramInfoChanged:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.RadioManager.ProgramInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.RadioManager.ProgramInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCurrentProgramInfoChanged(_arg0);
          return true;
        }
        case TRANSACTION_onTrafficAnnouncement:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onTrafficAnnouncement(_arg0);
          return true;
        }
        case TRANSACTION_onEmergencyAnnouncement:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onEmergencyAnnouncement(_arg0);
          return true;
        }
        case TRANSACTION_onAntennaState:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onAntennaState(_arg0);
          return true;
        }
        case TRANSACTION_onBackgroundScanAvailabilityChange:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onBackgroundScanAvailabilityChange(_arg0);
          return true;
        }
        case TRANSACTION_onBackgroundScanComplete:
        {
          data.enforceInterface(descriptor);
          this.onBackgroundScanComplete();
          return true;
        }
        case TRANSACTION_onProgramListChanged:
        {
          data.enforceInterface(descriptor);
          this.onProgramListChanged();
          return true;
        }
        case TRANSACTION_onProgramListUpdated:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.ProgramList.Chunk _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.ProgramList.Chunk.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onProgramListUpdated(_arg0);
          return true;
        }
        case TRANSACTION_onParametersUpdated:
        {
          data.enforceInterface(descriptor);
          java.util.Map _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readHashMap(cl);
          this.onParametersUpdated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.radio.ITunerCallback
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
      @Override public void onError(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTuneFailed(int result, android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          if ((selector!=null)) {
            _data.writeInt(1);
            selector.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTuneFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTuneFailed(result, selector);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConfigurationChanged(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfigurationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfigurationChanged(config);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCurrentProgramInfoChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCurrentProgramInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCurrentProgramInfoChanged(info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTrafficAnnouncement(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrafficAnnouncement, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrafficAnnouncement(active);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onEmergencyAnnouncement(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEmergencyAnnouncement, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEmergencyAnnouncement(active);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAntennaState(boolean connected) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((connected)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAntennaState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAntennaState(connected);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onBackgroundScanAvailabilityChange(boolean isAvailable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isAvailable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBackgroundScanAvailabilityChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBackgroundScanAvailabilityChange(isAvailable);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onBackgroundScanComplete() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBackgroundScanComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBackgroundScanComplete();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onProgramListChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgramListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgramListChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onProgramListUpdated(android.hardware.radio.ProgramList.Chunk chunk) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((chunk!=null)) {
            _data.writeInt(1);
            chunk.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgramListUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgramListUpdated(chunk);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
           */
      @Override public void onParametersUpdated(java.util.Map parameters) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeMap(parameters);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onParametersUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onParametersUpdated(parameters);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.radio.ITunerCallback sDefaultImpl;
    }
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onTuneFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onConfigurationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onCurrentProgramInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onTrafficAnnouncement = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onEmergencyAnnouncement = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onAntennaState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onBackgroundScanAvailabilityChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onBackgroundScanComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onProgramListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onProgramListUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onParametersUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(android.hardware.radio.ITunerCallback impl) {
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
    public static android.hardware.radio.ITunerCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onError(int status) throws android.os.RemoteException;
  public void onTuneFailed(int result, android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException;
  public void onConfigurationChanged(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException;
  public void onCurrentProgramInfoChanged(android.hardware.radio.RadioManager.ProgramInfo info) throws android.os.RemoteException;
  public void onTrafficAnnouncement(boolean active) throws android.os.RemoteException;
  public void onEmergencyAnnouncement(boolean active) throws android.os.RemoteException;
  public void onAntennaState(boolean connected) throws android.os.RemoteException;
  public void onBackgroundScanAvailabilityChange(boolean isAvailable) throws android.os.RemoteException;
  public void onBackgroundScanComplete() throws android.os.RemoteException;
  public void onProgramListChanged() throws android.os.RemoteException;
  public void onProgramListUpdated(android.hardware.radio.ProgramList.Chunk chunk) throws android.os.RemoteException;
  /**
       * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
       */
  public void onParametersUpdated(java.util.Map parameters) throws android.os.RemoteException;
}
