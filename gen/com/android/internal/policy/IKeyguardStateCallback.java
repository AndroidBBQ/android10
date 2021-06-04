/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.policy;
public interface IKeyguardStateCallback extends android.os.IInterface
{
  /** Default implementation for IKeyguardStateCallback. */
  public static class Default implements com.android.internal.policy.IKeyguardStateCallback
  {
    @Override public void onShowingStateChanged(boolean showing) throws android.os.RemoteException
    {
    }
    @Override public void onSimSecureStateChanged(boolean simSecure) throws android.os.RemoteException
    {
    }
    @Override public void onInputRestrictedStateChanged(boolean inputRestricted) throws android.os.RemoteException
    {
    }
    @Override public void onTrustedChanged(boolean trusted) throws android.os.RemoteException
    {
    }
    @Override public void onHasLockscreenWallpaperChanged(boolean hasLockscreenWallpaper) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.policy.IKeyguardStateCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.policy.IKeyguardStateCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.policy.IKeyguardStateCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.policy.IKeyguardStateCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.policy.IKeyguardStateCallback))) {
        return ((com.android.internal.policy.IKeyguardStateCallback)iin);
      }
      return new com.android.internal.policy.IKeyguardStateCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onShowingStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onShowingStateChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onSimSecureStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onSimSecureStateChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onInputRestrictedStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onInputRestrictedStateChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onTrustedChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onTrustedChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onHasLockscreenWallpaperChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onHasLockscreenWallpaperChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.policy.IKeyguardStateCallback
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
      @Override public void onShowingStateChanged(boolean showing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShowingStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShowingStateChanged(showing);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onSimSecureStateChanged(boolean simSecure) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((simSecure)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSimSecureStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSimSecureStateChanged(simSecure);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onInputRestrictedStateChanged(boolean inputRestricted) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((inputRestricted)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInputRestrictedStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInputRestrictedStateChanged(inputRestricted);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onTrustedChanged(boolean trusted) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((trusted)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustedChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustedChanged(trusted);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onHasLockscreenWallpaperChanged(boolean hasLockscreenWallpaper) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hasLockscreenWallpaper)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHasLockscreenWallpaperChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHasLockscreenWallpaperChanged(hasLockscreenWallpaper);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.policy.IKeyguardStateCallback sDefaultImpl;
    }
    static final int TRANSACTION_onShowingStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSimSecureStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onInputRestrictedStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onTrustedChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onHasLockscreenWallpaperChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.internal.policy.IKeyguardStateCallback impl) {
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
    public static com.android.internal.policy.IKeyguardStateCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onShowingStateChanged(boolean showing) throws android.os.RemoteException;
  public void onSimSecureStateChanged(boolean simSecure) throws android.os.RemoteException;
  public void onInputRestrictedStateChanged(boolean inputRestricted) throws android.os.RemoteException;
  public void onTrustedChanged(boolean trusted) throws android.os.RemoteException;
  public void onHasLockscreenWallpaperChanged(boolean hasLockscreenWallpaper) throws android.os.RemoteException;
}
