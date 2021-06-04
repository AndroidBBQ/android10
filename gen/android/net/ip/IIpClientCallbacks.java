/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.ip;
/** @hide */
public interface IIpClientCallbacks extends android.os.IInterface
{
  /** Default implementation for IIpClientCallbacks. */
  public static class Default implements android.net.ip.IIpClientCallbacks
  {
    @Override public void onIpClientCreated(android.net.ip.IIpClient ipClient) throws android.os.RemoteException
    {
    }
    @Override public void onPreDhcpAction() throws android.os.RemoteException
    {
    }
    @Override public void onPostDhcpAction() throws android.os.RemoteException
    {
    }
    // This is purely advisory and not an indication of provisioning
    // success or failure.  This is only here for callers that want to
    // expose DHCPv4 results to other APIs (e.g., WifiInfo#setInetAddress).
    // DHCPv4 or static IPv4 configuration failure or success can be
    // determined by whether or not the passed-in DhcpResults object is
    // null or not.

    @Override public void onNewDhcpResults(android.net.DhcpResultsParcelable dhcpResults) throws android.os.RemoteException
    {
    }
    @Override public void onProvisioningSuccess(android.net.LinkProperties newLp) throws android.os.RemoteException
    {
    }
    @Override public void onProvisioningFailure(android.net.LinkProperties newLp) throws android.os.RemoteException
    {
    }
    // Invoked on LinkProperties changes.

    @Override public void onLinkPropertiesChange(android.net.LinkProperties newLp) throws android.os.RemoteException
    {
    }
    // Called when the internal IpReachabilityMonitor (if enabled) has
    // detected the loss of a critical number of required neighbors.

    @Override public void onReachabilityLost(java.lang.String logMsg) throws android.os.RemoteException
    {
    }
    // Called when the IpClient state machine terminates.

    @Override public void onQuit() throws android.os.RemoteException
    {
    }
    // Install an APF program to filter incoming packets.

    @Override public void installPacketFilter(byte[] filter) throws android.os.RemoteException
    {
    }
    // Asynchronously read back the APF program & data buffer from the wifi driver.
    // Due to Wifi HAL limitations, the current implementation only supports dumping the entire
    // buffer. In response to this request, the driver returns the data buffer asynchronously
    // by sending an IpClient#EVENT_READ_PACKET_FILTER_COMPLETE message.

    @Override public void startReadPacketFilter() throws android.os.RemoteException
    {
    }
    // If multicast filtering cannot be accomplished with APF, this function will be called to
    // actuate multicast filtering using another means.

    @Override public void setFallbackMulticastFilter(boolean enabled) throws android.os.RemoteException
    {
    }
    // Enabled/disable Neighbor Discover offload functionality. This is
    // called, for example, whenever 464xlat is being started or stopped.

    @Override public void setNeighborDiscoveryOffload(boolean enable) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ip.IIpClientCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ip.IIpClientCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ip.IIpClientCallbacks interface,
     * generating a proxy if needed.
     */
    public static android.net.ip.IIpClientCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ip.IIpClientCallbacks))) {
        return ((android.net.ip.IIpClientCallbacks)iin);
      }
      return new android.net.ip.IIpClientCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_onIpClientCreated:
        {
          data.enforceInterface(descriptor);
          android.net.ip.IIpClient _arg0;
          _arg0 = android.net.ip.IIpClient.Stub.asInterface(data.readStrongBinder());
          this.onIpClientCreated(_arg0);
          return true;
        }
        case TRANSACTION_onPreDhcpAction:
        {
          data.enforceInterface(descriptor);
          this.onPreDhcpAction();
          return true;
        }
        case TRANSACTION_onPostDhcpAction:
        {
          data.enforceInterface(descriptor);
          this.onPostDhcpAction();
          return true;
        }
        case TRANSACTION_onNewDhcpResults:
        {
          data.enforceInterface(descriptor);
          android.net.DhcpResultsParcelable _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.DhcpResultsParcelable.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onNewDhcpResults(_arg0);
          return true;
        }
        case TRANSACTION_onProvisioningSuccess:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onProvisioningSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onProvisioningFailure:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onProvisioningFailure(_arg0);
          return true;
        }
        case TRANSACTION_onLinkPropertiesChange:
        {
          data.enforceInterface(descriptor);
          android.net.LinkProperties _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onLinkPropertiesChange(_arg0);
          return true;
        }
        case TRANSACTION_onReachabilityLost:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onReachabilityLost(_arg0);
          return true;
        }
        case TRANSACTION_onQuit:
        {
          data.enforceInterface(descriptor);
          this.onQuit();
          return true;
        }
        case TRANSACTION_installPacketFilter:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.installPacketFilter(_arg0);
          return true;
        }
        case TRANSACTION_startReadPacketFilter:
        {
          data.enforceInterface(descriptor);
          this.startReadPacketFilter();
          return true;
        }
        case TRANSACTION_setFallbackMulticastFilter:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setFallbackMulticastFilter(_arg0);
          return true;
        }
        case TRANSACTION_setNeighborDiscoveryOffload:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setNeighborDiscoveryOffload(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ip.IIpClientCallbacks
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
      @Override public void onIpClientCreated(android.net.ip.IIpClient ipClient) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ipClient!=null))?(ipClient.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onIpClientCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onIpClientCreated(ipClient);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPreDhcpAction() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPreDhcpAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPreDhcpAction();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPostDhcpAction() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPostDhcpAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPostDhcpAction();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // This is purely advisory and not an indication of provisioning
      // success or failure.  This is only here for callers that want to
      // expose DHCPv4 results to other APIs (e.g., WifiInfo#setInetAddress).
      // DHCPv4 or static IPv4 configuration failure or success can be
      // determined by whether or not the passed-in DhcpResults object is
      // null or not.

      @Override public void onNewDhcpResults(android.net.DhcpResultsParcelable dhcpResults) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((dhcpResults!=null)) {
            _data.writeInt(1);
            dhcpResults.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNewDhcpResults, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNewDhcpResults(dhcpResults);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onProvisioningSuccess(android.net.LinkProperties newLp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((newLp!=null)) {
            _data.writeInt(1);
            newLp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProvisioningSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProvisioningSuccess(newLp);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onProvisioningFailure(android.net.LinkProperties newLp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((newLp!=null)) {
            _data.writeInt(1);
            newLp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProvisioningFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProvisioningFailure(newLp);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Invoked on LinkProperties changes.

      @Override public void onLinkPropertiesChange(android.net.LinkProperties newLp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((newLp!=null)) {
            _data.writeInt(1);
            newLp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLinkPropertiesChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLinkPropertiesChange(newLp);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Called when the internal IpReachabilityMonitor (if enabled) has
      // detected the loss of a critical number of required neighbors.

      @Override public void onReachabilityLost(java.lang.String logMsg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(logMsg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReachabilityLost, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReachabilityLost(logMsg);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Called when the IpClient state machine terminates.

      @Override public void onQuit() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onQuit, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onQuit();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Install an APF program to filter incoming packets.

      @Override public void installPacketFilter(byte[] filter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(filter);
          boolean _status = mRemote.transact(Stub.TRANSACTION_installPacketFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().installPacketFilter(filter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Asynchronously read back the APF program & data buffer from the wifi driver.
      // Due to Wifi HAL limitations, the current implementation only supports dumping the entire
      // buffer. In response to this request, the driver returns the data buffer asynchronously
      // by sending an IpClient#EVENT_READ_PACKET_FILTER_COMPLETE message.

      @Override public void startReadPacketFilter() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startReadPacketFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startReadPacketFilter();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // If multicast filtering cannot be accomplished with APF, this function will be called to
      // actuate multicast filtering using another means.

      @Override public void setFallbackMulticastFilter(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFallbackMulticastFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFallbackMulticastFilter(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Enabled/disable Neighbor Discover offload functionality. This is
      // called, for example, whenever 464xlat is being started or stopped.

      @Override public void setNeighborDiscoveryOffload(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNeighborDiscoveryOffload, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNeighborDiscoveryOffload(enable);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.ip.IIpClientCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_onIpClientCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onPreDhcpAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onPostDhcpAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onNewDhcpResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onProvisioningSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onProvisioningFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onLinkPropertiesChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onReachabilityLost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onQuit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_installPacketFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_startReadPacketFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setFallbackMulticastFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setNeighborDiscoveryOffload = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.net.ip.IIpClientCallbacks impl) {
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
    public static android.net.ip.IIpClientCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onIpClientCreated(android.net.ip.IIpClient ipClient) throws android.os.RemoteException;
  public void onPreDhcpAction() throws android.os.RemoteException;
  public void onPostDhcpAction() throws android.os.RemoteException;
  // This is purely advisory and not an indication of provisioning
  // success or failure.  This is only here for callers that want to
  // expose DHCPv4 results to other APIs (e.g., WifiInfo#setInetAddress).
  // DHCPv4 or static IPv4 configuration failure or success can be
  // determined by whether or not the passed-in DhcpResults object is
  // null or not.

  public void onNewDhcpResults(android.net.DhcpResultsParcelable dhcpResults) throws android.os.RemoteException;
  public void onProvisioningSuccess(android.net.LinkProperties newLp) throws android.os.RemoteException;
  public void onProvisioningFailure(android.net.LinkProperties newLp) throws android.os.RemoteException;
  // Invoked on LinkProperties changes.

  public void onLinkPropertiesChange(android.net.LinkProperties newLp) throws android.os.RemoteException;
  // Called when the internal IpReachabilityMonitor (if enabled) has
  // detected the loss of a critical number of required neighbors.

  public void onReachabilityLost(java.lang.String logMsg) throws android.os.RemoteException;
  // Called when the IpClient state machine terminates.

  public void onQuit() throws android.os.RemoteException;
  // Install an APF program to filter incoming packets.

  public void installPacketFilter(byte[] filter) throws android.os.RemoteException;
  // Asynchronously read back the APF program & data buffer from the wifi driver.
  // Due to Wifi HAL limitations, the current implementation only supports dumping the entire
  // buffer. In response to this request, the driver returns the data buffer asynchronously
  // by sending an IpClient#EVENT_READ_PACKET_FILTER_COMPLETE message.

  public void startReadPacketFilter() throws android.os.RemoteException;
  // If multicast filtering cannot be accomplished with APF, this function will be called to
  // actuate multicast filtering using another means.

  public void setFallbackMulticastFilter(boolean enabled) throws android.os.RemoteException;
  // Enabled/disable Neighbor Discover offload functionality. This is
  // called, for example, whenever 464xlat is being started or stopped.

  public void setNeighborDiscoveryOffload(boolean enable) throws android.os.RemoteException;
}
