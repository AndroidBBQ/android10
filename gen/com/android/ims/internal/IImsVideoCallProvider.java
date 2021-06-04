/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * Internal remote interface for IMS's video call provider.
 *
 * At least initially, this aidl mirrors telecomm's {@link IVideoCallProvider}. We created a
 * separate aidl interface even though the methods and parameters are same because the
 * {@link IVideoCallProvider} was specifically created as a binder for inter-process communication
 * between Telecomm and Telephony.
 *
 * We don't want to use the same aidl in other places for communication, namely communication
 * between Telephony and the IMS Service, even if that communication may be for similar methods.
 * This decouples the communication among these processes. Similarly, third parties implementing a
 * video call provider will not have the benefit of accessing the internal
 * {@link IVideoCallProvider} aidl for interprocess communication.
 *
 * @see android.telecom.internal.IVideoCallProvider
 * @see android.telecom.VideoCallProvider
 * @hide
 */
public interface IImsVideoCallProvider extends android.os.IInterface
{
  /** Default implementation for IImsVideoCallProvider. */
  public static class Default implements com.android.ims.internal.IImsVideoCallProvider
  {
    @Override public void setCallback(com.android.ims.internal.IImsVideoCallCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setCamera(java.lang.String cameraId, int uid) throws android.os.RemoteException
    {
    }
    @Override public void setPreviewSurface(android.view.Surface surface) throws android.os.RemoteException
    {
    }
    @Override public void setDisplaySurface(android.view.Surface surface) throws android.os.RemoteException
    {
    }
    @Override public void setDeviceOrientation(int rotation) throws android.os.RemoteException
    {
    }
    @Override public void setZoom(float value) throws android.os.RemoteException
    {
    }
    @Override public void sendSessionModifyRequest(android.telecom.VideoProfile fromProfile, android.telecom.VideoProfile toProfile) throws android.os.RemoteException
    {
    }
    @Override public void sendSessionModifyResponse(android.telecom.VideoProfile responseProfile) throws android.os.RemoteException
    {
    }
    @Override public void requestCameraCapabilities() throws android.os.RemoteException
    {
    }
    @Override public void requestCallDataUsage() throws android.os.RemoteException
    {
    }
    @Override public void setPauseImage(android.net.Uri uri) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsVideoCallProvider
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsVideoCallProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsVideoCallProvider interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsVideoCallProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsVideoCallProvider))) {
        return ((com.android.ims.internal.IImsVideoCallProvider)iin);
      }
      return new com.android.ims.internal.IImsVideoCallProvider.Stub.Proxy(obj);
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
        case TRANSACTION_setCallback:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsVideoCallCallback _arg0;
          _arg0 = com.android.ims.internal.IImsVideoCallCallback.Stub.asInterface(data.readStrongBinder());
          this.setCallback(_arg0);
          return true;
        }
        case TRANSACTION_setCamera:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setCamera(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setPreviewSurface:
        {
          data.enforceInterface(descriptor);
          android.view.Surface _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setPreviewSurface(_arg0);
          return true;
        }
        case TRANSACTION_setDisplaySurface:
        {
          data.enforceInterface(descriptor);
          android.view.Surface _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.Surface.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setDisplaySurface(_arg0);
          return true;
        }
        case TRANSACTION_setDeviceOrientation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDeviceOrientation(_arg0);
          return true;
        }
        case TRANSACTION_setZoom:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setZoom(_arg0);
          return true;
        }
        case TRANSACTION_sendSessionModifyRequest:
        {
          data.enforceInterface(descriptor);
          android.telecom.VideoProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.VideoProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telecom.VideoProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.VideoProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.sendSessionModifyRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendSessionModifyResponse:
        {
          data.enforceInterface(descriptor);
          android.telecom.VideoProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.VideoProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.sendSessionModifyResponse(_arg0);
          return true;
        }
        case TRANSACTION_requestCameraCapabilities:
        {
          data.enforceInterface(descriptor);
          this.requestCameraCapabilities();
          return true;
        }
        case TRANSACTION_requestCallDataUsage:
        {
          data.enforceInterface(descriptor);
          this.requestCallDataUsage();
          return true;
        }
        case TRANSACTION_setPauseImage:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setPauseImage(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsVideoCallProvider
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
      @Override public void setCallback(com.android.ims.internal.IImsVideoCallCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setCamera(java.lang.String cameraId, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(cameraId);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCamera, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCamera(cameraId, uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPreviewSurface(android.view.Surface surface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPreviewSurface, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPreviewSurface(surface);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDisplaySurface(android.view.Surface surface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplaySurface, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisplaySurface(surface);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDeviceOrientation(int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeviceOrientation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeviceOrientation(rotation);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setZoom(float value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setZoom, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setZoom(value);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendSessionModifyRequest(android.telecom.VideoProfile fromProfile, android.telecom.VideoProfile toProfile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fromProfile!=null)) {
            _data.writeInt(1);
            fromProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((toProfile!=null)) {
            _data.writeInt(1);
            toProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendSessionModifyRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendSessionModifyRequest(fromProfile, toProfile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendSessionModifyResponse(android.telecom.VideoProfile responseProfile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((responseProfile!=null)) {
            _data.writeInt(1);
            responseProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendSessionModifyResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendSessionModifyResponse(responseProfile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestCameraCapabilities() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestCameraCapabilities, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestCameraCapabilities();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestCallDataUsage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestCallDataUsage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestCallDataUsage();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPauseImage(android.net.Uri uri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPauseImage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPauseImage(uri);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsVideoCallProvider sDefaultImpl;
    }
    static final int TRANSACTION_setCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setCamera = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setPreviewSurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setDisplaySurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setDeviceOrientation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setZoom = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_sendSessionModifyRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_sendSessionModifyResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_requestCameraCapabilities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_requestCallDataUsage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setPauseImage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsVideoCallProvider impl) {
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
    public static com.android.ims.internal.IImsVideoCallProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsVideoCallProvider.aidl:44:1:44:25")
  public void setCallback(com.android.ims.internal.IImsVideoCallCallback callback) throws android.os.RemoteException;
  public void setCamera(java.lang.String cameraId, int uid) throws android.os.RemoteException;
  public void setPreviewSurface(android.view.Surface surface) throws android.os.RemoteException;
  public void setDisplaySurface(android.view.Surface surface) throws android.os.RemoteException;
  public void setDeviceOrientation(int rotation) throws android.os.RemoteException;
  public void setZoom(float value) throws android.os.RemoteException;
  public void sendSessionModifyRequest(android.telecom.VideoProfile fromProfile, android.telecom.VideoProfile toProfile) throws android.os.RemoteException;
  public void sendSessionModifyResponse(android.telecom.VideoProfile responseProfile) throws android.os.RemoteException;
  public void requestCameraCapabilities() throws android.os.RemoteException;
  public void requestCallDataUsage() throws android.os.RemoteException;
  public void setPauseImage(android.net.Uri uri) throws android.os.RemoteException;
}
