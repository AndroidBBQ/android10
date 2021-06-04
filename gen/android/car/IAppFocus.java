/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/** @hide */
public interface IAppFocus extends android.os.IInterface
{
  /** Default implementation for IAppFocus. */
  public static class Default implements android.car.IAppFocus
  {
    @Override public void registerFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException
    {
    }
    @Override public void unregisterFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException
    {
    }
    @Override public int[] getActiveAppTypes() throws android.os.RemoteException
    {
      return null;
    }
    /** callback used as a token */
    @Override public boolean isOwningFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
    {
      return false;
    }
    /** callback used as a token */
    @Override public int requestAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
    {
      return 0;
    }
    /** callback used as a token */
    @Override public void abandonAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.IAppFocus
  {
    private static final java.lang.String DESCRIPTOR = "android.car.IAppFocus";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.IAppFocus interface,
     * generating a proxy if needed.
     */
    public static android.car.IAppFocus asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.IAppFocus))) {
        return ((android.car.IAppFocus)iin);
      }
      return new android.car.IAppFocus.Stub.Proxy(obj);
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
        case TRANSACTION_registerFocusListener:
        {
          data.enforceInterface(descriptor);
          android.car.IAppFocusListener _arg0;
          _arg0 = android.car.IAppFocusListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.registerFocusListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterFocusListener:
        {
          data.enforceInterface(descriptor);
          android.car.IAppFocusListener _arg0;
          _arg0 = android.car.IAppFocusListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.unregisterFocusListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActiveAppTypes:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getActiveAppTypes();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_isOwningFocus:
        {
          data.enforceInterface(descriptor);
          android.car.IAppFocusOwnershipCallback _arg0;
          _arg0 = android.car.IAppFocusOwnershipCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isOwningFocus(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestAppFocus:
        {
          data.enforceInterface(descriptor);
          android.car.IAppFocusOwnershipCallback _arg0;
          _arg0 = android.car.IAppFocusOwnershipCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.requestAppFocus(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_abandonAppFocus:
        {
          data.enforceInterface(descriptor);
          android.car.IAppFocusOwnershipCallback _arg0;
          _arg0 = android.car.IAppFocusOwnershipCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.abandonAppFocus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.IAppFocus
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
      @Override public void registerFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerFocusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerFocusListener(callback, appType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterFocusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterFocusListener(callback, appType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getActiveAppTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveAppTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveAppTypes();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** callback used as a token */
      @Override public boolean isOwningFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOwningFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOwningFocus(callback, appType);
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
      /** callback used as a token */
      @Override public int requestAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAppFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestAppFocus(callback, appType);
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
      /** callback used as a token */
      @Override public void abandonAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_abandonAppFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().abandonAppFocus(callback, appType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.IAppFocus sDefaultImpl;
    }
    static final int TRANSACTION_registerFocusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterFocusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getActiveAppTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isOwningFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_requestAppFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_abandonAppFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.car.IAppFocus impl) {
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
    public static android.car.IAppFocus getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException;
  public void unregisterFocusListener(android.car.IAppFocusListener callback, int appType) throws android.os.RemoteException;
  public int[] getActiveAppTypes() throws android.os.RemoteException;
  /** callback used as a token */
  public boolean isOwningFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException;
  /** callback used as a token */
  public int requestAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException;
  /** callback used as a token */
  public void abandonAppFocus(android.car.IAppFocusOwnershipCallback callback, int appType) throws android.os.RemoteException;
}
