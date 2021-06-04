/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * @hide
 */
public interface INetworkManagementService extends android.os.IInterface
{
  /** Default implementation for INetworkManagementService. */
  public static class Default implements android.os.INetworkManagementService
  {
    /**
         ** GENERAL
         **//**
         * Register an observer to receive events.
         */
    @Override public void registerObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException
    {
    }
    /**
         * Unregister an observer from receiving events.
         */
    @Override public void unregisterObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list of currently known network interfaces
         */
    @Override public java.lang.String[] listInterfaces() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Retrieves the specified interface config
         *
         */
    @Override public android.net.InterfaceConfiguration getInterfaceConfig(java.lang.String iface) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the configuration of the specified interface
         */
    @Override public void setInterfaceConfig(java.lang.String iface, android.net.InterfaceConfiguration cfg) throws android.os.RemoteException
    {
    }
    /**
         * Clear all IP addresses on the specified interface
         */
    @Override public void clearInterfaceAddresses(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set interface down
         */
    @Override public void setInterfaceDown(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set interface up
         */
    @Override public void setInterfaceUp(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set interface IPv6 privacy extensions
         */
    @Override public void setInterfaceIpv6PrivacyExtensions(java.lang.String iface, boolean enable) throws android.os.RemoteException
    {
    }
    /**
         * Disable IPv6 on an interface
         */
    @Override public void disableIpv6(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Enable IPv6 on an interface
         */
    @Override public void enableIpv6(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set IPv6 autoconf address generation mode.
         * This is a no-op if an unsupported mode is requested.
         */
    @Override public void setIPv6AddrGenMode(java.lang.String iface, int mode) throws android.os.RemoteException
    {
    }
    /**
         * Add the specified route to the interface.
         */
    @Override public void addRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException
    {
    }
    /**
         * Remove the specified route from the interface.
         */
    @Override public void removeRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException
    {
    }
    /**
         * Set the specified MTU size
         */
    @Override public void setMtu(java.lang.String iface, int mtu) throws android.os.RemoteException
    {
    }
    /**
         * Shuts down the service
         */
    @Override public void shutdown() throws android.os.RemoteException
    {
    }
    /**
         ** TETHERING RELATED
         **//**
         * Returns true if IP forwarding is enabled
         */
    @Override public boolean getIpForwardingEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Enables/Disables IP Forwarding
         */
    @Override public void setIpForwardingEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    /**
         * Start tethering services with the specified dhcp server range
         * arg is a set of start end pairs defining the ranges.
         */
    @Override public void startTethering(java.lang.String[] dhcpRanges) throws android.os.RemoteException
    {
    }
    /**
         * Stop currently running tethering services
         */
    @Override public void stopTethering() throws android.os.RemoteException
    {
    }
    /**
         * Returns true if tethering services are started
         */
    @Override public boolean isTetheringStarted() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Tethers the specified interface
         */
    @Override public void tetherInterface(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Untethers the specified interface
         */
    @Override public void untetherInterface(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list of currently tethered interfaces
         */
    @Override public java.lang.String[] listTetheredInterfaces() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the list of DNS forwarders (in order of priority)
         */
    @Override public void setDnsForwarders(android.net.Network network, java.lang.String[] dns) throws android.os.RemoteException
    {
    }
    /**
         * Returns the list of DNS forwarders (in order of priority)
         */
    @Override public java.lang.String[] getDnsForwarders() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Enables unidirectional packet forwarding from {@code fromIface} to
         * {@code toIface}.
         */
    @Override public void startInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException
    {
    }
    /**
         * Disables unidirectional packet forwarding from {@code fromIface} to
         * {@code toIface}.
         */
    @Override public void stopInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException
    {
    }
    /**
         *  Enables Network Address Translation between two interfaces.
         *  The address and netmask of the external interface is used for
         *  the NAT'ed network.
         */
    @Override public void enableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException
    {
    }
    /**
         *  Disables Network Address Translation between two interfaces.
         */
    @Override public void disableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException
    {
    }
    /**
         * Registers a {@code ITetheringStatsProvider} to provide tethering statistics.
         * All registered providers will be called in order, and their results will be added together.
         * Netd is always registered as a tethering stats provider.
         */
    @Override public void registerTetheringStatsProvider(android.net.ITetheringStatsProvider provider, java.lang.String name) throws android.os.RemoteException
    {
    }
    /**
         * Unregisters a previously-registered {@code ITetheringStatsProvider}.
         */
    @Override public void unregisterTetheringStatsProvider(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException
    {
    }
    /**
         * Reports that a tethering provider has reached a data limit.
         *
         * Currently triggers a global alert, which causes NetworkStatsService to poll counters and
         * re-evaluate data usage.
         *
         * This does not take an interface name because:
         * 1. The tethering offload stats provider cannot reliably determine the interface on which the
         *    limit was reached, because the HAL does not provide it.
         * 2. Firing an interface-specific alert instead of a global alert isn't really useful since in
         *    all cases of interest, the system responds to both in the same way - it polls stats, and
         *    then notifies NetworkPolicyManagerService of the fact.
         */
    @Override public void tetherLimitReached(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException
    {
    }
    /**
         ** DATA USAGE RELATED
         **//**
         * Return global network statistics summarized at an interface level,
         * without any UID-level granularity.
         */
    @Override public android.net.NetworkStats getNetworkStatsSummaryDev() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.NetworkStats getNetworkStatsSummaryXt() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return detailed network statistics with UID-level granularity,
         * including interface and tag details.
         */
    @Override public android.net.NetworkStats getNetworkStatsDetail() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return detailed network statistics for the requested UID and interfaces,
         * including interface and tag details.
         * @param uid UID to obtain statistics for, or {@link NetworkStats#UID_ALL}.
         * @param ifaces Interfaces to obtain statistics for, or {@link NetworkStats#INTERFACES_ALL}.
         */
    @Override public android.net.NetworkStats getNetworkStatsUidDetail(int uid, java.lang.String[] ifaces) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return summary of network statistics all tethering interfaces.
         */
    @Override public android.net.NetworkStats getNetworkStatsTethering(int how) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set quota for an interface.
         */
    @Override public void setInterfaceQuota(java.lang.String iface, long quotaBytes) throws android.os.RemoteException
    {
    }
    /**
         * Remove quota for an interface.
         */
    @Override public void removeInterfaceQuota(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set alert for an interface; requires that iface already has quota.
         */
    @Override public void setInterfaceAlert(java.lang.String iface, long alertBytes) throws android.os.RemoteException
    {
    }
    /**
         * Remove alert for an interface.
         */
    @Override public void removeInterfaceAlert(java.lang.String iface) throws android.os.RemoteException
    {
    }
    /**
         * Set alert across all interfaces.
         */
    @Override public void setGlobalAlert(long alertBytes) throws android.os.RemoteException
    {
    }
    /**
         * Control network activity of a UID over interfaces with a quota limit.
         */
    @Override public void setUidMeteredNetworkBlacklist(int uid, boolean enable) throws android.os.RemoteException
    {
    }
    @Override public void setUidMeteredNetworkWhitelist(int uid, boolean enable) throws android.os.RemoteException
    {
    }
    @Override public boolean setDataSaverModeEnabled(boolean enable) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setUidCleartextNetworkPolicy(int uid, int policy) throws android.os.RemoteException
    {
    }
    /**
         * Return status of bandwidth control module.
         */
    @Override public boolean isBandwidthControlEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets idletimer for an interface.
         *
         * This either initializes a new idletimer or increases its
         * reference-counting if an idletimer already exists for given
         * {@code iface}.
         *
         * {@code type} is the type of the interface, such as TYPE_MOBILE.
         *
         * Every {@code addIdleTimer} should be paired with a
         * {@link removeIdleTimer} to cleanup when the network disconnects.
         */
    @Override public void addIdleTimer(java.lang.String iface, int timeout, int type) throws android.os.RemoteException
    {
    }
    /**
         * Removes idletimer for an interface.
         */
    @Override public void removeIdleTimer(java.lang.String iface) throws android.os.RemoteException
    {
    }
    @Override public void setFirewallEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean isFirewallEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setFirewallInterfaceRule(java.lang.String iface, boolean allow) throws android.os.RemoteException
    {
    }
    @Override public void setFirewallUidRule(int chain, int uid, int rule) throws android.os.RemoteException
    {
    }
    @Override public void setFirewallUidRules(int chain, int[] uids, int[] rules) throws android.os.RemoteException
    {
    }
    @Override public void setFirewallChainEnabled(int chain, boolean enable) throws android.os.RemoteException
    {
    }
    /**
         * Set all packets from users in ranges to go through VPN specified by netId.
         */
    @Override public void addVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException
    {
    }
    /**
         * Clears the special VPN rules for users in ranges and VPN specified by netId.
         */
    @Override public void removeVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException
    {
    }
    /**
         * Start listening for mobile activity state changes.
         */
    @Override public void registerNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Stop listening for mobile activity state changes.
         */
    @Override public void unregisterNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Check whether the mobile radio is currently active.
         */
    @Override public boolean isNetworkActive() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Add an interface to a network.
         */
    @Override public void addInterfaceToNetwork(java.lang.String iface, int netId) throws android.os.RemoteException
    {
    }
    /**
         * Remove an Interface from a network.
         */
    @Override public void removeInterfaceFromNetwork(java.lang.String iface, int netId) throws android.os.RemoteException
    {
    }
    @Override public void addLegacyRouteForNetId(int netId, android.net.RouteInfo routeInfo, int uid) throws android.os.RemoteException
    {
    }
    @Override public void setDefaultNetId(int netId) throws android.os.RemoteException
    {
    }
    @Override public void clearDefaultNetId() throws android.os.RemoteException
    {
    }
    /**
         * Set permission for a network.
         * @param permission PERMISSION_NONE to clear permissions.
         *                   PERMISSION_NETWORK or PERMISSION_SYSTEM to set permission.
         */
    @Override public void setNetworkPermission(int netId, int permission) throws android.os.RemoteException
    {
    }
    /**
         * Allow UID to call protect().
         */
    @Override public void allowProtect(int uid) throws android.os.RemoteException
    {
    }
    /**
         * Deny UID from calling protect().
         */
    @Override public void denyProtect(int uid) throws android.os.RemoteException
    {
    }
    @Override public void addInterfaceToLocalNetwork(java.lang.String iface, java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException
    {
    }
    @Override public void removeInterfaceFromLocalNetwork(java.lang.String iface) throws android.os.RemoteException
    {
    }
    @Override public int removeRoutesFromLocalNetwork(java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setAllowOnlyVpnForUids(boolean enable, android.net.UidRange[] uidRanges) throws android.os.RemoteException
    {
    }
    @Override public boolean isNetworkRestricted(int uid) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.INetworkManagementService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.INetworkManagementService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.INetworkManagementService interface,
     * generating a proxy if needed.
     */
    public static android.os.INetworkManagementService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.INetworkManagementService))) {
        return ((android.os.INetworkManagementService)iin);
      }
      return new android.os.INetworkManagementService.Stub.Proxy(obj);
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
        case TRANSACTION_registerObserver:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkManagementEventObserver _arg0;
          _arg0 = android.net.INetworkManagementEventObserver.Stub.asInterface(data.readStrongBinder());
          this.registerObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterObserver:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkManagementEventObserver _arg0;
          _arg0 = android.net.INetworkManagementEventObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listInterfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.listInterfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getInterfaceConfig:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.InterfaceConfiguration _result = this.getInterfaceConfig(_arg0);
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
        case TRANSACTION_setInterfaceConfig:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.InterfaceConfiguration _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.InterfaceConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setInterfaceConfig(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearInterfaceAddresses:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.clearInterfaceAddresses(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInterfaceDown:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setInterfaceDown(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInterfaceUp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setInterfaceUp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInterfaceIpv6PrivacyExtensions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setInterfaceIpv6PrivacyExtensions(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableIpv6:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.disableIpv6(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableIpv6:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.enableIpv6(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setIPv6AddrGenMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setIPv6AddrGenMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addRoute:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.RouteInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.RouteInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addRoute(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeRoute:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.RouteInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.RouteInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.removeRoute(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMtu:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setMtu(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          this.shutdown();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getIpForwardingEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getIpForwardingEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setIpForwardingEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setIpForwardingEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startTethering:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          this.startTethering(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopTethering:
        {
          data.enforceInterface(descriptor);
          this.stopTethering();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTetheringStarted:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isTetheringStarted();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_tetherInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.tetherInterface(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_untetherInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.untetherInterface(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listTetheredInterfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.listTetheredInterfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_setDnsForwarders:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.setDnsForwarders(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDnsForwarders:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getDnsForwarders();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_startInterfaceForwarding:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.startInterfaceForwarding(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopInterfaceForwarding:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.stopInterfaceForwarding(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableNat:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.enableNat(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableNat:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.disableNat(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTetheringStatsProvider:
        {
          data.enforceInterface(descriptor);
          android.net.ITetheringStatsProvider _arg0;
          _arg0 = android.net.ITetheringStatsProvider.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.registerTetheringStatsProvider(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTetheringStatsProvider:
        {
          data.enforceInterface(descriptor);
          android.net.ITetheringStatsProvider _arg0;
          _arg0 = android.net.ITetheringStatsProvider.Stub.asInterface(data.readStrongBinder());
          this.unregisterTetheringStatsProvider(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_tetherLimitReached:
        {
          data.enforceInterface(descriptor);
          android.net.ITetheringStatsProvider _arg0;
          _arg0 = android.net.ITetheringStatsProvider.Stub.asInterface(data.readStrongBinder());
          this.tetherLimitReached(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNetworkStatsSummaryDev:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkStats _result = this.getNetworkStatsSummaryDev();
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
        case TRANSACTION_getNetworkStatsSummaryXt:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkStats _result = this.getNetworkStatsSummaryXt();
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
        case TRANSACTION_getNetworkStatsDetail:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkStats _result = this.getNetworkStatsDetail();
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
        case TRANSACTION_getNetworkStatsUidDetail:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          android.net.NetworkStats _result = this.getNetworkStatsUidDetail(_arg0, _arg1);
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
        case TRANSACTION_getNetworkStatsTethering:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NetworkStats _result = this.getNetworkStatsTethering(_arg0);
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
        case TRANSACTION_setInterfaceQuota:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          this.setInterfaceQuota(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeInterfaceQuota:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeInterfaceQuota(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInterfaceAlert:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          this.setInterfaceAlert(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeInterfaceAlert:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeInterfaceAlert(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setGlobalAlert:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setGlobalAlert(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUidMeteredNetworkBlacklist:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setUidMeteredNetworkBlacklist(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUidMeteredNetworkWhitelist:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setUidMeteredNetworkWhitelist(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDataSaverModeEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setDataSaverModeEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setUidCleartextNetworkPolicy:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setUidCleartextNetworkPolicy(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBandwidthControlEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isBandwidthControlEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addIdleTimer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.addIdleTimer(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeIdleTimer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeIdleTimer(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFirewallEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setFirewallEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isFirewallEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isFirewallEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setFirewallInterfaceRule:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setFirewallInterfaceRule(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFirewallUidRule:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setFirewallUidRule(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFirewallUidRules:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          int[] _arg2;
          _arg2 = data.createIntArray();
          this.setFirewallUidRules(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFirewallChainEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setFirewallChainEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addVpnUidRanges:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.UidRange[] _arg1;
          _arg1 = data.createTypedArray(android.net.UidRange.CREATOR);
          this.addVpnUidRanges(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeVpnUidRanges:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.UidRange[] _arg1;
          _arg1 = data.createTypedArray(android.net.UidRange.CREATOR);
          this.removeVpnUidRanges(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerNetworkActivityListener:
        {
          data.enforceInterface(descriptor);
          android.os.INetworkActivityListener _arg0;
          _arg0 = android.os.INetworkActivityListener.Stub.asInterface(data.readStrongBinder());
          this.registerNetworkActivityListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterNetworkActivityListener:
        {
          data.enforceInterface(descriptor);
          android.os.INetworkActivityListener _arg0;
          _arg0 = android.os.INetworkActivityListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterNetworkActivityListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isNetworkActive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isNetworkActive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addInterfaceToNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.addInterfaceToNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeInterfaceFromNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.removeInterfaceFromNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addLegacyRouteForNetId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.RouteInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.RouteInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.addLegacyRouteForNetId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDefaultNetId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDefaultNetId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearDefaultNetId:
        {
          data.enforceInterface(descriptor);
          this.clearDefaultNetId();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setNetworkPermission:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setNetworkPermission(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_allowProtect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.allowProtect(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_denyProtect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.denyProtect(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addInterfaceToLocalNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.net.RouteInfo> _arg1;
          _arg1 = data.createTypedArrayList(android.net.RouteInfo.CREATOR);
          this.addInterfaceToLocalNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeInterfaceFromLocalNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeInterfaceFromLocalNetwork(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeRoutesFromLocalNetwork:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.RouteInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.net.RouteInfo.CREATOR);
          int _result = this.removeRoutesFromLocalNetwork(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setAllowOnlyVpnForUids:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.net.UidRange[] _arg1;
          _arg1 = data.createTypedArray(android.net.UidRange.CREATOR);
          this.setAllowOnlyVpnForUids(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isNetworkRestricted:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isNetworkRestricted(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.INetworkManagementService
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
      /**
           ** GENERAL
           **//**
           * Register an observer to receive events.
           */
      @Override public void registerObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((obs!=null))?(obs.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerObserver(obs);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Unregister an observer from receiving events.
           */
      @Override public void unregisterObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((obs!=null))?(obs.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterObserver(obs);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns a list of currently known network interfaces
           */
      @Override public java.lang.String[] listInterfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listInterfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listInterfaces();
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
      /**
           * Retrieves the specified interface config
           *
           */
      @Override public android.net.InterfaceConfiguration getInterfaceConfig(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.InterfaceConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInterfaceConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInterfaceConfig(iface);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.InterfaceConfiguration.CREATOR.createFromParcel(_reply);
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
           * Sets the configuration of the specified interface
           */
      @Override public void setInterfaceConfig(java.lang.String iface, android.net.InterfaceConfiguration cfg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          if ((cfg!=null)) {
            _data.writeInt(1);
            cfg.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceConfig(iface, cfg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Clear all IP addresses on the specified interface
           */
      @Override public void clearInterfaceAddresses(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearInterfaceAddresses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearInterfaceAddresses(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set interface down
           */
      @Override public void setInterfaceDown(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceDown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceDown(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set interface up
           */
      @Override public void setInterfaceUp(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceUp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceUp(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set interface IPv6 privacy extensions
           */
      @Override public void setInterfaceIpv6PrivacyExtensions(java.lang.String iface, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceIpv6PrivacyExtensions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceIpv6PrivacyExtensions(iface, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Disable IPv6 on an interface
           */
      @Override public void disableIpv6(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableIpv6, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableIpv6(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Enable IPv6 on an interface
           */
      @Override public void enableIpv6(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableIpv6, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableIpv6(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set IPv6 autoconf address generation mode.
           * This is a no-op if an unsupported mode is requested.
           */
      @Override public void setIPv6AddrGenMode(java.lang.String iface, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIPv6AddrGenMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIPv6AddrGenMode(iface, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Add the specified route to the interface.
           */
      @Override public void addRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          if ((route!=null)) {
            _data.writeInt(1);
            route.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addRoute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addRoute(netId, route);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Remove the specified route from the interface.
           */
      @Override public void removeRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          if ((route!=null)) {
            _data.writeInt(1);
            route.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRoute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeRoute(netId, route);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set the specified MTU size
           */
      @Override public void setMtu(java.lang.String iface, int mtu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(mtu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMtu, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMtu(iface, mtu);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Shuts down the service
           */
      @Override public void shutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           ** TETHERING RELATED
           **//**
           * Returns true if IP forwarding is enabled
           */
      @Override public boolean getIpForwardingEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIpForwardingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIpForwardingEnabled();
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
      /**
           * Enables/Disables IP Forwarding
           */
      @Override public void setIpForwardingEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIpForwardingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIpForwardingEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Start tethering services with the specified dhcp server range
           * arg is a set of start end pairs defining the ranges.
           */
      @Override public void startTethering(java.lang.String[] dhcpRanges) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(dhcpRanges);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startTethering(dhcpRanges);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Stop currently running tethering services
           */
      @Override public void stopTethering() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopTethering();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns true if tethering services are started
           */
      @Override public boolean isTetheringStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTetheringStarted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTetheringStarted();
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
      /**
           * Tethers the specified interface
           */
      @Override public void tetherInterface(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_tetherInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tetherInterface(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Untethers the specified interface
           */
      @Override public void untetherInterface(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_untetherInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().untetherInterface(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns a list of currently tethered interfaces
           */
      @Override public java.lang.String[] listTetheredInterfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listTetheredInterfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listTetheredInterfaces();
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
      /**
           * Sets the list of DNS forwarders (in order of priority)
           */
      @Override public void setDnsForwarders(android.net.Network network, java.lang.String[] dns) throws android.os.RemoteException
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
          _data.writeStringArray(dns);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDnsForwarders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDnsForwarders(network, dns);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns the list of DNS forwarders (in order of priority)
           */
      @Override public java.lang.String[] getDnsForwarders() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDnsForwarders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDnsForwarders();
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
      /**
           * Enables unidirectional packet forwarding from {@code fromIface} to
           * {@code toIface}.
           */
      @Override public void startInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(fromIface);
          _data.writeString(toIface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInterfaceForwarding, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startInterfaceForwarding(fromIface, toIface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Disables unidirectional packet forwarding from {@code fromIface} to
           * {@code toIface}.
           */
      @Override public void stopInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(fromIface);
          _data.writeString(toIface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopInterfaceForwarding, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopInterfaceForwarding(fromIface, toIface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           *  Enables Network Address Translation between two interfaces.
           *  The address and netmask of the external interface is used for
           *  the NAT'ed network.
           */
      @Override public void enableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(internalInterface);
          _data.writeString(externalInterface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableNat, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableNat(internalInterface, externalInterface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           *  Disables Network Address Translation between two interfaces.
           */
      @Override public void disableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(internalInterface);
          _data.writeString(externalInterface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableNat, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableNat(internalInterface, externalInterface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Registers a {@code ITetheringStatsProvider} to provide tethering statistics.
           * All registered providers will be called in order, and their results will be added together.
           * Netd is always registered as a tethering stats provider.
           */
      @Override public void registerTetheringStatsProvider(android.net.ITetheringStatsProvider provider, java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((provider!=null))?(provider.asBinder()):(null)));
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTetheringStatsProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTetheringStatsProvider(provider, name);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Unregisters a previously-registered {@code ITetheringStatsProvider}.
           */
      @Override public void unregisterTetheringStatsProvider(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((provider!=null))?(provider.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTetheringStatsProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTetheringStatsProvider(provider);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Reports that a tethering provider has reached a data limit.
           *
           * Currently triggers a global alert, which causes NetworkStatsService to poll counters and
           * re-evaluate data usage.
           *
           * This does not take an interface name because:
           * 1. The tethering offload stats provider cannot reliably determine the interface on which the
           *    limit was reached, because the HAL does not provide it.
           * 2. Firing an interface-specific alert instead of a global alert isn't really useful since in
           *    all cases of interest, the system responds to both in the same way - it polls stats, and
           *    then notifies NetworkPolicyManagerService of the fact.
           */
      @Override public void tetherLimitReached(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((provider!=null))?(provider.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_tetherLimitReached, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tetherLimitReached(provider);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           ** DATA USAGE RELATED
           **//**
           * Return global network statistics summarized at an interface level,
           * without any UID-level granularity.
           */
      @Override public android.net.NetworkStats getNetworkStatsSummaryDev() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkStatsSummaryDev, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkStatsSummaryDev();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.NetworkStats getNetworkStatsSummaryXt() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkStatsSummaryXt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkStatsSummaryXt();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
           * Return detailed network statistics with UID-level granularity,
           * including interface and tag details.
           */
      @Override public android.net.NetworkStats getNetworkStatsDetail() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkStatsDetail, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkStatsDetail();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
           * Return detailed network statistics for the requested UID and interfaces,
           * including interface and tag details.
           * @param uid UID to obtain statistics for, or {@link NetworkStats#UID_ALL}.
           * @param ifaces Interfaces to obtain statistics for, or {@link NetworkStats#INTERFACES_ALL}.
           */
      @Override public android.net.NetworkStats getNetworkStatsUidDetail(int uid, java.lang.String[] ifaces) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeStringArray(ifaces);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkStatsUidDetail, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkStatsUidDetail(uid, ifaces);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
           * Return summary of network statistics all tethering interfaces.
           */
      @Override public android.net.NetworkStats getNetworkStatsTethering(int how) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(how);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkStatsTethering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkStatsTethering(how);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
           * Set quota for an interface.
           */
      @Override public void setInterfaceQuota(java.lang.String iface, long quotaBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeLong(quotaBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceQuota, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceQuota(iface, quotaBytes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Remove quota for an interface.
           */
      @Override public void removeInterfaceQuota(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeInterfaceQuota, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeInterfaceQuota(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set alert for an interface; requires that iface already has quota.
           */
      @Override public void setInterfaceAlert(java.lang.String iface, long alertBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeLong(alertBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInterfaceAlert, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInterfaceAlert(iface, alertBytes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Remove alert for an interface.
           */
      @Override public void removeInterfaceAlert(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeInterfaceAlert, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeInterfaceAlert(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set alert across all interfaces.
           */
      @Override public void setGlobalAlert(long alertBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(alertBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGlobalAlert, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGlobalAlert(alertBytes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Control network activity of a UID over interfaces with a quota limit.
           */
      @Override public void setUidMeteredNetworkBlacklist(int uid, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUidMeteredNetworkBlacklist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUidMeteredNetworkBlacklist(uid, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setUidMeteredNetworkWhitelist(int uid, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUidMeteredNetworkWhitelist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUidMeteredNetworkWhitelist(uid, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean setDataSaverModeEnabled(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataSaverModeEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDataSaverModeEnabled(enable);
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
      @Override public void setUidCleartextNetworkPolicy(int uid, int policy) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(policy);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUidCleartextNetworkPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUidCleartextNetworkPolicy(uid, policy);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Return status of bandwidth control module.
           */
      @Override public boolean isBandwidthControlEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBandwidthControlEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBandwidthControlEnabled();
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
      /**
           * Sets idletimer for an interface.
           *
           * This either initializes a new idletimer or increases its
           * reference-counting if an idletimer already exists for given
           * {@code iface}.
           *
           * {@code type} is the type of the interface, such as TYPE_MOBILE.
           *
           * Every {@code addIdleTimer} should be paired with a
           * {@link removeIdleTimer} to cleanup when the network disconnects.
           */
      @Override public void addIdleTimer(java.lang.String iface, int timeout, int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(timeout);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addIdleTimer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addIdleTimer(iface, timeout, type);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Removes idletimer for an interface.
           */
      @Override public void removeIdleTimer(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeIdleTimer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeIdleTimer(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFirewallEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFirewallEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFirewallEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isFirewallEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFirewallEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isFirewallEnabled();
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
      @Override public void setFirewallInterfaceRule(java.lang.String iface, boolean allow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(((allow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFirewallInterfaceRule, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFirewallInterfaceRule(iface, allow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFirewallUidRule(int chain, int uid, int rule) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(chain);
          _data.writeInt(uid);
          _data.writeInt(rule);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFirewallUidRule, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFirewallUidRule(chain, uid, rule);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFirewallUidRules(int chain, int[] uids, int[] rules) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(chain);
          _data.writeIntArray(uids);
          _data.writeIntArray(rules);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFirewallUidRules, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFirewallUidRules(chain, uids, rules);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFirewallChainEnabled(int chain, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(chain);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFirewallChainEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFirewallChainEnabled(chain, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set all packets from users in ranges to go through VPN specified by netId.
           */
      @Override public void addVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeTypedArray(ranges, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVpnUidRanges, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVpnUidRanges(netId, ranges);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Clears the special VPN rules for users in ranges and VPN specified by netId.
           */
      @Override public void removeVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeTypedArray(ranges, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVpnUidRanges, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeVpnUidRanges(netId, ranges);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Start listening for mobile activity state changes.
           */
      @Override public void registerNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerNetworkActivityListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerNetworkActivityListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Stop listening for mobile activity state changes.
           */
      @Override public void unregisterNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterNetworkActivityListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterNetworkActivityListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Check whether the mobile radio is currently active.
           */
      @Override public boolean isNetworkActive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNetworkActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNetworkActive();
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
      /**
           * Add an interface to a network.
           */
      @Override public void addInterfaceToNetwork(java.lang.String iface, int netId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(netId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addInterfaceToNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addInterfaceToNetwork(iface, netId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Remove an Interface from a network.
           */
      @Override public void removeInterfaceFromNetwork(java.lang.String iface, int netId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(netId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeInterfaceFromNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeInterfaceFromNetwork(iface, netId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addLegacyRouteForNetId(int netId, android.net.RouteInfo routeInfo, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          if ((routeInfo!=null)) {
            _data.writeInt(1);
            routeInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addLegacyRouteForNetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addLegacyRouteForNetId(netId, routeInfo, uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setDefaultNetId(int netId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultNetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultNetId(netId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearDefaultNetId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearDefaultNetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearDefaultNetId();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set permission for a network.
           * @param permission PERMISSION_NONE to clear permissions.
           *                   PERMISSION_NETWORK or PERMISSION_SYSTEM to set permission.
           */
      @Override public void setNetworkPermission(int netId, int permission) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeInt(permission);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNetworkPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNetworkPermission(netId, permission);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Allow UID to call protect().
           */
      @Override public void allowProtect(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_allowProtect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().allowProtect(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Deny UID from calling protect().
           */
      @Override public void denyProtect(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_denyProtect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().denyProtect(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addInterfaceToLocalNetwork(java.lang.String iface, java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeTypedList(routes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addInterfaceToLocalNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addInterfaceToLocalNetwork(iface, routes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeInterfaceFromLocalNetwork(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeInterfaceFromLocalNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeInterfaceFromLocalNetwork(iface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int removeRoutesFromLocalNetwork(java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(routes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRoutesFromLocalNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeRoutesFromLocalNetwork(routes);
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
      @Override public void setAllowOnlyVpnForUids(boolean enable, android.net.UidRange[] uidRanges) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeTypedArray(uidRanges, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAllowOnlyVpnForUids, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAllowOnlyVpnForUids(enable, uidRanges);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isNetworkRestricted(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNetworkRestricted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNetworkRestricted(uid);
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
      public static android.os.INetworkManagementService sDefaultImpl;
    }
    static final int TRANSACTION_registerObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_listInterfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getInterfaceConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setInterfaceConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_clearInterfaceAddresses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setInterfaceDown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setInterfaceUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setInterfaceIpv6PrivacyExtensions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_disableIpv6 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_enableIpv6 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setIPv6AddrGenMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_addRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_removeRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setMtu = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getIpForwardingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setIpForwardingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_startTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_stopTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_isTetheringStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_tetherInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_untetherInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_listTetheredInterfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_setDnsForwarders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getDnsForwarders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_startInterfaceForwarding = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_stopInterfaceForwarding = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_enableNat = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_disableNat = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_registerTetheringStatsProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_unregisterTetheringStatsProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_tetherLimitReached = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getNetworkStatsSummaryDev = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_getNetworkStatsSummaryXt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getNetworkStatsDetail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getNetworkStatsUidDetail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getNetworkStatsTethering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_setInterfaceQuota = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_removeInterfaceQuota = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setInterfaceAlert = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_removeInterfaceAlert = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_setGlobalAlert = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_setUidMeteredNetworkBlacklist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_setUidMeteredNetworkWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_setDataSaverModeEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_setUidCleartextNetworkPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_isBandwidthControlEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_addIdleTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_removeIdleTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_setFirewallEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_isFirewallEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_setFirewallInterfaceRule = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_setFirewallUidRule = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_setFirewallUidRules = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_setFirewallChainEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_addVpnUidRanges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_removeVpnUidRanges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_registerNetworkActivityListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_unregisterNetworkActivityListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_isNetworkActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_addInterfaceToNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_removeInterfaceFromNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_addLegacyRouteForNetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_setDefaultNetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_clearDefaultNetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_setNetworkPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_allowProtect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_denyProtect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_addInterfaceToLocalNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_removeInterfaceFromLocalNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_removeRoutesFromLocalNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_setAllowOnlyVpnForUids = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_isNetworkRestricted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    public static boolean setDefaultImpl(android.os.INetworkManagementService impl) {
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
    public static android.os.INetworkManagementService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       ** GENERAL
       **//**
       * Register an observer to receive events.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:41:1:41:25")
  public void registerObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException;
  /**
       * Unregister an observer from receiving events.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:47:1:47:25")
  public void unregisterObserver(android.net.INetworkManagementEventObserver obs) throws android.os.RemoteException;
  /**
       * Returns a list of currently known network interfaces
       */
  public java.lang.String[] listInterfaces() throws android.os.RemoteException;
  /**
       * Retrieves the specified interface config
       *
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:59:1:59:25")
  public android.net.InterfaceConfiguration getInterfaceConfig(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Sets the configuration of the specified interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:65:1:65:25")
  public void setInterfaceConfig(java.lang.String iface, android.net.InterfaceConfiguration cfg) throws android.os.RemoteException;
  /**
       * Clear all IP addresses on the specified interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:71:1:71:25")
  public void clearInterfaceAddresses(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set interface down
       */
  public void setInterfaceDown(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set interface up
       */
  public void setInterfaceUp(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set interface IPv6 privacy extensions
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:87:1:87:25")
  public void setInterfaceIpv6PrivacyExtensions(java.lang.String iface, boolean enable) throws android.os.RemoteException;
  /**
       * Disable IPv6 on an interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:93:1:93:25")
  public void disableIpv6(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Enable IPv6 on an interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:99:1:99:25")
  public void enableIpv6(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set IPv6 autoconf address generation mode.
       * This is a no-op if an unsupported mode is requested.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:106:1:106:25")
  public void setIPv6AddrGenMode(java.lang.String iface, int mode) throws android.os.RemoteException;
  /**
       * Add the specified route to the interface.
       */
  public void addRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException;
  /**
       * Remove the specified route from the interface.
       */
  public void removeRoute(int netId, android.net.RouteInfo route) throws android.os.RemoteException;
  /**
       * Set the specified MTU size
       */
  public void setMtu(java.lang.String iface, int mtu) throws android.os.RemoteException;
  /**
       * Shuts down the service
       */
  public void shutdown() throws android.os.RemoteException;
  /**
       ** TETHERING RELATED
       **//**
       * Returns true if IP forwarding is enabled
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:136:1:136:25")
  public boolean getIpForwardingEnabled() throws android.os.RemoteException;
  /**
       * Enables/Disables IP Forwarding
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:142:1:142:25")
  public void setIpForwardingEnabled(boolean enabled) throws android.os.RemoteException;
  /**
       * Start tethering services with the specified dhcp server range
       * arg is a set of start end pairs defining the ranges.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:149:1:149:25")
  public void startTethering(java.lang.String[] dhcpRanges) throws android.os.RemoteException;
  /**
       * Stop currently running tethering services
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:155:1:155:25")
  public void stopTethering() throws android.os.RemoteException;
  /**
       * Returns true if tethering services are started
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:161:1:161:25")
  public boolean isTetheringStarted() throws android.os.RemoteException;
  /**
       * Tethers the specified interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:167:1:167:25")
  public void tetherInterface(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Untethers the specified interface
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:173:1:173:25")
  public void untetherInterface(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Returns a list of currently tethered interfaces
       */
  public java.lang.String[] listTetheredInterfaces() throws android.os.RemoteException;
  /**
       * Sets the list of DNS forwarders (in order of priority)
       */
  public void setDnsForwarders(android.net.Network network, java.lang.String[] dns) throws android.os.RemoteException;
  /**
       * Returns the list of DNS forwarders (in order of priority)
       */
  public java.lang.String[] getDnsForwarders() throws android.os.RemoteException;
  /**
       * Enables unidirectional packet forwarding from {@code fromIface} to
       * {@code toIface}.
       */
  public void startInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException;
  /**
       * Disables unidirectional packet forwarding from {@code fromIface} to
       * {@code toIface}.
       */
  public void stopInterfaceForwarding(java.lang.String fromIface, java.lang.String toIface) throws android.os.RemoteException;
  /**
       *  Enables Network Address Translation between two interfaces.
       *  The address and netmask of the external interface is used for
       *  the NAT'ed network.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:208:1:208:25")
  public void enableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException;
  /**
       *  Disables Network Address Translation between two interfaces.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:214:1:214:25")
  public void disableNat(java.lang.String internalInterface, java.lang.String externalInterface) throws android.os.RemoteException;
  /**
       * Registers a {@code ITetheringStatsProvider} to provide tethering statistics.
       * All registered providers will be called in order, and their results will be added together.
       * Netd is always registered as a tethering stats provider.
       */
  public void registerTetheringStatsProvider(android.net.ITetheringStatsProvider provider, java.lang.String name) throws android.os.RemoteException;
  /**
       * Unregisters a previously-registered {@code ITetheringStatsProvider}.
       */
  public void unregisterTetheringStatsProvider(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException;
  /**
       * Reports that a tethering provider has reached a data limit.
       *
       * Currently triggers a global alert, which causes NetworkStatsService to poll counters and
       * re-evaluate data usage.
       *
       * This does not take an interface name because:
       * 1. The tethering offload stats provider cannot reliably determine the interface on which the
       *    limit was reached, because the HAL does not provide it.
       * 2. Firing an interface-specific alert instead of a global alert isn't really useful since in
       *    all cases of interest, the system responds to both in the same way - it polls stats, and
       *    then notifies NetworkPolicyManagerService of the fact.
       */
  public void tetherLimitReached(android.net.ITetheringStatsProvider provider) throws android.os.RemoteException;
  /**
       ** DATA USAGE RELATED
       **//**
       * Return global network statistics summarized at an interface level,
       * without any UID-level granularity.
       */
  public android.net.NetworkStats getNetworkStatsSummaryDev() throws android.os.RemoteException;
  public android.net.NetworkStats getNetworkStatsSummaryXt() throws android.os.RemoteException;
  /**
       * Return detailed network statistics with UID-level granularity,
       * including interface and tag details.
       */
  public android.net.NetworkStats getNetworkStatsDetail() throws android.os.RemoteException;
  /**
       * Return detailed network statistics for the requested UID and interfaces,
       * including interface and tag details.
       * @param uid UID to obtain statistics for, or {@link NetworkStats#UID_ALL}.
       * @param ifaces Interfaces to obtain statistics for, or {@link NetworkStats#INTERFACES_ALL}.
       */
  public android.net.NetworkStats getNetworkStatsUidDetail(int uid, java.lang.String[] ifaces) throws android.os.RemoteException;
  /**
       * Return summary of network statistics all tethering interfaces.
       */
  public android.net.NetworkStats getNetworkStatsTethering(int how) throws android.os.RemoteException;
  /**
       * Set quota for an interface.
       */
  public void setInterfaceQuota(java.lang.String iface, long quotaBytes) throws android.os.RemoteException;
  /**
       * Remove quota for an interface.
       */
  public void removeInterfaceQuota(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set alert for an interface; requires that iface already has quota.
       */
  public void setInterfaceAlert(java.lang.String iface, long alertBytes) throws android.os.RemoteException;
  /**
       * Remove alert for an interface.
       */
  public void removeInterfaceAlert(java.lang.String iface) throws android.os.RemoteException;
  /**
       * Set alert across all interfaces.
       */
  public void setGlobalAlert(long alertBytes) throws android.os.RemoteException;
  /**
       * Control network activity of a UID over interfaces with a quota limit.
       */
  public void setUidMeteredNetworkBlacklist(int uid, boolean enable) throws android.os.RemoteException;
  public void setUidMeteredNetworkWhitelist(int uid, boolean enable) throws android.os.RemoteException;
  public boolean setDataSaverModeEnabled(boolean enable) throws android.os.RemoteException;
  public void setUidCleartextNetworkPolicy(int uid, int policy) throws android.os.RemoteException;
  /**
       * Return status of bandwidth control module.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/INetworkManagementService.aidl:311:1:311:25")
  public boolean isBandwidthControlEnabled() throws android.os.RemoteException;
  /**
       * Sets idletimer for an interface.
       *
       * This either initializes a new idletimer or increases its
       * reference-counting if an idletimer already exists for given
       * {@code iface}.
       *
       * {@code type} is the type of the interface, such as TYPE_MOBILE.
       *
       * Every {@code addIdleTimer} should be paired with a
       * {@link removeIdleTimer} to cleanup when the network disconnects.
       */
  public void addIdleTimer(java.lang.String iface, int timeout, int type) throws android.os.RemoteException;
  /**
       * Removes idletimer for an interface.
       */
  public void removeIdleTimer(java.lang.String iface) throws android.os.RemoteException;
  public void setFirewallEnabled(boolean enabled) throws android.os.RemoteException;
  public boolean isFirewallEnabled() throws android.os.RemoteException;
  public void setFirewallInterfaceRule(java.lang.String iface, boolean allow) throws android.os.RemoteException;
  public void setFirewallUidRule(int chain, int uid, int rule) throws android.os.RemoteException;
  public void setFirewallUidRules(int chain, int[] uids, int[] rules) throws android.os.RemoteException;
  public void setFirewallChainEnabled(int chain, boolean enable) throws android.os.RemoteException;
  /**
       * Set all packets from users in ranges to go through VPN specified by netId.
       */
  public void addVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException;
  /**
       * Clears the special VPN rules for users in ranges and VPN specified by netId.
       */
  public void removeVpnUidRanges(int netId, android.net.UidRange[] ranges) throws android.os.RemoteException;
  /**
       * Start listening for mobile activity state changes.
       */
  public void registerNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException;
  /**
       * Stop listening for mobile activity state changes.
       */
  public void unregisterNetworkActivityListener(android.os.INetworkActivityListener listener) throws android.os.RemoteException;
  /**
       * Check whether the mobile radio is currently active.
       */
  public boolean isNetworkActive() throws android.os.RemoteException;
  /**
       * Add an interface to a network.
       */
  public void addInterfaceToNetwork(java.lang.String iface, int netId) throws android.os.RemoteException;
  /**
       * Remove an Interface from a network.
       */
  public void removeInterfaceFromNetwork(java.lang.String iface, int netId) throws android.os.RemoteException;
  public void addLegacyRouteForNetId(int netId, android.net.RouteInfo routeInfo, int uid) throws android.os.RemoteException;
  public void setDefaultNetId(int netId) throws android.os.RemoteException;
  public void clearDefaultNetId() throws android.os.RemoteException;
  /**
       * Set permission for a network.
       * @param permission PERMISSION_NONE to clear permissions.
       *                   PERMISSION_NETWORK or PERMISSION_SYSTEM to set permission.
       */
  public void setNetworkPermission(int netId, int permission) throws android.os.RemoteException;
  /**
       * Allow UID to call protect().
       */
  public void allowProtect(int uid) throws android.os.RemoteException;
  /**
       * Deny UID from calling protect().
       */
  public void denyProtect(int uid) throws android.os.RemoteException;
  public void addInterfaceToLocalNetwork(java.lang.String iface, java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException;
  public void removeInterfaceFromLocalNetwork(java.lang.String iface) throws android.os.RemoteException;
  public int removeRoutesFromLocalNetwork(java.util.List<android.net.RouteInfo> routes) throws android.os.RemoteException;
  public void setAllowOnlyVpnForUids(boolean enable, android.net.UidRange[] uidRanges) throws android.os.RemoteException;
  public boolean isNetworkRestricted(int uid) throws android.os.RemoteException;
}
