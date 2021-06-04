/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.soundtrigger;
/**
 * AIDL for the SoundTriggerDetectionService to run detection operations when triggered.
 *
 * {@hide}
 */
public interface ISoundTriggerDetectionService extends android.os.IInterface
{
  /** Default implementation for ISoundTriggerDetectionService. */
  public static class Default implements android.media.soundtrigger.ISoundTriggerDetectionService
  {
    @Override public void setClient(android.os.ParcelUuid uuid, android.os.Bundle params, android.media.soundtrigger.ISoundTriggerDetectionServiceClient client) throws android.os.RemoteException
    {
    }
    @Override public void removeClient(android.os.ParcelUuid uuid) throws android.os.RemoteException
    {
    }
    @Override public void onGenericRecognitionEvent(android.os.ParcelUuid uuid, int opId, android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent event) throws android.os.RemoteException
    {
    }
    @Override public void onError(android.os.ParcelUuid uuid, int opId, int status) throws android.os.RemoteException
    {
    }
    @Override public void onStopOperation(android.os.ParcelUuid uuid, int opId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.soundtrigger.ISoundTriggerDetectionService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.soundtrigger.ISoundTriggerDetectionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.soundtrigger.ISoundTriggerDetectionService interface,
     * generating a proxy if needed.
     */
    public static android.media.soundtrigger.ISoundTriggerDetectionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.soundtrigger.ISoundTriggerDetectionService))) {
        return ((android.media.soundtrigger.ISoundTriggerDetectionService)iin);
      }
      return new android.media.soundtrigger.ISoundTriggerDetectionService.Stub.Proxy(obj);
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
        case TRANSACTION_setClient:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.media.soundtrigger.ISoundTriggerDetectionServiceClient _arg2;
          _arg2 = android.media.soundtrigger.ISoundTriggerDetectionServiceClient.Stub.asInterface(data.readStrongBinder());
          this.setClient(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_removeClient:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeClient(_arg0);
          return true;
        }
        case TRANSACTION_onGenericRecognitionEvent:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onGenericRecognitionEvent(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onError(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onStopOperation:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onStopOperation(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.soundtrigger.ISoundTriggerDetectionService
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
      @Override public void setClient(android.os.ParcelUuid uuid, android.os.Bundle params, android.media.soundtrigger.ISoundTriggerDetectionServiceClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uuid!=null)) {
            _data.writeInt(1);
            uuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setClient(uuid, params, client);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeClient(android.os.ParcelUuid uuid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uuid!=null)) {
            _data.writeInt(1);
            uuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeClient(uuid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGenericRecognitionEvent(android.os.ParcelUuid uuid, int opId, android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uuid!=null)) {
            _data.writeInt(1);
            uuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(opId);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGenericRecognitionEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGenericRecognitionEvent(uuid, opId, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onError(android.os.ParcelUuid uuid, int opId, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uuid!=null)) {
            _data.writeInt(1);
            uuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(opId);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(uuid, opId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStopOperation(android.os.ParcelUuid uuid, int opId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uuid!=null)) {
            _data.writeInt(1);
            uuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(opId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStopOperation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStopOperation(uuid, opId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.soundtrigger.ISoundTriggerDetectionService sDefaultImpl;
    }
    static final int TRANSACTION_setClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onGenericRecognitionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onStopOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.media.soundtrigger.ISoundTriggerDetectionService impl) {
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
    public static android.media.soundtrigger.ISoundTriggerDetectionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setClient(android.os.ParcelUuid uuid, android.os.Bundle params, android.media.soundtrigger.ISoundTriggerDetectionServiceClient client) throws android.os.RemoteException;
  public void removeClient(android.os.ParcelUuid uuid) throws android.os.RemoteException;
  public void onGenericRecognitionEvent(android.os.ParcelUuid uuid, int opId, android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent event) throws android.os.RemoteException;
  public void onError(android.os.ParcelUuid uuid, int opId, int status) throws android.os.RemoteException;
  public void onStopOperation(android.os.ParcelUuid uuid, int opId) throws android.os.RemoteException;
}
