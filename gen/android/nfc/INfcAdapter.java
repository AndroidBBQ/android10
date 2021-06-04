/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.nfc;
/**
 * @hide
 */
public interface INfcAdapter extends android.os.IInterface
{
  /** Default implementation for INfcAdapter. */
  public static class Default implements android.nfc.INfcAdapter
  {
    @Override public android.nfc.INfcTag getNfcTagInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.nfc.INfcCardEmulation getNfcCardEmulationInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.nfc.INfcFCardEmulation getNfcFCardEmulationInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.nfc.INfcAdapterExtras getNfcAdapterExtrasInterface(java.lang.String pkg) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.nfc.INfcDta getNfcDtaInterface(java.lang.String pkg) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean disable(boolean saveState) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean enable() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean enableNdefPush() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean disableNdefPush() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isNdefPushEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void pausePolling(int timeoutInMs) throws android.os.RemoteException
    {
    }
    @Override public void resumePolling() throws android.os.RemoteException
    {
    }
    @Override public void setForegroundDispatch(android.app.PendingIntent intent, android.content.IntentFilter[] filters, android.nfc.TechListParcel techLists) throws android.os.RemoteException
    {
    }
    @Override public void setAppCallback(android.nfc.IAppCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void invokeBeam() throws android.os.RemoteException
    {
    }
    @Override public void invokeBeamInternal(android.nfc.BeamShareData shareData) throws android.os.RemoteException
    {
    }
    @Override public boolean ignore(int nativeHandle, int debounceMs, android.nfc.ITagRemovedCallback callback) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void dispatch(android.nfc.Tag tag) throws android.os.RemoteException
    {
    }
    @Override public void setReaderMode(android.os.IBinder b, android.nfc.IAppCallback callback, int flags, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void setP2pModes(int initatorModes, int targetModes) throws android.os.RemoteException
    {
    }
    @Override public void addNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler, int[] techList) throws android.os.RemoteException
    {
    }
    @Override public void removeNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler) throws android.os.RemoteException
    {
    }
    @Override public void verifyNfcPermission() throws android.os.RemoteException
    {
    }
    @Override public boolean isNfcSecureEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean deviceSupportsNfcSecure() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setNfcSecure(boolean enable) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.nfc.INfcAdapter
  {
    private static final java.lang.String DESCRIPTOR = "android.nfc.INfcAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.nfc.INfcAdapter interface,
     * generating a proxy if needed.
     */
    public static android.nfc.INfcAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.nfc.INfcAdapter))) {
        return ((android.nfc.INfcAdapter)iin);
      }
      return new android.nfc.INfcAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_getNfcTagInterface:
        {
          data.enforceInterface(descriptor);
          android.nfc.INfcTag _result = this.getNfcTagInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getNfcCardEmulationInterface:
        {
          data.enforceInterface(descriptor);
          android.nfc.INfcCardEmulation _result = this.getNfcCardEmulationInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getNfcFCardEmulationInterface:
        {
          data.enforceInterface(descriptor);
          android.nfc.INfcFCardEmulation _result = this.getNfcFCardEmulationInterface();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getNfcAdapterExtrasInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.nfc.INfcAdapterExtras _result = this.getNfcAdapterExtrasInterface(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getNfcDtaInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.nfc.INfcDta _result = this.getNfcDtaInterface(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_disable:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.disable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.enable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableNdefPush:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.enableNdefPush();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableNdefPush:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.disableNdefPush();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isNdefPushEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isNdefPushEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_pausePolling:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.pausePolling(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resumePolling:
        {
          data.enforceInterface(descriptor);
          this.resumePolling();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setForegroundDispatch:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.IntentFilter[] _arg1;
          _arg1 = data.createTypedArray(android.content.IntentFilter.CREATOR);
          android.nfc.TechListParcel _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.nfc.TechListParcel.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setForegroundDispatch(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAppCallback:
        {
          data.enforceInterface(descriptor);
          android.nfc.IAppCallback _arg0;
          _arg0 = android.nfc.IAppCallback.Stub.asInterface(data.readStrongBinder());
          this.setAppCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_invokeBeam:
        {
          data.enforceInterface(descriptor);
          this.invokeBeam();
          return true;
        }
        case TRANSACTION_invokeBeamInternal:
        {
          data.enforceInterface(descriptor);
          android.nfc.BeamShareData _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.nfc.BeamShareData.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.invokeBeamInternal(_arg0);
          return true;
        }
        case TRANSACTION_ignore:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.nfc.ITagRemovedCallback _arg2;
          _arg2 = android.nfc.ITagRemovedCallback.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.ignore(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_dispatch:
        {
          data.enforceInterface(descriptor);
          android.nfc.Tag _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.nfc.Tag.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.dispatch(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setReaderMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.nfc.IAppCallback _arg1;
          _arg1 = android.nfc.IAppCallback.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.setReaderMode(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setP2pModes:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setP2pModes(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addNfcUnlockHandler:
        {
          data.enforceInterface(descriptor);
          android.nfc.INfcUnlockHandler _arg0;
          _arg0 = android.nfc.INfcUnlockHandler.Stub.asInterface(data.readStrongBinder());
          int[] _arg1;
          _arg1 = data.createIntArray();
          this.addNfcUnlockHandler(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeNfcUnlockHandler:
        {
          data.enforceInterface(descriptor);
          android.nfc.INfcUnlockHandler _arg0;
          _arg0 = android.nfc.INfcUnlockHandler.Stub.asInterface(data.readStrongBinder());
          this.removeNfcUnlockHandler(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_verifyNfcPermission:
        {
          data.enforceInterface(descriptor);
          this.verifyNfcPermission();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isNfcSecureEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isNfcSecureEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_deviceSupportsNfcSecure:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.deviceSupportsNfcSecure();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setNfcSecure:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setNfcSecure(_arg0);
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
    private static class Proxy implements android.nfc.INfcAdapter
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
      @Override public android.nfc.INfcTag getNfcTagInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.INfcTag _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcTagInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcTagInterface();
          }
          _reply.readException();
          _result = android.nfc.INfcTag.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.nfc.INfcCardEmulation getNfcCardEmulationInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.INfcCardEmulation _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcCardEmulationInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcCardEmulationInterface();
          }
          _reply.readException();
          _result = android.nfc.INfcCardEmulation.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.nfc.INfcFCardEmulation getNfcFCardEmulationInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.INfcFCardEmulation _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcFCardEmulationInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcFCardEmulationInterface();
          }
          _reply.readException();
          _result = android.nfc.INfcFCardEmulation.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.nfc.INfcAdapterExtras getNfcAdapterExtrasInterface(java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.INfcAdapterExtras _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcAdapterExtrasInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcAdapterExtrasInterface(pkg);
          }
          _reply.readException();
          _result = android.nfc.INfcAdapterExtras.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.nfc.INfcDta getNfcDtaInterface(java.lang.String pkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.INfcDta _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcDtaInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcDtaInterface(pkg);
          }
          _reply.readException();
          _result = android.nfc.INfcDta.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getState();
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
      @Override public boolean disable(boolean saveState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((saveState)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_disable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disable(saveState);
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
      @Override public boolean enable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enable();
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
      @Override public boolean enableNdefPush() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableNdefPush, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableNdefPush();
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
      @Override public boolean disableNdefPush() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableNdefPush, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disableNdefPush();
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
      @Override public boolean isNdefPushEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNdefPushEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNdefPushEnabled();
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
      @Override public void pausePolling(int timeoutInMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeoutInMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pausePolling, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pausePolling(timeoutInMs);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resumePolling() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resumePolling, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resumePolling();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setForegroundDispatch(android.app.PendingIntent intent, android.content.IntentFilter[] filters, android.nfc.TechListParcel techLists) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedArray(filters, 0);
          if ((techLists!=null)) {
            _data.writeInt(1);
            techLists.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setForegroundDispatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setForegroundDispatch(intent, filters, techLists);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAppCallback(android.nfc.IAppCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void invokeBeam() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_invokeBeam, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().invokeBeam();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void invokeBeamInternal(android.nfc.BeamShareData shareData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((shareData!=null)) {
            _data.writeInt(1);
            shareData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_invokeBeamInternal, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().invokeBeamInternal(shareData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public boolean ignore(int nativeHandle, int debounceMs, android.nfc.ITagRemovedCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(nativeHandle);
          _data.writeInt(debounceMs);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_ignore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().ignore(nativeHandle, debounceMs, callback);
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
      @Override public void dispatch(android.nfc.Tag tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tag!=null)) {
            _data.writeInt(1);
            tag.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatch(tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setReaderMode(android.os.IBinder b, android.nfc.IAppCallback callback, int flags, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(b);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setReaderMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setReaderMode(b, callback, flags, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setP2pModes(int initatorModes, int targetModes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(initatorModes);
          _data.writeInt(targetModes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setP2pModes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setP2pModes(initatorModes, targetModes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler, int[] techList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((unlockHandler!=null))?(unlockHandler.asBinder()):(null)));
          _data.writeIntArray(techList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNfcUnlockHandler, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addNfcUnlockHandler(unlockHandler, techList);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((unlockHandler!=null))?(unlockHandler.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNfcUnlockHandler, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeNfcUnlockHandler(unlockHandler);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void verifyNfcPermission() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_verifyNfcPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().verifyNfcPermission();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isNfcSecureEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNfcSecureEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNfcSecureEnabled();
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
      @Override public boolean deviceSupportsNfcSecure() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deviceSupportsNfcSecure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().deviceSupportsNfcSecure();
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
      @Override public boolean setNfcSecure(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNfcSecure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNfcSecure(enable);
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
      public static android.nfc.INfcAdapter sDefaultImpl;
    }
    static final int TRANSACTION_getNfcTagInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getNfcCardEmulationInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getNfcFCardEmulationInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getNfcAdapterExtrasInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getNfcDtaInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_disable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_enable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_enableNdefPush = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_disableNdefPush = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isNdefPushEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_pausePolling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_resumePolling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setForegroundDispatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setAppCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_invokeBeam = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_invokeBeamInternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_ignore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_dispatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setReaderMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setP2pModes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_addNfcUnlockHandler = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_removeNfcUnlockHandler = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_verifyNfcPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_isNfcSecureEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_deviceSupportsNfcSecure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_setNfcSecure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    public static boolean setDefaultImpl(android.nfc.INfcAdapter impl) {
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
    public static android.nfc.INfcAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.nfc.INfcTag getNfcTagInterface() throws android.os.RemoteException;
  public android.nfc.INfcCardEmulation getNfcCardEmulationInterface() throws android.os.RemoteException;
  public android.nfc.INfcFCardEmulation getNfcFCardEmulationInterface() throws android.os.RemoteException;
  public android.nfc.INfcAdapterExtras getNfcAdapterExtrasInterface(java.lang.String pkg) throws android.os.RemoteException;
  public android.nfc.INfcDta getNfcDtaInterface(java.lang.String pkg) throws android.os.RemoteException;
  public int getState() throws android.os.RemoteException;
  public boolean disable(boolean saveState) throws android.os.RemoteException;
  public boolean enable() throws android.os.RemoteException;
  public boolean enableNdefPush() throws android.os.RemoteException;
  public boolean disableNdefPush() throws android.os.RemoteException;
  public boolean isNdefPushEnabled() throws android.os.RemoteException;
  public void pausePolling(int timeoutInMs) throws android.os.RemoteException;
  public void resumePolling() throws android.os.RemoteException;
  public void setForegroundDispatch(android.app.PendingIntent intent, android.content.IntentFilter[] filters, android.nfc.TechListParcel techLists) throws android.os.RemoteException;
  public void setAppCallback(android.nfc.IAppCallback callback) throws android.os.RemoteException;
  public void invokeBeam() throws android.os.RemoteException;
  public void invokeBeamInternal(android.nfc.BeamShareData shareData) throws android.os.RemoteException;
  public boolean ignore(int nativeHandle, int debounceMs, android.nfc.ITagRemovedCallback callback) throws android.os.RemoteException;
  public void dispatch(android.nfc.Tag tag) throws android.os.RemoteException;
  public void setReaderMode(android.os.IBinder b, android.nfc.IAppCallback callback, int flags, android.os.Bundle extras) throws android.os.RemoteException;
  public void setP2pModes(int initatorModes, int targetModes) throws android.os.RemoteException;
  public void addNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler, int[] techList) throws android.os.RemoteException;
  public void removeNfcUnlockHandler(android.nfc.INfcUnlockHandler unlockHandler) throws android.os.RemoteException;
  public void verifyNfcPermission() throws android.os.RemoteException;
  public boolean isNfcSecureEnabled() throws android.os.RemoteException;
  public boolean deviceSupportsNfcSecure() throws android.os.RemoteException;
  public boolean setNfcSecure(boolean enable) throws android.os.RemoteException;
}
