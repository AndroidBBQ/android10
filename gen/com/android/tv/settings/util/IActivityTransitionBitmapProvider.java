/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.tv.settings.util;
/** {@hide} */
public interface IActivityTransitionBitmapProvider extends android.os.IInterface
{
  /** Default implementation for IActivityTransitionBitmapProvider. */
  public static class Default implements com.android.tv.settings.util.IActivityTransitionBitmapProvider
  {
    @Override public android.graphics.Bitmap getTransitionBitmap() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.tv.settings.util.IActivityTransitionBitmapProvider
  {
    private static final java.lang.String DESCRIPTOR = "com.android.tv.settings.util.IActivityTransitionBitmapProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.tv.settings.util.IActivityTransitionBitmapProvider interface,
     * generating a proxy if needed.
     */
    public static com.android.tv.settings.util.IActivityTransitionBitmapProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.tv.settings.util.IActivityTransitionBitmapProvider))) {
        return ((com.android.tv.settings.util.IActivityTransitionBitmapProvider)iin);
      }
      return new com.android.tv.settings.util.IActivityTransitionBitmapProvider.Stub.Proxy(obj);
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
        case TRANSACTION_getTransitionBitmap:
        {
          data.enforceInterface(descriptor);
          android.graphics.Bitmap _result = this.getTransitionBitmap();
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
    private static class Proxy implements com.android.tv.settings.util.IActivityTransitionBitmapProvider
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
      @Override public android.graphics.Bitmap getTransitionBitmap() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Bitmap _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTransitionBitmap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTransitionBitmap();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
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
      public static com.android.tv.settings.util.IActivityTransitionBitmapProvider sDefaultImpl;
    }
    static final int TRANSACTION_getTransitionBitmap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.tv.settings.util.IActivityTransitionBitmapProvider impl) {
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
    public static com.android.tv.settings.util.IActivityTransitionBitmapProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.graphics.Bitmap getTransitionBitmap() throws android.os.RemoteException;
}
