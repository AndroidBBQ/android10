/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.association;
/** Callback for triggered association events. */
public interface IAssociationCallback extends android.os.IInterface
{
  /** Default implementation for IAssociationCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback
  {
    /** Triggered when IHU starts advertising for association successfully. */
    @Override public void onAssociationStartSuccess(java.lang.String deviceName) throws android.os.RemoteException
    {
    }
    /** Triggered when IHU failed to start advertising for association. */
    @Override public void onAssociationStartFailure() throws android.os.RemoteException
    {
    }
    /** Triggered when an error has been encountered during assocition with a new device. */
    @Override public void onAssociationError(int error) throws android.os.RemoteException
    {
    }
    /**  Triggered when a pairing code is available to be present. */
    @Override public void onVerificationCodeAvailable(java.lang.String code) throws android.os.RemoteException
    {
    }
    /** Triggered when the assocition has completed */
    @Override public void onAssociationCompleted() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback))) {
        return ((com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onAssociationStartSuccess:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onAssociationStartSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onAssociationStartFailure:
        {
          data.enforceInterface(descriptor);
          this.onAssociationStartFailure();
          return true;
        }
        case TRANSACTION_onAssociationError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onAssociationError(_arg0);
          return true;
        }
        case TRANSACTION_onVerificationCodeAvailable:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onVerificationCodeAvailable(_arg0);
          return true;
        }
        case TRANSACTION_onAssociationCompleted:
        {
          data.enforceInterface(descriptor);
          this.onAssociationCompleted();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback
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
      /** Triggered when IHU starts advertising for association successfully. */
      @Override public void onAssociationStartSuccess(java.lang.String deviceName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociationStartSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociationStartSuccess(deviceName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when IHU failed to start advertising for association. */
      @Override public void onAssociationStartFailure() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociationStartFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociationStartFailure();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when an error has been encountered during assocition with a new device. */
      @Override public void onAssociationError(int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociationError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociationError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**  Triggered when a pairing code is available to be present. */
      @Override public void onVerificationCodeAvailable(java.lang.String code) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(code);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVerificationCodeAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVerificationCodeAvailable(code);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when the assocition has completed */
      @Override public void onAssociationCompleted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociationCompleted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociationCompleted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback sDefaultImpl;
    }
    static final int TRANSACTION_onAssociationStartSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAssociationStartFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAssociationError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onVerificationCodeAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onAssociationCompleted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Triggered when IHU starts advertising for association successfully. */
  public void onAssociationStartSuccess(java.lang.String deviceName) throws android.os.RemoteException;
  /** Triggered when IHU failed to start advertising for association. */
  public void onAssociationStartFailure() throws android.os.RemoteException;
  /** Triggered when an error has been encountered during assocition with a new device. */
  public void onAssociationError(int error) throws android.os.RemoteException;
  /**  Triggered when a pairing code is available to be present. */
  public void onVerificationCodeAvailable(java.lang.String code) throws android.os.RemoteException;
  /** Triggered when the assocition has completed */
  public void onAssociationCompleted() throws android.os.RemoteException;
}
