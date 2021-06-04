/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * {@hide}
 */
public interface IMediaRouterService extends android.os.IInterface
{
  /** Default implementation for IMediaRouterService. */
  public static class Default implements android.media.IMediaRouterService
  {
    @Override public void registerClientAsUser(android.media.IMediaRouterClient client, java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public void unregisterClient(android.media.IMediaRouterClient client) throws android.os.RemoteException
    {
    }
    @Override public void registerClientGroupId(android.media.IMediaRouterClient client, java.lang.String groupId) throws android.os.RemoteException
    {
    }
    @Override public android.media.MediaRouterClientState getState(android.media.IMediaRouterClient client) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isPlaybackActive(android.media.IMediaRouterClient client) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setDiscoveryRequest(android.media.IMediaRouterClient client, int routeTypes, boolean activeScan) throws android.os.RemoteException
    {
    }
    @Override public void setSelectedRoute(android.media.IMediaRouterClient client, java.lang.String routeId, boolean explicit) throws android.os.RemoteException
    {
    }
    @Override public void requestSetVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int volume) throws android.os.RemoteException
    {
    }
    @Override public void requestUpdateVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int direction) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaRouterService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaRouterService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaRouterService interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaRouterService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaRouterService))) {
        return ((android.media.IMediaRouterService)iin);
      }
      return new android.media.IMediaRouterService.Stub.Proxy(obj);
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
        case TRANSACTION_registerClientAsUser:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.registerClientAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterClient:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          this.unregisterClient(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerClientGroupId:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.registerClientGroupId(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getState:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          android.media.MediaRouterClientState _result = this.getState(_arg0);
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
        case TRANSACTION_isPlaybackActive:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isPlaybackActive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDiscoveryRequest:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setDiscoveryRequest(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSelectedRoute:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setSelectedRoute(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestSetVolume:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.requestSetVolume(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestUpdateVolume:
        {
          data.enforceInterface(descriptor);
          android.media.IMediaRouterClient _arg0;
          _arg0 = android.media.IMediaRouterClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.requestUpdateVolume(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaRouterService
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
      @Override public void registerClientAsUser(android.media.IMediaRouterClient client, java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerClientAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerClientAsUser(client, packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterClient(android.media.IMediaRouterClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterClient(client);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerClientGroupId(android.media.IMediaRouterClient client, java.lang.String groupId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(groupId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerClientGroupId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerClientGroupId(client, groupId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.media.MediaRouterClientState getState(android.media.IMediaRouterClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.MediaRouterClientState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getState(client);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.MediaRouterClientState.CREATOR.createFromParcel(_reply);
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
      @Override public boolean isPlaybackActive(android.media.IMediaRouterClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isPlaybackActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isPlaybackActive(client);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setDiscoveryRequest(android.media.IMediaRouterClient client, int routeTypes, boolean activeScan) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(routeTypes);
          _data.writeInt(((activeScan)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDiscoveryRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDiscoveryRequest(client, routeTypes, activeScan);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSelectedRoute(android.media.IMediaRouterClient client, java.lang.String routeId, boolean explicit) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(routeId);
          _data.writeInt(((explicit)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSelectedRoute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSelectedRoute(client, routeId, explicit);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestSetVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int volume) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(routeId);
          _data.writeInt(volume);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestSetVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestSetVolume(client, routeId, volume);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestUpdateVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int direction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(routeId);
          _data.writeInt(direction);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUpdateVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestUpdateVolume(client, routeId, direction);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.IMediaRouterService sDefaultImpl;
    }
    static final int TRANSACTION_registerClientAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registerClientGroupId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isPlaybackActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setDiscoveryRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setSelectedRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_requestSetVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_requestUpdateVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.media.IMediaRouterService impl) {
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
    public static android.media.IMediaRouterService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerClientAsUser(android.media.IMediaRouterClient client, java.lang.String packageName, int userId) throws android.os.RemoteException;
  public void unregisterClient(android.media.IMediaRouterClient client) throws android.os.RemoteException;
  public void registerClientGroupId(android.media.IMediaRouterClient client, java.lang.String groupId) throws android.os.RemoteException;
  public android.media.MediaRouterClientState getState(android.media.IMediaRouterClient client) throws android.os.RemoteException;
  public boolean isPlaybackActive(android.media.IMediaRouterClient client) throws android.os.RemoteException;
  public void setDiscoveryRequest(android.media.IMediaRouterClient client, int routeTypes, boolean activeScan) throws android.os.RemoteException;
  public void setSelectedRoute(android.media.IMediaRouterClient client, java.lang.String routeId, boolean explicit) throws android.os.RemoteException;
  public void requestSetVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int volume) throws android.os.RemoteException;
  public void requestUpdateVolume(android.media.IMediaRouterClient client, java.lang.String routeId, int direction) throws android.os.RemoteException;
}
