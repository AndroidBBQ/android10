/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.ip;
/** @hide */
public interface IIpClient extends android.os.IInterface
{
  /** Default implementation for IIpClient. */
  public static class Default implements android.net.ip.IIpClient
  {
    @Override public void completedPreDhcpAction() throws android.os.RemoteException
    {
    }
    @Override public void confirmConfiguration() throws android.os.RemoteException
    {
    }
    @Override public void readPacketFilterComplete(byte[] data) throws android.os.RemoteException
    {
    }
    @Override public void shutdown() throws android.os.RemoteException
    {
    }
    @Override public void startProvisioning(android.net.ProvisioningConfigurationParcelable req) throws android.os.RemoteException
    {
    }
    @Override public void stop() throws android.os.RemoteException
    {
    }
    @Override public void setTcpBufferSizes(java.lang.String tcpBufferSizes) throws android.os.RemoteException
    {
    }
    @Override public void setHttpProxy(android.net.ProxyInfo proxyInfo) throws android.os.RemoteException
    {
    }
    @Override public void setMulticastFilter(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void addKeepalivePacketFilter(int slot, android.net.TcpKeepalivePacketDataParcelable pkt) throws android.os.RemoteException
    {
    }
    @Override public void removeKeepalivePacketFilter(int slot) throws android.os.RemoteException
    {
    }
    @Override public void setL2KeyAndGroupHint(java.lang.String l2Key, java.lang.String groupHint) throws android.os.RemoteException
    {
    }
    @Override public void addNattKeepalivePacketFilter(int slot, android.net.NattKeepalivePacketDataParcelable pkt) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ip.IIpClient
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ip.IIpClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ip.IIpClient interface,
     * generating a proxy if needed.
     */
    public static android.net.ip.IIpClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ip.IIpClient))) {
        return ((android.net.ip.IIpClient)iin);
      }
      return new android.net.ip.IIpClient.Stub.Proxy(obj);
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
        case TRANSACTION_completedPreDhcpAction:
        {
          data.enforceInterface(descriptor);
          this.completedPreDhcpAction();
          return true;
        }
        case TRANSACTION_confirmConfiguration:
        {
          data.enforceInterface(descriptor);
          this.confirmConfiguration();
          return true;
        }
        case TRANSACTION_readPacketFilterComplete:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.readPacketFilterComplete(_arg0);
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          this.shutdown();
          return true;
        }
        case TRANSACTION_startProvisioning:
        {
          data.enforceInterface(descriptor);
          android.net.ProvisioningConfigurationParcelable _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.ProvisioningConfigurationParcelable.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startProvisioning(_arg0);
          return true;
        }
        case TRANSACTION_stop:
        {
          data.enforceInterface(descriptor);
          this.stop();
          return true;
        }
        case TRANSACTION_setTcpBufferSizes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTcpBufferSizes(_arg0);
          return true;
        }
        case TRANSACTION_setHttpProxy:
        {
          data.enforceInterface(descriptor);
          android.net.ProxyInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.ProxyInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setHttpProxy(_arg0);
          return true;
        }
        case TRANSACTION_setMulticastFilter:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setMulticastFilter(_arg0);
          return true;
        }
        case TRANSACTION_addKeepalivePacketFilter:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.TcpKeepalivePacketDataParcelable _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.TcpKeepalivePacketDataParcelable.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addKeepalivePacketFilter(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeKeepalivePacketFilter:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeKeepalivePacketFilter(_arg0);
          return true;
        }
        case TRANSACTION_setL2KeyAndGroupHint:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setL2KeyAndGroupHint(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addNattKeepalivePacketFilter:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NattKeepalivePacketDataParcelable _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.NattKeepalivePacketDataParcelable.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addNattKeepalivePacketFilter(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ip.IIpClient
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
      @Override public void completedPreDhcpAction() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_completedPreDhcpAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().completedPreDhcpAction();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void confirmConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_confirmConfiguration, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().confirmConfiguration();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void readPacketFilterComplete(byte[] data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_readPacketFilterComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().readPacketFilterComplete(data);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void shutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startProvisioning(android.net.ProvisioningConfigurationParcelable req) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((req!=null)) {
            _data.writeInt(1);
            req.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startProvisioning, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startProvisioning(req);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stop() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stop, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stop();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setTcpBufferSizes(java.lang.String tcpBufferSizes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(tcpBufferSizes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTcpBufferSizes, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTcpBufferSizes(tcpBufferSizes);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setHttpProxy(android.net.ProxyInfo proxyInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((proxyInfo!=null)) {
            _data.writeInt(1);
            proxyInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHttpProxy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHttpProxy(proxyInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setMulticastFilter(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMulticastFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMulticastFilter(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addKeepalivePacketFilter(int slot, android.net.TcpKeepalivePacketDataParcelable pkt) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slot);
          if ((pkt!=null)) {
            _data.writeInt(1);
            pkt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addKeepalivePacketFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addKeepalivePacketFilter(slot, pkt);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeKeepalivePacketFilter(int slot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slot);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeKeepalivePacketFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeKeepalivePacketFilter(slot);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setL2KeyAndGroupHint(java.lang.String l2Key, java.lang.String groupHint) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(l2Key);
          _data.writeString(groupHint);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setL2KeyAndGroupHint, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setL2KeyAndGroupHint(l2Key, groupHint);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addNattKeepalivePacketFilter(int slot, android.net.NattKeepalivePacketDataParcelable pkt) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slot);
          if ((pkt!=null)) {
            _data.writeInt(1);
            pkt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNattKeepalivePacketFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addNattKeepalivePacketFilter(slot, pkt);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.ip.IIpClient sDefaultImpl;
    }
    static final int TRANSACTION_completedPreDhcpAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_confirmConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_readPacketFilterComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startProvisioning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_stop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setTcpBufferSizes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setHttpProxy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setMulticastFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_addKeepalivePacketFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_removeKeepalivePacketFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setL2KeyAndGroupHint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_addNattKeepalivePacketFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.net.ip.IIpClient impl) {
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
    public static android.net.ip.IIpClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void completedPreDhcpAction() throws android.os.RemoteException;
  public void confirmConfiguration() throws android.os.RemoteException;
  public void readPacketFilterComplete(byte[] data) throws android.os.RemoteException;
  public void shutdown() throws android.os.RemoteException;
  public void startProvisioning(android.net.ProvisioningConfigurationParcelable req) throws android.os.RemoteException;
  public void stop() throws android.os.RemoteException;
  public void setTcpBufferSizes(java.lang.String tcpBufferSizes) throws android.os.RemoteException;
  public void setHttpProxy(android.net.ProxyInfo proxyInfo) throws android.os.RemoteException;
  public void setMulticastFilter(boolean enabled) throws android.os.RemoteException;
  public void addKeepalivePacketFilter(int slot, android.net.TcpKeepalivePacketDataParcelable pkt) throws android.os.RemoteException;
  public void removeKeepalivePacketFilter(int slot) throws android.os.RemoteException;
  public void setL2KeyAndGroupHint(java.lang.String l2Key, java.lang.String groupHint) throws android.os.RemoteException;
  public void addNattKeepalivePacketFilter(int slot, android.net.NattKeepalivePacketDataParcelable pkt) throws android.os.RemoteException;
}
