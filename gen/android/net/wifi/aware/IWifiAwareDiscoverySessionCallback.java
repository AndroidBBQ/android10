/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.aware;
/**
 * Callback interface that WifiAwareManager implements
 *
 * {@hide}
 */
public interface IWifiAwareDiscoverySessionCallback extends android.os.IInterface
{
  /** Default implementation for IWifiAwareDiscoverySessionCallback. */
  public static class Default implements android.net.wifi.aware.IWifiAwareDiscoverySessionCallback
  {
    @Override public void onSessionStarted(int discoverySessionId) throws android.os.RemoteException
    {
    }
    @Override public void onSessionConfigSuccess() throws android.os.RemoteException
    {
    }
    @Override public void onSessionConfigFail(int reason) throws android.os.RemoteException
    {
    }
    @Override public void onSessionTerminated(int reason) throws android.os.RemoteException
    {
    }
    @Override public void onMatch(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter) throws android.os.RemoteException
    {
    }
    @Override public void onMatchWithDistance(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter, int distanceMm) throws android.os.RemoteException
    {
    }
    @Override public void onMessageSendSuccess(int messageId) throws android.os.RemoteException
    {
    }
    @Override public void onMessageSendFail(int messageId, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onMessageReceived(int peerId, byte[] message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.aware.IWifiAwareDiscoverySessionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.aware.IWifiAwareDiscoverySessionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.aware.IWifiAwareDiscoverySessionCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.aware.IWifiAwareDiscoverySessionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.aware.IWifiAwareDiscoverySessionCallback))) {
        return ((android.net.wifi.aware.IWifiAwareDiscoverySessionCallback)iin);
      }
      return new android.net.wifi.aware.IWifiAwareDiscoverySessionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSessionStarted:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSessionStarted(_arg0);
          return true;
        }
        case TRANSACTION_onSessionConfigSuccess:
        {
          data.enforceInterface(descriptor);
          this.onSessionConfigSuccess();
          return true;
        }
        case TRANSACTION_onSessionConfigFail:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSessionConfigFail(_arg0);
          return true;
        }
        case TRANSACTION_onSessionTerminated:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSessionTerminated(_arg0);
          return true;
        }
        case TRANSACTION_onMatch:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.onMatch(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onMatchWithDistance:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          int _arg3;
          _arg3 = data.readInt();
          this.onMatchWithDistance(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onMessageSendSuccess:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onMessageSendSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onMessageSendFail:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onMessageSendFail(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMessageReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.onMessageReceived(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.aware.IWifiAwareDiscoverySessionCallback
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
      @Override public void onSessionStarted(int discoverySessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(discoverySessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionStarted(discoverySessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionConfigSuccess() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionConfigSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionConfigSuccess();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionConfigFail(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionConfigFail, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionConfigFail(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionTerminated(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionTerminated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionTerminated(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMatch(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(peerId);
          _data.writeByteArray(serviceSpecificInfo);
          _data.writeByteArray(matchFilter);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMatch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMatch(peerId, serviceSpecificInfo, matchFilter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMatchWithDistance(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter, int distanceMm) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(peerId);
          _data.writeByteArray(serviceSpecificInfo);
          _data.writeByteArray(matchFilter);
          _data.writeInt(distanceMm);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMatchWithDistance, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMatchWithDistance(peerId, serviceSpecificInfo, matchFilter, distanceMm);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMessageSendSuccess(int messageId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageSendSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageSendSuccess(messageId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMessageSendFail(int messageId, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageSendFail, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageSendFail(messageId, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMessageReceived(int peerId, byte[] message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(peerId);
          _data.writeByteArray(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageReceived(peerId, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.aware.IWifiAwareDiscoverySessionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSessionStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSessionConfigSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSessionConfigFail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSessionTerminated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onMatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onMatchWithDistance = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onMessageSendSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onMessageSendFail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onMessageReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.net.wifi.aware.IWifiAwareDiscoverySessionCallback impl) {
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
    public static android.net.wifi.aware.IWifiAwareDiscoverySessionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSessionStarted(int discoverySessionId) throws android.os.RemoteException;
  public void onSessionConfigSuccess() throws android.os.RemoteException;
  public void onSessionConfigFail(int reason) throws android.os.RemoteException;
  public void onSessionTerminated(int reason) throws android.os.RemoteException;
  public void onMatch(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter) throws android.os.RemoteException;
  public void onMatchWithDistance(int peerId, byte[] serviceSpecificInfo, byte[] matchFilter, int distanceMm) throws android.os.RemoteException;
  public void onMessageSendSuccess(int messageId) throws android.os.RemoteException;
  public void onMessageSendFail(int messageId, int reason) throws android.os.RemoteException;
  public void onMessageReceived(int peerId, byte[] message) throws android.os.RemoteException;
}
