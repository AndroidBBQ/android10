/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.radio;
/** {@hide} */
public interface ITuner extends android.os.IInterface
{
  /** Default implementation for ITuner. */
  public static class Default implements android.hardware.radio.ITuner
  {
    @Override public void close() throws android.os.RemoteException
    {
    }
    @Override public boolean isClosed() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @throws IllegalArgumentException if config is not valid or null
         */
    @Override public void setConfiguration(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException
    {
    }
    @Override public android.hardware.radio.RadioManager.BandConfig getConfiguration() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @throws IllegalStateException if tuner was opened without audio
         */
    @Override public void setMuted(boolean mute) throws android.os.RemoteException
    {
    }
    @Override public boolean isMuted() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @throws IllegalStateException if called out of sequence
         */
    @Override public void step(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException
    {
    }
    /**
         * @throws IllegalStateException if called out of sequence
         */
    @Override public void scan(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException
    {
    }
    /**
         * @throws IllegalArgumentException if invalid arguments are passed
         * @throws IllegalStateException if called out of sequence
         */
    @Override public void tune(android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException
    {
    }
    /**
         * @throws IllegalStateException if called out of sequence
         */
    @Override public void cancel() throws android.os.RemoteException
    {
    }
    @Override public void cancelAnnouncement() throws android.os.RemoteException
    {
    }
    @Override public android.graphics.Bitmap getImage(int id) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @return {@code true} if the scan was properly scheduled,
         *          {@code false} if the scan feature is unavailable
         */
    @Override public boolean startBackgroundScan() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startProgramListUpdates(android.hardware.radio.ProgramList.Filter filter) throws android.os.RemoteException
    {
    }
    @Override public void stopProgramListUpdates() throws android.os.RemoteException
    {
    }
    @Override public boolean isConfigFlagSupported(int flag) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isConfigFlagSet(int flag) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setConfigFlag(int flag, boolean value) throws android.os.RemoteException
    {
    }
    /**
         * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
         * @return Vendor-specific key-value pairs, must be Map<String, String>
         */
    @Override public java.util.Map setParameters(java.util.Map parameters) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @param keys Parameter keys to fetch
         * @return Vendor-specific key-value pairs, must be Map<String, String>
         */
    @Override public java.util.Map getParameters(java.util.List<java.lang.String> keys) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.radio.ITuner
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.radio.ITuner";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.radio.ITuner interface,
     * generating a proxy if needed.
     */
    public static android.hardware.radio.ITuner asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.radio.ITuner))) {
        return ((android.hardware.radio.ITuner)iin);
      }
      return new android.hardware.radio.ITuner.Stub.Proxy(obj);
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
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isClosed:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isClosed();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setConfiguration:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.RadioManager.BandConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.RadioManager.BandConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setConfiguration(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getConfiguration:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.RadioManager.BandConfig _result = this.getConfiguration();
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
        case TRANSACTION_setMuted:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setMuted(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isMuted:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isMuted();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_step:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.step(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_scan:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.scan(_arg0, _arg1);
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
          this.tune(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancel:
        {
          data.enforceInterface(descriptor);
          this.cancel();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelAnnouncement:
        {
          data.enforceInterface(descriptor);
          this.cancelAnnouncement();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getImage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Bitmap _result = this.getImage(_arg0);
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
        case TRANSACTION_startBackgroundScan:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.startBackgroundScan();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startProgramListUpdates:
        {
          data.enforceInterface(descriptor);
          android.hardware.radio.ProgramList.Filter _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.radio.ProgramList.Filter.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startProgramListUpdates(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopProgramListUpdates:
        {
          data.enforceInterface(descriptor);
          this.stopProgramListUpdates();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isConfigFlagSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isConfigFlagSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isConfigFlagSet:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isConfigFlagSet(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setConfigFlag:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setConfigFlag(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setParameters:
        {
          data.enforceInterface(descriptor);
          java.util.Map _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readHashMap(cl);
          java.util.Map _result = this.setParameters(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_getParameters:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          java.util.Map _result = this.getParameters(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.radio.ITuner
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
      @Override public void close() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isClosed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isClosed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isClosed();
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
           * @throws IllegalArgumentException if config is not valid or null
           */
      @Override public void setConfiguration(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConfiguration(config);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.hardware.radio.RadioManager.BandConfig getConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.radio.RadioManager.BandConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfiguration();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.radio.RadioManager.BandConfig.CREATOR.createFromParcel(_reply);
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
           * @throws IllegalStateException if tuner was opened without audio
           */
      @Override public void setMuted(boolean mute) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((mute)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMuted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMuted(mute);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isMuted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMuted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMuted();
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
           * @throws IllegalStateException if called out of sequence
           */
      @Override public void step(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((directionDown)?(1):(0)));
          _data.writeInt(((skipSubChannel)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_step, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().step(directionDown, skipSubChannel);
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
           * @throws IllegalStateException if called out of sequence
           */
      @Override public void scan(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((directionDown)?(1):(0)));
          _data.writeInt(((skipSubChannel)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_scan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scan(directionDown, skipSubChannel);
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
           * @throws IllegalArgumentException if invalid arguments are passed
           * @throws IllegalStateException if called out of sequence
           */
      @Override public void tune(android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((selector!=null)) {
            _data.writeInt(1);
            selector.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_tune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tune(selector);
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
           * @throws IllegalStateException if called out of sequence
           */
      @Override public void cancel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancel();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelAnnouncement() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAnnouncement, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAnnouncement();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.graphics.Bitmap getImage(int id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Bitmap _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImage(id);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
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
           * @return {@code true} if the scan was properly scheduled,
           *          {@code false} if the scan feature is unavailable
           */
      @Override public boolean startBackgroundScan() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startBackgroundScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startBackgroundScan();
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
      @Override public void startProgramListUpdates(android.hardware.radio.ProgramList.Filter filter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((filter!=null)) {
            _data.writeInt(1);
            filter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startProgramListUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startProgramListUpdates(filter);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopProgramListUpdates() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopProgramListUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopProgramListUpdates();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isConfigFlagSupported(int flag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isConfigFlagSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isConfigFlagSupported(flag);
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
      @Override public boolean isConfigFlagSet(int flag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isConfigFlagSet, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isConfigFlagSet(flag);
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
      @Override public void setConfigFlag(int flag, boolean value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flag);
          _data.writeInt(((value)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConfigFlag, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConfigFlag(flag, value);
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
           * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
           * @return Vendor-specific key-value pairs, must be Map<String, String>
           */
      @Override public java.util.Map setParameters(java.util.Map parameters) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeMap(parameters);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setParameters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setParameters(parameters);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @param keys Parameter keys to fetch
           * @return Vendor-specific key-value pairs, must be Map<String, String>
           */
      @Override public java.util.Map getParameters(java.util.List<java.lang.String> keys) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(keys);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getParameters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getParameters(keys);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.hardware.radio.ITuner sDefaultImpl;
    }
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isClosed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setMuted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isMuted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_step = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_scan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_tune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_cancel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_cancelAnnouncement = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getImage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_startBackgroundScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_startProgramListUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_stopProgramListUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_isConfigFlagSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_isConfigFlagSet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setConfigFlag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setParameters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getParameters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    public static boolean setDefaultImpl(android.hardware.radio.ITuner impl) {
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
    public static android.hardware.radio.ITuner getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void close() throws android.os.RemoteException;
  public boolean isClosed() throws android.os.RemoteException;
  /**
       * @throws IllegalArgumentException if config is not valid or null
       */
  public void setConfiguration(android.hardware.radio.RadioManager.BandConfig config) throws android.os.RemoteException;
  public android.hardware.radio.RadioManager.BandConfig getConfiguration() throws android.os.RemoteException;
  /**
       * @throws IllegalStateException if tuner was opened without audio
       */
  public void setMuted(boolean mute) throws android.os.RemoteException;
  public boolean isMuted() throws android.os.RemoteException;
  /**
       * @throws IllegalStateException if called out of sequence
       */
  public void step(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException;
  /**
       * @throws IllegalStateException if called out of sequence
       */
  public void scan(boolean directionDown, boolean skipSubChannel) throws android.os.RemoteException;
  /**
       * @throws IllegalArgumentException if invalid arguments are passed
       * @throws IllegalStateException if called out of sequence
       */
  public void tune(android.hardware.radio.ProgramSelector selector) throws android.os.RemoteException;
  /**
       * @throws IllegalStateException if called out of sequence
       */
  public void cancel() throws android.os.RemoteException;
  public void cancelAnnouncement() throws android.os.RemoteException;
  public android.graphics.Bitmap getImage(int id) throws android.os.RemoteException;
  /**
       * @return {@code true} if the scan was properly scheduled,
       *          {@code false} if the scan feature is unavailable
       */
  public boolean startBackgroundScan() throws android.os.RemoteException;
  public void startProgramListUpdates(android.hardware.radio.ProgramList.Filter filter) throws android.os.RemoteException;
  public void stopProgramListUpdates() throws android.os.RemoteException;
  public boolean isConfigFlagSupported(int flag) throws android.os.RemoteException;
  public boolean isConfigFlagSet(int flag) throws android.os.RemoteException;
  public void setConfigFlag(int flag, boolean value) throws android.os.RemoteException;
  /**
       * @param parameters Vendor-specific key-value pairs, must be Map<String, String>
       * @return Vendor-specific key-value pairs, must be Map<String, String>
       */
  public java.util.Map setParameters(java.util.Map parameters) throws android.os.RemoteException;
  /**
       * @param keys Parameter keys to fetch
       * @return Vendor-specific key-value pairs, must be Map<String, String>
       */
  public java.util.Map getParameters(java.util.List<java.lang.String> keys) throws android.os.RemoteException;
}
