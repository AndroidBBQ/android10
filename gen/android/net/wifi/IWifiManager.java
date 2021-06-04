/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface that allows controlling and querying Wi-Fi connectivity.
 *
 * {@hide}
 */
public interface IWifiManager extends android.os.IInterface
{
  /** Default implementation for IWifiManager. */
  public static class Default implements android.net.wifi.IWifiManager
  {
    @Override public long getSupportedFeatures() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public android.net.wifi.WifiActivityEnergyInfo reportActivityInfo() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests the controller activity info asynchronously.
         * The implementor is expected to reply with the
         * {@link android.net.wifi.WifiActivityEnergyInfo} object placed into the Bundle with the key
         * {@link android.os.BatteryStats#RESULT_RECEIVER_CONTROLLER_KEY}. The result code is ignored.
         */
    @Override public void requestActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getPrivilegedConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.Map getAllMatchingFqdnsForScanResults(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.Map getMatchingOsuProviders(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.Map getMatchingPasspointConfigsForOsuProviders(java.util.List<android.net.wifi.hotspot2.OsuProvider> osuProviders) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int addOrUpdateNetwork(android.net.wifi.WifiConfiguration config, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean addOrUpdatePasspointConfiguration(android.net.wifi.hotspot2.PasspointConfiguration config, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removePasspointConfiguration(java.lang.String fqdn, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.net.wifi.hotspot2.PasspointConfiguration> getPasspointConfigurations(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.net.wifi.WifiConfiguration> getWifiConfigsForPasspointProfiles(java.util.List<java.lang.String> fqdnList) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void queryPasspointIcon(long bssid, java.lang.String fileName) throws android.os.RemoteException
    {
    }
    @Override public int matchProviderWithCurrentNetwork(java.lang.String fqdn) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void deauthenticateNetwork(long holdoff, boolean ess) throws android.os.RemoteException
    {
    }
    @Override public boolean removeNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean enableNetwork(int netId, boolean disableOthers, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean disableNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean startScan(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.net.wifi.ScanResult> getScanResults(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean disconnect(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean reconnect(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean reassociate(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.net.wifi.WifiInfo getConnectionInfo(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setWifiEnabled(java.lang.String packageName, boolean enable) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getWifiEnabledState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setCountryCode(java.lang.String country) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getCountryCode() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isDualBandSupported() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean needs5GHzToAnyApBandConversion() throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.net.DhcpInfo getDhcpInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isScanAlwaysAvailable() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean acquireWifiLock(android.os.IBinder lock, int lockType, java.lang.String tag, android.os.WorkSource ws) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void updateWifiLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws) throws android.os.RemoteException
    {
    }
    @Override public boolean releaseWifiLock(android.os.IBinder lock) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void initializeMulticastFiltering() throws android.os.RemoteException
    {
    }
    @Override public boolean isMulticastEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void acquireMulticastLock(android.os.IBinder binder, java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public void releaseMulticastLock(java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public void updateInterfaceIpState(java.lang.String ifaceName, int mode) throws android.os.RemoteException
    {
    }
    @Override public boolean startSoftAp(android.net.wifi.WifiConfiguration wifiConfig) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean stopSoftAp() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int startLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void stopLocalOnlyHotspot() throws android.os.RemoteException
    {
    }
    @Override public void startWatchLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override public void stopWatchLocalOnlyHotspot() throws android.os.RemoteException
    {
    }
    @Override public int getWifiApEnabledState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.net.wifi.WifiConfiguration getWifiApConfiguration() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setWifiApConfiguration(android.net.wifi.WifiConfiguration wifiConfig, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyUserOfApBandConversion(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public android.os.Messenger getWifiServiceMessenger(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void enableTdls(java.lang.String remoteIPAddress, boolean enable) throws android.os.RemoteException
    {
    }
    @Override public void enableTdlsWithMacAddress(java.lang.String remoteMacAddress, boolean enable) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getCurrentNetworkWpsNfcConfigurationToken() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void enableVerboseLogging(int verbose) throws android.os.RemoteException
    {
    }
    @Override public int getVerboseLoggingLevel() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void enableWifiConnectivityManager(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void disableEphemeralNetwork(java.lang.String SSID, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void factoryReset(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public android.net.Network getCurrentNetwork() throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] retrieveBackupData() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void restoreBackupData(byte[] data) throws android.os.RemoteException
    {
    }
    @Override public void restoreSupplicantBackupData(byte[] supplicantData, byte[] ipConfigData) throws android.os.RemoteException
    {
    }
    @Override public void startSubscriptionProvisioning(android.net.wifi.hotspot2.OsuProvider provider, android.net.wifi.hotspot2.IProvisioningCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void registerSoftApCallback(android.os.IBinder binder, android.net.wifi.ISoftApCallback callback, int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void unregisterSoftApCallback(int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void addOnWifiUsabilityStatsListener(android.os.IBinder binder, android.net.wifi.IOnWifiUsabilityStatsListener listener, int listenerIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void removeOnWifiUsabilityStatsListener(int listenerIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void registerTrafficStateCallback(android.os.IBinder binder, android.net.wifi.ITrafficStateCallback callback, int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void unregisterTrafficStateCallback(int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void registerNetworkRequestMatchCallback(android.os.IBinder binder, android.net.wifi.INetworkRequestMatchCallback callback, int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public void unregisterNetworkRequestMatchCallback(int callbackIdentifier) throws android.os.RemoteException
    {
    }
    @Override public int addNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int removeNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String[] getFactoryMacAddresses() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setDeviceMobilityState(int state) throws android.os.RemoteException
    {
    }
    @Override public void startDppAsConfiguratorInitiator(android.os.IBinder binder, java.lang.String enrolleeUri, int selectedNetworkId, int netRole, android.net.wifi.IDppCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void startDppAsEnrolleeInitiator(android.os.IBinder binder, java.lang.String configuratorUri, android.net.wifi.IDppCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void stopDppSession() throws android.os.RemoteException
    {
    }
    @Override public void updateWifiUsabilityScore(int seqNum, int score, int predictionHorizonSec) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.IWifiManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.IWifiManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.IWifiManager interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.IWifiManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.IWifiManager))) {
        return ((android.net.wifi.IWifiManager)iin);
      }
      return new android.net.wifi.IWifiManager.Stub.Proxy(obj);
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
        case TRANSACTION_getSupportedFeatures:
        {
          data.enforceInterface(descriptor);
          long _result = this.getSupportedFeatures();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_reportActivityInfo:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiActivityEnergyInfo _result = this.reportActivityInfo();
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
        case TRANSACTION_requestActivityInfo:
        {
          data.enforceInterface(descriptor);
          android.os.ResultReceiver _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.requestActivityInfo(_arg0);
          return true;
        }
        case TRANSACTION_getConfiguredNetworks:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _result = this.getConfiguredNetworks(_arg0);
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
        case TRANSACTION_getPrivilegedConfiguredNetworks:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _result = this.getPrivilegedConfiguredNetworks(_arg0);
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
        case TRANSACTION_getAllMatchingFqdnsForScanResults:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.ScanResult> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.ScanResult.CREATOR);
          java.util.Map _result = this.getAllMatchingFqdnsForScanResults(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_getMatchingOsuProviders:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.ScanResult> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.ScanResult.CREATOR);
          java.util.Map _result = this.getMatchingOsuProviders(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_getMatchingPasspointConfigsForOsuProviders:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.hotspot2.OsuProvider> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.hotspot2.OsuProvider.CREATOR);
          java.util.Map _result = this.getMatchingPasspointConfigsForOsuProviders(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_addOrUpdateNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.addOrUpdateNetwork(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addOrUpdatePasspointConfiguration:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.hotspot2.PasspointConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.hotspot2.PasspointConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.addOrUpdatePasspointConfiguration(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removePasspointConfiguration:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.removePasspointConfiguration(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPasspointConfigurations:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.net.wifi.hotspot2.PasspointConfiguration> _result = this.getPasspointConfigurations(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getWifiConfigsForPasspointProfiles:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          java.util.List<android.net.wifi.WifiConfiguration> _result = this.getWifiConfigsForPasspointProfiles(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_queryPasspointIcon:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.queryPasspointIcon(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_matchProviderWithCurrentNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.matchProviderWithCurrentNetwork(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_deauthenticateNetwork:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.deauthenticateNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeNetwork:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.removeNetwork(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableNetwork:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.enableNetwork(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableNetwork:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.disableNetwork(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startScan:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.startScan(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getScanResults:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.net.wifi.ScanResult> _result = this.getScanResults(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.disconnect(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_reconnect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.reconnect(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_reassociate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.reassociate(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getConnectionInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.wifi.WifiInfo _result = this.getConnectionInfo(_arg0);
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
        case TRANSACTION_setWifiEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.setWifiEnabled(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getWifiEnabledState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getWifiEnabledState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setCountryCode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setCountryCode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCountryCode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getCountryCode();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_isDualBandSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDualBandSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_needs5GHzToAnyApBandConversion:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.needs5GHzToAnyApBandConversion();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getDhcpInfo:
        {
          data.enforceInterface(descriptor);
          android.net.DhcpInfo _result = this.getDhcpInfo();
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
        case TRANSACTION_isScanAlwaysAvailable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isScanAlwaysAvailable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_acquireWifiLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.WorkSource _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _result = this.acquireWifiLock(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateWifiLockWorkSource:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updateWifiLockWorkSource(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseWifiLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.releaseWifiLock(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_initializeMulticastFiltering:
        {
          data.enforceInterface(descriptor);
          this.initializeMulticastFiltering();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isMulticastEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isMulticastEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_acquireMulticastLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.acquireMulticastLock(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseMulticastLock:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.releaseMulticastLock(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateInterfaceIpState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.updateInterfaceIpState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startSoftAp:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.startSoftAp(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopSoftAp:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.stopSoftAp();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startLocalOnlyHotspot:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.startLocalOnlyHotspot(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopLocalOnlyHotspot:
        {
          data.enforceInterface(descriptor);
          this.stopLocalOnlyHotspot();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWatchLocalOnlyHotspot:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.startWatchLocalOnlyHotspot(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWatchLocalOnlyHotspot:
        {
          data.enforceInterface(descriptor);
          this.stopWatchLocalOnlyHotspot();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWifiApEnabledState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getWifiApEnabledState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getWifiApConfiguration:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _result = this.getWifiApConfiguration();
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
        case TRANSACTION_setWifiApConfiguration:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.setWifiApConfiguration(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyUserOfApBandConversion:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.notifyUserOfApBandConversion(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWifiServiceMessenger:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Messenger _result = this.getWifiServiceMessenger(_arg0);
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
        case TRANSACTION_enableTdls:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.enableTdls(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableTdlsWithMacAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.enableTdlsWithMacAddress(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentNetworkWpsNfcConfigurationToken:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getCurrentNetworkWpsNfcConfigurationToken();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_enableVerboseLogging:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableVerboseLogging(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVerboseLoggingLevel:
        {
          data.enforceInterface(descriptor);
          int _result = this.getVerboseLoggingLevel();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_enableWifiConnectivityManager:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.enableWifiConnectivityManager(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableEphemeralNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.disableEphemeralNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_factoryReset:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.factoryReset(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentNetwork:
        {
          data.enforceInterface(descriptor);
          android.net.Network _result = this.getCurrentNetwork();
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
        case TRANSACTION_retrieveBackupData:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.retrieveBackupData();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_restoreBackupData:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.restoreBackupData(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restoreSupplicantBackupData:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.restoreSupplicantBackupData(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startSubscriptionProvisioning:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.hotspot2.OsuProvider _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.hotspot2.OsuProvider.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.wifi.hotspot2.IProvisioningCallback _arg1;
          _arg1 = android.net.wifi.hotspot2.IProvisioningCallback.Stub.asInterface(data.readStrongBinder());
          this.startSubscriptionProvisioning(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerSoftApCallback:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.wifi.ISoftApCallback _arg1;
          _arg1 = android.net.wifi.ISoftApCallback.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.registerSoftApCallback(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterSoftApCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.unregisterSoftApCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addOnWifiUsabilityStatsListener:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.wifi.IOnWifiUsabilityStatsListener _arg1;
          _arg1 = android.net.wifi.IOnWifiUsabilityStatsListener.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.addOnWifiUsabilityStatsListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOnWifiUsabilityStatsListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeOnWifiUsabilityStatsListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTrafficStateCallback:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.wifi.ITrafficStateCallback _arg1;
          _arg1 = android.net.wifi.ITrafficStateCallback.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.registerTrafficStateCallback(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTrafficStateCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.unregisterTrafficStateCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerNetworkRequestMatchCallback:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.net.wifi.INetworkRequestMatchCallback _arg1;
          _arg1 = android.net.wifi.INetworkRequestMatchCallback.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.registerNetworkRequestMatchCallback(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterNetworkRequestMatchCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.unregisterNetworkRequestMatchCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addNetworkSuggestions:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.WifiNetworkSuggestion> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.WifiNetworkSuggestion.CREATOR);
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.addNetworkSuggestions(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeNetworkSuggestions:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.WifiNetworkSuggestion> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.WifiNetworkSuggestion.CREATOR);
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.removeNetworkSuggestions(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getFactoryMacAddresses:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getFactoryMacAddresses();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_setDeviceMobilityState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDeviceMobilityState(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startDppAsConfiguratorInitiator:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.net.wifi.IDppCallback _arg4;
          _arg4 = android.net.wifi.IDppCallback.Stub.asInterface(data.readStrongBinder());
          this.startDppAsConfiguratorInitiator(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startDppAsEnrolleeInitiator:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.wifi.IDppCallback _arg2;
          _arg2 = android.net.wifi.IDppCallback.Stub.asInterface(data.readStrongBinder());
          this.startDppAsEnrolleeInitiator(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopDppSession:
        {
          data.enforceInterface(descriptor);
          this.stopDppSession();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateWifiUsabilityScore:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.updateWifiUsabilityScore(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.IWifiManager
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
      @Override public long getSupportedFeatures() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedFeatures();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.wifi.WifiActivityEnergyInfo reportActivityInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.wifi.WifiActivityEnergyInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportActivityInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().reportActivityInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.wifi.WifiActivityEnergyInfo.CREATOR.createFromParcel(_reply);
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
      /**
           * Requests the controller activity info asynchronously.
           * The implementor is expected to reply with the
           * {@link android.net.wifi.WifiActivityEnergyInfo} object placed into the Bundle with the key
           * {@link android.os.BatteryStats#RESULT_RECEIVER_CONTROLLER_KEY}. The result code is ignored.
           */
      @Override public void requestActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestActivityInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestActivityInfo(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfiguredNetworks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfiguredNetworks(packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.pm.ParceledListSlice getPrivilegedConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrivilegedConfiguredNetworks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPrivilegedConfiguredNetworks(packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      @Override public java.util.Map getAllMatchingFqdnsForScanResults(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(scanResult);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllMatchingFqdnsForScanResults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllMatchingFqdnsForScanResults(scanResult);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.Map getMatchingOsuProviders(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(scanResult);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMatchingOsuProviders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMatchingOsuProviders(scanResult);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.Map getMatchingPasspointConfigsForOsuProviders(java.util.List<android.net.wifi.hotspot2.OsuProvider> osuProviders) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(osuProviders);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMatchingPasspointConfigsForOsuProviders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMatchingPasspointConfigsForOsuProviders(osuProviders);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int addOrUpdateNetwork(android.net.wifi.WifiConfiguration config, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOrUpdateNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addOrUpdateNetwork(config, packageName);
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
      @Override public boolean addOrUpdatePasspointConfiguration(android.net.wifi.hotspot2.PasspointConfiguration config, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOrUpdatePasspointConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addOrUpdatePasspointConfiguration(config, packageName);
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
      @Override public boolean removePasspointConfiguration(java.lang.String fqdn, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(fqdn);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePasspointConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removePasspointConfiguration(fqdn, packageName);
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
      @Override public java.util.List<android.net.wifi.hotspot2.PasspointConfiguration> getPasspointConfigurations(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.net.wifi.hotspot2.PasspointConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPasspointConfigurations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPasspointConfigurations(packageName);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.net.wifi.hotspot2.PasspointConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.net.wifi.WifiConfiguration> getWifiConfigsForPasspointProfiles(java.util.List<java.lang.String> fqdnList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.net.wifi.WifiConfiguration> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(fqdnList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiConfigsForPasspointProfiles, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiConfigsForPasspointProfiles(fqdnList);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.net.wifi.WifiConfiguration.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void queryPasspointIcon(long bssid, java.lang.String fileName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(bssid);
          _data.writeString(fileName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryPasspointIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().queryPasspointIcon(bssid, fileName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int matchProviderWithCurrentNetwork(java.lang.String fqdn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(fqdn);
          boolean _status = mRemote.transact(Stub.TRANSACTION_matchProviderWithCurrentNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().matchProviderWithCurrentNetwork(fqdn);
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
      @Override public void deauthenticateNetwork(long holdoff, boolean ess) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(holdoff);
          _data.writeInt(((ess)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deauthenticateNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deauthenticateNetwork(holdoff, ess);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean removeNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeNetwork(netId, packageName);
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
      @Override public boolean enableNetwork(int netId, boolean disableOthers, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeInt(((disableOthers)?(1):(0)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableNetwork(netId, disableOthers, packageName);
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
      @Override public boolean disableNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disableNetwork(netId, packageName);
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
      @Override public boolean startScan(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startScan(packageName);
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
      @Override public java.util.List<android.net.wifi.ScanResult> getScanResults(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.net.wifi.ScanResult> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getScanResults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getScanResults(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.net.wifi.ScanResult.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean disconnect(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disconnect(packageName);
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
      @Override public boolean reconnect(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reconnect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().reconnect(packageName);
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
      @Override public boolean reassociate(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reassociate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().reassociate(packageName);
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
      @Override public android.net.wifi.WifiInfo getConnectionInfo(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.wifi.WifiInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConnectionInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConnectionInfo(callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.wifi.WifiInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setWifiEnabled(java.lang.String packageName, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWifiEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setWifiEnabled(packageName, enable);
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
      @Override public int getWifiEnabledState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiEnabledState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiEnabledState();
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
      @Override public void setCountryCode(java.lang.String country) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(country);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCountryCode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCountryCode(country);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getCountryCode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCountryCode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCountryCode();
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
      @Override public boolean isDualBandSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDualBandSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDualBandSupported();
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
      @Override public boolean needs5GHzToAnyApBandConversion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_needs5GHzToAnyApBandConversion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().needs5GHzToAnyApBandConversion();
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
      @Override public android.net.DhcpInfo getDhcpInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.DhcpInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDhcpInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDhcpInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.DhcpInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean isScanAlwaysAvailable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isScanAlwaysAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isScanAlwaysAvailable();
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
      @Override public boolean acquireWifiLock(android.os.IBinder lock, int lockType, java.lang.String tag, android.os.WorkSource ws) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          _data.writeInt(lockType);
          _data.writeString(tag);
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireWifiLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().acquireWifiLock(lock, lockType, tag, ws);
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
      @Override public void updateWifiLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateWifiLockWorkSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateWifiLockWorkSource(lock, ws);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean releaseWifiLock(android.os.IBinder lock) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseWifiLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().releaseWifiLock(lock);
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
      @Override public void initializeMulticastFiltering() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_initializeMulticastFiltering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initializeMulticastFiltering();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isMulticastEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMulticastEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMulticastEnabled();
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
      @Override public void acquireMulticastLock(android.os.IBinder binder, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireMulticastLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acquireMulticastLock(binder, tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releaseMulticastLock(java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseMulticastLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseMulticastLock(tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateInterfaceIpState(java.lang.String ifaceName, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(ifaceName);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateInterfaceIpState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateInterfaceIpState(ifaceName, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean startSoftAp(android.net.wifi.WifiConfiguration wifiConfig) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((wifiConfig!=null)) {
            _data.writeInt(1);
            wifiConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSoftAp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startSoftAp(wifiConfig);
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
      @Override public boolean stopSoftAp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopSoftAp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopSoftAp();
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
      @Override public int startLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder, java.lang.String packageName) throws android.os.RemoteException
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
          _data.writeStrongBinder(binder);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startLocalOnlyHotspot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startLocalOnlyHotspot(messenger, binder, packageName);
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
      @Override public void stopLocalOnlyHotspot() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopLocalOnlyHotspot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopLocalOnlyHotspot();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startWatchLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
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
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchLocalOnlyHotspot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWatchLocalOnlyHotspot(messenger, binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopWatchLocalOnlyHotspot() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWatchLocalOnlyHotspot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWatchLocalOnlyHotspot();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getWifiApEnabledState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiApEnabledState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiApEnabledState();
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
      @Override public android.net.wifi.WifiConfiguration getWifiApConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.wifi.WifiConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiApConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiApConfiguration();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setWifiApConfiguration(android.net.wifi.WifiConfiguration wifiConfig, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((wifiConfig!=null)) {
            _data.writeInt(1);
            wifiConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWifiApConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setWifiApConfiguration(wifiConfig, packageName);
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
      @Override public void notifyUserOfApBandConversion(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyUserOfApBandConversion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyUserOfApBandConversion(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Messenger getWifiServiceMessenger(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Messenger _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWifiServiceMessenger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWifiServiceMessenger(packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Messenger.CREATOR.createFromParcel(_reply);
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
      @Override public void enableTdls(java.lang.String remoteIPAddress, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(remoteIPAddress);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableTdls, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableTdls(remoteIPAddress, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void enableTdlsWithMacAddress(java.lang.String remoteMacAddress, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(remoteMacAddress);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableTdlsWithMacAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableTdlsWithMacAddress(remoteMacAddress, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getCurrentNetworkWpsNfcConfigurationToken() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentNetworkWpsNfcConfigurationToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentNetworkWpsNfcConfigurationToken();
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
      @Override public void enableVerboseLogging(int verbose) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(verbose);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableVerboseLogging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableVerboseLogging(verbose);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getVerboseLoggingLevel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVerboseLoggingLevel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVerboseLoggingLevel();
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
      @Override public void enableWifiConnectivityManager(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableWifiConnectivityManager, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableWifiConnectivityManager(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disableEphemeralNetwork(java.lang.String SSID, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(SSID);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableEphemeralNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableEphemeralNetwork(SSID, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void factoryReset(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_factoryReset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().factoryReset(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.Network getCurrentNetwork() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Network _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentNetwork();
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
      @Override public byte[] retrieveBackupData() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_retrieveBackupData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().retrieveBackupData();
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
      @Override public void restoreBackupData(byte[] data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreBackupData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreBackupData(data);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void restoreSupplicantBackupData(byte[] supplicantData, byte[] ipConfigData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(supplicantData);
          _data.writeByteArray(ipConfigData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreSupplicantBackupData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreSupplicantBackupData(supplicantData, ipConfigData);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startSubscriptionProvisioning(android.net.wifi.hotspot2.OsuProvider provider, android.net.wifi.hotspot2.IProvisioningCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provider!=null)) {
            _data.writeInt(1);
            provider.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSubscriptionProvisioning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSubscriptionProvisioning(provider, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerSoftApCallback(android.os.IBinder binder, android.net.wifi.ISoftApCallback callback, int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerSoftApCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerSoftApCallback(binder, callback, callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterSoftApCallback(int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterSoftApCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterSoftApCallback(callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addOnWifiUsabilityStatsListener(android.os.IBinder binder, android.net.wifi.IOnWifiUsabilityStatsListener listener, int listenerIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(listenerIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnWifiUsabilityStatsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnWifiUsabilityStatsListener(binder, listener, listenerIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOnWifiUsabilityStatsListener(int listenerIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(listenerIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOnWifiUsabilityStatsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOnWifiUsabilityStatsListener(listenerIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerTrafficStateCallback(android.os.IBinder binder, android.net.wifi.ITrafficStateCallback callback, int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTrafficStateCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTrafficStateCallback(binder, callback, callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterTrafficStateCallback(int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTrafficStateCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTrafficStateCallback(callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerNetworkRequestMatchCallback(android.os.IBinder binder, android.net.wifi.INetworkRequestMatchCallback callback, int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerNetworkRequestMatchCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerNetworkRequestMatchCallback(binder, callback, callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterNetworkRequestMatchCallback(int callbackIdentifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callbackIdentifier);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterNetworkRequestMatchCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterNetworkRequestMatchCallback(callbackIdentifier);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int addNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(networkSuggestions);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNetworkSuggestions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addNetworkSuggestions(networkSuggestions, packageName);
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
      @Override public int removeNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(networkSuggestions);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNetworkSuggestions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeNetworkSuggestions(networkSuggestions, packageName);
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
      @Override public java.lang.String[] getFactoryMacAddresses() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFactoryMacAddresses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFactoryMacAddresses();
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
      @Override public void setDeviceMobilityState(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeviceMobilityState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeviceMobilityState(state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startDppAsConfiguratorInitiator(android.os.IBinder binder, java.lang.String enrolleeUri, int selectedNetworkId, int netRole, android.net.wifi.IDppCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeString(enrolleeUri);
          _data.writeInt(selectedNetworkId);
          _data.writeInt(netRole);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startDppAsConfiguratorInitiator, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startDppAsConfiguratorInitiator(binder, enrolleeUri, selectedNetworkId, netRole, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startDppAsEnrolleeInitiator(android.os.IBinder binder, java.lang.String configuratorUri, android.net.wifi.IDppCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeString(configuratorUri);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startDppAsEnrolleeInitiator, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startDppAsEnrolleeInitiator(binder, configuratorUri, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopDppSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopDppSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopDppSession();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateWifiUsabilityScore(int seqNum, int score, int predictionHorizonSec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seqNum);
          _data.writeInt(score);
          _data.writeInt(predictionHorizonSec);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateWifiUsabilityScore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateWifiUsabilityScore(seqNum, score, predictionHorizonSec);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.wifi.IWifiManager sDefaultImpl;
    }
    static final int TRANSACTION_getSupportedFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_reportActivityInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestActivityInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getConfiguredNetworks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getPrivilegedConfiguredNetworks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getAllMatchingFqdnsForScanResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getMatchingOsuProviders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getMatchingPasspointConfigsForOsuProviders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_addOrUpdateNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_addOrUpdatePasspointConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_removePasspointConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getPasspointConfigurations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getWifiConfigsForPasspointProfiles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_queryPasspointIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_matchProviderWithCurrentNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_deauthenticateNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_removeNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_enableNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_disableNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_startScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getScanResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_reconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_reassociate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getConnectionInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setWifiEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getWifiEnabledState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_setCountryCode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getCountryCode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_isDualBandSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_needs5GHzToAnyApBandConversion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getDhcpInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_isScanAlwaysAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_acquireWifiLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_updateWifiLockWorkSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_releaseWifiLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_initializeMulticastFiltering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_isMulticastEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_acquireMulticastLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_releaseMulticastLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_updateInterfaceIpState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_startSoftAp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_stopSoftAp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_startLocalOnlyHotspot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_stopLocalOnlyHotspot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_startWatchLocalOnlyHotspot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_stopWatchLocalOnlyHotspot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_getWifiApEnabledState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_getWifiApConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_setWifiApConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_notifyUserOfApBandConversion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_getWifiServiceMessenger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_enableTdls = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_enableTdlsWithMacAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_getCurrentNetworkWpsNfcConfigurationToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_enableVerboseLogging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_getVerboseLoggingLevel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_enableWifiConnectivityManager = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_disableEphemeralNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_factoryReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_getCurrentNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_retrieveBackupData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_restoreBackupData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_restoreSupplicantBackupData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_startSubscriptionProvisioning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_registerSoftApCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_unregisterSoftApCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_addOnWifiUsabilityStatsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_removeOnWifiUsabilityStatsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_registerTrafficStateCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_unregisterTrafficStateCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_registerNetworkRequestMatchCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_unregisterNetworkRequestMatchCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_addNetworkSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_removeNetworkSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_getFactoryMacAddresses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_setDeviceMobilityState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_startDppAsConfiguratorInitiator = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_startDppAsEnrolleeInitiator = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_stopDppSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_updateWifiUsabilityScore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    public static boolean setDefaultImpl(android.net.wifi.IWifiManager impl) {
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
    public static android.net.wifi.IWifiManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public long getSupportedFeatures() throws android.os.RemoteException;
  public android.net.wifi.WifiActivityEnergyInfo reportActivityInfo() throws android.os.RemoteException;
  /**
       * Requests the controller activity info asynchronously.
       * The implementor is expected to reply with the
       * {@link android.net.wifi.WifiActivityEnergyInfo} object placed into the Bundle with the key
       * {@link android.os.BatteryStats#RESULT_RECEIVER_CONTROLLER_KEY}. The result code is ignored.
       */
  public void requestActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getPrivilegedConfiguredNetworks(java.lang.String packageName) throws android.os.RemoteException;
  public java.util.Map getAllMatchingFqdnsForScanResults(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException;
  public java.util.Map getMatchingOsuProviders(java.util.List<android.net.wifi.ScanResult> scanResult) throws android.os.RemoteException;
  public java.util.Map getMatchingPasspointConfigsForOsuProviders(java.util.List<android.net.wifi.hotspot2.OsuProvider> osuProviders) throws android.os.RemoteException;
  public int addOrUpdateNetwork(android.net.wifi.WifiConfiguration config, java.lang.String packageName) throws android.os.RemoteException;
  public boolean addOrUpdatePasspointConfiguration(android.net.wifi.hotspot2.PasspointConfiguration config, java.lang.String packageName) throws android.os.RemoteException;
  public boolean removePasspointConfiguration(java.lang.String fqdn, java.lang.String packageName) throws android.os.RemoteException;
  public java.util.List<android.net.wifi.hotspot2.PasspointConfiguration> getPasspointConfigurations(java.lang.String packageName) throws android.os.RemoteException;
  public java.util.List<android.net.wifi.WifiConfiguration> getWifiConfigsForPasspointProfiles(java.util.List<java.lang.String> fqdnList) throws android.os.RemoteException;
  public void queryPasspointIcon(long bssid, java.lang.String fileName) throws android.os.RemoteException;
  public int matchProviderWithCurrentNetwork(java.lang.String fqdn) throws android.os.RemoteException;
  public void deauthenticateNetwork(long holdoff, boolean ess) throws android.os.RemoteException;
  public boolean removeNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException;
  public boolean enableNetwork(int netId, boolean disableOthers, java.lang.String packageName) throws android.os.RemoteException;
  public boolean disableNetwork(int netId, java.lang.String packageName) throws android.os.RemoteException;
  public boolean startScan(java.lang.String packageName) throws android.os.RemoteException;
  public java.util.List<android.net.wifi.ScanResult> getScanResults(java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean disconnect(java.lang.String packageName) throws android.os.RemoteException;
  public boolean reconnect(java.lang.String packageName) throws android.os.RemoteException;
  public boolean reassociate(java.lang.String packageName) throws android.os.RemoteException;
  public android.net.wifi.WifiInfo getConnectionInfo(java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean setWifiEnabled(java.lang.String packageName, boolean enable) throws android.os.RemoteException;
  public int getWifiEnabledState() throws android.os.RemoteException;
  public void setCountryCode(java.lang.String country) throws android.os.RemoteException;
  public java.lang.String getCountryCode() throws android.os.RemoteException;
  public boolean isDualBandSupported() throws android.os.RemoteException;
  public boolean needs5GHzToAnyApBandConversion() throws android.os.RemoteException;
  public android.net.DhcpInfo getDhcpInfo() throws android.os.RemoteException;
  public boolean isScanAlwaysAvailable() throws android.os.RemoteException;
  public boolean acquireWifiLock(android.os.IBinder lock, int lockType, java.lang.String tag, android.os.WorkSource ws) throws android.os.RemoteException;
  public void updateWifiLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws) throws android.os.RemoteException;
  public boolean releaseWifiLock(android.os.IBinder lock) throws android.os.RemoteException;
  public void initializeMulticastFiltering() throws android.os.RemoteException;
  public boolean isMulticastEnabled() throws android.os.RemoteException;
  public void acquireMulticastLock(android.os.IBinder binder, java.lang.String tag) throws android.os.RemoteException;
  public void releaseMulticastLock(java.lang.String tag) throws android.os.RemoteException;
  public void updateInterfaceIpState(java.lang.String ifaceName, int mode) throws android.os.RemoteException;
  public boolean startSoftAp(android.net.wifi.WifiConfiguration wifiConfig) throws android.os.RemoteException;
  public boolean stopSoftAp() throws android.os.RemoteException;
  public int startLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder, java.lang.String packageName) throws android.os.RemoteException;
  public void stopLocalOnlyHotspot() throws android.os.RemoteException;
  public void startWatchLocalOnlyHotspot(android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException;
  public void stopWatchLocalOnlyHotspot() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/wifi/java/android/net/wifi/IWifiManager.aidl:150:1:150:25")
  public int getWifiApEnabledState() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/wifi/java/android/net/wifi/IWifiManager.aidl:153:1:153:25")
  public android.net.wifi.WifiConfiguration getWifiApConfiguration() throws android.os.RemoteException;
  public boolean setWifiApConfiguration(android.net.wifi.WifiConfiguration wifiConfig, java.lang.String packageName) throws android.os.RemoteException;
  public void notifyUserOfApBandConversion(java.lang.String packageName) throws android.os.RemoteException;
  public android.os.Messenger getWifiServiceMessenger(java.lang.String packageName) throws android.os.RemoteException;
  public void enableTdls(java.lang.String remoteIPAddress, boolean enable) throws android.os.RemoteException;
  public void enableTdlsWithMacAddress(java.lang.String remoteMacAddress, boolean enable) throws android.os.RemoteException;
  public java.lang.String getCurrentNetworkWpsNfcConfigurationToken() throws android.os.RemoteException;
  public void enableVerboseLogging(int verbose) throws android.os.RemoteException;
  public int getVerboseLoggingLevel() throws android.os.RemoteException;
  public void enableWifiConnectivityManager(boolean enabled) throws android.os.RemoteException;
  public void disableEphemeralNetwork(java.lang.String SSID, java.lang.String packageName) throws android.os.RemoteException;
  public void factoryReset(java.lang.String packageName) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/wifi/java/android/net/wifi/IWifiManager.aidl:178:1:178:25")
  public android.net.Network getCurrentNetwork() throws android.os.RemoteException;
  public byte[] retrieveBackupData() throws android.os.RemoteException;
  public void restoreBackupData(byte[] data) throws android.os.RemoteException;
  public void restoreSupplicantBackupData(byte[] supplicantData, byte[] ipConfigData) throws android.os.RemoteException;
  public void startSubscriptionProvisioning(android.net.wifi.hotspot2.OsuProvider provider, android.net.wifi.hotspot2.IProvisioningCallback callback) throws android.os.RemoteException;
  public void registerSoftApCallback(android.os.IBinder binder, android.net.wifi.ISoftApCallback callback, int callbackIdentifier) throws android.os.RemoteException;
  public void unregisterSoftApCallback(int callbackIdentifier) throws android.os.RemoteException;
  public void addOnWifiUsabilityStatsListener(android.os.IBinder binder, android.net.wifi.IOnWifiUsabilityStatsListener listener, int listenerIdentifier) throws android.os.RemoteException;
  public void removeOnWifiUsabilityStatsListener(int listenerIdentifier) throws android.os.RemoteException;
  public void registerTrafficStateCallback(android.os.IBinder binder, android.net.wifi.ITrafficStateCallback callback, int callbackIdentifier) throws android.os.RemoteException;
  public void unregisterTrafficStateCallback(int callbackIdentifier) throws android.os.RemoteException;
  public void registerNetworkRequestMatchCallback(android.os.IBinder binder, android.net.wifi.INetworkRequestMatchCallback callback, int callbackIdentifier) throws android.os.RemoteException;
  public void unregisterNetworkRequestMatchCallback(int callbackIdentifier) throws android.os.RemoteException;
  public int addNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException;
  public int removeNetworkSuggestions(java.util.List<android.net.wifi.WifiNetworkSuggestion> networkSuggestions, java.lang.String packageName) throws android.os.RemoteException;
  public java.lang.String[] getFactoryMacAddresses() throws android.os.RemoteException;
  public void setDeviceMobilityState(int state) throws android.os.RemoteException;
  public void startDppAsConfiguratorInitiator(android.os.IBinder binder, java.lang.String enrolleeUri, int selectedNetworkId, int netRole, android.net.wifi.IDppCallback callback) throws android.os.RemoteException;
  public void startDppAsEnrolleeInitiator(android.os.IBinder binder, java.lang.String configuratorUri, android.net.wifi.IDppCallback callback) throws android.os.RemoteException;
  public void stopDppSession() throws android.os.RemoteException;
  public void updateWifiUsabilityScore(int seqNum, int score, int predictionHorizonSec) throws android.os.RemoteException;
}
