/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Interface that answers queries about, and allows changing, the
 * state of network connectivity.
 *//** {@hide} */
public interface IConnectivityManager extends android.os.IInterface
{
  /** Default implementation for IConnectivityManager. */
  public static class Default implements android.net.IConnectivityManager
  {
    @Override public android.net.Network getActiveNetwork() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.Network getActiveNetworkForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkInfo getActiveNetworkInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkInfo getActiveNetworkInfoForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkInfo getNetworkInfo(int networkType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkInfo getNetworkInfoForUid(android.net.Network network, int uid, boolean ignoreBlocked) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkInfo[] getAllNetworkInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.Network getNetworkForType(int networkType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.Network[] getAllNetworks() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkCapabilities[] getDefaultNetworkCapabilitiesForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isNetworkSupported(int networkType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.net.LinkProperties getActiveLinkProperties() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.LinkProperties getLinkPropertiesForType(int networkType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.LinkProperties getLinkProperties(android.net.Network network) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkCapabilities getNetworkCapabilities(android.net.Network network) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkState[] getAllNetworkState() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkQuotaInfo getActiveNetworkQuotaInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isActiveNetworkMetered() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean requestRouteToHostAddress(int networkType, byte[] hostAddress) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int tether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int untether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getLastTetherError(java.lang.String iface) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isTetheringSupported(java.lang.String callerPkg) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startTethering(int type, android.os.ResultReceiver receiver, boolean showProvisioningUi, java.lang.String callerPkg) throws android.os.RemoteException
    {
    }
    @Override public void stopTethering(int type, java.lang.String callerPkg) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String[] getTetherableIfaces() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetheredIfaces() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetheringErroredIfaces() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetheredDhcpRanges() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetherableUsbRegexs() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetherableWifiRegexs() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getTetherableBluetoothRegexs() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int setUsbTethering(boolean enable, java.lang.String callerPkg) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void reportInetCondition(int networkType, int percentage) throws android.os.RemoteException
    {
    }
    @Override public void reportNetworkConnectivity(android.net.Network network, boolean hasConnectivity) throws android.os.RemoteException
    {
    }
    @Override public android.net.ProxyInfo getGlobalProxy() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setGlobalProxy(android.net.ProxyInfo p) throws android.os.RemoteException
    {
    }
    @Override public android.net.ProxyInfo getProxyForNetwork(android.net.Network nework) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean prepareVpn(java.lang.String oldPackage, java.lang.String newPackage, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setVpnPackageAuthorization(java.lang.String packageName, int userId, boolean authorized) throws android.os.RemoteException
    {
    }
    @Override public android.os.ParcelFileDescriptor establishVpn(com.android.internal.net.VpnConfig config) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.net.VpnConfig getVpnConfig(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void startLegacyVpn(com.android.internal.net.VpnProfile profile) throws android.os.RemoteException
    {
    }
    @Override public com.android.internal.net.LegacyVpnInfo getLegacyVpnInfo(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean updateLockdownVpn() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isAlwaysOnVpnPackageSupported(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAlwaysOnVpnPackage(int userId, java.lang.String packageName, boolean lockdown, java.util.List<java.lang.String> lockdownWhitelist) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String getAlwaysOnVpnPackage(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isVpnLockdownEnabled(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<java.lang.String> getVpnLockdownWhitelist(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int checkMobileProvisioning(int suggestedTimeOutMs) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getMobileProvisioningUrl() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setProvisioningNotificationVisible(boolean visible, int networkType, java.lang.String action) throws android.os.RemoteException
    {
    }
    @Override public void setAirplaneMode(boolean enable) throws android.os.RemoteException
    {
    }
    @Override public int registerNetworkFactory(android.os.Messenger messenger, java.lang.String name) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean requestBandwidthUpdate(android.net.Network network) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void unregisterNetworkFactory(android.os.Messenger messenger) throws android.os.RemoteException
    {
    }
    @Override public int registerNetworkAgent(android.os.Messenger messenger, android.net.NetworkInfo ni, android.net.LinkProperties lp, android.net.NetworkCapabilities nc, int score, android.net.NetworkMisc misc, int factorySerialNumber) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.net.NetworkRequest requestNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, int timeoutSec, android.os.IBinder binder, int legacy) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkRequest pendingRequestForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void releasePendingNetworkRequest(android.app.PendingIntent operation) throws android.os.RemoteException
    {
    }
    @Override public android.net.NetworkRequest listenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void pendingListenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException
    {
    }
    @Override public void releaseNetworkRequest(android.net.NetworkRequest networkRequest) throws android.os.RemoteException
    {
    }
    @Override public void setAcceptUnvalidated(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException
    {
    }
    @Override public void setAcceptPartialConnectivity(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException
    {
    }
    @Override public void setAvoidUnvalidated(android.net.Network network) throws android.os.RemoteException
    {
    }
    @Override public void startCaptivePortalApp(android.net.Network network) throws android.os.RemoteException
    {
    }
    @Override public void startCaptivePortalAppInternal(android.net.Network network, android.os.Bundle appExtras) throws android.os.RemoteException
    {
    }
    @Override public boolean shouldAvoidBadWifi() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getMultipathPreference(android.net.Network Network) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.net.NetworkRequest getDefaultRequest() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getRestoreDefaultNetworkDelay(int networkType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean addVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setUnderlyingNetworksForVpn(android.net.Network[] networks) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void factoryReset() throws android.os.RemoteException
    {
    }
    @Override public void startNattKeepalive(android.net.Network network, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, int srcPort, java.lang.String dstAddr) throws android.os.RemoteException
    {
    }
    @Override public void startNattKeepaliveWithFd(android.net.Network network, java.io.FileDescriptor fd, int resourceId, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, java.lang.String dstAddr) throws android.os.RemoteException
    {
    }
    @Override public void startTcpKeepalive(android.net.Network network, java.io.FileDescriptor fd, int intervalSeconds, android.net.ISocketKeepaliveCallback cb) throws android.os.RemoteException
    {
    }
    @Override public void stopKeepalive(android.net.Network network, int slot) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getCaptivePortalServerUrl() throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] getNetworkWatchlistConfigHash() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getConnectionOwnerUid(android.net.ConnectionInfo connectionInfo) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isCallerCurrentAlwaysOnVpnApp() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isCallerCurrentAlwaysOnVpnLockdownApp() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void getLatestTetheringEntitlementResult(int type, android.os.ResultReceiver receiver, boolean showEntitlementUi, java.lang.String callerPkg) throws android.os.RemoteException
    {
    }
    @Override public void registerTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException
    {
    }
    @Override public void unregisterTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException
    {
    }
    @Override public android.os.IBinder startOrGetTestNetworkService() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.IConnectivityManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.IConnectivityManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.IConnectivityManager interface,
     * generating a proxy if needed.
     */
    public static android.net.IConnectivityManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.IConnectivityManager))) {
        return ((android.net.IConnectivityManager)iin);
      }
      return new android.net.IConnectivityManager.Stub.Proxy(obj);
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
        case TRANSACTION_getActiveNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.Network _result = this.getActiveNetwork();
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
        case TRANSACTION_getActiveNetworkForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.net.Network _result = this.getActiveNetworkForUid(_arg0, _arg1);
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
        case TRANSACTION_getActiveNetworkInfo:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkInfo _result = this.getActiveNetworkInfo();
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
        case TRANSACTION_getActiveNetworkInfoForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.net.NetworkInfo _result = this.getActiveNetworkInfoForUid(_arg0, _arg1);
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
        case TRANSACTION_getNetworkInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NetworkInfo _result = this.getNetworkInfo(_arg0);
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
        case TRANSACTION_getNetworkInfoForUid:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.net.NetworkInfo _result = this.getNetworkInfoForUid(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getAllNetworkInfo:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkInfo[] _result = this.getAllNetworkInfo();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getNetworkForType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.Network _result = this.getNetworkForType(_arg0);
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
        case TRANSACTION_getAllNetworks:
        {
          data.enforceInterface(descriptor);
          android.net.Network[] _result = this.getAllNetworks();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getDefaultNetworkCapabilitiesForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NetworkCapabilities[] _result = this.getDefaultNetworkCapabilitiesForUser(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_isNetworkSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isNetworkSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getActiveLinkProperties:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _result = this.getActiveLinkProperties();
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
        case TRANSACTION_getLinkPropertiesForType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.LinkProperties _result = this.getLinkPropertiesForType(_arg0);
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
        case TRANSACTION_getLinkProperties:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.LinkProperties _result = this.getLinkProperties(_arg0);
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
        case TRANSACTION_getNetworkCapabilities:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.NetworkCapabilities _result = this.getNetworkCapabilities(_arg0);
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
        case TRANSACTION_getAllNetworkState:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkState[] _result = this.getAllNetworkState();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getActiveNetworkQuotaInfo:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkQuotaInfo _result = this.getActiveNetworkQuotaInfo();
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
        case TRANSACTION_isActiveNetworkMetered:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isActiveNetworkMetered();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestRouteToHostAddress:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          boolean _result = this.requestRouteToHostAddress(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_tether:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.tether(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_untether:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.untether(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getLastTetherError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getLastTetherError(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isTetheringSupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isTetheringSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startTethering:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.startTethering(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopTethering:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.stopTethering(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTetherableIfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetherableIfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetheredIfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetheredIfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetheringErroredIfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetheringErroredIfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetheredDhcpRanges:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetheredDhcpRanges();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetherableUsbRegexs:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetherableUsbRegexs();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetherableWifiRegexs:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetherableWifiRegexs();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getTetherableBluetoothRegexs:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTetherableBluetoothRegexs();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_setUsbTethering:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.setUsbTethering(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_reportInetCondition:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.reportInetCondition(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportNetworkConnectivity:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.reportNetworkConnectivity(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGlobalProxy:
        {
          data.enforceInterface(descriptor);
          android.net.ProxyInfo _result = this.getGlobalProxy();
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
        case TRANSACTION_setGlobalProxy:
        {
          data.enforceInterface(descriptor);
          android.net.ProxyInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.ProxyInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setGlobalProxy(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProxyForNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.ProxyInfo _result = this.getProxyForNetwork(_arg0);
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
        case TRANSACTION_prepareVpn:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.prepareVpn(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVpnPackageAuthorization:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setVpnPackageAuthorization(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_establishVpn:
        {
          data.enforceInterface(descriptor);
          com.android.internal.net.VpnConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.internal.net.VpnConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelFileDescriptor _result = this.establishVpn(_arg0);
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
        case TRANSACTION_getVpnConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.net.VpnConfig _result = this.getVpnConfig(_arg0);
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
        case TRANSACTION_startLegacyVpn:
        {
          data.enforceInterface(descriptor);
          com.android.internal.net.VpnProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.internal.net.VpnProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startLegacyVpn(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLegacyVpnInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.net.LegacyVpnInfo _result = this.getLegacyVpnInfo(_arg0);
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
        case TRANSACTION_updateLockdownVpn:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.updateLockdownVpn();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isAlwaysOnVpnPackageSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isAlwaysOnVpnPackageSupported(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAlwaysOnVpnPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.util.List<java.lang.String> _arg3;
          _arg3 = data.createStringArrayList();
          boolean _result = this.setAlwaysOnVpnPackage(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAlwaysOnVpnPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getAlwaysOnVpnPackage(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_isVpnLockdownEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isVpnLockdownEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getVpnLockdownWhitelist:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _result = this.getVpnLockdownWhitelist(_arg0);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_checkMobileProvisioning:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.checkMobileProvisioning(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMobileProvisioningUrl:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getMobileProvisioningUrl();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setProvisioningNotificationVisible:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setProvisioningNotificationVisible(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAirplaneMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setAirplaneMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerNetworkFactory:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.registerNetworkFactory(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestBandwidthUpdate:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.requestBandwidthUpdate(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterNetworkFactory:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.unregisterNetworkFactory(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerNetworkAgent:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.NetworkInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.NetworkInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.net.LinkProperties _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.net.NetworkCapabilities _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          android.net.NetworkMisc _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.net.NetworkMisc.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          int _result = this.registerNetworkAgent(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkCapabilities _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Messenger _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          int _arg4;
          _arg4 = data.readInt();
          android.net.NetworkRequest _result = this.requestNetwork(_arg0, _arg1, _arg2, _arg3, _arg4);
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
        case TRANSACTION_pendingRequestForNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkCapabilities _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.net.NetworkRequest _result = this.pendingRequestForNetwork(_arg0, _arg1);
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
        case TRANSACTION_releasePendingNetworkRequest:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.releasePendingNetworkRequest(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listenForNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkCapabilities _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Messenger _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          android.net.NetworkRequest _result = this.listenForNetwork(_arg0, _arg1, _arg2);
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
        case TRANSACTION_pendingListenForNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkCapabilities _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.pendingListenForNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseNetworkRequest:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.NetworkRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.releaseNetworkRequest(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAcceptUnvalidated:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setAcceptUnvalidated(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAcceptPartialConnectivity:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setAcceptPartialConnectivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAvoidUnvalidated:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setAvoidUnvalidated(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startCaptivePortalApp:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startCaptivePortalApp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startCaptivePortalAppInternal:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
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
          this.startCaptivePortalAppInternal(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shouldAvoidBadWifi:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.shouldAvoidBadWifi();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getMultipathPreference:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getMultipathPreference(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDefaultRequest:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkRequest _result = this.getDefaultRequest();
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
        case TRANSACTION_getRestoreDefaultNetworkDelay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getRestoreDefaultNetworkDelay(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addVpnAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.addVpnAddress(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeVpnAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.removeVpnAddress(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setUnderlyingNetworksForVpn:
        {
          data.enforceInterface(descriptor);
          android.net.Network[] _arg0;
          _arg0 = data.createTypedArray(android.net.Network.CREATOR);
          boolean _result = this.setUnderlyingNetworksForVpn(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_factoryReset:
        {
          data.enforceInterface(descriptor);
          this.factoryReset();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startNattKeepalive:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.net.ISocketKeepaliveCallback _arg2;
          _arg2 = android.net.ISocketKeepaliveCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.startNattKeepalive(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startNattKeepaliveWithFd:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.io.FileDescriptor _arg1;
          _arg1 = data.readRawFileDescriptor();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.net.ISocketKeepaliveCallback _arg4;
          _arg4 = android.net.ISocketKeepaliveCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg5;
          _arg5 = data.readString();
          java.lang.String _arg6;
          _arg6 = data.readString();
          this.startNattKeepaliveWithFd(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startTcpKeepalive:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.io.FileDescriptor _arg1;
          _arg1 = data.readRawFileDescriptor();
          int _arg2;
          _arg2 = data.readInt();
          android.net.ISocketKeepaliveCallback _arg3;
          _arg3 = android.net.ISocketKeepaliveCallback.Stub.asInterface(data.readStrongBinder());
          this.startTcpKeepalive(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopKeepalive:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.stopKeepalive(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCaptivePortalServerUrl:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getCaptivePortalServerUrl();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getNetworkWatchlistConfigHash:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getNetworkWatchlistConfigHash();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_getConnectionOwnerUid:
        {
          data.enforceInterface(descriptor);
          android.net.ConnectionInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.ConnectionInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getConnectionOwnerUid(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isCallerCurrentAlwaysOnVpnApp:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isCallerCurrentAlwaysOnVpnApp();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isCallerCurrentAlwaysOnVpnLockdownApp:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isCallerCurrentAlwaysOnVpnLockdownApp();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLatestTetheringEntitlementResult:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.getLatestTetheringEntitlementResult(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTetheringEventCallback:
        {
          data.enforceInterface(descriptor);
          android.net.ITetheringEventCallback _arg0;
          _arg0 = android.net.ITetheringEventCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.registerTetheringEventCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTetheringEventCallback:
        {
          data.enforceInterface(descriptor);
          android.net.ITetheringEventCallback _arg0;
          _arg0 = android.net.ITetheringEventCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterTetheringEventCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startOrGetTestNetworkService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _result = this.startOrGetTestNetworkService();
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.IConnectivityManager
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
      @Override public android.net.Network getActiveNetwork() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Network _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveNetwork();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Network.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.Network getActiveNetworkForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Network _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((ignoreBlocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveNetworkForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveNetworkForUid(uid, ignoreBlocked);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Network.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkInfo getActiveNetworkInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveNetworkInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveNetworkInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkInfo getActiveNetworkInfoForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((ignoreBlocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveNetworkInfoForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveNetworkInfoForUid(uid, ignoreBlocked);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkInfo getNetworkInfo(int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkInfo(networkType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkInfo getNetworkInfoForUid(android.net.Network network, int uid, boolean ignoreBlocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(uid);
          _data.writeInt(((ignoreBlocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkInfoForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkInfoForUid(network, uid, ignoreBlocked);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkInfo[] getAllNetworkInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllNetworkInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllNetworkInfo();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.NetworkInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.Network getNetworkForType(int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Network _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkForType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkForType(networkType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Network.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.Network[] getAllNetworks() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Network[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllNetworks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllNetworks();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.Network.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.NetworkCapabilities[] getDefaultNetworkCapabilitiesForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkCapabilities[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultNetworkCapabilitiesForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultNetworkCapabilitiesForUser(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.NetworkCapabilities.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean isNetworkSupported(int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNetworkSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNetworkSupported(networkType);
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
      @Override public android.net.LinkProperties getActiveLinkProperties() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.LinkProperties _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveLinkProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveLinkProperties();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.LinkProperties.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.LinkProperties getLinkPropertiesForType(int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.LinkProperties _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLinkPropertiesForType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLinkPropertiesForType(networkType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.LinkProperties.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.LinkProperties getLinkProperties(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.LinkProperties _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLinkProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLinkProperties(network);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.LinkProperties.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkCapabilities getNetworkCapabilities(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkCapabilities _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkCapabilities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkCapabilities(network);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkCapabilities.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkState[] getAllNetworkState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkState[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllNetworkState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllNetworkState();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.NetworkState.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.NetworkQuotaInfo getActiveNetworkQuotaInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkQuotaInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveNetworkQuotaInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveNetworkQuotaInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkQuotaInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean isActiveNetworkMetered() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActiveNetworkMetered, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActiveNetworkMetered();
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
      @Override public boolean requestRouteToHostAddress(int networkType, byte[] hostAddress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          _data.writeByteArray(hostAddress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestRouteToHostAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestRouteToHostAddress(networkType, hostAddress);
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
      @Override public int tether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_tether, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().tether(iface, callerPkg);
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
      @Override public int untether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_untether, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().untether(iface, callerPkg);
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
      @Override public int getLastTetherError(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastTetherError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastTetherError(iface);
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
      @Override public boolean isTetheringSupported(java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTetheringSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTetheringSupported(callerPkg);
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
      @Override public void startTethering(int type, android.os.ResultReceiver receiver, boolean showProvisioningUi, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          if ((receiver!=null)) {
            _data.writeInt(1);
            receiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((showProvisioningUi)?(1):(0)));
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startTethering(type, receiver, showProvisioningUi, callerPkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopTethering(int type, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopTethering(type, callerPkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String[] getTetherableIfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherableIfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherableIfaces();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetheredIfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetheredIfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetheredIfaces();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetheringErroredIfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetheringErroredIfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetheringErroredIfaces();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetheredDhcpRanges() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetheredDhcpRanges, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetheredDhcpRanges();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetherableUsbRegexs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherableUsbRegexs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherableUsbRegexs();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetherableWifiRegexs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherableWifiRegexs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherableWifiRegexs();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getTetherableBluetoothRegexs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherableBluetoothRegexs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherableBluetoothRegexs();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int setUsbTethering(boolean enable, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUsbTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setUsbTethering(enable, callerPkg);
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
      @Override public void reportInetCondition(int networkType, int percentage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          _data.writeInt(percentage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportInetCondition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportInetCondition(networkType, percentage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportNetworkConnectivity(android.net.Network network, boolean hasConnectivity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((hasConnectivity)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportNetworkConnectivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportNetworkConnectivity(network, hasConnectivity);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.ProxyInfo getGlobalProxy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.ProxyInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGlobalProxy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGlobalProxy();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.ProxyInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void setGlobalProxy(android.net.ProxyInfo p) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((p!=null)) {
            _data.writeInt(1);
            p.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGlobalProxy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGlobalProxy(p);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.ProxyInfo getProxyForNetwork(android.net.Network nework) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.ProxyInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((nework!=null)) {
            _data.writeInt(1);
            nework.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProxyForNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProxyForNetwork(nework);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.ProxyInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean prepareVpn(java.lang.String oldPackage, java.lang.String newPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(oldPackage);
          _data.writeString(newPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareVpn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().prepareVpn(oldPackage, newPackage, userId);
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
      @Override public void setVpnPackageAuthorization(java.lang.String packageName, int userId, boolean authorized) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          _data.writeInt(((authorized)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVpnPackageAuthorization, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVpnPackageAuthorization(packageName, userId, authorized);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.ParcelFileDescriptor establishVpn(com.android.internal.net.VpnConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_establishVpn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().establishVpn(config);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
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
      @Override public com.android.internal.net.VpnConfig getVpnConfig(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.net.VpnConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVpnConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVpnConfig(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.net.VpnConfig.CREATOR.createFromParcel(_reply);
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
      @Override public void startLegacyVpn(com.android.internal.net.VpnProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startLegacyVpn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startLegacyVpn(profile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public com.android.internal.net.LegacyVpnInfo getLegacyVpnInfo(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.net.LegacyVpnInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLegacyVpnInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLegacyVpnInfo(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.net.LegacyVpnInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean updateLockdownVpn() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateLockdownVpn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateLockdownVpn();
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
      @Override public boolean isAlwaysOnVpnPackageSupported(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAlwaysOnVpnPackageSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAlwaysOnVpnPackageSupported(userId, packageName);
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
      @Override public boolean setAlwaysOnVpnPackage(int userId, java.lang.String packageName, boolean lockdown, java.util.List<java.lang.String> lockdownWhitelist) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          _data.writeInt(((lockdown)?(1):(0)));
          _data.writeStringList(lockdownWhitelist);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAlwaysOnVpnPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAlwaysOnVpnPackage(userId, packageName, lockdown, lockdownWhitelist);
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
      @Override public java.lang.String getAlwaysOnVpnPackage(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAlwaysOnVpnPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAlwaysOnVpnPackage(userId);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean isVpnLockdownEnabled(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVpnLockdownEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVpnLockdownEnabled(userId);
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
      @Override public java.util.List<java.lang.String> getVpnLockdownWhitelist(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVpnLockdownWhitelist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVpnLockdownWhitelist(userId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int checkMobileProvisioning(int suggestedTimeOutMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(suggestedTimeOutMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkMobileProvisioning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkMobileProvisioning(suggestedTimeOutMs);
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
      @Override public java.lang.String getMobileProvisioningUrl() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMobileProvisioningUrl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMobileProvisioningUrl();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setProvisioningNotificationVisible(boolean visible, int networkType, java.lang.String action) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          _data.writeInt(networkType);
          _data.writeString(action);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProvisioningNotificationVisible, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProvisioningNotificationVisible(visible, networkType, action);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAirplaneMode(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAirplaneMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAirplaneMode(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int registerNetworkFactory(android.os.Messenger messenger, java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerNetworkFactory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerNetworkFactory(messenger, name);
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
      @Override public boolean requestBandwidthUpdate(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBandwidthUpdate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestBandwidthUpdate(network);
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
      @Override public void unregisterNetworkFactory(android.os.Messenger messenger) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterNetworkFactory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterNetworkFactory(messenger);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int registerNetworkAgent(android.os.Messenger messenger, android.net.NetworkInfo ni, android.net.LinkProperties lp, android.net.NetworkCapabilities nc, int score, android.net.NetworkMisc misc, int factorySerialNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((ni!=null)) {
            _data.writeInt(1);
            ni.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
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
          _data.writeInt(score);
          if ((misc!=null)) {
            _data.writeInt(1);
            misc.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(factorySerialNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerNetworkAgent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerNetworkAgent(messenger, ni, lp, nc, score, misc, factorySerialNumber);
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
      @Override public android.net.NetworkRequest requestNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, int timeoutSec, android.os.IBinder binder, int legacy) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(timeoutSec);
          _data.writeStrongBinder(binder);
          _data.writeInt(legacy);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestNetwork(networkCapabilities, messenger, timeoutSec, binder, legacy);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkRequest.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkRequest pendingRequestForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((operation!=null)) {
            _data.writeInt(1);
            operation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_pendingRequestForNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().pendingRequestForNetwork(networkCapabilities, operation);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkRequest.CREATOR.createFromParcel(_reply);
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
      @Override public void releasePendingNetworkRequest(android.app.PendingIntent operation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((operation!=null)) {
            _data.writeInt(1);
            operation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_releasePendingNetworkRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releasePendingNetworkRequest(operation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.NetworkRequest listenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listenForNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listenForNetwork(networkCapabilities, messenger, binder);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkRequest.CREATOR.createFromParcel(_reply);
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
      @Override public void pendingListenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((operation!=null)) {
            _data.writeInt(1);
            operation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_pendingListenForNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pendingListenForNetwork(networkCapabilities, operation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releaseNetworkRequest(android.net.NetworkRequest networkRequest) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((networkRequest!=null)) {
            _data.writeInt(1);
            networkRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseNetworkRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseNetworkRequest(networkRequest);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAcceptUnvalidated(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((accept)?(1):(0)));
          _data.writeInt(((always)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAcceptUnvalidated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAcceptUnvalidated(network, accept, always);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAcceptPartialConnectivity(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((accept)?(1):(0)));
          _data.writeInt(((always)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAcceptPartialConnectivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAcceptPartialConnectivity(network, accept, always);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAvoidUnvalidated(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAvoidUnvalidated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAvoidUnvalidated(network);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startCaptivePortalApp(android.net.Network network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startCaptivePortalApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startCaptivePortalApp(network);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startCaptivePortalAppInternal(android.net.Network network, android.os.Bundle appExtras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((appExtras!=null)) {
            _data.writeInt(1);
            appExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startCaptivePortalAppInternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startCaptivePortalAppInternal(network, appExtras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean shouldAvoidBadWifi() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldAvoidBadWifi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldAvoidBadWifi();
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
      @Override public int getMultipathPreference(android.net.Network Network) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((Network!=null)) {
            _data.writeInt(1);
            Network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMultipathPreference, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMultipathPreference(Network);
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
      @Override public android.net.NetworkRequest getDefaultRequest() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultRequest();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkRequest.CREATOR.createFromParcel(_reply);
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
      @Override public int getRestoreDefaultNetworkDelay(int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRestoreDefaultNetworkDelay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRestoreDefaultNetworkDelay(networkType);
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
      @Override public boolean addVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          _data.writeInt(prefixLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVpnAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addVpnAddress(address, prefixLength);
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
      @Override public boolean removeVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(address);
          _data.writeInt(prefixLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVpnAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeVpnAddress(address, prefixLength);
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
      @Override public boolean setUnderlyingNetworksForVpn(android.net.Network[] networks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(networks, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUnderlyingNetworksForVpn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setUnderlyingNetworksForVpn(networks);
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
      @Override public void factoryReset() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_factoryReset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().factoryReset();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startNattKeepalive(android.net.Network network, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, int srcPort, java.lang.String dstAddr) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(intervalSeconds);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeString(srcAddr);
          _data.writeInt(srcPort);
          _data.writeString(dstAddr);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startNattKeepalive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startNattKeepalive(network, intervalSeconds, cb, srcAddr, srcPort, dstAddr);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startNattKeepaliveWithFd(android.net.Network network, java.io.FileDescriptor fd, int resourceId, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, java.lang.String dstAddr) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeRawFileDescriptor(fd);
          _data.writeInt(resourceId);
          _data.writeInt(intervalSeconds);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeString(srcAddr);
          _data.writeString(dstAddr);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startNattKeepaliveWithFd, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startNattKeepaliveWithFd(network, fd, resourceId, intervalSeconds, cb, srcAddr, dstAddr);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startTcpKeepalive(android.net.Network network, java.io.FileDescriptor fd, int intervalSeconds, android.net.ISocketKeepaliveCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeRawFileDescriptor(fd);
          _data.writeInt(intervalSeconds);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startTcpKeepalive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startTcpKeepalive(network, fd, intervalSeconds, cb);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopKeepalive(android.net.Network network, int slot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(slot);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopKeepalive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopKeepalive(network, slot);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getCaptivePortalServerUrl() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCaptivePortalServerUrl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCaptivePortalServerUrl();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public byte[] getNetworkWatchlistConfigHash() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkWatchlistConfigHash, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkWatchlistConfigHash();
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getConnectionOwnerUid(android.net.ConnectionInfo connectionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((connectionInfo!=null)) {
            _data.writeInt(1);
            connectionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConnectionOwnerUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConnectionOwnerUid(connectionInfo);
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
      @Override public boolean isCallerCurrentAlwaysOnVpnApp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCallerCurrentAlwaysOnVpnApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isCallerCurrentAlwaysOnVpnApp();
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
      @Override public boolean isCallerCurrentAlwaysOnVpnLockdownApp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCallerCurrentAlwaysOnVpnLockdownApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isCallerCurrentAlwaysOnVpnLockdownApp();
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
      @Override public void getLatestTetheringEntitlementResult(int type, android.os.ResultReceiver receiver, boolean showEntitlementUi, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          if ((receiver!=null)) {
            _data.writeInt(1);
            receiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((showEntitlementUi)?(1):(0)));
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLatestTetheringEntitlementResult, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getLatestTetheringEntitlementResult(type, receiver, showEntitlementUi, callerPkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTetheringEventCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTetheringEventCallback(callback, callerPkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callerPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTetheringEventCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTetheringEventCallback(callback, callerPkg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.IBinder startOrGetTestNetworkService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startOrGetTestNetworkService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startOrGetTestNetworkService();
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.net.IConnectivityManager sDefaultImpl;
    }
    static final int TRANSACTION_getActiveNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getActiveNetworkForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getActiveNetworkInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getActiveNetworkInfoForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getNetworkInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getNetworkInfoForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getAllNetworkInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getNetworkForType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getAllNetworks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getDefaultNetworkCapabilitiesForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isNetworkSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getActiveLinkProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getLinkPropertiesForType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getLinkProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getNetworkCapabilities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getAllNetworkState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getActiveNetworkQuotaInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_isActiveNetworkMetered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_requestRouteToHostAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_tether = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_untether = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getLastTetherError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_isTetheringSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_startTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_stopTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getTetherableIfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getTetheredIfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getTetheringErroredIfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getTetheredDhcpRanges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getTetherableUsbRegexs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getTetherableWifiRegexs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getTetherableBluetoothRegexs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_setUsbTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_reportInetCondition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_reportNetworkConnectivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getGlobalProxy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_setGlobalProxy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getProxyForNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_prepareVpn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_setVpnPackageAuthorization = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_establishVpn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getVpnConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_startLegacyVpn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_getLegacyVpnInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_updateLockdownVpn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_isAlwaysOnVpnPackageSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_setAlwaysOnVpnPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_getAlwaysOnVpnPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_isVpnLockdownEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_getVpnLockdownWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_checkMobileProvisioning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_getMobileProvisioningUrl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_setProvisioningNotificationVisible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_setAirplaneMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_registerNetworkFactory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_requestBandwidthUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_unregisterNetworkFactory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_registerNetworkAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_requestNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_pendingRequestForNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_releasePendingNetworkRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_listenForNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_pendingListenForNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_releaseNetworkRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_setAcceptUnvalidated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_setAcceptPartialConnectivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_setAvoidUnvalidated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_startCaptivePortalApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_startCaptivePortalAppInternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_shouldAvoidBadWifi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_getMultipathPreference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_getDefaultRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_getRestoreDefaultNetworkDelay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_addVpnAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_removeVpnAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_setUnderlyingNetworksForVpn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_factoryReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_startNattKeepalive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_startNattKeepaliveWithFd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_startTcpKeepalive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_stopKeepalive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_getCaptivePortalServerUrl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_getNetworkWatchlistConfigHash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_getConnectionOwnerUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_isCallerCurrentAlwaysOnVpnApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_isCallerCurrentAlwaysOnVpnLockdownApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_getLatestTetheringEntitlementResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_registerTetheringEventCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_unregisterTetheringEventCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_startOrGetTestNetworkService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    public static boolean setDefaultImpl(android.net.IConnectivityManager impl) {
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
    public static android.net.IConnectivityManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.net.Network getActiveNetwork() throws android.os.RemoteException;
  public android.net.Network getActiveNetworkForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:52:1:52:25")
  public android.net.NetworkInfo getActiveNetworkInfo() throws android.os.RemoteException;
  public android.net.NetworkInfo getActiveNetworkInfoForUid(int uid, boolean ignoreBlocked) throws android.os.RemoteException;
  public android.net.NetworkInfo getNetworkInfo(int networkType) throws android.os.RemoteException;
  public android.net.NetworkInfo getNetworkInfoForUid(android.net.Network network, int uid, boolean ignoreBlocked) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:57:1:57:25")
  public android.net.NetworkInfo[] getAllNetworkInfo() throws android.os.RemoteException;
  public android.net.Network getNetworkForType(int networkType) throws android.os.RemoteException;
  public android.net.Network[] getAllNetworks() throws android.os.RemoteException;
  public android.net.NetworkCapabilities[] getDefaultNetworkCapabilitiesForUser(int userId) throws android.os.RemoteException;
  public boolean isNetworkSupported(int networkType) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:65:1:65:25")
  public android.net.LinkProperties getActiveLinkProperties() throws android.os.RemoteException;
  public android.net.LinkProperties getLinkPropertiesForType(int networkType) throws android.os.RemoteException;
  public android.net.LinkProperties getLinkProperties(android.net.Network network) throws android.os.RemoteException;
  public android.net.NetworkCapabilities getNetworkCapabilities(android.net.Network network) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:72:1:72:25")
  public android.net.NetworkState[] getAllNetworkState() throws android.os.RemoteException;
  public android.net.NetworkQuotaInfo getActiveNetworkQuotaInfo() throws android.os.RemoteException;
  public boolean isActiveNetworkMetered() throws android.os.RemoteException;
  public boolean requestRouteToHostAddress(int networkType, byte[] hostAddress) throws android.os.RemoteException;
  public int tether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException;
  public int untether(java.lang.String iface, java.lang.String callerPkg) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:84:1:84:25")
  public int getLastTetherError(java.lang.String iface) throws android.os.RemoteException;
  public boolean isTetheringSupported(java.lang.String callerPkg) throws android.os.RemoteException;
  public void startTethering(int type, android.os.ResultReceiver receiver, boolean showProvisioningUi, java.lang.String callerPkg) throws android.os.RemoteException;
  public void stopTethering(int type, java.lang.String callerPkg) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:94:1:94:25")
  public java.lang.String[] getTetherableIfaces() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:97:1:97:25")
  public java.lang.String[] getTetheredIfaces() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:100:1:100:25")
  public java.lang.String[] getTetheringErroredIfaces() throws android.os.RemoteException;
  public java.lang.String[] getTetheredDhcpRanges() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:105:1:105:25")
  public java.lang.String[] getTetherableUsbRegexs() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:108:1:108:25")
  public java.lang.String[] getTetherableWifiRegexs() throws android.os.RemoteException;
  public java.lang.String[] getTetherableBluetoothRegexs() throws android.os.RemoteException;
  public int setUsbTethering(boolean enable, java.lang.String callerPkg) throws android.os.RemoteException;
  public void reportInetCondition(int networkType, int percentage) throws android.os.RemoteException;
  public void reportNetworkConnectivity(android.net.Network network, boolean hasConnectivity) throws android.os.RemoteException;
  public android.net.ProxyInfo getGlobalProxy() throws android.os.RemoteException;
  public void setGlobalProxy(android.net.ProxyInfo p) throws android.os.RemoteException;
  public android.net.ProxyInfo getProxyForNetwork(android.net.Network nework) throws android.os.RemoteException;
  public boolean prepareVpn(java.lang.String oldPackage, java.lang.String newPackage, int userId) throws android.os.RemoteException;
  public void setVpnPackageAuthorization(java.lang.String packageName, int userId, boolean authorized) throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor establishVpn(com.android.internal.net.VpnConfig config) throws android.os.RemoteException;
  public com.android.internal.net.VpnConfig getVpnConfig(int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/IConnectivityManager.aidl:133:1:133:25")
  public void startLegacyVpn(com.android.internal.net.VpnProfile profile) throws android.os.RemoteException;
  public com.android.internal.net.LegacyVpnInfo getLegacyVpnInfo(int userId) throws android.os.RemoteException;
  public boolean updateLockdownVpn() throws android.os.RemoteException;
  public boolean isAlwaysOnVpnPackageSupported(int userId, java.lang.String packageName) throws android.os.RemoteException;
  public boolean setAlwaysOnVpnPackage(int userId, java.lang.String packageName, boolean lockdown, java.util.List<java.lang.String> lockdownWhitelist) throws android.os.RemoteException;
  public java.lang.String getAlwaysOnVpnPackage(int userId) throws android.os.RemoteException;
  public boolean isVpnLockdownEnabled(int userId) throws android.os.RemoteException;
  public java.util.List<java.lang.String> getVpnLockdownWhitelist(int userId) throws android.os.RemoteException;
  public int checkMobileProvisioning(int suggestedTimeOutMs) throws android.os.RemoteException;
  public java.lang.String getMobileProvisioningUrl() throws android.os.RemoteException;
  public void setProvisioningNotificationVisible(boolean visible, int networkType, java.lang.String action) throws android.os.RemoteException;
  public void setAirplaneMode(boolean enable) throws android.os.RemoteException;
  public int registerNetworkFactory(android.os.Messenger messenger, java.lang.String name) throws android.os.RemoteException;
  public boolean requestBandwidthUpdate(android.net.Network network) throws android.os.RemoteException;
  public void unregisterNetworkFactory(android.os.Messenger messenger) throws android.os.RemoteException;
  public int registerNetworkAgent(android.os.Messenger messenger, android.net.NetworkInfo ni, android.net.LinkProperties lp, android.net.NetworkCapabilities nc, int score, android.net.NetworkMisc misc, int factorySerialNumber) throws android.os.RemoteException;
  public android.net.NetworkRequest requestNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, int timeoutSec, android.os.IBinder binder, int legacy) throws android.os.RemoteException;
  public android.net.NetworkRequest pendingRequestForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException;
  public void releasePendingNetworkRequest(android.app.PendingIntent operation) throws android.os.RemoteException;
  public android.net.NetworkRequest listenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException;
  public void pendingListenForNetwork(android.net.NetworkCapabilities networkCapabilities, android.app.PendingIntent operation) throws android.os.RemoteException;
  public void releaseNetworkRequest(android.net.NetworkRequest networkRequest) throws android.os.RemoteException;
  public void setAcceptUnvalidated(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException;
  public void setAcceptPartialConnectivity(android.net.Network network, boolean accept, boolean always) throws android.os.RemoteException;
  public void setAvoidUnvalidated(android.net.Network network) throws android.os.RemoteException;
  public void startCaptivePortalApp(android.net.Network network) throws android.os.RemoteException;
  public void startCaptivePortalAppInternal(android.net.Network network, android.os.Bundle appExtras) throws android.os.RemoteException;
  public boolean shouldAvoidBadWifi() throws android.os.RemoteException;
  public int getMultipathPreference(android.net.Network Network) throws android.os.RemoteException;
  public android.net.NetworkRequest getDefaultRequest() throws android.os.RemoteException;
  public int getRestoreDefaultNetworkDelay(int networkType) throws android.os.RemoteException;
  public boolean addVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException;
  public boolean removeVpnAddress(java.lang.String address, int prefixLength) throws android.os.RemoteException;
  public boolean setUnderlyingNetworksForVpn(android.net.Network[] networks) throws android.os.RemoteException;
  public void factoryReset() throws android.os.RemoteException;
  public void startNattKeepalive(android.net.Network network, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, int srcPort, java.lang.String dstAddr) throws android.os.RemoteException;
  public void startNattKeepaliveWithFd(android.net.Network network, java.io.FileDescriptor fd, int resourceId, int intervalSeconds, android.net.ISocketKeepaliveCallback cb, java.lang.String srcAddr, java.lang.String dstAddr) throws android.os.RemoteException;
  public void startTcpKeepalive(android.net.Network network, java.io.FileDescriptor fd, int intervalSeconds, android.net.ISocketKeepaliveCallback cb) throws android.os.RemoteException;
  public void stopKeepalive(android.net.Network network, int slot) throws android.os.RemoteException;
  public java.lang.String getCaptivePortalServerUrl() throws android.os.RemoteException;
  public byte[] getNetworkWatchlistConfigHash() throws android.os.RemoteException;
  public int getConnectionOwnerUid(android.net.ConnectionInfo connectionInfo) throws android.os.RemoteException;
  public boolean isCallerCurrentAlwaysOnVpnApp() throws android.os.RemoteException;
  public boolean isCallerCurrentAlwaysOnVpnLockdownApp() throws android.os.RemoteException;
  public void getLatestTetheringEntitlementResult(int type, android.os.ResultReceiver receiver, boolean showEntitlementUi, java.lang.String callerPkg) throws android.os.RemoteException;
  public void registerTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException;
  public void unregisterTetheringEventCallback(android.net.ITetheringEventCallback callback, java.lang.String callerPkg) throws android.os.RemoteException;
  public android.os.IBinder startOrGetTestNetworkService() throws android.os.RemoteException;
}
