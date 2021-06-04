/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * Helper interface for ITvInputSession to allow the TV input to notify the system service when a
 * new session has been created.
 * @hide
 */
public interface ITvInputSessionCallback extends android.os.IInterface
{
  /** Default implementation for ITvInputSessionCallback. */
  public static class Default implements android.media.tv.ITvInputSessionCallback
  {
    @Override public void onSessionCreated(android.media.tv.ITvInputSession session, android.os.IBinder hardwareSessionToken) throws android.os.RemoteException
    {
    }
    @Override public void onSessionEvent(java.lang.String name, android.os.Bundle args) throws android.os.RemoteException
    {
    }
    @Override public void onChannelRetuned(android.net.Uri channelUri) throws android.os.RemoteException
    {
    }
    @Override public void onTracksChanged(java.util.List<android.media.tv.TvTrackInfo> tracks) throws android.os.RemoteException
    {
    }
    @Override public void onTrackSelected(int type, java.lang.String trackId) throws android.os.RemoteException
    {
    }
    @Override public void onVideoAvailable() throws android.os.RemoteException
    {
    }
    @Override public void onVideoUnavailable(int reason) throws android.os.RemoteException
    {
    }
    @Override public void onContentAllowed() throws android.os.RemoteException
    {
    }
    @Override public void onContentBlocked(java.lang.String rating) throws android.os.RemoteException
    {
    }
    @Override public void onLayoutSurface(int left, int top, int right, int bottom) throws android.os.RemoteException
    {
    }
    @Override public void onTimeShiftStatusChanged(int status) throws android.os.RemoteException
    {
    }
    @Override public void onTimeShiftStartPositionChanged(long timeMs) throws android.os.RemoteException
    {
    }
    @Override public void onTimeShiftCurrentPositionChanged(long timeMs) throws android.os.RemoteException
    {
    }
    // For the recording session

    @Override public void onTuned(android.net.Uri channelUri) throws android.os.RemoteException
    {
    }
    @Override public void onRecordingStopped(android.net.Uri recordedProgramUri) throws android.os.RemoteException
    {
    }
    @Override public void onError(int error) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputSessionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputSessionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputSessionCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputSessionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputSessionCallback))) {
        return ((android.media.tv.ITvInputSessionCallback)iin);
      }
      return new android.media.tv.ITvInputSessionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSessionCreated:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputSession _arg0;
          _arg0 = android.media.tv.ITvInputSession.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.onSessionCreated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSessionEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onSessionEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onChannelRetuned:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onChannelRetuned(_arg0);
          return true;
        }
        case TRANSACTION_onTracksChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.tv.TvTrackInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.media.tv.TvTrackInfo.CREATOR);
          this.onTracksChanged(_arg0);
          return true;
        }
        case TRANSACTION_onTrackSelected:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onTrackSelected(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onVideoAvailable:
        {
          data.enforceInterface(descriptor);
          this.onVideoAvailable();
          return true;
        }
        case TRANSACTION_onVideoUnavailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onVideoUnavailable(_arg0);
          return true;
        }
        case TRANSACTION_onContentAllowed:
        {
          data.enforceInterface(descriptor);
          this.onContentAllowed();
          return true;
        }
        case TRANSACTION_onContentBlocked:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onContentBlocked(_arg0);
          return true;
        }
        case TRANSACTION_onLayoutSurface:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.onLayoutSurface(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onTimeShiftStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onTimeShiftStatusChanged(_arg0);
          return true;
        }
        case TRANSACTION_onTimeShiftStartPositionChanged:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onTimeShiftStartPositionChanged(_arg0);
          return true;
        }
        case TRANSACTION_onTimeShiftCurrentPositionChanged:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onTimeShiftCurrentPositionChanged(_arg0);
          return true;
        }
        case TRANSACTION_onTuned:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onTuned(_arg0);
          return true;
        }
        case TRANSACTION_onRecordingStopped:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onRecordingStopped(_arg0);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputSessionCallback
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
      @Override public void onSessionCreated(android.media.tv.ITvInputSession session, android.os.IBinder hardwareSessionToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeStrongBinder(hardwareSessionToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionCreated(session, hardwareSessionToken);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionEvent(java.lang.String name, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionEvent(name, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onChannelRetuned(android.net.Uri channelUri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((channelUri!=null)) {
            _data.writeInt(1);
            channelUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onChannelRetuned, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onChannelRetuned(channelUri);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTracksChanged(java.util.List<android.media.tv.TvTrackInfo> tracks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(tracks);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTracksChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTracksChanged(tracks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTrackSelected(int type, java.lang.String trackId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          _data.writeString(trackId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrackSelected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrackSelected(type, trackId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVideoAvailable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVideoAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVideoAvailable();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVideoUnavailable(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVideoUnavailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVideoUnavailable(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onContentAllowed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onContentAllowed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onContentAllowed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onContentBlocked(java.lang.String rating) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rating);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onContentBlocked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onContentBlocked(rating);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLayoutSurface(int left, int top, int right, int bottom) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(left);
          _data.writeInt(top);
          _data.writeInt(right);
          _data.writeInt(bottom);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayoutSurface, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayoutSurface(left, top, right, bottom);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTimeShiftStatusChanged(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTimeShiftStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTimeShiftStatusChanged(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTimeShiftStartPositionChanged(long timeMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timeMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTimeShiftStartPositionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTimeShiftStartPositionChanged(timeMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTimeShiftCurrentPositionChanged(long timeMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timeMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTimeShiftCurrentPositionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTimeShiftCurrentPositionChanged(timeMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // For the recording session

      @Override public void onTuned(android.net.Uri channelUri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((channelUri!=null)) {
            _data.writeInt(1);
            channelUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTuned, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTuned(channelUri);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRecordingStopped(android.net.Uri recordedProgramUri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((recordedProgramUri!=null)) {
            _data.writeInt(1);
            recordedProgramUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRecordingStopped, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRecordingStopped(recordedProgramUri);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onError(int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputSessionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSessionCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSessionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onChannelRetuned = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onTracksChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onTrackSelected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onVideoAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onVideoUnavailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onContentAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onContentBlocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onLayoutSurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onTimeShiftStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onTimeShiftStartPositionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onTimeShiftCurrentPositionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onTuned = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onRecordingStopped = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(android.media.tv.ITvInputSessionCallback impl) {
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
    public static android.media.tv.ITvInputSessionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSessionCreated(android.media.tv.ITvInputSession session, android.os.IBinder hardwareSessionToken) throws android.os.RemoteException;
  public void onSessionEvent(java.lang.String name, android.os.Bundle args) throws android.os.RemoteException;
  public void onChannelRetuned(android.net.Uri channelUri) throws android.os.RemoteException;
  public void onTracksChanged(java.util.List<android.media.tv.TvTrackInfo> tracks) throws android.os.RemoteException;
  public void onTrackSelected(int type, java.lang.String trackId) throws android.os.RemoteException;
  public void onVideoAvailable() throws android.os.RemoteException;
  public void onVideoUnavailable(int reason) throws android.os.RemoteException;
  public void onContentAllowed() throws android.os.RemoteException;
  public void onContentBlocked(java.lang.String rating) throws android.os.RemoteException;
  public void onLayoutSurface(int left, int top, int right, int bottom) throws android.os.RemoteException;
  public void onTimeShiftStatusChanged(int status) throws android.os.RemoteException;
  public void onTimeShiftStartPositionChanged(long timeMs) throws android.os.RemoteException;
  public void onTimeShiftCurrentPositionChanged(long timeMs) throws android.os.RemoteException;
  // For the recording session

  public void onTuned(android.net.Uri channelUri) throws android.os.RemoteException;
  public void onRecordingStopped(android.net.Uri recordedProgramUri) throws android.os.RemoteException;
  public void onError(int error) throws android.os.RemoteException;
}
