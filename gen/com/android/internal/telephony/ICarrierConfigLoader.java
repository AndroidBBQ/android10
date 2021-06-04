/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
/**
 * Interface used to interact with the CarrierConfigLoader
 */
public interface ICarrierConfigLoader extends android.os.IInterface
{
  /** Default implementation for ICarrierConfigLoader. */
  public static class Default implements com.android.internal.telephony.ICarrierConfigLoader
  {
    @Override public android.os.PersistableBundle getConfigForSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void overrideConfig(int subId, android.os.PersistableBundle overrides) throws android.os.RemoteException
    {
    }
    @Override public void notifyConfigChangedForSubId(int subId) throws android.os.RemoteException
    {
    }
    @Override public void updateConfigForPhoneId(int phoneId, java.lang.String simState) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getDefaultCarrierServicePackageName() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.ICarrierConfigLoader
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.ICarrierConfigLoader";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.ICarrierConfigLoader interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.ICarrierConfigLoader asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.ICarrierConfigLoader))) {
        return ((com.android.internal.telephony.ICarrierConfigLoader)iin);
      }
      return new com.android.internal.telephony.ICarrierConfigLoader.Stub.Proxy(obj);
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
        case TRANSACTION_getConfigForSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.PersistableBundle _result = this.getConfigForSubId(_arg0, _arg1);
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
        case TRANSACTION_overrideConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.PersistableBundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.PersistableBundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.overrideConfig(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyConfigChangedForSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyConfigChangedForSubId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateConfigForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.updateConfigForPhoneId(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDefaultCarrierServicePackageName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getDefaultCarrierServicePackageName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.ICarrierConfigLoader
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
      @Override public android.os.PersistableBundle getConfigForSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.PersistableBundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigForSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigForSubId(subId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.PersistableBundle.CREATOR.createFromParcel(_reply);
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
      @Override public void overrideConfig(int subId, android.os.PersistableBundle overrides) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          if ((overrides!=null)) {
            _data.writeInt(1);
            overrides.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_overrideConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().overrideConfig(subId, overrides);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyConfigChangedForSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyConfigChangedForSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyConfigChangedForSubId(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateConfigForPhoneId(int phoneId, java.lang.String simState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeString(simState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateConfigForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateConfigForPhoneId(phoneId, simState);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getDefaultCarrierServicePackageName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultCarrierServicePackageName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultCarrierServicePackageName();
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
      public static com.android.internal.telephony.ICarrierConfigLoader sDefaultImpl;
    }
    static final int TRANSACTION_getConfigForSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_overrideConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyConfigChangedForSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_updateConfigForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getDefaultCarrierServicePackageName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.internal.telephony.ICarrierConfigLoader impl) {
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
    public static com.android.internal.telephony.ICarrierConfigLoader getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ICarrierConfigLoader.aidl:26:1:26:25")
  public android.os.PersistableBundle getConfigForSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void overrideConfig(int subId, android.os.PersistableBundle overrides) throws android.os.RemoteException;
  public void notifyConfigChangedForSubId(int subId) throws android.os.RemoteException;
  public void updateConfigForPhoneId(int phoneId, java.lang.String simState) throws android.os.RemoteException;
  public java.lang.String getDefaultCarrierServicePackageName() throws android.os.RemoteException;
}
