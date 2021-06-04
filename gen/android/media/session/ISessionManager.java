/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * Interface to the MediaSessionManagerService
 * @hide
 */
public interface ISessionManager extends android.os.IInterface
{
  /** Default implementation for ISessionManager. */
  public static class Default implements android.media.session.ISessionManager
  {
    @Override public android.media.session.ISession createSession(java.lang.String packageName, android.media.session.ISessionCallback sessionCb, java.lang.String tag, android.os.Bundle sessionInfo, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void notifySession2Created(android.media.Session2Token sessionToken) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.media.session.MediaSession.Token> getSessions(android.content.ComponentName compName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getSession2Tokens(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void dispatchMediaKeyEvent(java.lang.String packageName, boolean asSystemService, android.view.KeyEvent keyEvent, boolean needWakeLock) throws android.os.RemoteException
    {
    }
    @Override public boolean dispatchMediaKeyEventToSessionAsSystemService(java.lang.String packageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void dispatchVolumeKeyEvent(java.lang.String packageName, java.lang.String opPackageName, boolean asSystemService, android.view.KeyEvent keyEvent, int stream, boolean musicOnly) throws android.os.RemoteException
    {
    }
    @Override public void dispatchVolumeKeyEventToSessionAsSystemService(java.lang.String packageName, java.lang.String opPackageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException
    {
    }
    @Override public void dispatchAdjustVolume(java.lang.String packageName, java.lang.String opPackageName, int suggestedStream, int delta, int flags) throws android.os.RemoteException
    {
    }
    @Override public void addSessionsListener(android.media.session.IActiveSessionsListener listener, android.content.ComponentName compName, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeSessionsListener(android.media.session.IActiveSessionsListener listener) throws android.os.RemoteException
    {
    }
    @Override public void addSession2TokensListener(android.media.session.ISession2TokensListener listener, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeSession2TokensListener(android.media.session.ISession2TokensListener listener) throws android.os.RemoteException
    {
    }
    @Override public void registerRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException
    {
    }
    @Override public void unregisterRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException
    {
    }
    // For PhoneWindowManager to precheck media keys

    @Override public boolean isGlobalPriorityActive() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setCallback(android.media.session.ICallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setOnVolumeKeyLongPressListener(android.media.session.IOnVolumeKeyLongPressListener listener) throws android.os.RemoteException
    {
    }
    @Override public void setOnMediaKeyListener(android.media.session.IOnMediaKeyListener listener) throws android.os.RemoteException
    {
    }
    @Override public boolean isTrusted(java.lang.String controllerPackageName, int controllerPid, int controllerUid) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.ISessionManager
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.ISessionManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.ISessionManager interface,
     * generating a proxy if needed.
     */
    public static android.media.session.ISessionManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.ISessionManager))) {
        return ((android.media.session.ISessionManager)iin);
      }
      return new android.media.session.ISessionManager.Stub.Proxy(obj);
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
        case TRANSACTION_createSession:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.ISessionCallback _arg1;
          _arg1 = android.media.session.ISessionCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          android.media.session.ISession _result = this.createSession(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_notifySession2Created:
        {
          data.enforceInterface(descriptor);
          android.media.Session2Token _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.Session2Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifySession2Created(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSessions:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.media.session.MediaSession.Token> _result = this.getSessions(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getSession2Tokens:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getSession2Tokens(_arg0);
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
        case TRANSACTION_dispatchMediaKeyEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.view.KeyEvent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.dispatchMediaKeyEvent(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dispatchMediaKeyEventToSessionAsSystemService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.media.session.MediaSession.Token _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.view.KeyEvent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.dispatchMediaKeyEventToSessionAsSystemService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_dispatchVolumeKeyEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.view.KeyEvent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.dispatchVolumeKeyEvent(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dispatchVolumeKeyEventToSessionAsSystemService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.session.MediaSession.Token _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.view.KeyEvent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.dispatchVolumeKeyEventToSessionAsSystemService(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dispatchAdjustVolume:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.dispatchAdjustVolume(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addSessionsListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.IActiveSessionsListener _arg0;
          _arg0 = android.media.session.IActiveSessionsListener.Stub.asInterface(data.readStrongBinder());
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.addSessionsListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeSessionsListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.IActiveSessionsListener _arg0;
          _arg0 = android.media.session.IActiveSessionsListener.Stub.asInterface(data.readStrongBinder());
          this.removeSessionsListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addSession2TokensListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.ISession2TokensListener _arg0;
          _arg0 = android.media.session.ISession2TokensListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.addSession2TokensListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeSession2TokensListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.ISession2TokensListener _arg0;
          _arg0 = android.media.session.ISession2TokensListener.Stub.asInterface(data.readStrongBinder());
          this.removeSession2TokensListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerRemoteVolumeController:
        {
          data.enforceInterface(descriptor);
          android.media.IRemoteVolumeController _arg0;
          _arg0 = android.media.IRemoteVolumeController.Stub.asInterface(data.readStrongBinder());
          this.registerRemoteVolumeController(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterRemoteVolumeController:
        {
          data.enforceInterface(descriptor);
          android.media.IRemoteVolumeController _arg0;
          _arg0 = android.media.IRemoteVolumeController.Stub.asInterface(data.readStrongBinder());
          this.unregisterRemoteVolumeController(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isGlobalPriorityActive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isGlobalPriorityActive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setCallback:
        {
          data.enforceInterface(descriptor);
          android.media.session.ICallback _arg0;
          _arg0 = android.media.session.ICallback.Stub.asInterface(data.readStrongBinder());
          this.setCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setOnVolumeKeyLongPressListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.IOnVolumeKeyLongPressListener _arg0;
          _arg0 = android.media.session.IOnVolumeKeyLongPressListener.Stub.asInterface(data.readStrongBinder());
          this.setOnVolumeKeyLongPressListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setOnMediaKeyListener:
        {
          data.enforceInterface(descriptor);
          android.media.session.IOnMediaKeyListener _arg0;
          _arg0 = android.media.session.IOnMediaKeyListener.Stub.asInterface(data.readStrongBinder());
          this.setOnMediaKeyListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTrusted:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.isTrusted(_arg0, _arg1, _arg2);
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
    private static class Proxy implements android.media.session.ISessionManager
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
      @Override public android.media.session.ISession createSession(java.lang.String packageName, android.media.session.ISessionCallback sessionCb, java.lang.String tag, android.os.Bundle sessionInfo, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.session.ISession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((sessionCb!=null))?(sessionCb.asBinder()):(null)));
          _data.writeString(tag);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createSession(packageName, sessionCb, tag, sessionInfo, userId);
          }
          _reply.readException();
          _result = android.media.session.ISession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void notifySession2Created(android.media.Session2Token sessionToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifySession2Created, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifySession2Created(sessionToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.media.session.MediaSession.Token> getSessions(android.content.ComponentName compName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.media.session.MediaSession.Token> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((compName!=null)) {
            _data.writeInt(1);
            compName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSessions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSessions(compName, userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.media.session.MediaSession.Token.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.pm.ParceledListSlice getSession2Tokens(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSession2Tokens, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSession2Tokens(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      @Override public void dispatchMediaKeyEvent(java.lang.String packageName, boolean asSystemService, android.view.KeyEvent keyEvent, boolean needWakeLock) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((asSystemService)?(1):(0)));
          if ((keyEvent!=null)) {
            _data.writeInt(1);
            keyEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((needWakeLock)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchMediaKeyEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchMediaKeyEvent(packageName, asSystemService, keyEvent, needWakeLock);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean dispatchMediaKeyEventToSessionAsSystemService(java.lang.String packageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((keyEvent!=null)) {
            _data.writeInt(1);
            keyEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchMediaKeyEventToSessionAsSystemService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().dispatchMediaKeyEventToSessionAsSystemService(packageName, sessionToken, keyEvent);
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
      @Override public void dispatchVolumeKeyEvent(java.lang.String packageName, java.lang.String opPackageName, boolean asSystemService, android.view.KeyEvent keyEvent, int stream, boolean musicOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          _data.writeInt(((asSystemService)?(1):(0)));
          if ((keyEvent!=null)) {
            _data.writeInt(1);
            keyEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(stream);
          _data.writeInt(((musicOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchVolumeKeyEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchVolumeKeyEvent(packageName, opPackageName, asSystemService, keyEvent, stream, musicOnly);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dispatchVolumeKeyEventToSessionAsSystemService(java.lang.String packageName, java.lang.String opPackageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          if ((sessionToken!=null)) {
            _data.writeInt(1);
            sessionToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((keyEvent!=null)) {
            _data.writeInt(1);
            keyEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchVolumeKeyEventToSessionAsSystemService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchVolumeKeyEventToSessionAsSystemService(packageName, opPackageName, sessionToken, keyEvent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dispatchAdjustVolume(java.lang.String packageName, java.lang.String opPackageName, int suggestedStream, int delta, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(opPackageName);
          _data.writeInt(suggestedStream);
          _data.writeInt(delta);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchAdjustVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchAdjustVolume(packageName, opPackageName, suggestedStream, delta, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addSessionsListener(android.media.session.IActiveSessionsListener listener, android.content.ComponentName compName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          if ((compName!=null)) {
            _data.writeInt(1);
            compName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSessionsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSessionsListener(listener, compName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeSessionsListener(android.media.session.IActiveSessionsListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSessionsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSessionsListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addSession2TokensListener(android.media.session.ISession2TokensListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSession2TokensListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSession2TokensListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeSession2TokensListener(android.media.session.ISession2TokensListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSession2TokensListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSession2TokensListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((rvc!=null))?(rvc.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerRemoteVolumeController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerRemoteVolumeController(rvc);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((rvc!=null))?(rvc.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterRemoteVolumeController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterRemoteVolumeController(rvc);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // For PhoneWindowManager to precheck media keys

      @Override public boolean isGlobalPriorityActive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isGlobalPriorityActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isGlobalPriorityActive();
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
      @Override public void setCallback(android.media.session.ICallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setOnVolumeKeyLongPressListener(android.media.session.IOnVolumeKeyLongPressListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOnVolumeKeyLongPressListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOnVolumeKeyLongPressListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setOnMediaKeyListener(android.media.session.IOnMediaKeyListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOnMediaKeyListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOnMediaKeyListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isTrusted(java.lang.String controllerPackageName, int controllerPid, int controllerUid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(controllerPackageName);
          _data.writeInt(controllerPid);
          _data.writeInt(controllerUid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTrusted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTrusted(controllerPackageName, controllerPid, controllerUid);
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
      public static android.media.session.ISessionManager sDefaultImpl;
    }
    static final int TRANSACTION_createSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_notifySession2Created = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getSessions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getSession2Tokens = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_dispatchMediaKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_dispatchMediaKeyEventToSessionAsSystemService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_dispatchVolumeKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_dispatchVolumeKeyEventToSessionAsSystemService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_dispatchAdjustVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_addSessionsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_removeSessionsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addSession2TokensListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_removeSession2TokensListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_registerRemoteVolumeController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_unregisterRemoteVolumeController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_isGlobalPriorityActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setOnVolumeKeyLongPressListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setOnMediaKeyListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_isTrusted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    public static boolean setDefaultImpl(android.media.session.ISessionManager impl) {
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
    public static android.media.session.ISessionManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.media.session.ISession createSession(java.lang.String packageName, android.media.session.ISessionCallback sessionCb, java.lang.String tag, android.os.Bundle sessionInfo, int userId) throws android.os.RemoteException;
  public void notifySession2Created(android.media.Session2Token sessionToken) throws android.os.RemoteException;
  public java.util.List<android.media.session.MediaSession.Token> getSessions(android.content.ComponentName compName, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getSession2Tokens(int userId) throws android.os.RemoteException;
  public void dispatchMediaKeyEvent(java.lang.String packageName, boolean asSystemService, android.view.KeyEvent keyEvent, boolean needWakeLock) throws android.os.RemoteException;
  public boolean dispatchMediaKeyEventToSessionAsSystemService(java.lang.String packageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException;
  public void dispatchVolumeKeyEvent(java.lang.String packageName, java.lang.String opPackageName, boolean asSystemService, android.view.KeyEvent keyEvent, int stream, boolean musicOnly) throws android.os.RemoteException;
  public void dispatchVolumeKeyEventToSessionAsSystemService(java.lang.String packageName, java.lang.String opPackageName, android.media.session.MediaSession.Token sessionToken, android.view.KeyEvent keyEvent) throws android.os.RemoteException;
  public void dispatchAdjustVolume(java.lang.String packageName, java.lang.String opPackageName, int suggestedStream, int delta, int flags) throws android.os.RemoteException;
  public void addSessionsListener(android.media.session.IActiveSessionsListener listener, android.content.ComponentName compName, int userId) throws android.os.RemoteException;
  public void removeSessionsListener(android.media.session.IActiveSessionsListener listener) throws android.os.RemoteException;
  public void addSession2TokensListener(android.media.session.ISession2TokensListener listener, int userId) throws android.os.RemoteException;
  public void removeSession2TokensListener(android.media.session.ISession2TokensListener listener) throws android.os.RemoteException;
  public void registerRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException;
  public void unregisterRemoteVolumeController(android.media.IRemoteVolumeController rvc) throws android.os.RemoteException;
  // For PhoneWindowManager to precheck media keys

  public boolean isGlobalPriorityActive() throws android.os.RemoteException;
  public void setCallback(android.media.session.ICallback callback) throws android.os.RemoteException;
  public void setOnVolumeKeyLongPressListener(android.media.session.IOnVolumeKeyLongPressListener listener) throws android.os.RemoteException;
  public void setOnMediaKeyListener(android.media.session.IOnMediaKeyListener listener) throws android.os.RemoteException;
  public boolean isTrusted(java.lang.String controllerPackageName, int controllerPid, int controllerUid) throws android.os.RemoteException;
}
