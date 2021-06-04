/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See ImsService and MmTelFeature for more information.
 * {@hide}
 */
public interface IImsServiceController extends android.os.IInterface
{
  /** Default implementation for IImsServiceController. */
  public static class Default implements android.telephony.ims.aidl.IImsServiceController
  {
    @Override public void setListener(android.telephony.ims.aidl.IImsServiceControllerListener l) throws android.os.RemoteException
    {
    }
    @Override public android.telephony.ims.aidl.IImsMmTelFeature createMmTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.aidl.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.stub.ImsFeatureConfiguration querySupportedImsFeatures() throws android.os.RemoteException
    {
      return null;
    }
    // Synchronous call to ensure the ImsService is ready before continuing with feature creation.

    @Override public void notifyImsServiceReadyForFeatureCreation() throws android.os.RemoteException
    {
    }
    @Override public void removeImsFeature(int slotId, int featureType, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
    {
    }
    @Override public android.telephony.ims.aidl.IImsConfig getConfig(int slotId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.aidl.IImsRegistration getRegistration(int slotId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void enableIms(int slotId) throws android.os.RemoteException
    {
    }
    @Override public void disableIms(int slotId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsServiceController
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsServiceController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsServiceController interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsServiceController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsServiceController))) {
        return ((android.telephony.ims.aidl.IImsServiceController)iin);
      }
      return new android.telephony.ims.aidl.IImsServiceController.Stub.Proxy(obj);
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
        case TRANSACTION_setListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsServiceControllerListener _arg0;
          _arg0 = android.telephony.ims.aidl.IImsServiceControllerListener.Stub.asInterface(data.readStrongBinder());
          this.setListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createMmTelFeature:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsFeatureStatusCallback _arg1;
          _arg1 = com.android.ims.internal.IImsFeatureStatusCallback.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.aidl.IImsMmTelFeature _result = this.createMmTelFeature(_arg0, _arg1);
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
          android.telephony.ims.aidl.IImsRcsFeature _result = this.createRcsFeature(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_querySupportedImsFeatures:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.stub.ImsFeatureConfiguration _result = this.querySupportedImsFeatures();
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
        case TRANSACTION_notifyImsServiceReadyForFeatureCreation:
        {
          data.enforceInterface(descriptor);
          this.notifyImsServiceReadyForFeatureCreation();
          reply.writeNoException();
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
        case TRANSACTION_getConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsConfig _result = this.getConfig(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getRegistration:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsRegistration _result = this.getRegistration(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_enableIms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableIms(_arg0);
          return true;
        }
        case TRANSACTION_disableIms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableIms(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsServiceController
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
      @Override public void setListener(android.telephony.ims.aidl.IImsServiceControllerListener l) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((l!=null))?(l.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setListener(l);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.telephony.ims.aidl.IImsMmTelFeature createMmTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsMmTelFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createMmTelFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createMmTelFeature(slotId, c);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsMmTelFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.telephony.ims.aidl.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsRcsFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRcsFeature, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createRcsFeature(slotId, c);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsRcsFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.telephony.ims.stub.ImsFeatureConfiguration querySupportedImsFeatures() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.stub.ImsFeatureConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_querySupportedImsFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().querySupportedImsFeatures();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.stub.ImsFeatureConfiguration.CREATOR.createFromParcel(_reply);
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
      // Synchronous call to ensure the ImsService is ready before continuing with feature creation.

      @Override public void notifyImsServiceReadyForFeatureCreation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyImsServiceReadyForFeatureCreation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyImsServiceReadyForFeatureCreation();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      @Override public android.telephony.ims.aidl.IImsConfig getConfig(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfig(slotId);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsConfig.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.telephony.ims.aidl.IImsRegistration getRegistration(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsRegistration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRegistration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRegistration(slotId);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsRegistration.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void enableIms(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableIms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableIms(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disableIms(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableIms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableIms(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsServiceController sDefaultImpl;
    }
    static final int TRANSACTION_setListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_createMmTelFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createRcsFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_querySupportedImsFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_notifyImsServiceReadyForFeatureCreation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeImsFeature = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getRegistration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_enableIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_disableIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsServiceController impl) {
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
    public static android.telephony.ims.aidl.IImsServiceController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setListener(android.telephony.ims.aidl.IImsServiceControllerListener l) throws android.os.RemoteException;
  public android.telephony.ims.aidl.IImsMmTelFeature createMmTelFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public android.telephony.ims.aidl.IImsRcsFeature createRcsFeature(int slotId, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public android.telephony.ims.stub.ImsFeatureConfiguration querySupportedImsFeatures() throws android.os.RemoteException;
  // Synchronous call to ensure the ImsService is ready before continuing with feature creation.

  public void notifyImsServiceReadyForFeatureCreation() throws android.os.RemoteException;
  public void removeImsFeature(int slotId, int featureType, com.android.ims.internal.IImsFeatureStatusCallback c) throws android.os.RemoteException;
  public android.telephony.ims.aidl.IImsConfig getConfig(int slotId) throws android.os.RemoteException;
  public android.telephony.ims.aidl.IImsRegistration getRegistration(int slotId) throws android.os.RemoteException;
  public void enableIms(int slotId) throws android.os.RemoteException;
  public void disableIms(int slotId) throws android.os.RemoteException;
}
