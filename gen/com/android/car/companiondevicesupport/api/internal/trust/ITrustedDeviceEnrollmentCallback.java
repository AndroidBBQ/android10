/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.trust;
/** Callback for triggered trusted device enrollment events. */
public interface ITrustedDeviceEnrollmentCallback extends android.os.IInterface
{
  /** Default implementation for ITrustedDeviceEnrollmentCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback
  {
    /** Triggered when credentials validation is needed to authenticate a new escrow token. */
    @Override public void onValidateCredentialsRequest() throws android.os.RemoteException
    {
    }
    /** Triggered when an error happens during trusted device enrollment. */
    @Override public void onTrustedDeviceEnrollmentError(int error) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback))) {
        return ((com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onValidateCredentialsRequest:
        {
          data.enforceInterface(descriptor);
          this.onValidateCredentialsRequest();
          return true;
        }
        case TRANSACTION_onTrustedDeviceEnrollmentError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onTrustedDeviceEnrollmentError(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback
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
      /** Triggered when credentials validation is needed to authenticate a new escrow token. */
      @Override public void onValidateCredentialsRequest() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onValidateCredentialsRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onValidateCredentialsRequest();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when an error happens during trusted device enrollment. */
      @Override public void onTrustedDeviceEnrollmentError(int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustedDeviceEnrollmentError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustedDeviceEnrollmentError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback sDefaultImpl;
    }
    static final int TRANSACTION_onValidateCredentialsRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onTrustedDeviceEnrollmentError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Triggered when credentials validation is needed to authenticate a new escrow token. */
  public void onValidateCredentialsRequest() throws android.os.RemoteException;
  /** Triggered when an error happens during trusted device enrollment. */
  public void onTrustedDeviceEnrollmentError(int error) throws android.os.RemoteException;
}
