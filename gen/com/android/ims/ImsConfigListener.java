/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims;
/**
 * Used by IMS config client to monitor the config operation results.
 * {@hide}
 */
public interface ImsConfigListener extends android.os.IInterface
{
  /** Default implementation for ImsConfigListener. */
  public static class Default implements com.android.ims.ImsConfigListener
  {
    /**
         * Notifies client the value of the get operation result on the feature config item.
         * The arguments are the same as passed to com.android.ims.ImsConfig#getFeatureValue.
         *
         * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
         * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
         * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         * @return void.
         */
    @Override public void onGetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the set value operation result for feature config item.
         * Used by clients that need to be notified the set operation result.
         * The arguments are the same as passed to com.android.ims.ImsConfig#setFeatureValue.
         * The arguments are repeated in the callback to enable the listener to understand
         * which configuration attempt failed.
         *
         * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
         * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
         * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         *
         * @return void.
         */
    @Override public void onSetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the value of the get operation result on the video quality item.
         *
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         * @param quality. as defined in com.android.ims.ImsConfig#OperationValuesConstants.
         * @return void
         *
         * @throws ImsException if calling the IMS service results in an error.
         */
    @Override public void onGetVideoQuality(int status, int quality) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the set value operation result for video quality item.
         * Used by clients that need to be notified the set operation result.
         *
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         * @return void
         *
         * @throws ImsException if calling the IMS service results in an error.
         */
    @Override public void onSetVideoQuality(int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.ImsConfigListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.ImsConfigListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.ImsConfigListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.ImsConfigListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.ImsConfigListener))) {
        return ((com.android.ims.ImsConfigListener)iin);
      }
      return new com.android.ims.ImsConfigListener.Stub.Proxy(obj);
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
        case TRANSACTION_onGetFeatureResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.onGetFeatureResponse(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onSetFeatureResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.onSetFeatureResponse(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onGetVideoQuality:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGetVideoQuality(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSetVideoQuality:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSetVideoQuality(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.ImsConfigListener
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
           * Notifies client the value of the get operation result on the feature config item.
           * The arguments are the same as passed to com.android.ims.ImsConfig#getFeatureValue.
           *
           * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
           * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
           * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           * @return void.
           */
      @Override public void onGetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(feature);
          _data.writeInt(network);
          _data.writeInt(value);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetFeatureResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetFeatureResponse(feature, network, value, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the set value operation result for feature config item.
           * Used by clients that need to be notified the set operation result.
           * The arguments are the same as passed to com.android.ims.ImsConfig#setFeatureValue.
           * The arguments are repeated in the callback to enable the listener to understand
           * which configuration attempt failed.
           *
           * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
           * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
           * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           *
           * @return void.
           */
      @Override public void onSetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(feature);
          _data.writeInt(network);
          _data.writeInt(value);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetFeatureResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetFeatureResponse(feature, network, value, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the value of the get operation result on the video quality item.
           *
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           * @param quality. as defined in com.android.ims.ImsConfig#OperationValuesConstants.
           * @return void
           *
           * @throws ImsException if calling the IMS service results in an error.
           */
      @Override public void onGetVideoQuality(int status, int quality) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          _data.writeInt(quality);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetVideoQuality, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetVideoQuality(status, quality);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the set value operation result for video quality item.
           * Used by clients that need to be notified the set operation result.
           *
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           * @return void
           *
           * @throws ImsException if calling the IMS service results in an error.
           */
      @Override public void onSetVideoQuality(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetVideoQuality, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetVideoQuality(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.ImsConfigListener sDefaultImpl;
    }
    static final int TRANSACTION_onGetFeatureResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSetFeatureResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onGetVideoQuality = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSetVideoQuality = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.ims.ImsConfigListener impl) {
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
    public static com.android.ims.ImsConfigListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies client the value of the get operation result on the feature config item.
       * The arguments are the same as passed to com.android.ims.ImsConfig#getFeatureValue.
       *
       * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
       * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
       * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       * @return void.
       */
  public void onGetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException;
  /**
       * Notifies client the set value operation result for feature config item.
       * Used by clients that need to be notified the set operation result.
       * The arguments are the same as passed to com.android.ims.ImsConfig#setFeatureValue.
       * The arguments are repeated in the callback to enable the listener to understand
       * which configuration attempt failed.
       *
       * @param feature. as defined in com.android.ims.ImsConfig#FeatureConstants.
       * @param network. as defined in android.telephony.TelephonyManager#NETWORK_TYPE_XXX.
       * @param value. as defined in com.android.ims.ImsConfig#FeatureValueConstants.
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       *
       * @return void.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/ImsConfigListener.aidl:50:1:50:25")
  public void onSetFeatureResponse(int feature, int network, int value, int status) throws android.os.RemoteException;
  /**
       * Notifies client the value of the get operation result on the video quality item.
       *
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       * @param quality. as defined in com.android.ims.ImsConfig#OperationValuesConstants.
       * @return void
       *
       * @throws ImsException if calling the IMS service results in an error.
       */
  public void onGetVideoQuality(int status, int quality) throws android.os.RemoteException;
  /**
       * Notifies client the set value operation result for video quality item.
       * Used by clients that need to be notified the set operation result.
       *
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       * @return void
       *
       * @throws ImsException if calling the IMS service results in an error.
       */
  public void onSetVideoQuality(int status) throws android.os.RemoteException;
}
