/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
public interface IMediaContainerService extends android.os.IInterface
{
  /** Default implementation for IMediaContainerService. */
  public static class Default implements com.android.internal.app.IMediaContainerService
  {
    @Override public int copyPackage(java.lang.String packagePath, com.android.internal.os.IParcelFileDescriptorFactory target) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.content.pm.PackageInfoLite getMinimalPackageInfo(java.lang.String packagePath, int flags, java.lang.String abiOverride) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.res.ObbInfo getObbInfo(java.lang.String filename) throws android.os.RemoteException
    {
      return null;
    }
    @Override public long calculateInstalledSize(java.lang.String packagePath, java.lang.String abiOverride) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IMediaContainerService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IMediaContainerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IMediaContainerService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IMediaContainerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IMediaContainerService))) {
        return ((com.android.internal.app.IMediaContainerService)iin);
      }
      return new com.android.internal.app.IMediaContainerService.Stub.Proxy(obj);
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
        case TRANSACTION_copyPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.os.IParcelFileDescriptorFactory _arg1;
          _arg1 = com.android.internal.os.IParcelFileDescriptorFactory.Stub.asInterface(data.readStrongBinder());
          int _result = this.copyPackage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMinimalPackageInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.pm.PackageInfoLite _result = this.getMinimalPackageInfo(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getObbInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.res.ObbInfo _result = this.getObbInfo(_arg0);
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
        case TRANSACTION_calculateInstalledSize:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.calculateInstalledSize(_arg0, _arg1);
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
    private static class Proxy implements com.android.internal.app.IMediaContainerService
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
      @Override public int copyPackage(java.lang.String packagePath, com.android.internal.os.IParcelFileDescriptorFactory target) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packagePath);
          _data.writeStrongBinder((((target!=null))?(target.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_copyPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().copyPackage(packagePath, target);
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
      @Override public android.content.pm.PackageInfoLite getMinimalPackageInfo(java.lang.String packagePath, int flags, java.lang.String abiOverride) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.PackageInfoLite _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packagePath);
          _data.writeInt(flags);
          _data.writeString(abiOverride);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMinimalPackageInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMinimalPackageInfo(packagePath, flags, abiOverride);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.PackageInfoLite.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.res.ObbInfo getObbInfo(java.lang.String filename) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.res.ObbInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(filename);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getObbInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getObbInfo(filename);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.res.ObbInfo.CREATOR.createFromParcel(_reply);
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
      @Override public long calculateInstalledSize(java.lang.String packagePath, java.lang.String abiOverride) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packagePath);
          _data.writeString(abiOverride);
          boolean _status = mRemote.transact(Stub.TRANSACTION_calculateInstalledSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().calculateInstalledSize(packagePath, abiOverride);
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
      public static com.android.internal.app.IMediaContainerService sDefaultImpl;
    }
    static final int TRANSACTION_copyPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getMinimalPackageInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getObbInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_calculateInstalledSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.app.IMediaContainerService impl) {
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
    public static com.android.internal.app.IMediaContainerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int copyPackage(java.lang.String packagePath, com.android.internal.os.IParcelFileDescriptorFactory target) throws android.os.RemoteException;
  public android.content.pm.PackageInfoLite getMinimalPackageInfo(java.lang.String packagePath, int flags, java.lang.String abiOverride) throws android.os.RemoteException;
  public android.content.res.ObbInfo getObbInfo(java.lang.String filename) throws android.os.RemoteException;
  public long calculateInstalledSize(java.lang.String packagePath, java.lang.String abiOverride) throws android.os.RemoteException;
}
