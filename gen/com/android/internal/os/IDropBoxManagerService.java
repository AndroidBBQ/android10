/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.os;
/**
 * "Backend" interface used by {@link android.os.DropBoxManager} to talk to the
 * DropBoxManagerService that actually implements the drop box functionality.
 *
 * @see DropBoxManager
 * @hide
 */
public interface IDropBoxManagerService extends android.os.IInterface
{
  /** Default implementation for IDropBoxManagerService. */
  public static class Default implements com.android.internal.os.IDropBoxManagerService
  {
    /**
         * @see DropBoxManager#addText
         * @see DropBoxManager#addData
         * @see DropBoxManager#addFile
         */
    @Override public void add(android.os.DropBoxManager.Entry entry) throws android.os.RemoteException
    {
    }
    /** @see DropBoxManager#getNextEntry */
    @Override public boolean isTagEnabled(java.lang.String tag) throws android.os.RemoteException
    {
      return false;
    }
    /** @see DropBoxManager#getNextEntry */
    @Override public android.os.DropBoxManager.Entry getNextEntry(java.lang.String tag, long millis, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.os.IDropBoxManagerService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.os.IDropBoxManagerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.os.IDropBoxManagerService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.os.IDropBoxManagerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.os.IDropBoxManagerService))) {
        return ((com.android.internal.os.IDropBoxManagerService)iin);
      }
      return new com.android.internal.os.IDropBoxManagerService.Stub.Proxy(obj);
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
        case TRANSACTION_add:
        {
          data.enforceInterface(descriptor);
          android.os.DropBoxManager.Entry _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.DropBoxManager.Entry.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.add(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTagEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isTagEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getNextEntry:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.DropBoxManager.Entry _result = this.getNextEntry(_arg0, _arg1, _arg2);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.os.IDropBoxManagerService
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
      /**
           * @see DropBoxManager#addText
           * @see DropBoxManager#addData
           * @see DropBoxManager#addFile
           */
      @Override public void add(android.os.DropBoxManager.Entry entry) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((entry!=null)) {
            _data.writeInt(1);
            entry.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_add, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().add(entry);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** @see DropBoxManager#getNextEntry */
      @Override public boolean isTagEnabled(java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTagEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTagEnabled(tag);
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
      /** @see DropBoxManager#getNextEntry */
      @Override public android.os.DropBoxManager.Entry getNextEntry(java.lang.String tag, long millis, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.DropBoxManager.Entry _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(tag);
          _data.writeLong(millis);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNextEntry, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNextEntry(tag, millis, packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.DropBoxManager.Entry.CREATOR.createFromParcel(_reply);
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
      public static com.android.internal.os.IDropBoxManagerService sDefaultImpl;
    }
    static final int TRANSACTION_add = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isTagEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getNextEntry = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.os.IDropBoxManagerService impl) {
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
    public static com.android.internal.os.IDropBoxManagerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * @see DropBoxManager#addText
       * @see DropBoxManager#addData
       * @see DropBoxManager#addFile
       */
  public void add(android.os.DropBoxManager.Entry entry) throws android.os.RemoteException;
  /** @see DropBoxManager#getNextEntry */
  public boolean isTagEnabled(java.lang.String tag) throws android.os.RemoteException;
  /** @see DropBoxManager#getNextEntry */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/os/IDropBoxManagerService.aidl:40:1:40:25")
  public android.os.DropBoxManager.Entry getNextEntry(java.lang.String tag, long millis, java.lang.String packageName) throws android.os.RemoteException;
}
