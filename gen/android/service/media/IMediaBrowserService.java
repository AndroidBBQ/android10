/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.media;
/**
 * Media API allows clients to browse through hierarchy of a user’s media collection,
 * playback a specific media entry and interact with the now playing queue.
 * @hide
 */
public interface IMediaBrowserService extends android.os.IInterface
{
  /** Default implementation for IMediaBrowserService. */
  public static class Default implements android.service.media.IMediaBrowserService
  {
    @Override public void connect(java.lang.String pkg, android.os.Bundle rootHints, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void disconnect(android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void addSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void removeSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void getMediaItem(java.lang.String uri, android.os.ResultReceiver cb, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void addSubscription(java.lang.String uri, android.os.IBinder token, android.os.Bundle options, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void removeSubscription(java.lang.String uri, android.os.IBinder token, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.media.IMediaBrowserService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.media.IMediaBrowserService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.media.IMediaBrowserService interface,
     * generating a proxy if needed.
     */
    public static android.service.media.IMediaBrowserService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.media.IMediaBrowserService))) {
        return ((android.service.media.IMediaBrowserService)iin);
      }
      return new android.service.media.IMediaBrowserService.Stub.Proxy(obj);
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
        case TRANSACTION_connect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.media.IMediaBrowserServiceCallbacks _arg2;
          _arg2 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.connect(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          android.service.media.IMediaBrowserServiceCallbacks _arg0;
          _arg0 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.disconnect(_arg0);
          return true;
        }
        case TRANSACTION_addSubscriptionDeprecated:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.service.media.IMediaBrowserServiceCallbacks _arg1;
          _arg1 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.addSubscriptionDeprecated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeSubscriptionDeprecated:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.service.media.IMediaBrowserServiceCallbacks _arg1;
          _arg1 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.removeSubscriptionDeprecated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getMediaItem:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.media.IMediaBrowserServiceCallbacks _arg2;
          _arg2 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.getMediaItem(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_addSubscription:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.service.media.IMediaBrowserServiceCallbacks _arg3;
          _arg3 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.addSubscription(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_removeSubscription:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.service.media.IMediaBrowserServiceCallbacks _arg2;
          _arg2 = android.service.media.IMediaBrowserServiceCallbacks.Stub.asInterface(data.readStrongBinder());
          this.removeSubscription(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.media.IMediaBrowserService
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
      @Override public void connect(java.lang.String pkg, android.os.Bundle rootHints, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          if ((rootHints!=null)) {
            _data.writeInt(1);
            rootHints.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_connect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connect(pkg, rootHints, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disconnect(android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect(callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uri);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSubscriptionDeprecated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSubscriptionDeprecated(uri, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uri);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSubscriptionDeprecated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSubscriptionDeprecated(uri, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getMediaItem(java.lang.String uri, android.os.ResultReceiver cb, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uri);
          if ((cb!=null)) {
            _data.writeInt(1);
            cb.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMediaItem, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getMediaItem(uri, cb, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addSubscription(java.lang.String uri, android.os.IBinder token, android.os.Bundle options, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uri);
          _data.writeStrongBinder(token);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSubscription, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSubscription(uri, token, options, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeSubscription(java.lang.String uri, android.os.IBinder token, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uri);
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSubscription, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSubscription(uri, token, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.media.IMediaBrowserService sDefaultImpl;
    }
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_addSubscriptionDeprecated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_removeSubscriptionDeprecated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getMediaItem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.service.media.IMediaBrowserService impl) {
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
    public static android.service.media.IMediaBrowserService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void connect(java.lang.String pkg, android.os.Bundle rootHints, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void disconnect(android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void addSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void removeSubscriptionDeprecated(java.lang.String uri, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void getMediaItem(java.lang.String uri, android.os.ResultReceiver cb, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void addSubscription(java.lang.String uri, android.os.IBinder token, android.os.Bundle options, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
  public void removeSubscription(java.lang.String uri, android.os.IBinder token, android.service.media.IMediaBrowserServiceCallbacks callbacks) throws android.os.RemoteException;
}
