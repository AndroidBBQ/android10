/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.nfc;
/**
 * @hide
 */
public interface INfcCardEmulation extends android.os.IInterface
{
  /** Default implementation for INfcCardEmulation. */
  public static class Default implements android.nfc.INfcCardEmulation
  {
    @Override public boolean isDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isDefaultServiceForAid(int userHandle, android.content.ComponentName service, java.lang.String aid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setDefaultForNextTap(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean registerAidGroupForService(int userHandle, android.content.ComponentName service, android.nfc.cardemulation.AidGroup aidGroup) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setOffHostForService(int userHandle, android.content.ComponentName service, java.lang.String offHostSecureElement) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean unsetOffHostForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.nfc.cardemulation.AidGroup getAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean removeAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.nfc.cardemulation.ApduServiceInfo> getServices(int userHandle, java.lang.String category) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setPreferredService(android.content.ComponentName service) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean unsetPreferredService() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean supportsAidPrefixRegistration() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.nfc.INfcCardEmulation
  {
    private static final java.lang.String DESCRIPTOR = "android.nfc.INfcCardEmulation";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.nfc.INfcCardEmulation interface,
     * generating a proxy if needed.
     */
    public static android.nfc.INfcCardEmulation asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.nfc.INfcCardEmulation))) {
        return ((android.nfc.INfcCardEmulation)iin);
      }
      return new android.nfc.INfcCardEmulation.Stub.Proxy(obj);
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
        case TRANSACTION_isDefaultServiceForCategory:
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
          boolean _result = this.isDefaultServiceForCategory(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDefaultServiceForAid:
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
          boolean _result = this.isDefaultServiceForAid(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDefaultServiceForCategory:
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
          boolean _result = this.setDefaultServiceForCategory(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDefaultForNextTap:
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
          boolean _result = this.setDefaultForNextTap(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerAidGroupForService:
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
          android.nfc.cardemulation.AidGroup _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.nfc.cardemulation.AidGroup.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.registerAidGroupForService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setOffHostForService:
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
          boolean _result = this.setOffHostForService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unsetOffHostForService:
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
          boolean _result = this.unsetOffHostForService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAidGroupForService:
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
          android.nfc.cardemulation.AidGroup _result = this.getAidGroupForService(_arg0, _arg1, _arg2);
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
        case TRANSACTION_removeAidGroupForService:
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
          boolean _result = this.removeAidGroupForService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.nfc.cardemulation.ApduServiceInfo> _result = this.getServices(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setPreferredService:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.setPreferredService(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unsetPreferredService:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.unsetPreferredService();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supportsAidPrefixRegistration:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.supportsAidPrefixRegistration();
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
    private static class Proxy implements android.nfc.INfcCardEmulation
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
      @Override public boolean isDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
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
          _data.writeString(category);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDefaultServiceForCategory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDefaultServiceForCategory(userHandle, service, category);
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
      @Override public boolean isDefaultServiceForAid(int userHandle, android.content.ComponentName service, java.lang.String aid) throws android.os.RemoteException
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
          _data.writeString(aid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDefaultServiceForAid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDefaultServiceForAid(userHandle, service, aid);
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
      @Override public boolean setDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
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
          _data.writeString(category);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultServiceForCategory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDefaultServiceForCategory(userHandle, service, category);
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
      @Override public boolean setDefaultForNextTap(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultForNextTap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDefaultForNextTap(userHandle, service);
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
      @Override public boolean registerAidGroupForService(int userHandle, android.content.ComponentName service, android.nfc.cardemulation.AidGroup aidGroup) throws android.os.RemoteException
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
          if ((aidGroup!=null)) {
            _data.writeInt(1);
            aidGroup.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAidGroupForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerAidGroupForService(userHandle, service, aidGroup);
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
      @Override public boolean setOffHostForService(int userHandle, android.content.ComponentName service, java.lang.String offHostSecureElement) throws android.os.RemoteException
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
          _data.writeString(offHostSecureElement);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOffHostForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setOffHostForService(userHandle, service, offHostSecureElement);
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
      @Override public boolean unsetOffHostForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_unsetOffHostForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unsetOffHostForService(userHandle, service);
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
      @Override public android.nfc.cardemulation.AidGroup getAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.nfc.cardemulation.AidGroup _result;
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
          _data.writeString(category);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAidGroupForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAidGroupForService(userHandle, service, category);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.nfc.cardemulation.AidGroup.CREATOR.createFromParcel(_reply);
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
      @Override public boolean removeAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException
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
          _data.writeString(category);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAidGroupForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeAidGroupForService(userHandle, service, category);
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
      @Override public java.util.List<android.nfc.cardemulation.ApduServiceInfo> getServices(int userHandle, java.lang.String category) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.nfc.cardemulation.ApduServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          _data.writeString(category);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServices(userHandle, category);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.nfc.cardemulation.ApduServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean setPreferredService(android.content.ComponentName service) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPreferredService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setPreferredService(service);
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
      @Override public boolean unsetPreferredService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unsetPreferredService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unsetPreferredService();
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
      @Override public boolean supportsAidPrefixRegistration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supportsAidPrefixRegistration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supportsAidPrefixRegistration();
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
      public static android.nfc.INfcCardEmulation sDefaultImpl;
    }
    static final int TRANSACTION_isDefaultServiceForCategory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isDefaultServiceForAid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setDefaultServiceForCategory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setDefaultForNextTap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_registerAidGroupForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setOffHostForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_unsetOffHostForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getAidGroupForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_removeAidGroupForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setPreferredService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_unsetPreferredService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_supportsAidPrefixRegistration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.nfc.INfcCardEmulation impl) {
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
    public static android.nfc.INfcCardEmulation getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException;
  public boolean isDefaultServiceForAid(int userHandle, android.content.ComponentName service, java.lang.String aid) throws android.os.RemoteException;
  public boolean setDefaultServiceForCategory(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException;
  public boolean setDefaultForNextTap(int userHandle, android.content.ComponentName service) throws android.os.RemoteException;
  public boolean registerAidGroupForService(int userHandle, android.content.ComponentName service, android.nfc.cardemulation.AidGroup aidGroup) throws android.os.RemoteException;
  public boolean setOffHostForService(int userHandle, android.content.ComponentName service, java.lang.String offHostSecureElement) throws android.os.RemoteException;
  public boolean unsetOffHostForService(int userHandle, android.content.ComponentName service) throws android.os.RemoteException;
  public android.nfc.cardemulation.AidGroup getAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException;
  public boolean removeAidGroupForService(int userHandle, android.content.ComponentName service, java.lang.String category) throws android.os.RemoteException;
  public java.util.List<android.nfc.cardemulation.ApduServiceInfo> getServices(int userHandle, java.lang.String category) throws android.os.RemoteException;
  public boolean setPreferredService(android.content.ComponentName service) throws android.os.RemoteException;
  public boolean unsetPreferredService() throws android.os.RemoteException;
  public boolean supportsAidPrefixRegistration() throws android.os.RemoteException;
}
