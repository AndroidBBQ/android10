/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** @hide */
public interface INetworkMonitor extends android.os.IInterface
{
  /** Default implementation for INetworkMonitor. */
  public static class Default implements android.net.INetworkMonitor
  {
    @Override public void start() throws android.os.RemoteException
    {
    }
    @Override public void launchCaptivePortalApp() throws android.os.RemoteException
    {
    }
    @Override public void notifyCaptivePortalAppFinished(int response) throws android.os.RemoteException
    {
    }
    @Override public void setAcceptPartialConnectivity() throws android.os.RemoteException
    {
    }
    @Override public void forceReevaluation(int uid) throws android.os.RemoteException
    {
    }
    @Override public void notifyPrivateDnsChanged(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException
    {
    }
    @Override public void notifyDnsResponse(int returnCode) throws android.os.RemoteException
    {
    }
    @Override public void notifyNetworkConnected(android.net.LinkProperties lp, android.net.NetworkCapabilities nc) throws android.os.RemoteException
    {
    }
    @Override public void notifyNetworkDisconnected() throws android.os.RemoteException
    {
    }
    @Override public void notifyLinkPropertiesChanged(android.net.LinkProperties lp) throws android.os.RemoteException
    {
    }
    @Override public void notifyNetworkCapabilitiesChanged(android.net.NetworkCapabilities nc) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkMonitor
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkMonitor";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkMonitor interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkMonitor asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkMonitor))) {
        return ((android.net.INetworkMonitor)iin);
      }
      return new android.net.INetworkMonitor.Stub.Proxy(obj);
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
        case TRANSACTION_start:
        {
          data.enforceInterface(descriptor);
          this.start();
          return true;
        }
        case TRANSACTION_launchCaptivePortalApp:
        {
          data.enforceInterface(descriptor);
          this.launchCaptivePortalApp();
          return true;
        }
        case TRANSACTION_notifyCaptivePortalAppFinished:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyCaptivePortalAppFinished(_arg0);
          return true;
        }
        case TRANSACTION_setAcceptPartialConnectivity:
        {
          data.enforceInterface(descriptor);
          this.setAcceptPartialConnectivity();
          return true;
        }
        case TRANSACTION_forceReevaluation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.forceReevaluation(_arg0);
          return true;
        }
        case TRANSACTION_notifyPrivateDnsChanged:
        {
          data.enforceInterface(descriptor);
          android.net.PrivateDnsConfigParcel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.PrivateDnsConfigParcel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyPrivateDnsChanged(_arg0);
          return true;
        }
        case TRANSACTION_notifyDnsResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyDnsResponse(_arg0);
          return true;
        }
        case TRANSACTION_notifyNetworkConnected:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.NetworkCapabilities _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyNetworkConnected(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyNetworkDisconnected:
        {
          data.enforceInterface(descriptor);
          this.notifyNetworkDisconnected();
          return true;
        }
        case TRANSACTION_notifyLinkPropertiesChanged:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyLinkPropertiesChanged(_arg0);
          return true;
        }
        case TRANSACTION_notifyNetworkCapabilitiesChanged:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkCapabilities _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyNetworkCapabilitiesChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkMonitor
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
      @Override public void start() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_start, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().start();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void launchCaptivePortalApp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchCaptivePortalApp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().launchCaptivePortalApp();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyCaptivePortalAppFinished(int response) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(response);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCaptivePortalAppFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCaptivePortalAppFinished(response);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAcceptPartialConnectivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAcceptPartialConnectivity, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAcceptPartialConnectivity();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void forceReevaluation(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceReevaluation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceReevaluation(uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyPrivateDnsChanged(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPrivateDnsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPrivateDnsChanged(config);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyDnsResponse(int returnCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(returnCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDnsResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDnsResponse(returnCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyNetworkConnected(android.net.LinkProperties lp, android.net.NetworkCapabilities nc) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((lp!=null)) {
            _data.writeInt(1);
            lp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((nc!=null)) {
            _data.writeInt(1);
            nc.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyNetworkConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyNetworkConnected(lp, nc);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyNetworkDisconnected() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyNetworkDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyNetworkDisconnected();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyLinkPropertiesChanged(android.net.LinkProperties lp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((lp!=null)) {
            _data.writeInt(1);
            lp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyLinkPropertiesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyLinkPropertiesChanged(lp);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyNetworkCapabilitiesChanged(android.net.NetworkCapabilities nc) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((nc!=null)) {
            _data.writeInt(1);
            nc.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyNetworkCapabilitiesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyNetworkCapabilitiesChanged(nc);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkMonitor sDefaultImpl;
    }
    static final int TRANSACTION_start = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_launchCaptivePortalApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyCaptivePortalAppFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setAcceptPartialConnectivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_forceReevaluation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_notifyPrivateDnsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_notifyDnsResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_notifyNetworkConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_notifyNetworkDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_notifyLinkPropertiesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_notifyNetworkCapabilitiesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.net.INetworkMonitor impl) {
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
    public static android.net.INetworkMonitor getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // After a network has been tested this result can be sent with EVENT_NETWORK_TESTED.
  // The network should be used as a default internet connection.  It was found to be:
  // 1. a functioning network providing internet access, or
  // 2. a captive portal and the user decided to use it as is.

  public static final int NETWORK_TEST_RESULT_VALID = 0;
  // After a network has been tested this result can be sent with EVENT_NETWORK_TESTED.
  // The network should not be used as a default internet connection.  It was found to be:
  // 1. a captive portal and the user is prompted to sign-in, or
  // 2. a captive portal and the user did not want to use it, or
  // 3. a broken network (e.g. DNS failed, connect failed, HTTP request failed).

  public static final int NETWORK_TEST_RESULT_INVALID = 1;
  // After a network has been tested, this result can be sent with EVENT_NETWORK_TESTED.
  // The network may be used as a default internet connection, but it was found to be a partial
  // connectivity network which can get the pass result for http probe but get the failed result
  // for https probe.

  public static final int NETWORK_TEST_RESULT_PARTIAL_CONNECTIVITY = 2;
  // Network validation flags indicate probe result and types. If no NETWORK_VALIDATION_RESULT_*
  // are set, then it's equal to NETWORK_TEST_RESULT_INVALID. If NETWORK_VALIDATION_RESULT_VALID
  // is set, then the network validates and equal to NETWORK_TEST_RESULT_VALID. If
  // NETWORK_VALIDATION_RESULT_PARTIAL is set, then the network has partial connectivity which
  // is equal to NETWORK_TEST_RESULT_PARTIAL_CONNECTIVITY. NETWORK_VALIDATION_PROBE_* is set
  // when the specific probe result of the network is resolved.

  public static final int NETWORK_VALIDATION_RESULT_VALID = 1;
  public static final int NETWORK_VALIDATION_RESULT_PARTIAL = 2;
  public static final int NETWORK_VALIDATION_PROBE_DNS = 4;
  public static final int NETWORK_VALIDATION_PROBE_HTTP = 8;
  public static final int NETWORK_VALIDATION_PROBE_HTTPS = 16;
  public static final int NETWORK_VALIDATION_PROBE_FALLBACK = 32;
  public static final int NETWORK_VALIDATION_PROBE_PRIVDNS = 64;
  public void start() throws android.os.RemoteException;
  public void launchCaptivePortalApp() throws android.os.RemoteException;
  public void notifyCaptivePortalAppFinished(int response) throws android.os.RemoteException;
  public void setAcceptPartialConnectivity() throws android.os.RemoteException;
  public void forceReevaluation(int uid) throws android.os.RemoteException;
  public void notifyPrivateDnsChanged(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException;
  public void notifyDnsResponse(int returnCode) throws android.os.RemoteException;
  public void notifyNetworkConnected(android.net.LinkProperties lp, android.net.NetworkCapabilities nc) throws android.os.RemoteException;
  public void notifyNetworkDisconnected() throws android.os.RemoteException;
  public void notifyLinkPropertiesChanged(android.net.LinkProperties lp) throws android.os.RemoteException;
  public void notifyNetworkCapabilitiesChanged(android.net.NetworkCapabilities nc) throws android.os.RemoteException;
}
