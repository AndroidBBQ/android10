/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.location;
/**
 * Binder interface for manager of all location providers.
 * @hide
 */
public interface ILocationProviderManager extends android.os.IInterface
{
  /** Default implementation for ILocationProviderManager. */
  public static class Default implements com.android.internal.location.ILocationProviderManager
  {
    @Override public void onSetAdditionalProviderPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException
    {
    }
    @Override public void onSetEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void onSetProperties(com.android.internal.location.ProviderProperties properties) throws android.os.RemoteException
    {
    }
    @Override public void onReportLocation(android.location.Location location) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.location.ILocationProviderManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.location.ILocationProviderManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.location.ILocationProviderManager interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.location.ILocationProviderManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.location.ILocationProviderManager))) {
        return ((com.android.internal.location.ILocationProviderManager)iin);
      }
      return new com.android.internal.location.ILocationProviderManager.Stub.Proxy(obj);
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
        case TRANSACTION_onSetAdditionalProviderPackages:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          this.onSetAdditionalProviderPackages(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onSetEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onSetEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onSetProperties:
        {
          data.enforceInterface(descriptor);
          com.android.internal.location.ProviderProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.internal.location.ProviderProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSetProperties(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onReportLocation:
        {
          data.enforceInterface(descriptor);
          android.location.Location _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Location.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onReportLocation(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.location.ILocationProviderManager
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
      @Override public void onSetAdditionalProviderPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetAdditionalProviderPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetAdditionalProviderPackages(packageNames);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onSetEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onSetProperties(com.android.internal.location.ProviderProperties properties) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((properties!=null)) {
            _data.writeInt(1);
            properties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetProperties(properties);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onReportLocation(android.location.Location location) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReportLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReportLocation(location);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.location.ILocationProviderManager sDefaultImpl;
    }
    static final int TRANSACTION_onSetAdditionalProviderPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSetEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSetProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onReportLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.location.ILocationProviderManager impl) {
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
    public static com.android.internal.location.ILocationProviderManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSetAdditionalProviderPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/com/android/internal/location/ILocationProviderManager.aidl:31:1:31:25")
  public void onSetEnabled(boolean enabled) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/com/android/internal/location/ILocationProviderManager.aidl:34:1:34:25")
  public void onSetProperties(com.android.internal.location.ProviderProperties properties) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/com/android/internal/location/ILocationProviderManager.aidl:37:1:37:25")
  public void onReportLocation(android.location.Location location) throws android.os.RemoteException;
}
