/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.mediatek.tunerservice;
public interface IMtkTuner extends android.os.IInterface
{
  /** Default implementation for IMtkTuner. */
  public static class Default implements com.mediatek.tunerservice.IMtkTuner
  {
    @Override public boolean tune(int frequency, java.lang.String modulation, int timeOutMs) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void addPidFilter(int pid, int filterType) throws android.os.RemoteException
    {
    }
    @Override public void closeAllPidFilters() throws android.os.RemoteException
    {
    }
    @Override public void stopTune() throws android.os.RemoteException
    {
    }
    @Override public byte[] getTsData(int maxDataSize, int timeOutMs) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setHasPendingTune(boolean hasPendingTune) throws android.os.RemoteException
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
  public static abstract class Stub extends android.os.Binder implements com.mediatek.tunerservice.IMtkTuner
  {
    private static final java.lang.String DESCRIPTOR = "com.mediatek.tunerservice.IMtkTuner";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.mediatek.tunerservice.IMtkTuner interface,
     * generating a proxy if needed.
     */
    public static com.mediatek.tunerservice.IMtkTuner asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.mediatek.tunerservice.IMtkTuner))) {
        return ((com.mediatek.tunerservice.IMtkTuner)iin);
      }
      return new com.mediatek.tunerservice.IMtkTuner.Stub.Proxy(obj);
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
        case TRANSACTION_tune:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.tune(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addPidFilter:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.addPidFilter(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeAllPidFilters:
        {
          data.enforceInterface(descriptor);
          this.closeAllPidFilters();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopTune:
        {
          data.enforceInterface(descriptor);
          this.stopTune();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTsData:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _result = this.getTsData(_arg0, _arg1);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_setHasPendingTune:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setHasPendingTune(_arg0);
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
    private static class Proxy implements com.mediatek.tunerservice.IMtkTuner
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
      @Override public boolean tune(int frequency, java.lang.String modulation, int timeOutMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(frequency);
          _data.writeString(modulation);
          _data.writeInt(timeOutMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_tune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().tune(frequency, modulation, timeOutMs);
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
      @Override public void addPidFilter(int pid, int filterType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pid);
          _data.writeInt(filterType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPidFilter, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPidFilter(pid, filterType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeAllPidFilters() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeAllPidFilters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeAllPidFilters();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopTune() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopTune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopTune();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public byte[] getTsData(int maxDataSize, int timeOutMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(maxDataSize);
          _data.writeInt(timeOutMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTsData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTsData(maxDataSize, timeOutMs);
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
      @Override public void setHasPendingTune(boolean hasPendingTune) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hasPendingTune)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHasPendingTune, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHasPendingTune(hasPendingTune);
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
      public static com.mediatek.tunerservice.IMtkTuner sDefaultImpl;
    }
    static final int TRANSACTION_tune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addPidFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_closeAllPidFilters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_stopTune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getTsData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setHasPendingTune = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_release = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(com.mediatek.tunerservice.IMtkTuner impl) {
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
    public static com.mediatek.tunerservice.IMtkTuner getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean tune(int frequency, java.lang.String modulation, int timeOutMs) throws android.os.RemoteException;
  public void addPidFilter(int pid, int filterType) throws android.os.RemoteException;
  public void closeAllPidFilters() throws android.os.RemoteException;
  public void stopTune() throws android.os.RemoteException;
  public byte[] getTsData(int maxDataSize, int timeOutMs) throws android.os.RemoteException;
  public void setHasPendingTune(boolean hasPendingTune) throws android.os.RemoteException;
  public void release() throws android.os.RemoteException;
}
