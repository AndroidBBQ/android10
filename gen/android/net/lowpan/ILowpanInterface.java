/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.lowpan;
/** {@hide} */
public interface ILowpanInterface extends android.os.IInterface
{
  /** Default implementation for ILowpanInterface. */
  public static class Default implements android.net.lowpan.ILowpanInterface
  {
    // Methods

    @Override public java.lang.String getName() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getNcpVersion() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getDriverVersion() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.lowpan.LowpanChannelInfo[] getSupportedChannels() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getSupportedNetworkTypes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] getMacAddress() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean isUp() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isCommissioned() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isConnected() throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String getState() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getRole() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getPartitionId() throws android.os.RemoteException
    {
      return null;
    }
    @Override public byte[] getExtendedAddress() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.lowpan.LowpanIdentity getLowpanIdentity() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.lowpan.LowpanCredential getLowpanCredential() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getLinkAddresses() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.IpPrefix[] getLinkNetworks() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void join(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
    {
    }
    @Override public void form(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
    {
    }
    @Override public void attach(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
    {
    }
    @Override public void leave() throws android.os.RemoteException
    {
    }
    @Override public void reset() throws android.os.RemoteException
    {
    }
    @Override public void startCommissioningSession(android.net.lowpan.LowpanBeaconInfo beaconInfo) throws android.os.RemoteException
    {
    }
    @Override public void closeCommissioningSession() throws android.os.RemoteException
    {
    }
    @Override public void sendToCommissioner(byte[] packet) throws android.os.RemoteException
    {
    }
    @Override public void beginLowPower() throws android.os.RemoteException
    {
    }
    @Override public void pollForData() throws android.os.RemoteException
    {
    }
    @Override public void onHostWake() throws android.os.RemoteException
    {
    }
    @Override public void addListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException
    {
    }
    @Override public void startNetScan(java.util.Map properties, android.net.lowpan.ILowpanNetScanCallback listener) throws android.os.RemoteException
    {
    }
    @Override public void stopNetScan() throws android.os.RemoteException
    {
    }
    @Override public void startEnergyScan(java.util.Map properties, android.net.lowpan.ILowpanEnergyScanCallback listener) throws android.os.RemoteException
    {
    }
    @Override public void stopEnergyScan() throws android.os.RemoteException
    {
    }
    @Override public void addOnMeshPrefix(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException
    {
    }
    @Override public void removeOnMeshPrefix(android.net.IpPrefix prefix) throws android.os.RemoteException
    {
    }
    @Override public void addExternalRoute(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException
    {
    }
    @Override public void removeExternalRoute(android.net.IpPrefix prefix) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.lowpan.ILowpanInterface
  {
    private static final java.lang.String DESCRIPTOR = "android.net.lowpan.ILowpanInterface";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.lowpan.ILowpanInterface interface,
     * generating a proxy if needed.
     */
    public static android.net.lowpan.ILowpanInterface asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.lowpan.ILowpanInterface))) {
        return ((android.net.lowpan.ILowpanInterface)iin);
      }
      return new android.net.lowpan.ILowpanInterface.Stub.Proxy(obj);
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
        case TRANSACTION_getName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getNcpVersion:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getNcpVersion();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getDriverVersion:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getDriverVersion();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSupportedChannels:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanChannelInfo[] _result = this.getSupportedChannels();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getSupportedNetworkTypes:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getSupportedNetworkTypes();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getMacAddress:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getMacAddress();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_isEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isUp:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isUp();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isCommissioned:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isCommissioned();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isConnected:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isConnected();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getState();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getRole:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getRole();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getPartitionId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getPartitionId();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getExtendedAddress:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getExtendedAddress();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_getLowpanIdentity:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanIdentity _result = this.getLowpanIdentity();
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
        case TRANSACTION_getLowpanCredential:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanCredential _result = this.getLowpanCredential();
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
        case TRANSACTION_getLinkAddresses:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getLinkAddresses();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getLinkNetworks:
        {
          data.enforceInterface(descriptor);
          android.net.IpPrefix[] _result = this.getLinkNetworks();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_join:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanProvision _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.lowpan.LowpanProvision.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.join(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_form:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanProvision _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.lowpan.LowpanProvision.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.form(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_attach:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanProvision _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.lowpan.LowpanProvision.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.attach(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_leave:
        {
          data.enforceInterface(descriptor);
          this.leave();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reset:
        {
          data.enforceInterface(descriptor);
          this.reset();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startCommissioningSession:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.LowpanBeaconInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.lowpan.LowpanBeaconInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startCommissioningSession(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeCommissioningSession:
        {
          data.enforceInterface(descriptor);
          this.closeCommissioningSession();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendToCommissioner:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.sendToCommissioner(_arg0);
          return true;
        }
        case TRANSACTION_beginLowPower:
        {
          data.enforceInterface(descriptor);
          this.beginLowPower();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_pollForData:
        {
          data.enforceInterface(descriptor);
          this.pollForData();
          return true;
        }
        case TRANSACTION_onHostWake:
        {
          data.enforceInterface(descriptor);
          this.onHostWake();
          return true;
        }
        case TRANSACTION_addListener:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterfaceListener _arg0;
          _arg0 = android.net.lowpan.ILowpanInterfaceListener.Stub.asInterface(data.readStrongBinder());
          this.addListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeListener:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterfaceListener _arg0;
          _arg0 = android.net.lowpan.ILowpanInterfaceListener.Stub.asInterface(data.readStrongBinder());
          this.removeListener(_arg0);
          return true;
        }
        case TRANSACTION_startNetScan:
        {
          data.enforceInterface(descriptor);
          java.util.Map _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readHashMap(cl);
          android.net.lowpan.ILowpanNetScanCallback _arg1;
          _arg1 = android.net.lowpan.ILowpanNetScanCallback.Stub.asInterface(data.readStrongBinder());
          this.startNetScan(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopNetScan:
        {
          data.enforceInterface(descriptor);
          this.stopNetScan();
          return true;
        }
        case TRANSACTION_startEnergyScan:
        {
          data.enforceInterface(descriptor);
          java.util.Map _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readHashMap(cl);
          android.net.lowpan.ILowpanEnergyScanCallback _arg1;
          _arg1 = android.net.lowpan.ILowpanEnergyScanCallback.Stub.asInterface(data.readStrongBinder());
          this.startEnergyScan(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopEnergyScan:
        {
          data.enforceInterface(descriptor);
          this.stopEnergyScan();
          return true;
        }
        case TRANSACTION_addOnMeshPrefix:
        {
          data.enforceInterface(descriptor);
          android.net.IpPrefix _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.IpPrefix.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.addOnMeshPrefix(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOnMeshPrefix:
        {
          data.enforceInterface(descriptor);
          android.net.IpPrefix _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.IpPrefix.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeOnMeshPrefix(_arg0);
          return true;
        }
        case TRANSACTION_addExternalRoute:
        {
          data.enforceInterface(descriptor);
          android.net.IpPrefix _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.IpPrefix.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.addExternalRoute(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeExternalRoute:
        {
          data.enforceInterface(descriptor);
          android.net.IpPrefix _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.IpPrefix.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeExternalRoute(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.lowpan.ILowpanInterface
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
      // Methods

      @Override public java.lang.String getName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getName();
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
      @Override public java.lang.String getNcpVersion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNcpVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNcpVersion();
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
      @Override public java.lang.String getDriverVersion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDriverVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDriverVersion();
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
      @Override public android.net.lowpan.LowpanChannelInfo[] getSupportedChannels() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.lowpan.LowpanChannelInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedChannels, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedChannels();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.lowpan.LowpanChannelInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getSupportedNetworkTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedNetworkTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedNetworkTypes();
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
      @Override public byte[] getMacAddress() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMacAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMacAddress();
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
      @Override public boolean isEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isEnabled();
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
      @Override public void setEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isUp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUp();
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
      @Override public boolean isCommissioned() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCommissioned, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isCommissioned();
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
      @Override public boolean isConnected() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isConnected, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isConnected();
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
      @Override public java.lang.String getState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getState();
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
      @Override public java.lang.String getRole() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRole, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRole();
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
      @Override public java.lang.String getPartitionId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPartitionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPartitionId();
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
      @Override public byte[] getExtendedAddress() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getExtendedAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getExtendedAddress();
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
      @Override public android.net.lowpan.LowpanIdentity getLowpanIdentity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.lowpan.LowpanIdentity _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLowpanIdentity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLowpanIdentity();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.lowpan.LowpanIdentity.CREATOR.createFromParcel(_reply);
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
      @Override public android.net.lowpan.LowpanCredential getLowpanCredential() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.lowpan.LowpanCredential _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLowpanCredential, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLowpanCredential();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.lowpan.LowpanCredential.CREATOR.createFromParcel(_reply);
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
      @Override public java.lang.String[] getLinkAddresses() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLinkAddresses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLinkAddresses();
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
      @Override public android.net.IpPrefix[] getLinkNetworks() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpPrefix[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLinkNetworks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLinkNetworks();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.net.IpPrefix.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void join(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provision!=null)) {
            _data.writeInt(1);
            provision.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_join, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().join(provision);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void form(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provision!=null)) {
            _data.writeInt(1);
            provision.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_form, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().form(provision);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void attach(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provision!=null)) {
            _data.writeInt(1);
            provision.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_attach, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attach(provision);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void leave() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_leave, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().leave();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reset() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reset();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startCommissioningSession(android.net.lowpan.LowpanBeaconInfo beaconInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((beaconInfo!=null)) {
            _data.writeInt(1);
            beaconInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startCommissioningSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startCommissioningSession(beaconInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeCommissioningSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeCommissioningSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeCommissioningSession();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendToCommissioner(byte[] packet) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(packet);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendToCommissioner, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendToCommissioner(packet);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void beginLowPower() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_beginLowPower, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().beginLowPower();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void pollForData() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pollForData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pollForData();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onHostWake() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHostWake, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHostWake();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeListener, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeListener(listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startNetScan(java.util.Map properties, android.net.lowpan.ILowpanNetScanCallback listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeMap(properties);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startNetScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startNetScan(properties, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopNetScan() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopNetScan, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopNetScan();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startEnergyScan(java.util.Map properties, android.net.lowpan.ILowpanEnergyScanCallback listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeMap(properties);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startEnergyScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startEnergyScan(properties, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopEnergyScan() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopEnergyScan, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopEnergyScan();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addOnMeshPrefix(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((prefix!=null)) {
            _data.writeInt(1);
            prefix.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnMeshPrefix, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnMeshPrefix(prefix, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOnMeshPrefix(android.net.IpPrefix prefix) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((prefix!=null)) {
            _data.writeInt(1);
            prefix.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOnMeshPrefix, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOnMeshPrefix(prefix);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addExternalRoute(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((prefix!=null)) {
            _data.writeInt(1);
            prefix.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addExternalRoute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addExternalRoute(prefix, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeExternalRoute(android.net.IpPrefix prefix) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((prefix!=null)) {
            _data.writeInt(1);
            prefix.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeExternalRoute, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeExternalRoute(prefix);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.lowpan.ILowpanInterface sDefaultImpl;
    }
    static final int TRANSACTION_getName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getNcpVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getDriverVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getSupportedChannels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getSupportedNetworkTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getMacAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_isUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isCommissioned = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getRole = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getPartitionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getExtendedAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getLowpanIdentity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getLowpanCredential = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getLinkAddresses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getLinkNetworks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_join = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_form = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_attach = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_leave = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_reset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_startCommissioningSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_closeCommissioningSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_sendToCommissioner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_beginLowPower = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_pollForData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_onHostWake = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_addListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_removeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_startNetScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_stopNetScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_startEnergyScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_stopEnergyScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_addOnMeshPrefix = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_removeOnMeshPrefix = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_addExternalRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_removeExternalRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    public static boolean setDefaultImpl(android.net.lowpan.ILowpanInterface impl) {
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
    public static android.net.lowpan.ILowpanInterface getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // These are here for the sake of C++ interface implementations.

  public static final String PERM_ACCESS_LOWPAN_STATE = "android.permission.ACCESS_LOWPAN_STATE";
  public static final String PERM_CHANGE_LOWPAN_STATE = "android.permission.CHANGE_LOWPAN_STATE";
  public static final String PERM_READ_LOWPAN_CREDENTIAL = "android.permission.READ_LOWPAN_CREDENTIAL";
  /**
       * Channel mask key.
       * Used for setting a channel mask when starting a scan.
       * Type: int[]
       * */
  public static final String KEY_CHANNEL_MASK = "android.net.lowpan.property.CHANNEL_MASK";
  /**
       * Max Transmit Power Key.
       * Used for setting the maximum transmit power when starting a network scan.
       * Type: Integer
       * */
  public static final String KEY_MAX_TX_POWER = "android.net.lowpan.property.MAX_TX_POWER";
  // Interface States

  public static final String STATE_OFFLINE = "offline";
  public static final String STATE_COMMISSIONING = "commissioning";
  public static final String STATE_ATTACHING = "attaching";
  public static final String STATE_ATTACHED = "attached";
  public static final String STATE_FAULT = "fault";
  // Device Roles

  public static final String ROLE_END_DEVICE = "end-device";
  public static final String ROLE_ROUTER = "router";
  public static final String ROLE_SLEEPY_END_DEVICE = "sleepy-end-device";
  public static final String ROLE_SLEEPY_ROUTER = "sleepy-router";
  public static final String ROLE_LEADER = "leader";
  public static final String ROLE_COORDINATOR = "coordinator";
  public static final String ROLE_DETACHED = "detached";
  public static final String NETWORK_TYPE_UNKNOWN = "unknown";
  /**
       * Network type for Thread 1.x networks.
       *
       * @see android.net.lowpan.LowpanIdentity#getType
       * @see #getLowpanIdentity
       */
  public static final String NETWORK_TYPE_THREAD_V1 = "org.threadgroup.thread.v1";
  // Service-Specific Error Code Constants

  public static final int ERROR_UNSPECIFIED = 1;
  public static final int ERROR_INVALID_ARGUMENT = 2;
  public static final int ERROR_DISABLED = 3;
  public static final int ERROR_WRONG_STATE = 4;
  public static final int ERROR_TIMEOUT = 5;
  public static final int ERROR_IO_FAILURE = 6;
  public static final int ERROR_NCP_PROBLEM = 7;
  public static final int ERROR_BUSY = 8;
  public static final int ERROR_ALREADY = 9;
  public static final int ERROR_CANCELED = 10;
  public static final int ERROR_FEATURE_NOT_SUPPORTED = 11;
  public static final int ERROR_JOIN_FAILED_UNKNOWN = 12;
  public static final int ERROR_JOIN_FAILED_AT_SCAN = 13;
  public static final int ERROR_JOIN_FAILED_AT_AUTH = 14;
  public static final int ERROR_FORM_FAILED_AT_SCAN = 15;
  // Methods

  public java.lang.String getName() throws android.os.RemoteException;
  public java.lang.String getNcpVersion() throws android.os.RemoteException;
  public java.lang.String getDriverVersion() throws android.os.RemoteException;
  public android.net.lowpan.LowpanChannelInfo[] getSupportedChannels() throws android.os.RemoteException;
  public java.lang.String[] getSupportedNetworkTypes() throws android.os.RemoteException;
  public byte[] getMacAddress() throws android.os.RemoteException;
  public boolean isEnabled() throws android.os.RemoteException;
  public void setEnabled(boolean enabled) throws android.os.RemoteException;
  public boolean isUp() throws android.os.RemoteException;
  public boolean isCommissioned() throws android.os.RemoteException;
  public boolean isConnected() throws android.os.RemoteException;
  public java.lang.String getState() throws android.os.RemoteException;
  public java.lang.String getRole() throws android.os.RemoteException;
  public java.lang.String getPartitionId() throws android.os.RemoteException;
  public byte[] getExtendedAddress() throws android.os.RemoteException;
  public android.net.lowpan.LowpanIdentity getLowpanIdentity() throws android.os.RemoteException;
  public android.net.lowpan.LowpanCredential getLowpanCredential() throws android.os.RemoteException;
  public java.lang.String[] getLinkAddresses() throws android.os.RemoteException;
  public android.net.IpPrefix[] getLinkNetworks() throws android.os.RemoteException;
  public void join(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException;
  public void form(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException;
  public void attach(android.net.lowpan.LowpanProvision provision) throws android.os.RemoteException;
  public void leave() throws android.os.RemoteException;
  public void reset() throws android.os.RemoteException;
  public void startCommissioningSession(android.net.lowpan.LowpanBeaconInfo beaconInfo) throws android.os.RemoteException;
  public void closeCommissioningSession() throws android.os.RemoteException;
  public void sendToCommissioner(byte[] packet) throws android.os.RemoteException;
  public void beginLowPower() throws android.os.RemoteException;
  public void pollForData() throws android.os.RemoteException;
  public void onHostWake() throws android.os.RemoteException;
  public void addListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException;
  public void removeListener(android.net.lowpan.ILowpanInterfaceListener listener) throws android.os.RemoteException;
  public void startNetScan(java.util.Map properties, android.net.lowpan.ILowpanNetScanCallback listener) throws android.os.RemoteException;
  public void stopNetScan() throws android.os.RemoteException;
  public void startEnergyScan(java.util.Map properties, android.net.lowpan.ILowpanEnergyScanCallback listener) throws android.os.RemoteException;
  public void stopEnergyScan() throws android.os.RemoteException;
  public void addOnMeshPrefix(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException;
  public void removeOnMeshPrefix(android.net.IpPrefix prefix) throws android.os.RemoteException;
  public void addExternalRoute(android.net.IpPrefix prefix, int flags) throws android.os.RemoteException;
  public void removeExternalRoute(android.net.IpPrefix prefix) throws android.os.RemoteException;
}
