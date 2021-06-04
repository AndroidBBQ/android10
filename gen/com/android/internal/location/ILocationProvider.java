/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.location;
/**
 * Binder interface for services that implement location providers. Do not implement this directly,
 * extend {@link LocationProviderBase} instead.
 * @hide
 */
public interface ILocationProvider extends android.os.IInterface
{
  /** Default implementation for ILocationProvider. */
  public static class Default implements com.android.internal.location.ILocationProvider
  {
    @Override public void setLocationProviderManager(com.android.internal.location.ILocationProviderManager manager) throws android.os.RemoteException
    {
    }
    @Override public void setRequest(com.android.internal.location.ProviderRequest request, android.os.WorkSource ws) throws android.os.RemoteException
    {
    }
    @Override public void sendExtraCommand(java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    // --- deprecated and will be removed the future ---

    @Override public int getStatus(android.os.Bundle extras) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public long getStatusUpdateTime() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.location.ILocationProvider
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.location.ILocationProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.location.ILocationProvider interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.location.ILocationProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.location.ILocationProvider))) {
        return ((com.android.internal.location.ILocationProvider)iin);
      }
      return new com.android.internal.location.ILocationProvider.Stub.Proxy(obj);
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
        case TRANSACTION_setLocationProviderManager:
        {
          data.enforceInterface(descriptor);
          com.android.internal.location.ILocationProviderManager _arg0;
          _arg0 = com.android.internal.location.ILocationProviderManager.Stub.asInterface(data.readStrongBinder());
          this.setLocationProviderManager(_arg0);
          return true;
        }
        case TRANSACTION_setRequest:
        {
          data.enforceInterface(descriptor);
          com.android.internal.location.ProviderRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.internal.location.ProviderRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendExtraCommand:
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
          this.sendExtraCommand(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getStatus:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          _arg0 = new android.os.Bundle();
          int _result = this.getStatus(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg0!=null)) {
            reply.writeInt(1);
            _arg0.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getStatusUpdateTime:
        {
          data.enforceInterface(descriptor);
          long _result = this.getStatusUpdateTime();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.location.ILocationProvider
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
      @Override public void setLocationProviderManager(com.android.internal.location.ILocationProviderManager manager) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((manager!=null))?(manager.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLocationProviderManager, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLocationProviderManager(manager);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRequest(com.android.internal.location.ProviderRequest request, android.os.WorkSource ws) throws android.os.RemoteException
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
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRequest(request, ws);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendExtraCommand(java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(command);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendExtraCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendExtraCommand(command, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // --- deprecated and will be removed the future ---

      @Override public int getStatus(android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStatus(extras);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            extras.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getStatusUpdateTime() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStatusUpdateTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStatusUpdateTime();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.location.ILocationProvider sDefaultImpl;
    }
    static final int TRANSACTION_setLocationProviderManager = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_sendExtraCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getStatusUpdateTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.internal.location.ILocationProvider impl) {
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
    public static com.android.internal.location.ILocationProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setLocationProviderManager(com.android.internal.location.ILocationProviderManager manager) throws android.os.RemoteException;
  public void setRequest(com.android.internal.location.ProviderRequest request, android.os.WorkSource ws) throws android.os.RemoteException;
  public void sendExtraCommand(java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException;
  // --- deprecated and will be removed the future ---

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/com/android/internal/location/ILocationProvider.aidl:39:1:39:25")
  public int getStatus(android.os.Bundle extras) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/com/android/internal/location/ILocationProvider.aidl:41:1:41:25")
  public long getStatusUpdateTime() throws android.os.RemoteException;
}
