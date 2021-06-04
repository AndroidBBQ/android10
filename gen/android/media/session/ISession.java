/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * Interface to a MediaSession in the system.
 * @hide
 */
public interface ISession extends android.os.IInterface
{
  /** Default implementation for ISession. */
  public static class Default implements android.media.session.ISession
  {
    @Override public void sendEvent(java.lang.String event, android.os.Bundle data) throws android.os.RemoteException
    {
    }
    @Override public android.media.session.ISessionController getController() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFlags(int flags) throws android.os.RemoteException
    {
    }
    @Override public void setActive(boolean active) throws android.os.RemoteException
    {
    }
    @Override public void setMediaButtonReceiver(android.app.PendingIntent mbr) throws android.os.RemoteException
    {
    }
    @Override public void setLaunchPendingIntent(android.app.PendingIntent pi) throws android.os.RemoteException
    {
    }
    @Override public void destroySession() throws android.os.RemoteException
    {
    }
    // These commands are for the TransportPerformer

    @Override public void setMetadata(android.media.MediaMetadata metadata, long duration, java.lang.String metadataDescription) throws android.os.RemoteException
    {
    }
    @Override public void setPlaybackState(android.media.session.PlaybackState state) throws android.os.RemoteException
    {
    }
    @Override public void setQueue(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException
    {
    }
    @Override public void setQueueTitle(java.lang.CharSequence title) throws android.os.RemoteException
    {
    }
    @Override public void setExtras(android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void setRatingType(int type) throws android.os.RemoteException
    {
    }
    // These commands relate to volume handling

    @Override public void setPlaybackToLocal(android.media.AudioAttributes attributes) throws android.os.RemoteException
    {
    }
    @Override public void setPlaybackToRemote(int control, int max) throws android.os.RemoteException
    {
    }
    @Override public void setCurrentVolume(int currentVolume) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ISession
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ISession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ISession interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ISession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ISession))) {
        return ((android.media.session.ISession)iin);
      }
      return new android.media.session.ISession.Stub.Proxy(obj);
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
        case TRANSACTION_sendEvent:
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
          this.sendEvent(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getController:
        {
          data.enforceInterface(descriptor);
          android.media.session.ISessionController _result = this.getController();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_setFlags:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setFlags(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setActive:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setActive(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMediaButtonReceiver:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setMediaButtonReceiver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setLaunchPendingIntent:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setLaunchPendingIntent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_destroySession:
        {
          data.enforceInterface(descriptor);
          this.destroySession();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMetadata:
        {
          data.enforceInterface(descriptor);
          android.media.MediaMetadata _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.MediaMetadata.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setMetadata(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPlaybackState:
        {
          data.enforceInterface(descriptor);
          android.media.session.PlaybackState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.PlaybackState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setPlaybackState(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setQueue:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setQueue(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setQueueTitle:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setQueueTitle(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setExtras:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setExtras(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRatingType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setRatingType(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPlaybackToLocal:
        {
          data.enforceInterface(descriptor);
          android.media.AudioAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setPlaybackToLocal(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPlaybackToRemote:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setPlaybackToRemote(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCurrentVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setCurrentVolume(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.ISession
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
      @Override public void sendEvent(java.lang.String event, android.os.Bundle data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(event);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendEvent(event, data);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.media.session.ISessionController getController() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.session.ISessionController _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getController();
          }
          _reply.readException();
          _result = android.media.session.ISessionController.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFlags(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFlags, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFlags(flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setActive(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActive(active);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setMediaButtonReceiver(android.app.PendingIntent mbr) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((mbr!=null)) {
            _data.writeInt(1);
            mbr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMediaButtonReceiver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMediaButtonReceiver(mbr);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setLaunchPendingIntent(android.app.PendingIntent pi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pi!=null)) {
            _data.writeInt(1);
            pi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLaunchPendingIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLaunchPendingIntent(pi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void destroySession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroySession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroySession();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These commands are for the TransportPerformer

      @Override public void setMetadata(android.media.MediaMetadata metadata, long duration, java.lang.String metadataDescription) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((metadata!=null)) {
            _data.writeInt(1);
            metadata.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(duration);
          _data.writeString(metadataDescription);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMetadata, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMetadata(metadata, duration, metadataDescription);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPlaybackState(android.media.session.PlaybackState state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPlaybackState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPlaybackState(state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setQueue(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queue!=null)) {
            _data.writeInt(1);
            queue.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setQueue, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setQueue(queue);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setQueueTitle(java.lang.CharSequence title) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (title!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(title, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setQueueTitle, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setQueueTitle(title);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setExtras(android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setExtras(extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRatingType(int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRatingType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRatingType(type);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // These commands relate to volume handling

      @Override public void setPlaybackToLocal(android.media.AudioAttributes attributes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((attributes!=null)) {
            _data.writeInt(1);
            attributes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPlaybackToLocal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPlaybackToLocal(attributes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPlaybackToRemote(int control, int max) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(control);
          _data.writeInt(max);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPlaybackToRemote, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPlaybackToRemote(control, max);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setCurrentVolume(int currentVolume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(currentVolume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCurrentVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCurrentVolume(currentVolume);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.session.ISession sDefaultImpl;
    }
    static final int TRANSACTION_sendEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setFlags = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setMediaButtonReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setLaunchPendingIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_destroySession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setPlaybackState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setQueue = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setQueueTitle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setRatingType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setPlaybackToLocal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setPlaybackToRemote = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setCurrentVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(android.media.session.ISession impl) {
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
    public static android.media.session.ISession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void sendEvent(java.lang.String event, android.os.Bundle data) throws android.os.RemoteException;
  public android.media.session.ISessionController getController() throws android.os.RemoteException;
  public void setFlags(int flags) throws android.os.RemoteException;
  public void setActive(boolean active) throws android.os.RemoteException;
  public void setMediaButtonReceiver(android.app.PendingIntent mbr) throws android.os.RemoteException;
  public void setLaunchPendingIntent(android.app.PendingIntent pi) throws android.os.RemoteException;
  public void destroySession() throws android.os.RemoteException;
  // These commands are for the TransportPerformer

  public void setMetadata(android.media.MediaMetadata metadata, long duration, java.lang.String metadataDescription) throws android.os.RemoteException;
  public void setPlaybackState(android.media.session.PlaybackState state) throws android.os.RemoteException;
  public void setQueue(android.content.pm.ParceledListSlice queue) throws android.os.RemoteException;
  public void setQueueTitle(java.lang.CharSequence title) throws android.os.RemoteException;
  public void setExtras(android.os.Bundle extras) throws android.os.RemoteException;
  public void setRatingType(int type) throws android.os.RemoteException;
  // These commands relate to volume handling

  public void setPlaybackToLocal(android.media.AudioAttributes attributes) throws android.os.RemoteException;
  public void setPlaybackToRemote(int control, int max) throws android.os.RemoteException;
  public void setCurrentVolume(int currentVolume) throws android.os.RemoteException;
}
