/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.cts.verifier.projection;
public interface IProjectionService extends android.os.IInterface
{
  /** Default implementation for IProjectionService. */
  public static class Default implements com.android.cts.verifier.projection.IProjectionService
  {
    @Override public void startRendering(android.view.Surface surface, int width, int height, int density, int viewType) throws android.os.RemoteException
    {
    }
    @Override public void stopRendering() throws android.os.RemoteException
    {
    }
    @Override public void onTouchEvent(android.view.MotionEvent event) throws android.os.RemoteException
    {
    }
    @Override public void onKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.cts.verifier.projection.IProjectionService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.cts.verifier.projection.IProjectionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.cts.verifier.projection.IProjectionService interface,
     * generating a proxy if needed.
     */
    public static com.android.cts.verifier.projection.IProjectionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.cts.verifier.projection.IProjectionService))) {
        return ((com.android.cts.verifier.projection.IProjectionService)iin);
      }
      return new com.android.cts.verifier.projection.IProjectionService.Stub.Proxy(obj);
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
        case TRANSACTION_startRendering:
        {
          data.enforceInterface(descriptor);
          android.view.Surface _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.startRendering(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopRendering:
        {
          data.enforceInterface(descriptor);
          this.stopRendering();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onTouchEvent:
        {
          data.enforceInterface(descriptor);
          android.view.MotionEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.MotionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onTouchEvent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onKeyEvent:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onKeyEvent(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.cts.verifier.projection.IProjectionService
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
      @Override public void startRendering(android.view.Surface surface, int width, int height, int density, int viewType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(width);
          _data.writeInt(height);
          _data.writeInt(density);
          _data.writeInt(viewType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRendering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startRendering(surface, width, height, density, viewType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopRendering() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRendering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopRendering();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onTouchEvent(android.view.MotionEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTouchEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTouchEvent(event);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onKeyEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onKeyEvent(event);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.cts.verifier.projection.IProjectionService sDefaultImpl;
    }
    static final int TRANSACTION_startRendering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopRendering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onTouchEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.cts.verifier.projection.IProjectionService impl) {
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
    public static com.android.cts.verifier.projection.IProjectionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startRendering(android.view.Surface surface, int width, int height, int density, int viewType) throws android.os.RemoteException;
  public void stopRendering() throws android.os.RemoteException;
  public void onTouchEvent(android.view.MotionEvent event) throws android.os.RemoteException;
  public void onKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException;
}
