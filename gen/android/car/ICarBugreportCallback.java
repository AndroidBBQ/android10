/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/**
  * Callback for carbugreport service
  * {@hide}
  */
public interface ICarBugreportCallback extends android.os.IInterface
{
  /** Default implementation for ICarBugreportCallback. */
  public static class Default implements android.car.ICarBugreportCallback
  {
    /**
         * Called on an error condition. The error codes are defined as
         * {@link android.car.CarBugreportManager.CarBugreportManagerCallback.CarBugreportErrorCode}
         */
    @Override public void onError(int errorCode) throws android.os.RemoteException
    {
    }
    /**
         * Called when the bugreport progress changes. Progress value is a number between 0.0 and 100.0.
         *
         * <p>Never called after {@link #onError()} or {@link onFinished()}.
         */
    @Override public void onProgress(float progress) throws android.os.RemoteException
    {
    }
    /**
         * Called when taking bugreport finishes successfully.
         */
    @Override public void onFinished() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICarBugreportCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICarBugreportCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICarBugreportCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.ICarBugreportCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICarBugreportCallback))) {
        return ((android.car.ICarBugreportCallback)iin);
      }
      return new android.car.ICarBugreportCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onError(_arg0);
          return true;
        }
        case TRANSACTION_onProgress:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.onProgress(_arg0);
          return true;
        }
        case TRANSACTION_onFinished:
        {
          data.enforceInterface(descriptor);
          this.onFinished();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICarBugreportCallback
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
           * Called on an error condition. The error codes are defined as
           * {@link android.car.CarBugreportManager.CarBugreportManagerCallback.CarBugreportErrorCode}
           */
      @Override public void onError(int errorCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errorCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(errorCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the bugreport progress changes. Progress value is a number between 0.0 and 100.0.
           *
           * <p>Never called after {@link #onError()} or {@link onFinished()}.
           */
      @Override public void onProgress(float progress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(progress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgress(progress);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when taking bugreport finishes successfully.
           */
      @Override public void onFinished() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFinished();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.ICarBugreportCallback sDefaultImpl;
    }
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.car.ICarBugreportCallback impl) {
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
    public static android.car.ICarBugreportCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called on an error condition. The error codes are defined as
       * {@link android.car.CarBugreportManager.CarBugreportManagerCallback.CarBugreportErrorCode}
       */
  public void onError(int errorCode) throws android.os.RemoteException;
  /**
       * Called when the bugreport progress changes. Progress value is a number between 0.0 and 100.0.
       *
       * <p>Never called after {@link #onError()} or {@link onFinished()}.
       */
  public void onProgress(float progress) throws android.os.RemoteException;
  /**
       * Called when taking bugreport finishes successfully.
       */
  public void onFinished() throws android.os.RemoteException;
}
