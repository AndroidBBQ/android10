/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * See ImsService and MMTelFeature for more information.
 * {@hide}
 */
public interface IImsServiceController extends android.os.IInterface
{
  /** Default implementation for IImsServiceController. */
  public static class Default implements com.android.ims.internal.IImsServiceController
  {
    @Override public com.android.ims.internal.IImsMMTelFeature createEmergencyMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsMMTelFeature createMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.ims.internal.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void removeImsFeature(int slotId, int featureType, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsServiceController
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsServiceController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsServiceController interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsServiceController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsServiceController))) {
        return ((com.android.ims.internal.IImsServiceController)iin);
      }
      return new com.android.ims.internal.IImsServiceController.Stub.Proxy(obj);
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
        case TRANSACTION_createEmergencyMMTelFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsFeatureStatusCallback _arg1;
          _arg1 = com.android.ims.internal.IImsFeatureStatusCallback.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.IImsMMTelFeature _result = this.createEmergencyMMTelFeature(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_createMMTelFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsFeatureStatusCallback _arg1;
          _arg1 = com.android.ims.internal.IImsFeatureStatusCallback.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.IImsMMTelFeature _result = this.createMMTelFeature(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_createRcsFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsFeatureStatusCallback _arg1;
          _arg1 = com.android.ims.internal.IImsFeatureStatusCallback.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.IImsRcsFeature _result = this.createRcsFeature(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_removeImsFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.ims.internal.IImsFeatureStatusCallback _arg2;
          _arg2 = com.android.ims.internal.IImsFeatureStatusCallback.Stub.asInterface(data.readStrongBinder());
          this.removeImsFeature(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsServiceController
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
      @Override public com.android.ims.internal.IImsMMTelFeature createEmergencyMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsMMTelFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createEmergencyMMTelFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createEmergencyMMTelFeature(slotId, c);
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsMMTelFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.ims.internal.IImsMMTelFeature createMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsMMTelFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createMMTelFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createMMTelFeature(slotId, c);
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsMMTelFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.ims.internal.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.IImsRcsFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRcsFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createRcsFeature(slotId, c);
          }
          _reply.readException();
          _result = com.android.ims.internal.IImsRcsFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void removeImsFeature(int slotId, int featureType, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(featureType);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeImsFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeImsFeature(slotId, featureType, c);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsServiceController sDefaultImpl;
    }
    static final int TRANSACTION_createEmergencyMMTelFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_createMMTelFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createRcsFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_removeImsFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsServiceController impl) {
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
    public static com.android.ims.internal.IImsServiceController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public com.android.ims.internal.IImsMMTelFeature createEmergencyMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public com.android.ims.internal.IImsMMTelFeature createMMTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public com.android.ims.internal.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public void removeImsFeature(int slotId, int featureType, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
}
