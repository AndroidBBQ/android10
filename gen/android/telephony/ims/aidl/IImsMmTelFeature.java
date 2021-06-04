/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See MmTelFeature for more information.
 * {@hide}
 */
public interface IImsMmTelFeature extends android.os.IInterface
{
  /** Default implementation for IImsMmTelFeature. */
  public static class Default implements android.telephony.ims.aidl.IImsMmTelFeature
  {
    @Override public void setListener(android.telephony.ims.aidl.IImsMmTelListener l) throws android.os.RemoteException
    {
    }
    @Override public int getFeatureState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.telephony.ims.ImsCallProfile createCallProfile(int callSessionType, int callType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsCallSession createCallSession(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int shouldProcessCall(java.lang.String[] uris) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public com.android.ims.internal.IImsUt getUtInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsEcbm getEcbmInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setUiTtyMode(int uiTtyMode, android.os.Message onCompleteMessage) throws android.os.RemoteException
    {
    }
    @Override public com.android.ims.internal.IImsMultiEndpoint getMultiEndpointInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int queryCapabilityStatus() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void addCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    @Override public void removeCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    @Override public void changeCapabilitiesConfiguration(android.telephony.ims.feature.CapabilityChangeRequest request, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    @Override public void queryCapabilityConfiguration(int capability, int radioTech, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    // SMS APIs

    @Override public void setSmsListener(android.telephony.ims.aidl.IImsSmsListener l) throws android.os.RemoteException
    {
    }
    @Override public void sendSms(int token, int messageRef, java.lang.String format, java.lang.String smsc, boolean retry, byte[] pdu) throws android.os.RemoteException
    {
    }
    @Override public void acknowledgeSms(int token, int messageRef, int result) throws android.os.RemoteException
    {
    }
    @Override public void acknowledgeSmsReport(int token, int messageRef, int result) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getSmsFormat() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void onSmsReady() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsMmTelFeature
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsMmTelFeature";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsMmTelFeature interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsMmTelFeature asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsMmTelFeature))) {
        return ((android.telephony.ims.aidl.IImsMmTelFeature)iin);
      }
      return new android.telephony.ims.aidl.IImsMmTelFeature.Stub.Proxy(obj);
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
        case TRANSACTION_setListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsMmTelListener _arg0;
          _arg0 = android.telephony.ims.aidl.IImsMmTelListener.Stub.asInterface(data.readStrongBinder());
          this.setListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFeatureState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getFeatureState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createCallProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsCallProfile _result = this.createCallProfile(_arg0, _arg1);
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
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          com.android.ims.internal.IImsCallSession _result = this.createCallSession(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_shouldProcessCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          int _result = this.shouldProcessCall(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
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
        case TRANSACTION_getEcbmInterface:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsEcbm _result = this.getEcbmInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_setUiTtyMode:
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
          this.setUiTtyMode(_arg0, _arg1);
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
        case TRANSACTION_queryCapabilityStatus:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCapabilityStatus();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addCapabilityCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsCapabilityCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.addCapabilityCallback(_arg0);
          return true;
        }
        case TRANSACTION_removeCapabilityCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsCapabilityCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.removeCapabilityCallback(_arg0);
          return true;
        }
        case TRANSACTION_changeCapabilitiesConfiguration:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.feature.CapabilityChangeRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.feature.CapabilityChangeRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.ims.aidl.IImsCapabilityCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.changeCapabilitiesConfiguration(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_queryCapabilityConfiguration:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.aidl.IImsCapabilityCallback _arg2;
          _arg2 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.queryCapabilityConfiguration(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setSmsListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsSmsListener _arg0;
          _arg0 = android.telephony.ims.aidl.IImsSmsListener.Stub.asInterface(data.readStrongBinder());
          this.setSmsListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendSms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          byte[] _arg5;
          _arg5 = data.createByteArray();
          this.sendSms(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_acknowledgeSms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.acknowledgeSms(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_acknowledgeSmsReport:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.acknowledgeSmsReport(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getSmsFormat:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getSmsFormat();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_onSmsReady:
        {
          data.enforceInterface(descriptor);
          this.onSmsReady();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsMmTelFeature
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
      @Override public void setListener(android.telephony.ims.aidl.IImsMmTelListener l) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((l!=null))?(l.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setListener(l);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getFeatureState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFeatureState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFeatureState();
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
      @Override public android.telephony.ims.ImsCallProfile createCallProfile(int callSessionType, int callType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.ImsCallProfile _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callSessionType);
          _data.writeInt(callType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createCallProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createCallProfile(callSessionType, callType);
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
      @Override public com.android.ims.internal.IImsCallSession createCallSession(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsCallSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createCallSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createCallSession(profile);
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
      @Override public int shouldProcessCall(java.lang.String[] uris) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(uris);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldProcessCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldProcessCall(uris);
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
      @Override public void setUiTtyMode(int uiTtyMode, android.os.Message onCompleteMessage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uiTtyMode);
          if ((onCompleteMessage!=null)) {
            _data.writeInt(1);
            onCompleteMessage.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUiTtyMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUiTtyMode(uiTtyMode, onCompleteMessage);
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
      @Override public int queryCapabilityStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCapabilityStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCapabilityStatus();
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
      @Override public void addCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCapabilityCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addCapabilityCallback(c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeCapabilityCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeCapabilityCallback(c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void changeCapabilitiesConfiguration(android.telephony.ims.feature.CapabilityChangeRequest request, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_changeCapabilitiesConfiguration, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().changeCapabilitiesConfiguration(request, c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void queryCapabilityConfiguration(int capability, int radioTech, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(capability);
          _data.writeInt(radioTech);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCapabilityConfiguration, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().queryCapabilityConfiguration(capability, radioTech, c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // SMS APIs

      @Override public void setSmsListener(android.telephony.ims.aidl.IImsSmsListener l) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((l!=null))?(l.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSmsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSmsListener(l);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendSms(int token, int messageRef, java.lang.String format, java.lang.String smsc, boolean retry, byte[] pdu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(messageRef);
          _data.writeString(format);
          _data.writeString(smsc);
          _data.writeInt(((retry)?(1):(0)));
          _data.writeByteArray(pdu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendSms(token, messageRef, format, smsc, retry, pdu);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void acknowledgeSms(int token, int messageRef, int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(messageRef);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acknowledgeSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acknowledgeSms(token, messageRef, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void acknowledgeSmsReport(int token, int messageRef, int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(messageRef);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acknowledgeSmsReport, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acknowledgeSmsReport(token, messageRef, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public java.lang.String getSmsFormat() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSmsFormat, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSmsFormat();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void onSmsReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSmsReady, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSmsReady();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsMmTelFeature sDefaultImpl;
    }
    static final int TRANSACTION_setListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getFeatureState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createCallProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_createCallSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_shouldProcessCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getUtInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getEcbmInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setUiTtyMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getMultiEndpointInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_queryCapabilityStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_addCapabilityCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_removeCapabilityCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_changeCapabilitiesConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_queryCapabilityConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setSmsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_sendSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_acknowledgeSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_acknowledgeSmsReport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getSmsFormat = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_onSmsReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsMmTelFeature impl) {
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
    public static android.telephony.ims.aidl.IImsMmTelFeature getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setListener(android.telephony.ims.aidl.IImsMmTelListener l) throws android.os.RemoteException;
  public int getFeatureState() throws android.os.RemoteException;
  public android.telephony.ims.ImsCallProfile createCallProfile(int callSessionType, int callType) throws android.os.RemoteException;
  public com.android.ims.internal.IImsCallSession createCallSession(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public int shouldProcessCall(java.lang.String[] uris) throws android.os.RemoteException;
  public com.android.ims.internal.IImsUt getUtInterface() throws android.os.RemoteException;
  public com.android.ims.internal.IImsEcbm getEcbmInterface() throws android.os.RemoteException;
  public void setUiTtyMode(int uiTtyMode, android.os.Message onCompleteMessage) throws android.os.RemoteException;
  public com.android.ims.internal.IImsMultiEndpoint getMultiEndpointInterface() throws android.os.RemoteException;
  public int queryCapabilityStatus() throws android.os.RemoteException;
  public void addCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  public void removeCapabilityCallback(android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  public void changeCapabilitiesConfiguration(android.telephony.ims.feature.CapabilityChangeRequest request, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  public void queryCapabilityConfiguration(int capability, int radioTech, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  // SMS APIs

  public void setSmsListener(android.telephony.ims.aidl.IImsSmsListener l) throws android.os.RemoteException;
  public void sendSms(int token, int messageRef, java.lang.String format, java.lang.String smsc, boolean retry, byte[] pdu) throws android.os.RemoteException;
  public void acknowledgeSms(int token, int messageRef, int result) throws android.os.RemoteException;
  public void acknowledgeSmsReport(int token, int messageRef, int result) throws android.os.RemoteException;
  public java.lang.String getSmsFormat() throws android.os.RemoteException;
  public void onSmsReady() throws android.os.RemoteException;
}
