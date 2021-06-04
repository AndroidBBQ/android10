/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.net;
/** {@hide} */
public interface INetworkWatchlistManager extends android.os.IInterface
{
  /** Default implementation for INetworkWatchlistManager. */
  public static class Default implements com.android.internal.net.INetworkWatchlistManager
  {
    @Override public boolean startWatchlistLogging() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean stopWatchlistLogging() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void reloadWatchlist() throws android.os.RemoteException
    {
    }
    @Override public void reportWatchlistIfNecessary() throws android.os.RemoteException
    {
    }
    @Override public byte[] getWatchlistConfigHash() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.net.INetworkWatchlistManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.net.INetworkWatchlistManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.net.INetworkWatchlistManager interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.net.INetworkWatchlistManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.net.INetworkWatchlistManager))) {
        return ((com.android.internal.net.INetworkWatchlistManager)iin);
      }
      return new com.android.internal.net.INetworkWatchlistManager.Stub.Proxy(obj);
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
        case TRANSACTION_startWatchlistLogging:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.startWatchlistLogging();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopWatchlistLogging:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.stopWatchlistLogging();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_reloadWatchlist:
        {
          data.enforceInterface(descriptor);
          this.reloadWatchlist();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportWatchlistIfNecessary:
        {
          data.enforceInterface(descriptor);
          this.reportWatchlistIfNecessary();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWatchlistConfigHash:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getWatchlistConfigHash();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.net.INetworkWatchlistManager
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
      @Override public boolean startWatchlistLogging() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchlistLogging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startWatchlistLogging();
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
      @Override public boolean stopWatchlistLogging() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWatchlistLogging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopWatchlistLogging();
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
      @Override public void reloadWatchlist() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reloadWatchlist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reloadWatchlist();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportWatchlistIfNecessary() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportWatchlistIfNecessary, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportWatchlistIfNecessary();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public byte[] getWatchlistConfigHash() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWatchlistConfigHash, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWatchlistConfigHash();
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.net.INetworkWatchlistManager sDefaultImpl;
    }
    static final int TRANSACTION_startWatchlistLogging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopWatchlistLogging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_reloadWatchlist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_reportWatchlistIfNecessary = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getWatchlistConfigHash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.internal.net.INetworkWatchlistManager impl) {
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
    public static com.android.internal.net.INetworkWatchlistManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean startWatchlistLogging() throws android.os.RemoteException;
  public boolean stopWatchlistLogging() throws android.os.RemoteException;
  public void reloadWatchlist() throws android.os.RemoteException;
  public void reportWatchlistIfNecessary() throws android.os.RemoteException;
  public byte[] getWatchlistConfigHash() throws android.os.RemoteException;
}
