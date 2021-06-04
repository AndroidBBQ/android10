/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.emailcommon.service;
public interface IPolicyService extends android.os.IInterface
{
  /** Default implementation for IPolicyService. */
  public static class Default implements com.android.emailcommon.service.IPolicyService
  {
    @Override public boolean isActive(com.android.emailcommon.provider.Policy policies) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setAccountHoldFlag(long accountId, boolean newState) throws android.os.RemoteException
    {
    }
    // Legacy compatability for Exchange shipped with KK

    @Override public void setAccountPolicy(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey) throws android.os.RemoteException
    {
    }
    // New version

    @Override public void setAccountPolicy2(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey, boolean notify) throws android.os.RemoteException
    {
    }
    @Override public void remoteWipe() throws android.os.RemoteException
    {
    }
    @Override public boolean canDisableCamera() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.emailcommon.service.IPolicyService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.emailcommon.service.IPolicyService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.emailcommon.service.IPolicyService interface,
     * generating a proxy if needed.
     */
    public static com.android.emailcommon.service.IPolicyService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.emailcommon.service.IPolicyService))) {
        return ((com.android.emailcommon.service.IPolicyService)iin);
      }
      return new com.android.emailcommon.service.IPolicyService.Stub.Proxy(obj);
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
        case TRANSACTION_isActive:
        {
          data.enforceInterface(descriptor);
          com.android.emailcommon.provider.Policy _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.emailcommon.provider.Policy.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isActive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAccountHoldFlag:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setAccountHoldFlag(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAccountPolicy:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          com.android.emailcommon.provider.Policy _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.emailcommon.provider.Policy.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setAccountPolicy(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAccountPolicy2:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          com.android.emailcommon.provider.Policy _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.emailcommon.provider.Policy.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.setAccountPolicy2(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_remoteWipe:
        {
          data.enforceInterface(descriptor);
          this.remoteWipe();
          return true;
        }
        case TRANSACTION_canDisableCamera:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.canDisableCamera();
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
    private static class Proxy implements com.android.emailcommon.service.IPolicyService
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
      @Override public boolean isActive(com.android.emailcommon.provider.Policy policies) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((policies!=null)) {
            _data.writeInt(1);
            policies.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActive(policies);
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
      @Override public void setAccountHoldFlag(long accountId, boolean newState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          _data.writeInt(((newState)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAccountHoldFlag, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAccountHoldFlag(accountId, newState);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Legacy compatability for Exchange shipped with KK

      @Override public void setAccountPolicy(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          if ((policy!=null)) {
            _data.writeInt(1);
            policy.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(securityKey);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAccountPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAccountPolicy(accountId, policy, securityKey);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // New version

      @Override public void setAccountPolicy2(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey, boolean notify) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          if ((policy!=null)) {
            _data.writeInt(1);
            policy.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(securityKey);
          _data.writeInt(((notify)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAccountPolicy2, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAccountPolicy2(accountId, policy, securityKey, notify);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void remoteWipe() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_remoteWipe, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remoteWipe();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public boolean canDisableCamera() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canDisableCamera, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canDisableCamera();
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
      public static com.android.emailcommon.service.IPolicyService sDefaultImpl;
    }
    static final int TRANSACTION_isActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setAccountHoldFlag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setAccountPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setAccountPolicy2 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_remoteWipe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_canDisableCamera = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(com.android.emailcommon.service.IPolicyService impl) {
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
    public static com.android.emailcommon.service.IPolicyService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isActive(com.android.emailcommon.provider.Policy policies) throws android.os.RemoteException;
  public void setAccountHoldFlag(long accountId, boolean newState) throws android.os.RemoteException;
  // Legacy compatability for Exchange shipped with KK

  public void setAccountPolicy(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey) throws android.os.RemoteException;
  // New version

  public void setAccountPolicy2(long accountId, com.android.emailcommon.provider.Policy policy, java.lang.String securityKey, boolean notify) throws android.os.RemoteException;
  public void remoteWipe() throws android.os.RemoteException;
  public boolean canDisableCamera() throws android.os.RemoteException;
}
