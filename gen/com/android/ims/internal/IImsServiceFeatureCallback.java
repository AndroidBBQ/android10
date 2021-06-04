/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 *  Interface from ImsResolver to ImsServiceProxy in ImsManager.
 * Callback to ImsManager when a feature changes in the ImsServiceController.
 * {@hide}
 */
public interface IImsServiceFeatureCallback extends android.os.IInterface
{
  /** Default implementation for IImsServiceFeatureCallback. */
  public static class Default implements com.android.ims.internal.IImsServiceFeatureCallback
  {
    @Override public void imsFeatureCreated(int slotId, int feature) throws android.os.RemoteException
    {
    }
    @Override public void imsFeatureRemoved(int slotId, int feature) throws android.os.RemoteException
    {
    }
    @Override public void imsStatusChanged(int slotId, int feature, int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsServiceFeatureCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsServiceFeatureCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsServiceFeatureCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsServiceFeatureCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsServiceFeatureCallback))) {
        return ((com.android.ims.internal.IImsServiceFeatureCallback)iin);
      }
      return new com.android.ims.internal.IImsServiceFeatureCallback.Stub.Proxy(obj);
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
        case TRANSACTION_imsFeatureCreated:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.imsFeatureCreated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_imsFeatureRemoved:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.imsFeatureRemoved(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_imsStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.imsStatusChanged(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsServiceFeatureCallback
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
      @Override public void imsFeatureCreated(int slotId, int feature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(feature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_imsFeatureCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().imsFeatureCreated(slotId, feature);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void imsFeatureRemoved(int slotId, int feature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(feature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_imsFeatureRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().imsFeatureRemoved(slotId, feature);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void imsStatusChanged(int slotId, int feature, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(feature);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_imsStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().imsStatusChanged(slotId, feature, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsServiceFeatureCallback sDefaultImpl;
    }
    static final int TRANSACTION_imsFeatureCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_imsFeatureRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_imsStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsServiceFeatureCallback impl) {
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
    public static com.android.ims.internal.IImsServiceFeatureCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void imsFeatureCreated(int slotId, int feature) throws android.os.RemoteException;
  public void imsFeatureRemoved(int slotId, int feature) throws android.os.RemoteException;
  public void imsStatusChanged(int slotId, int feature, int status) throws android.os.RemoteException;
}
