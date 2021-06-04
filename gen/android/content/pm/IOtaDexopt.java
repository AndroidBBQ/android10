/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * A/B OTA dexopting service.
 *
 * {@hide}
 */
public interface IOtaDexopt extends android.os.IInterface
{
  /** Default implementation for IOtaDexopt. */
  public static class Default implements android.content.pm.IOtaDexopt
  {
    /**
         * Prepare for A/B OTA dexopt. Initialize internal structures.
         *
         * Calls to the other methods are only valid after a call to prepare. You may not call
         * prepare twice without a cleanup call.
         */
    @Override public void prepare() throws android.os.RemoteException
    {
    }
    /**
         * Clean up all internal state.
         */
    @Override public void cleanup() throws android.os.RemoteException
    {
    }
    /**
         * Check whether all updates have been performed.
         */
    @Override public boolean isDone() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return the progress (0..1) made in this session. When {@link #isDone() isDone} returns
         * true, the progress value will be 1.
         */
    @Override public float getProgress() throws android.os.RemoteException
    {
      return 0.0f;
    }
    /**
         * Optimize the next package. Note: this command is synchronous, that is, only returns after
         * the package has been dexopted (or dexopting failed).
         *
         * Note: this will be removed after a transition period. Use nextDexoptCommand instead.
         */
    @Override public void dexoptNextPackage() throws android.os.RemoteException
    {
    }
    /**
         * Get the optimization parameters for the next package.
         */
    @Override public java.lang.String nextDexoptCommand() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IOtaDexopt
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IOtaDexopt";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IOtaDexopt interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IOtaDexopt asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IOtaDexopt))) {
        return ((android.content.pm.IOtaDexopt)iin);
      }
      return new android.content.pm.IOtaDexopt.Stub.Proxy(obj);
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
        case TRANSACTION_prepare:
        {
          data.enforceInterface(descriptor);
          this.prepare();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cleanup:
        {
          data.enforceInterface(descriptor);
          this.cleanup();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDone:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDone();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getProgress:
        {
          data.enforceInterface(descriptor);
          float _result = this.getProgress();
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_dexoptNextPackage:
        {
          data.enforceInterface(descriptor);
          this.dexoptNextPackage();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_nextDexoptCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.nextDexoptCommand();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.IOtaDexopt
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
           * Prepare for A/B OTA dexopt. Initialize internal structures.
           *
           * Calls to the other methods are only valid after a call to prepare. You may not call
           * prepare twice without a cleanup call.
           */
      @Override public void prepare() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepare, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepare();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Clean up all internal state.
           */
      @Override public void cleanup() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cleanup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cleanup();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Check whether all updates have been performed.
           */
      @Override public boolean isDone() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDone();
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
      /**
           * Return the progress (0..1) made in this session. When {@link #isDone() isDone} returns
           * true, the progress value will be 1.
           */
      @Override public float getProgress() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProgress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProgress();
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Optimize the next package. Note: this command is synchronous, that is, only returns after
           * the package has been dexopted (or dexopting failed).
           *
           * Note: this will be removed after a transition period. Use nextDexoptCommand instead.
           */
      @Override public void dexoptNextPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dexoptNextPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dexoptNextPackage();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Get the optimization parameters for the next package.
           */
      @Override public java.lang.String nextDexoptCommand() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nextDexoptCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nextDexoptCommand();
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
      public static android.content.pm.IOtaDexopt sDefaultImpl;
    }
    static final int TRANSACTION_prepare = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_cleanup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isDone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_dexoptNextPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_nextDexoptCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.content.pm.IOtaDexopt impl) {
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
    public static android.content.pm.IOtaDexopt getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Prepare for A/B OTA dexopt. Initialize internal structures.
       *
       * Calls to the other methods are only valid after a call to prepare. You may not call
       * prepare twice without a cleanup call.
       */
  public void prepare() throws android.os.RemoteException;
  /**
       * Clean up all internal state.
       */
  public void cleanup() throws android.os.RemoteException;
  /**
       * Check whether all updates have been performed.
       */
  public boolean isDone() throws android.os.RemoteException;
  /**
       * Return the progress (0..1) made in this session. When {@link #isDone() isDone} returns
       * true, the progress value will be 1.
       */
  public float getProgress() throws android.os.RemoteException;
  /**
       * Optimize the next package. Note: this command is synchronous, that is, only returns after
       * the package has been dexopted (or dexopting failed).
       *
       * Note: this will be removed after a transition period. Use nextDexoptCommand instead.
       */
  public void dexoptNextPackage() throws android.os.RemoteException;
  /**
       * Get the optimization parameters for the next package.
       */
  public java.lang.String nextDexoptCommand() throws android.os.RemoteException;
}
