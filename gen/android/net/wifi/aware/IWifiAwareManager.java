/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.aware;
/**
 * Interface that WifiAwareService implements
 *
 * {@hide}
 */
public interface IWifiAwareManager extends android.os.IInterface
{
  /** Default implementation for IWifiAwareManager. */
  public static class Default implements android.net.wifi.aware.IWifiAwareManager
  {
    // Aware API

    @Override public boolean isUsageEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.net.wifi.aware.Characteristics getCharacteristics() throws android.os.RemoteException
    {
      return null;
    }
    // client API

    @Override public void connect(android.os.IBinder binder, java.lang.String callingPackage, android.net.wifi.aware.IWifiAwareEventCallback callback, android.net.wifi.aware.ConfigRequest configRequest, boolean notifyOnIdentityChanged) throws android.os.RemoteException
    {
    }
    @Override public void disconnect(int clientId, android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override public void publish(java.lang.String callingPackage, int clientId, android.net.wifi.aware.PublishConfig publishConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void subscribe(java.lang.String callingPackage, int clientId, android.net.wifi.aware.SubscribeConfig subscribeConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException
    {
    }
    // session API

    @Override public void updatePublish(int clientId, int discoverySessionId, android.net.wifi.aware.PublishConfig publishConfig) throws android.os.RemoteException
    {
    }
    @Override public void updateSubscribe(int clientId, int discoverySessionId, android.net.wifi.aware.SubscribeConfig subscribeConfig) throws android.os.RemoteException
    {
    }
    @Override public void sendMessage(int clientId, int discoverySessionId, int peerId, byte[] message, int messageId, int retryCount) throws android.os.RemoteException
    {
    }
    @Override public void terminateSession(int clientId, int discoverySessionId) throws android.os.RemoteException
    {
    }
    // internal APIs: intended to be used between System Services (restricted permissions)

    @Override public void requestMacAddresses(int uid, java.util.List peerIds, android.net.wifi.aware.IWifiAwareMacAddressProvider callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.aware.IWifiAwareManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.aware.IWifiAwareManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.aware.IWifiAwareManager interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.aware.IWifiAwareManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.aware.IWifiAwareManager))) {
        return ((android.net.wifi.aware.IWifiAwareManager)iin);
      }
      return new android.net.wifi.aware.IWifiAwareManager.Stub.Proxy(obj);
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
        case TRANSACTION_isUsageEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isUsageEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCharacteristics:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.aware.Characteristics _result = this.getCharacteristics();
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
        case TRANSACTION_connect:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.wifi.aware.IWifiAwareEventCallback _arg2;
          _arg2 = android.net.wifi.aware.IWifiAwareEventCallback.Stub.asInterface(data.readStrongBinder());
          android.net.wifi.aware.ConfigRequest _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.net.wifi.aware.ConfigRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.connect(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.disconnect(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_publish:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.net.wifi.aware.PublishConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.wifi.aware.PublishConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.net.wifi.aware.IWifiAwareDiscoverySessionCallback _arg3;
          _arg3 = android.net.wifi.aware.IWifiAwareDiscoverySessionCallback.Stub.asInterface(data.readStrongBinder());
          this.publish(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_subscribe:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.net.wifi.aware.SubscribeConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.wifi.aware.SubscribeConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.net.wifi.aware.IWifiAwareDiscoverySessionCallback _arg3;
          _arg3 = android.net.wifi.aware.IWifiAwareDiscoverySessionCallback.Stub.asInterface(data.readStrongBinder());
          this.subscribe(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updatePublish:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.net.wifi.aware.PublishConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.wifi.aware.PublishConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.updatePublish(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateSubscribe:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.net.wifi.aware.SubscribeConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.wifi.aware.SubscribeConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.updateSubscribe(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          this.sendMessage(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_terminateSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.terminateSession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestMacAddresses:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List _arg1;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg1 = data.readArrayList(cl);
          android.net.wifi.aware.IWifiAwareMacAddressProvider _arg2;
          _arg2 = android.net.wifi.aware.IWifiAwareMacAddressProvider.Stub.asInterface(data.readStrongBinder());
          this.requestMacAddresses(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.aware.IWifiAwareManager
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
      // Aware API

      @Override public boolean isUsageEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUsageEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUsageEnabled();
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
      @Override public android.net.wifi.aware.Characteristics getCharacteristics() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.wifi.aware.Characteristics _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCharacteristics, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCharacteristics();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.wifi.aware.Characteristics.CREATOR.createFromParcel(_reply);
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
      // client API

      @Override public void connect(android.os.IBinder binder, java.lang.String callingPackage, android.net.wifi.aware.IWifiAwareEventCallback callback, android.net.wifi.aware.ConfigRequest configRequest, boolean notifyOnIdentityChanged) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((configRequest!=null)) {
            _data.writeInt(1);
            configRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((notifyOnIdentityChanged)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_connect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connect(binder, callingPackage, callback, configRequest, notifyOnIdentityChanged);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disconnect(int clientId, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect(clientId, binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void publish(java.lang.String callingPackage, int clientId, android.net.wifi.aware.PublishConfig publishConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(clientId);
          if ((publishConfig!=null)) {
            _data.writeInt(1);
            publishConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_publish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().publish(callingPackage, clientId, publishConfig, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void subscribe(java.lang.String callingPackage, int clientId, android.net.wifi.aware.SubscribeConfig subscribeConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(clientId);
          if ((subscribeConfig!=null)) {
            _data.writeInt(1);
            subscribeConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_subscribe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().subscribe(callingPackage, clientId, subscribeConfig, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // session API

      @Override public void updatePublish(int clientId, int discoverySessionId, android.net.wifi.aware.PublishConfig publishConfig) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(discoverySessionId);
          if ((publishConfig!=null)) {
            _data.writeInt(1);
            publishConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePublish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updatePublish(clientId, discoverySessionId, publishConfig);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateSubscribe(int clientId, int discoverySessionId, android.net.wifi.aware.SubscribeConfig subscribeConfig) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(discoverySessionId);
          if ((subscribeConfig!=null)) {
            _data.writeInt(1);
            subscribeConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateSubscribe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateSubscribe(clientId, discoverySessionId, subscribeConfig);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendMessage(int clientId, int discoverySessionId, int peerId, byte[] message, int messageId, int retryCount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(discoverySessionId);
          _data.writeInt(peerId);
          _data.writeByteArray(message);
          _data.writeInt(messageId);
          _data.writeInt(retryCount);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMessage(clientId, discoverySessionId, peerId, message, messageId, retryCount);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void terminateSession(int clientId, int discoverySessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(discoverySessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_terminateSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().terminateSession(clientId, discoverySessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // internal APIs: intended to be used between System Services (restricted permissions)

      @Override public void requestMacAddresses(int uid, java.util.List peerIds, android.net.wifi.aware.IWifiAwareMacAddressProvider callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeList(peerIds);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestMacAddresses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestMacAddresses(uid, peerIds, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.wifi.aware.IWifiAwareManager sDefaultImpl;
    }
    static final int TRANSACTION_isUsageEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getCharacteristics = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_publish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_subscribe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_updatePublish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_updateSubscribe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_sendMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_terminateSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_requestMacAddresses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.net.wifi.aware.IWifiAwareManager impl) {
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
    public static android.net.wifi.aware.IWifiAwareManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Aware API

  public boolean isUsageEnabled() throws android.os.RemoteException;
  public android.net.wifi.aware.Characteristics getCharacteristics() throws android.os.RemoteException;
  // client API

  public void connect(android.os.IBinder binder, java.lang.String callingPackage, android.net.wifi.aware.IWifiAwareEventCallback callback, android.net.wifi.aware.ConfigRequest configRequest, boolean notifyOnIdentityChanged) throws android.os.RemoteException;
  public void disconnect(int clientId, android.os.IBinder binder) throws android.os.RemoteException;
  public void publish(java.lang.String callingPackage, int clientId, android.net.wifi.aware.PublishConfig publishConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException;
  public void subscribe(java.lang.String callingPackage, int clientId, android.net.wifi.aware.SubscribeConfig subscribeConfig, android.net.wifi.aware.IWifiAwareDiscoverySessionCallback callback) throws android.os.RemoteException;
  // session API

  public void updatePublish(int clientId, int discoverySessionId, android.net.wifi.aware.PublishConfig publishConfig) throws android.os.RemoteException;
  public void updateSubscribe(int clientId, int discoverySessionId, android.net.wifi.aware.SubscribeConfig subscribeConfig) throws android.os.RemoteException;
  public void sendMessage(int clientId, int discoverySessionId, int peerId, byte[] message, int messageId, int retryCount) throws android.os.RemoteException;
  public void terminateSession(int clientId, int discoverySessionId) throws android.os.RemoteException;
  // internal APIs: intended to be used between System Services (restricted permissions)

  public void requestMacAddresses(int uid, java.util.List peerIds, android.net.wifi.aware.IWifiAwareMacAddressProvider callback) throws android.os.RemoteException;
}
