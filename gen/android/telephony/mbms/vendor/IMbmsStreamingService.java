/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms.vendor;
/**
 * @hide
 */
public interface IMbmsStreamingService extends android.os.IInterface
{
  /** Default implementation for IMbmsStreamingService. */
  public static class Default implements android.telephony.mbms.vendor.IMbmsStreamingService
  {
    @Override public int initialize(android.telephony.mbms.IMbmsStreamingSessionCallback callback, int subId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int requestUpdateStreamingServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startStreaming(int subId, java.lang.String serviceId, android.telephony.mbms.IStreamingServiceCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.net.Uri getPlaybackUri(int subId, java.lang.String serviceId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void stopStreaming(int subId, java.lang.String serviceId) throws android.os.RemoteException
    {
    }
    @Override public void dispose(int subId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.vendor.IMbmsStreamingService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.vendor.IMbmsStreamingService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.vendor.IMbmsStreamingService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.vendor.IMbmsStreamingService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.vendor.IMbmsStreamingService))) {
        return ((android.telephony.mbms.vendor.IMbmsStreamingService)iin);
      }
      return new android.telephony.mbms.vendor.IMbmsStreamingService.Stub.Proxy(obj);
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
        case TRANSACTION_initialize:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.IMbmsStreamingSessionCallback _arg0;
          _arg0 = android.telephony.mbms.IMbmsStreamingSessionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.initialize(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestUpdateStreamingServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          int _result = this.requestUpdateStreamingServices(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startStreaming:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.mbms.IStreamingServiceCallback _arg2;
          _arg2 = android.telephony.mbms.IStreamingServiceCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.startStreaming(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getPlaybackUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _result = this.getPlaybackUri(_arg0, _arg1);
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
        case TRANSACTION_stopStreaming:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.stopStreaming(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dispose:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.dispose(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.vendor.IMbmsStreamingService
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
      @Override public int initialize(android.telephony.mbms.IMbmsStreamingSessionCallback callback, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_initialize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().initialize(callback, subId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int requestUpdateStreamingServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStringList(serviceClasses);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUpdateStreamingServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestUpdateStreamingServices(subId, serviceClasses);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int startStreaming(int subId, java.lang.String serviceId, android.telephony.mbms.IStreamingServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(serviceId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startStreaming, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startStreaming(subId, serviceId, callback);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.Uri getPlaybackUri(int subId, java.lang.String serviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(serviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPlaybackUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPlaybackUri(subId, serviceId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
      @Override public void stopStreaming(int subId, java.lang.String serviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(serviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopStreaming, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopStreaming(subId, serviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dispose(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispose, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispose(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.telephony.mbms.vendor.IMbmsStreamingService sDefaultImpl;
    }
    static final int TRANSACTION_initialize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_requestUpdateStreamingServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startStreaming = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getPlaybackUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_stopStreaming = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_dispose = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.telephony.mbms.vendor.IMbmsStreamingService impl) {
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
    public static android.telephony.mbms.vendor.IMbmsStreamingService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/android/telephony/mbms/vendor/IMbmsStreamingService.aidl:29:1:29:25")
  public int initialize(android.telephony.mbms.IMbmsStreamingSessionCallback callback, int subId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/android/telephony/mbms/vendor/IMbmsStreamingService.aidl:32:1:32:25")
  public int requestUpdateStreamingServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/android/telephony/mbms/vendor/IMbmsStreamingService.aidl:35:1:35:25")
  public int startStreaming(int subId, java.lang.String serviceId, android.telephony.mbms.IStreamingServiceCallback callback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/android/telephony/mbms/vendor/IMbmsStreamingService.aidl:39:1:39:25")
  public android.net.Uri getPlaybackUri(int subId, java.lang.String serviceId) throws android.os.RemoteException;
  public void stopStreaming(int subId, java.lang.String serviceId) throws android.os.RemoteException;
  public void dispose(int subId) throws android.os.RemoteException;
}
