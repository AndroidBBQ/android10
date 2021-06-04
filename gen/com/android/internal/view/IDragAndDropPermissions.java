/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Interface to allow a drop receiver to request permissions for URIs passed along with ClipData
 * contained in DragEvent.
 */
public interface IDragAndDropPermissions extends android.os.IInterface
{
  /** Default implementation for IDragAndDropPermissions. */
  public static class Default implements com.android.internal.view.IDragAndDropPermissions
  {
    @Override public void take(android.os.IBinder activityToken) throws android.os.RemoteException
    {
    }
    @Override public void takeTransient(android.os.IBinder transientToken) throws android.os.RemoteException
    {
    }
    @Override public void release() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IDragAndDropPermissions
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IDragAndDropPermissions";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IDragAndDropPermissions interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IDragAndDropPermissions asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IDragAndDropPermissions))) {
        return ((com.android.internal.view.IDragAndDropPermissions)iin);
      }
      return new com.android.internal.view.IDragAndDropPermissions.Stub.Proxy(obj);
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
        case TRANSACTION_take:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.take(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_takeTransient:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.takeTransient(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_release:
        {
          data.enforceInterface(descriptor);
          this.release();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IDragAndDropPermissions
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
      @Override public void take(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_take, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().take(activityToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void takeTransient(android.os.IBinder transientToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(transientToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_takeTransient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().takeTransient(transientToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void release() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_release, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().release();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.view.IDragAndDropPermissions sDefaultImpl;
    }
    static final int TRANSACTION_take = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_takeTransient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_release = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.view.IDragAndDropPermissions impl) {
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
    public static com.android.internal.view.IDragAndDropPermissions getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void take(android.os.IBinder activityToken) throws android.os.RemoteException;
  public void takeTransient(android.os.IBinder transientToken) throws android.os.RemoteException;
  public void release() throws android.os.RemoteException;
}
