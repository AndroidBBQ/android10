/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
public interface IBinderThreadPriorityService extends android.os.IInterface
{
  /** Default implementation for IBinderThreadPriorityService. */
  public static class Default implements android.os.IBinderThreadPriorityService
  {
    @Override public int getThreadPriority() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getThreadSchedulerGroup() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void callBack(android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException
    {
    }
    @Override public void setPriorityAndCallBack(int priority, android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IBinderThreadPriorityService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IBinderThreadPriorityService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IBinderThreadPriorityService interface,
     * generating a proxy if needed.
     */
    public static android.os.IBinderThreadPriorityService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IBinderThreadPriorityService))) {
        return ((android.os.IBinderThreadPriorityService)iin);
      }
      return new android.os.IBinderThreadPriorityService.Stub.Proxy(obj);
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
        case TRANSACTION_getThreadPriority:
        {
          data.enforceInterface(descriptor);
          int _result = this.getThreadPriority();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getThreadSchedulerGroup:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getThreadSchedulerGroup();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_callBack:
        {
          data.enforceInterface(descriptor);
          android.os.IBinderThreadPriorityService _arg0;
          _arg0 = android.os.IBinderThreadPriorityService.Stub.asInterface(data.readStrongBinder());
          this.callBack(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPriorityAndCallBack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinderThreadPriorityService _arg1;
          _arg1 = android.os.IBinderThreadPriorityService.Stub.asInterface(data.readStrongBinder());
          this.setPriorityAndCallBack(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IBinderThreadPriorityService
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
      @Override public int getThreadPriority() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getThreadPriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getThreadPriority();
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
      @Override public java.lang.String getThreadSchedulerGroup() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getThreadSchedulerGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getThreadSchedulerGroup();
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
      @Override public void callBack(android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((recurse!=null))?(recurse.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_callBack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callBack(recurse);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPriorityAndCallBack(int priority, android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(priority);
          _data.writeStrongBinder((((recurse!=null))?(recurse.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPriorityAndCallBack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPriorityAndCallBack(priority, recurse);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IBinderThreadPriorityService sDefaultImpl;
    }
    static final int TRANSACTION_getThreadPriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getThreadSchedulerGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_callBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setPriorityAndCallBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.os.IBinderThreadPriorityService impl) {
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
    public static android.os.IBinderThreadPriorityService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int getThreadPriority() throws android.os.RemoteException;
  public java.lang.String getThreadSchedulerGroup() throws android.os.RemoteException;
  public void callBack(android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException;
  public void setPriorityAndCallBack(int priority, android.os.IBinderThreadPriorityService recurse) throws android.os.RemoteException;
}
