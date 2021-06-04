/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.face;
/**
 * Communication channel from the FaceService back to FaceAuthenticationManager.
 * @hide
 */
public interface IFaceServiceReceiver extends android.os.IInterface
{
  /** Default implementation for IFaceServiceReceiver. */
  public static class Default implements android.hardware.face.IFaceServiceReceiver
  {
    @Override public void onEnrollResult(long deviceId, int faceId, int remaining) throws android.os.RemoteException
    {
    }
    @Override public void onAcquired(long deviceId, int acquiredInfo, int vendorCode) throws android.os.RemoteException
    {
    }
    @Override public void onAuthenticationSucceeded(long deviceId, android.hardware.face.Face face, int userId) throws android.os.RemoteException
    {
    }
    @Override public void onAuthenticationFailed(long deviceId) throws android.os.RemoteException
    {
    }
    @Override public void onError(long deviceId, int error, int vendorCode) throws android.os.RemoteException
    {
    }
    @Override public void onRemoved(long deviceId, int faceId, int remaining) throws android.os.RemoteException
    {
    }
    @Override public void onEnumerated(long deviceId, int faceId, int remaining) throws android.os.RemoteException
    {
    }
    @Override public void onFeatureSet(boolean success, int feature) throws android.os.RemoteException
    {
    }
    @Override public void onFeatureGet(boolean success, int feature, boolean value) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.face.IFaceServiceReceiver
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.face.IFaceServiceReceiver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.face.IFaceServiceReceiver interface,
     * generating a proxy if needed.
     */
    public static android.hardware.face.IFaceServiceReceiver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.face.IFaceServiceReceiver))) {
        return ((android.hardware.face.IFaceServiceReceiver)iin);
      }
      return new android.hardware.face.IFaceServiceReceiver.Stub.Proxy(obj);
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
        case TRANSACTION_onEnrollResult:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onEnrollResult(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAcquired:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onAcquired(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAuthenticationSucceeded:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.hardware.face.Face _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.face.Face.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.onAuthenticationSucceeded(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAuthenticationFailed:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onAuthenticationFailed(_arg0);
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onError(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onRemoved:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onRemoved(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onEnumerated:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onEnumerated(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onFeatureSet:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onFeatureSet(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onFeatureGet:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onFeatureGet(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.face.IFaceServiceReceiver
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
      @Override public void onEnrollResult(long deviceId, int faceId, int remaining) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeInt(faceId);
          _data.writeInt(remaining);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnrollResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnrollResult(deviceId, faceId, remaining);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAcquired(long deviceId, int acquiredInfo, int vendorCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeInt(acquiredInfo);
          _data.writeInt(vendorCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAcquired, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAcquired(deviceId, acquiredInfo, vendorCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAuthenticationSucceeded(long deviceId, android.hardware.face.Face face, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          if ((face!=null)) {
            _data.writeInt(1);
            face.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAuthenticationSucceeded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAuthenticationSucceeded(deviceId, face, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAuthenticationFailed(long deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAuthenticationFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAuthenticationFailed(deviceId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onError(long deviceId, int error, int vendorCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeInt(error);
          _data.writeInt(vendorCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(deviceId, error, vendorCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRemoved(long deviceId, int faceId, int remaining) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeInt(faceId);
          _data.writeInt(remaining);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRemoved(deviceId, faceId, remaining);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onEnumerated(long deviceId, int faceId, int remaining) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(deviceId);
          _data.writeInt(faceId);
          _data.writeInt(remaining);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnumerated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnumerated(deviceId, faceId, remaining);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFeatureSet(boolean success, int feature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((success)?(1):(0)));
          _data.writeInt(feature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFeatureSet, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFeatureSet(success, feature);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFeatureGet(boolean success, int feature, boolean value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((success)?(1):(0)));
          _data.writeInt(feature);
          _data.writeInt(((value)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFeatureGet, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFeatureGet(success, feature, value);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.face.IFaceServiceReceiver sDefaultImpl;
    }
    static final int TRANSACTION_onEnrollResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAcquired = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAuthenticationSucceeded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onAuthenticationFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onEnumerated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onFeatureSet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onFeatureGet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.hardware.face.IFaceServiceReceiver impl) {
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
    public static android.hardware.face.IFaceServiceReceiver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onEnrollResult(long deviceId, int faceId, int remaining) throws android.os.RemoteException;
  public void onAcquired(long deviceId, int acquiredInfo, int vendorCode) throws android.os.RemoteException;
  public void onAuthenticationSucceeded(long deviceId, android.hardware.face.Face face, int userId) throws android.os.RemoteException;
  public void onAuthenticationFailed(long deviceId) throws android.os.RemoteException;
  public void onError(long deviceId, int error, int vendorCode) throws android.os.RemoteException;
  public void onRemoved(long deviceId, int faceId, int remaining) throws android.os.RemoteException;
  public void onEnumerated(long deviceId, int faceId, int remaining) throws android.os.RemoteException;
  public void onFeatureSet(boolean success, int feature) throws android.os.RemoteException;
  public void onFeatureGet(boolean success, int feature, boolean value) throws android.os.RemoteException;
}
