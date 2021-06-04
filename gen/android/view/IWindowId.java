/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/** {@hide} */
public interface IWindowId extends android.os.IInterface
{
  /** Default implementation for IWindowId. */
  public static class Default implements android.view.IWindowId
  {
    @Override public void registerFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException
    {
    }
    @Override public void unregisterFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException
    {
    }
    @Override public boolean isFocused() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IWindowId
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IWindowId";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IWindowId interface,
     * generating a proxy if needed.
     */
    public static android.view.IWindowId asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IWindowId))) {
        return ((android.view.IWindowId)iin);
      }
      return new android.view.IWindowId.Stub.Proxy(obj);
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
        case TRANSACTION_registerFocusObserver:
        {
          data.enforceInterface(descriptor);
          android.view.IWindowFocusObserver _arg0;
          _arg0 = android.view.IWindowFocusObserver.Stub.asInterface(data.readStrongBinder());
          this.registerFocusObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterFocusObserver:
        {
          data.enforceInterface(descriptor);
          android.view.IWindowFocusObserver _arg0;
          _arg0 = android.view.IWindowFocusObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterFocusObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isFocused:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isFocused();
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
    private static class Proxy implements android.view.IWindowId
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
      @Override public void registerFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerFocusObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerFocusObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterFocusObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterFocusObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isFocused() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFocused, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isFocused();
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
      public static android.view.IWindowId sDefaultImpl;
    }
    static final int TRANSACTION_registerFocusObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterFocusObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isFocused = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.view.IWindowId impl) {
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
    public static android.view.IWindowId getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException;
  public void unregisterFocusObserver(android.view.IWindowFocusObserver observer) throws android.os.RemoteException;
  public boolean isFocused() throws android.os.RemoteException;
}
