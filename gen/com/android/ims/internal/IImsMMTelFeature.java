/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * See MMTelFeature for more information.
 * {@hide}
 */
public interface IImsMMTelFeature extends android.os.IInterface
{
  /** Default implementation for IImsMMTelFeature. */
  public static class Default implements com.android.ims.internal.IImsMMTelFeature
  {
    @Override public int startSession(android.app.PendingIntent incomingCallIntent, com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void endSession(int sessionId) throws android.os.RemoteException
    {
    }
    @Override public boolean isConnected(int callSessionType, int callType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isOpened() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getFeatureStatus() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void addRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
    {
    }
    @Override public android.telephony.ims.ImsCallProfile createCallProfile(int sessionId, int callSessionType, int callType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsCallSession createCallSession(int sessionId, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsCallSession getPendingCallSession(int sessionId, java.lang.String callId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsUt getUtInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsConfig getConfigInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void turnOnIms() throws android.os.RemoteException
    {
    }
    @Override public void turnOffIms() throws android.os.RemoteException
    {
    }
    @Override public com.android.ims.internal.IImsEcbm getEcbmInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setUiTTYMode(int uiTtyMode, android.os.Message onComplete) throws android.os.RemoteException
    {
    }
    @Override public com.android.ims.internal.IImsMultiEndpoint getMultiEndpointInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsMMTelFeature
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsMMTelFeature";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsMMTelFeature interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsMMTelFeature asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsMMTelFeature))) {
        return ((com.android.ims.internal.IImsMMTelFeature)iin);
      }
      return new com.android.ims.internal.IImsMMTelFeature.Stub.Proxy(obj);
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
        case TRANSACTION_startSession:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          com.android.ims.internal.IImsRegistrationListener _arg1;
          _arg1 = com.android.ims.internal.IImsRegistrationListener.Stub.asInterface(data.readStrongBinder());
          int _result = this.startSession(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_endSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.endSession(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isConnected:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isConnected(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isOpened:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isOpened();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getFeatureStatus:
        {
          data.enforceInterface(descriptor);
          int _result = this.getFeatureStatus();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addRegistrationListener:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsRegistrationListener _arg0;
          _arg0 = com.android.ims.internal.IImsRegistrationListener.Stub.asInterface(data.readStrongBinder());
          this.addRegistrationListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeRegistrationListener:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsRegistrationListener _arg0;
          _arg0 = com.android.ims.internal.IImsRegistrationListener.Stub.asInterface(data.readStrongBinder());
          this.removeRegistrationListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createCallProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.telephony.ims.ImsCallProfile _result = this.createCallProfile(_arg0, _arg1, _arg2);
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
        case TRANSACTION_createCallSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.ImsCallProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          com.android.ims.internal.IImsCallSession _result = this.createCallSession(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getPendingCallSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.ims.internal.IImsCallSession _result = this.getPendingCallSession(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getUtInterface:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _result = this.getUtInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getConfigInterface:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsConfig _result = this.getConfigInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_turnOnIms:
        {
          data.enforceInterface(descriptor);
          this.turnOnIms();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_turnOffIms:
        {
          data.enforceInterface(descriptor);
          this.turnOffIms();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEcbmInterface:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsEcbm _result = this.getEcbmInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_setUiTTYMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Message _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Message.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setUiTTYMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMultiEndpointInterface:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsMultiEndpoint _result = this.getMultiEndpointInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsMMTelFeature
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
      @Override public int startSession(android.app.PendingIntent incomingCallIntent, com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((incomingCallIntent!=null)) {
            _data.writeInt(1);
            incomingCallIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startSession(incomingCallIntent, listener);
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
      @Override public void endSession(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_endSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().endSession(sessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isConnected(int callSessionType, int callType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callSessionType);
          _data.writeInt(callType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isConnected, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isConnected(callSessionType, callType);
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
      @Override public boolean isOpened() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOpened, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOpened();
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
      @Override public int getFeatureStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFeatureStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFeatureStatus();
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
      @Override public void addRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addRegistrationListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addRegistrationListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRegistrationListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeRegistrationListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.telephony.ims.ImsCallProfile createCallProfile(int sessionId, int callSessionType, int callType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.ImsCallProfile _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeInt(callSessionType);
          _data.writeInt(callType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createCallProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createCallProfile(sessionId, callSessionType, callType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(_reply);
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
      @Override public com.android.ims.internal.IImsCallSession createCallSession(int sessionId, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsCallSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createCallSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createCallSession(sessionId, profile);
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsCallSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.ims.internal.IImsCallSession getPendingCallSession(int sessionId, java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsCallSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPendingCallSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPendingCallSession(sessionId, callId);
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsCallSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.ims.internal.IImsUt getUtInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsUt _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUtInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUtInterface();
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsUt.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.ims.internal.IImsConfig getConfigInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigInterface();
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsConfig.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void turnOnIms() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_turnOnIms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().turnOnIms();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void turnOffIms() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_turnOffIms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().turnOffIms();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public com.android.ims.internal.IImsEcbm getEcbmInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsEcbm _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEcbmInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEcbmInterface();
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsEcbm.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setUiTTYMode(int uiTtyMode, android.os.Message onComplete) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uiTtyMode);
          if ((onComplete!=null)) {
            _data.writeInt(1);
            onComplete.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUiTTYMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUiTTYMode(uiTtyMode, onComplete);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public com.android.ims.internal.IImsMultiEndpoint getMultiEndpointInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsMultiEndpoint _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMultiEndpointInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMultiEndpointInterface();
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsMultiEndpoint.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.ims.internal.IImsMMTelFeature sDefaultImpl;
    }
    static final int TRANSACTION_startSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_endSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isOpened = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getFeatureStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addRegistrationListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeRegistrationListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_createCallProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_createCallSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getPendingCallSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getUtInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getConfigInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_turnOnIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_turnOffIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getEcbmInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setUiTTYMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getMultiEndpointInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsMMTelFeature impl) {
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
    public static com.android.ims.internal.IImsMMTelFeature getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int startSession(android.app.PendingIntent incomingCallIntent, com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException;
  public void endSession(int sessionId) throws android.os.RemoteException;
  public boolean isConnected(int callSessionType, int callType) throws android.os.RemoteException;
  public boolean isOpened() throws android.os.RemoteException;
  public int getFeatureStatus() throws android.os.RemoteException;
  public void addRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException;
  public void removeRegistrationListener(com.android.ims.internal.IImsRegistrationListener listener) throws android.os.RemoteException;
  public android.telephony.ims.ImsCallProfile createCallProfile(int sessionId, int callSessionType, int callType) throws android.os.RemoteException;
  public com.android.ims.internal.IImsCallSession createCallSession(int sessionId, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public com.android.ims.internal.IImsCallSession getPendingCallSession(int sessionId, java.lang.String callId) throws android.os.RemoteException;
  public com.android.ims.internal.IImsUt getUtInterface() throws android.os.RemoteException;
  public com.android.ims.internal.IImsConfig getConfigInterface() throws android.os.RemoteException;
  public void turnOnIms() throws android.os.RemoteException;
  public void turnOffIms() throws android.os.RemoteException;
  public com.android.ims.internal.IImsEcbm getEcbmInterface() throws android.os.RemoteException;
  public void setUiTTYMode(int uiTtyMode, android.os.Message onComplete) throws android.os.RemoteException;
  public com.android.ims.internal.IImsMultiEndpoint getMultiEndpointInterface() throws android.os.RemoteException;
}
