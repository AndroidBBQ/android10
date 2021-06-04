/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.nfc;
/**
 * @hide
 */
public interface INfcFCardEmulation extends android.os.IInterface
{
  /** Default implementation for INfcFCardEmulation. */
  public static class Default implements android.nfc.INfcFCardEmulation
  {
    @Override public java.lang.String getSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean registerSystemCodeForService(int userHandle, android.content.ComponentName service, java.lang.String systemCode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String getNfcid2ForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setNfcid2ForService(int userHandle, android.content.ComponentName service, java.lang.String nfcid2) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean enableNfcFForegroundService(android.content.ComponentName service) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean disableNfcFForegroundService() throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.nfc.cardemulation.NfcFServiceInfo> getNfcFServices(int userHandle) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getMaxNumOfRegisterableSystemCodes() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.nfc.INfcFCardEmulation
  {
    private static final java.lang.String DESCRIPTOR = "android.nfc.INfcFCardEmulation";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.nfc.INfcFCardEmulation interface,
     * generating a proxy if needed.
     */
    public static android.nfc.INfcFCardEmulation asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.nfc.INfcFCardEmulation))) {
        return ((android.nfc.INfcFCardEmulation)iin);
      }
      return new android.nfc.INfcFCardEmulation.Stub.Proxy(obj);
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
        case TRANSACTION_getSystemCodeForService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _result = this.getSystemCodeForService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_registerSystemCodeForService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.registerSystemCodeForService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeSystemCodeForService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.removeSystemCodeForService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNfcid2ForService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _result = this.getNfcid2ForService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setNfcid2ForService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.setNfcid2ForService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableNfcFForegroundService:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.enableNfcFForegroundService(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableNfcFForegroundService:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.disableNfcFForegroundService();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNfcFServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.nfc.cardemulation.NfcFServiceInfo> _result = this.getNfcFServices(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getMaxNumOfRegisterableSystemCodes:
        {
          data.enforceInterface(descriptor);
          int _result = this.getMaxNumOfRegisterableSystemCodes();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.nfc.INfcFCardEmulation
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
      @Override public java.lang.String getSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSystemCodeForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSystemCodeForService(userHandle, service);
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
      @Override public boolean registerSystemCodeForService(int userHandle, android.content.ComponentName service, java.lang.String systemCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(systemCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerSystemCodeForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerSystemCodeForService(userHandle, service, systemCode);
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
      @Override public boolean removeSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSystemCodeForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeSystemCodeForService(userHandle, service);
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
      @Override public java.lang.String getNfcid2ForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcid2ForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcid2ForService(userHandle, service);
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
      @Override public boolean setNfcid2ForService(int userHandle, android.content.ComponentName service, java.lang.String nfcid2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(nfcid2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNfcid2ForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNfcid2ForService(userHandle, service, nfcid2);
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
      @Override public boolean enableNfcFForegroundService(android.content.ComponentName service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableNfcFForegroundService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableNfcFForegroundService(service);
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
      @Override public boolean disableNfcFForegroundService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableNfcFForegroundService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disableNfcFForegroundService();
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
      @Override public java.util.List<android.nfc.cardemulation.NfcFServiceInfo> getNfcFServices(int userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.nfc.cardemulation.NfcFServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNfcFServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNfcFServices(userHandle);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.nfc.cardemulation.NfcFServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getMaxNumOfRegisterableSystemCodes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMaxNumOfRegisterableSystemCodes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMaxNumOfRegisterableSystemCodes();
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
      public static android.nfc.INfcFCardEmulation sDefaultImpl;
    }
    static final int TRANSACTION_getSystemCodeForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerSystemCodeForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeSystemCodeForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getNfcid2ForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setNfcid2ForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_enableNfcFForegroundService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_disableNfcFForegroundService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getNfcFServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getMaxNumOfRegisterableSystemCodes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.nfc.INfcFCardEmulation impl) {
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
    public static android.nfc.INfcFCardEmulation getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.lang.String getSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException;
  public boolean registerSystemCodeForService(int userHandle, android.content.ComponentName service, java.lang.String systemCode) throws android.os.RemoteException;
  public boolean removeSystemCodeForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException;
  public java.lang.String getNfcid2ForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException;
  public boolean setNfcid2ForService(int userHandle, android.content.ComponentName service, java.lang.String nfcid2) throws android.os.RemoteException;
  public boolean enableNfcFForegroundService(android.content.ComponentName service) throws android.os.RemoteException;
  public boolean disableNfcFForegroundService() throws android.os.RemoteException;
  public java.util.List<android.nfc.cardemulation.NfcFServiceInfo> getNfcFServices(int userHandle) throws android.os.RemoteException;
  public int getMaxNumOfRegisterableSystemCodes() throws android.os.RemoteException;
}
