/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.settings;
/**
 * Binder interface for {@link android.car.settings.CarConfigurationManager}.
 *
 * @hide
 */
public interface ICarConfigurationManager extends android.os.IInterface
{
  /** Default implementation for ICarConfigurationManager. */
  public static class Default implements android.car.settings.ICarConfigurationManager
  {
    /**
         * Returns the configuration for SpeedBump if it exists. If it does not, then a default
         * configuration will be returned.
         */
    @Override public android.car.settings.SpeedBumpConfiguration getSpeedBumpConfiguration() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.settings.ICarConfigurationManager
  {
    private static final java.lang.String DESCRIPTOR = "android.car.settings.ICarConfigurationManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.settings.ICarConfigurationManager interface,
     * generating a proxy if needed.
     */
    public static android.car.settings.ICarConfigurationManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.settings.ICarConfigurationManager))) {
        return ((android.car.settings.ICarConfigurationManager)iin);
      }
      return new android.car.settings.ICarConfigurationManager.Stub.Proxy(obj);
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
        case TRANSACTION_getSpeedBumpConfiguration:
        {
          data.enforceInterface(descriptor);
          android.car.settings.SpeedBumpConfiguration _result = this.getSpeedBumpConfiguration();
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
    private static class Proxy implements android.car.settings.ICarConfigurationManager
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
           * Returns the configuration for SpeedBump if it exists. If it does not, then a default
           * configuration will be returned.
           */
      @Override public android.car.settings.SpeedBumpConfiguration getSpeedBumpConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.settings.SpeedBumpConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSpeedBumpConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSpeedBumpConfiguration();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.settings.SpeedBumpConfiguration.CREATOR.createFromParcel(_reply);
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
      public static android.car.settings.ICarConfigurationManager sDefaultImpl;
    }
    static final int TRANSACTION_getSpeedBumpConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.settings.ICarConfigurationManager impl) {
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
    public static android.car.settings.ICarConfigurationManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns the configuration for SpeedBump if it exists. If it does not, then a default
       * configuration will be returned.
       */
  public android.car.settings.SpeedBumpConfiguration getSpeedBumpConfiguration() throws android.os.RemoteException;
}
