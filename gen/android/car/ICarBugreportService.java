/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/**
 * Binder interface for {@link android.car.CarBugreportManager}.
 *
 * @hide
 */
public interface ICarBugreportService extends android.os.IInterface
{
  /** Default implementation for ICarBugreportService. */
  public static class Default implements android.car.ICarBugreportService
  {
    /**
         * Starts bugreport service to capture a zipped bugreport. The caller needs to provide
         * two file descriptors. The "output" file descriptor will be used to provide the actual
         * zip file. The "extra_output" file descriptor will be provided to add files that does not
         * exist in the original file.
         * The file descriptor is written by the service and will be read by the client.
         */
    @Override public void requestBugreport(android.os.ParcelFileDescriptor output, android.os.ParcelFileDescriptor extraOutput, android.car.ICarBugreportCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICarBugreportService
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICarBugreportService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICarBugreportService interface,
     * generating a proxy if needed.
     */
    public static android.car.ICarBugreportService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICarBugreportService))) {
        return ((android.car.ICarBugreportService)iin);
      }
      return new android.car.ICarBugreportService.Stub.Proxy(obj);
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
        case TRANSACTION_requestBugreport:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.car.ICarBugreportCallback _arg2;
          _arg2 = android.car.ICarBugreportCallback.Stub.asInterface(data.readStrongBinder());
          this.requestBugreport(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICarBugreportService
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
           * Starts bugreport service to capture a zipped bugreport. The caller needs to provide
           * two file descriptors. The "output" file descriptor will be used to provide the actual
           * zip file. The "extra_output" file descriptor will be provided to add files that does not
           * exist in the original file.
           * The file descriptor is written by the service and will be read by the client.
           */
      @Override public void requestBugreport(android.os.ParcelFileDescriptor output, android.os.ParcelFileDescriptor extraOutput, android.car.ICarBugreportCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((output!=null)) {
            _data.writeInt(1);
            output.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extraOutput!=null)) {
            _data.writeInt(1);
            extraOutput.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBugreport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestBugreport(output, extraOutput, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.ICarBugreportService sDefaultImpl;
    }
    static final int TRANSACTION_requestBugreport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.ICarBugreportService impl) {
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
    public static android.car.ICarBugreportService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Starts bugreport service to capture a zipped bugreport. The caller needs to provide
       * two file descriptors. The "output" file descriptor will be used to provide the actual
       * zip file. The "extra_output" file descriptor will be provided to add files that does not
       * exist in the original file.
       * The file descriptor is written by the service and will be read by the client.
       */
  public void requestBugreport(android.os.ParcelFileDescriptor output, android.os.ParcelFileDescriptor extraOutput, android.car.ICarBugreportCallback callback) throws android.os.RemoteException;
}
