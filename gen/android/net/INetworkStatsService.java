/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** {@hide} */
public interface INetworkStatsService extends android.os.IInterface
{
  /** Default implementation for INetworkStatsService. */
  public static class Default implements android.net.INetworkStatsService
  {
    /** Start a statistics query session. */
    @Override public android.net.INetworkStatsSession openSession() throws android.os.RemoteException
    {
      return null;
    }
    /** Start a statistics query session. If calling package is profile or device owner then it is
         *  granted automatic access if apiLevel is NetworkStatsManager.API_LEVEL_DPC_ALLOWED. If
         *  apiLevel is at least NetworkStatsManager.API_LEVEL_REQUIRES_PACKAGE_USAGE_STATS then
         *  PACKAGE_USAGE_STATS permission is always checked. If PACKAGE_USAGE_STATS is not granted
         *  READ_NETWORK_USAGE_STATS is checked for.
         */
    @Override public android.net.INetworkStatsSession openSessionForUsageStats(int flags, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /** Return data layer snapshot of UID network usage. */
    @Override public android.net.NetworkStats getDataLayerSnapshotForUid(int uid) throws android.os.RemoteException
    {
      return null;
    }
    /** Get a detailed snapshot of stats since boot for all UIDs.
        *
        * <p>Results will not always be limited to stats on requiredIfaces when specified: stats for
        * interfaces stacked on the specified interfaces, or for interfaces on which the specified
        * interfaces are stacked on, will also be included.
        * @param requiredIfaces Interface names to get data for, or {@link NetworkStats#INTERFACES_ALL}.
        */
    @Override public android.net.NetworkStats getDetailedUidStats(java.lang.String[] requiredIfaces) throws android.os.RemoteException
    {
      return null;
    }
    /** Return set of any ifaces associated with mobile networks since boot. */
    @Override public java.lang.String[] getMobileIfaces() throws android.os.RemoteException
    {
      return null;
    }
    /** Increment data layer count of operations performed for UID and tag. */
    @Override public void incrementOperationCount(int uid, int tag, int operationCount) throws android.os.RemoteException
    {
    }
    /** Force update of ifaces. */
    @Override public void forceUpdateIfaces(android.net.Network[] defaultNetworks, android.net.NetworkState[] networkStates, java.lang.String activeIface) throws android.os.RemoteException
    {
    }
    /** Force update of statistics. */
    @Override public void forceUpdate() throws android.os.RemoteException
    {
    }
    /** Registers a callback on data usage. */
    @Override public android.net.DataUsageRequest registerUsageCallback(java.lang.String callingPackage, android.net.DataUsageRequest request, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
    {
      return null;
    }
    /** Unregisters a callback on data usage. */
    @Override public void unregisterUsageRequest(android.net.DataUsageRequest request) throws android.os.RemoteException
    {
    }
    /** Get the uid stats information since boot */
    @Override public long getUidStats(int uid, int type) throws android.os.RemoteException
    {
      return 0L;
    }
    /** Get the iface stats information since boot */
    @Override public long getIfaceStats(java.lang.String iface, int type) throws android.os.RemoteException
    {
      return 0L;
    }
    /** Get the total network stats information since boot */
    @Override public long getTotalStats(int type) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkStatsService
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkStatsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkStatsService interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkStatsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkStatsService))) {
        return ((android.net.INetworkStatsService)iin);
      }
      return new android.net.INetworkStatsService.Stub.Proxy(obj);
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
        case TRANSACTION_openSession:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkStatsSession _result = this.openSession();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_openSessionForUsageStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.INetworkStatsSession _result = this.openSessionForUsageStats(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getDataLayerSnapshotForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NetworkStats _result = this.getDataLayerSnapshotForUid(_arg0);
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
        case TRANSACTION_getDetailedUidStats:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          android.net.NetworkStats _result = this.getDetailedUidStats(_arg0);
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
        case TRANSACTION_getMobileIfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getMobileIfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_incrementOperationCount:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.incrementOperationCount(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forceUpdateIfaces:
        {
          data.enforceInterface(descriptor);
          android.net.Network[] _arg0;
          _arg0 = data.createTypedArray(android.net.Network.CREATOR);
          android.net.NetworkState[] _arg1;
          _arg1 = data.createTypedArray(android.net.NetworkState.CREATOR);
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.forceUpdateIfaces(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forceUpdate:
        {
          data.enforceInterface(descriptor);
          this.forceUpdate();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerUsageCallback:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.DataUsageRequest _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.DataUsageRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Messenger _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          android.net.DataUsageRequest _result = this.registerUsageCallback(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_unregisterUsageRequest:
        {
          data.enforceInterface(descriptor);
          android.net.DataUsageRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.DataUsageRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.unregisterUsageRequest(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUidStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long _result = this.getUidStats(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getIfaceStats:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _result = this.getIfaceStats(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getTotalStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _result = this.getTotalStats(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkStatsService
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
      /** Start a statistics query session. */
      @Override public android.net.INetworkStatsSession openSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.INetworkStatsSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openSession();
          }
          _reply.readException();
          _result = android.net.INetworkStatsSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Start a statistics query session. If calling package is profile or device owner then it is
           *  granted automatic access if apiLevel is NetworkStatsManager.API_LEVEL_DPC_ALLOWED. If
           *  apiLevel is at least NetworkStatsManager.API_LEVEL_REQUIRES_PACKAGE_USAGE_STATS then
           *  PACKAGE_USAGE_STATS permission is always checked. If PACKAGE_USAGE_STATS is not granted
           *  READ_NETWORK_USAGE_STATS is checked for.
           */
      @Override public android.net.INetworkStatsSession openSessionForUsageStats(int flags, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.INetworkStatsSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openSessionForUsageStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openSessionForUsageStats(flags, callingPackage);
          }
          _reply.readException();
          _result = android.net.INetworkStatsSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Return data layer snapshot of UID network usage. */
      @Override public android.net.NetworkStats getDataLayerSnapshotForUid(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataLayerSnapshotForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataLayerSnapshotForUid(uid);
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
      /** Get a detailed snapshot of stats since boot for all UIDs.
          *
          * <p>Results will not always be limited to stats on requiredIfaces when specified: stats for
          * interfaces stacked on the specified interfaces, or for interfaces on which the specified
          * interfaces are stacked on, will also be included.
          * @param requiredIfaces Interface names to get data for, or {@link NetworkStats#INTERFACES_ALL}.
          */
      @Override public android.net.NetworkStats getDetailedUidStats(java.lang.String[] requiredIfaces) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(requiredIfaces);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDetailedUidStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDetailedUidStats(requiredIfaces);
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
      /** Return set of any ifaces associated with mobile networks since boot. */
      @Override public java.lang.String[] getMobileIfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMobileIfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMobileIfaces();
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
      /** Increment data layer count of operations performed for UID and tag. */
      @Override public void incrementOperationCount(int uid, int tag, int operationCount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(tag);
          _data.writeInt(operationCount);
          boolean _status = mRemote.transact(Stub.TRANSACTION_incrementOperationCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().incrementOperationCount(uid, tag, operationCount);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Force update of ifaces. */
      @Override public void forceUpdateIfaces(android.net.Network[] defaultNetworks, android.net.NetworkState[] networkStates, java.lang.String activeIface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(defaultNetworks, 0);
          _data.writeTypedArray(networkStates, 0);
          _data.writeString(activeIface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceUpdateIfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceUpdateIfaces(defaultNetworks, networkStates, activeIface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Force update of statistics. */
      @Override public void forceUpdate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceUpdate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceUpdate();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Registers a callback on data usage. */
      @Override public android.net.DataUsageRequest registerUsageCallback(java.lang.String callingPackage, android.net.DataUsageRequest request, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.DataUsageRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUsageCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerUsageCallback(callingPackage, request, messenger, binder);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.DataUsageRequest.CREATOR.createFromParcel(_reply);
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
      /** Unregisters a callback on data usage. */
      @Override public void unregisterUsageRequest(android.net.DataUsageRequest request) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUsageRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUsageRequest(request);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Get the uid stats information since boot */
      @Override public long getUidStats(int uid, int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUidStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUidStats(uid, type);
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
      /** Get the iface stats information since boot */
      @Override public long getIfaceStats(java.lang.String iface, int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIfaceStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIfaceStats(iface, type);
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
      /** Get the total network stats information since boot */
      @Override public long getTotalStats(int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTotalStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTotalStats(type);
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
      public static android.net.INetworkStatsService sDefaultImpl;
    }
    static final int TRANSACTION_openSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_openSessionForUsageStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getDataLayerSnapshotForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getDetailedUidStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getMobileIfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_incrementOperationCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_forceUpdateIfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_forceUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_registerUsageCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_unregisterUsageRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getUidStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getIfaceStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getTotalStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.net.INetworkStatsService impl) {
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
    public static android.net.INetworkStatsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Start a statistics query session. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/INetworkStatsService.aidl:34:1:34:25")
  public android.net.INetworkStatsSession openSession() throws android.os.RemoteException;
  /** Start a statistics query session. If calling package is profile or device owner then it is
       *  granted automatic access if apiLevel is NetworkStatsManager.API_LEVEL_DPC_ALLOWED. If
       *  apiLevel is at least NetworkStatsManager.API_LEVEL_REQUIRES_PACKAGE_USAGE_STATS then
       *  PACKAGE_USAGE_STATS permission is always checked. If PACKAGE_USAGE_STATS is not granted
       *  READ_NETWORK_USAGE_STATS is checked for.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/INetworkStatsService.aidl:43:1:43:25")
  public android.net.INetworkStatsSession openSessionForUsageStats(int flags, java.lang.String callingPackage) throws android.os.RemoteException;
  /** Return data layer snapshot of UID network usage. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/INetworkStatsService.aidl:47:1:47:25")
  public android.net.NetworkStats getDataLayerSnapshotForUid(int uid) throws android.os.RemoteException;
  /** Get a detailed snapshot of stats since boot for all UIDs.
      *
      * <p>Results will not always be limited to stats on requiredIfaces when specified: stats for
      * interfaces stacked on the specified interfaces, or for interfaces on which the specified
      * interfaces are stacked on, will also be included.
      * @param requiredIfaces Interface names to get data for, or {@link NetworkStats#INTERFACES_ALL}.
      */
  public android.net.NetworkStats getDetailedUidStats(java.lang.String[] requiredIfaces) throws android.os.RemoteException;
  /** Return set of any ifaces associated with mobile networks since boot. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/INetworkStatsService.aidl:60:1:60:25")
  public java.lang.String[] getMobileIfaces() throws android.os.RemoteException;
  /** Increment data layer count of operations performed for UID and tag. */
  public void incrementOperationCount(int uid, int tag, int operationCount) throws android.os.RemoteException;
  /** Force update of ifaces. */
  public void forceUpdateIfaces(android.net.Network[] defaultNetworks, android.net.NetworkState[] networkStates, java.lang.String activeIface) throws android.os.RemoteException;
  /** Force update of statistics. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/net/INetworkStatsService.aidl:72:1:72:25")
  public void forceUpdate() throws android.os.RemoteException;
  /** Registers a callback on data usage. */
  public android.net.DataUsageRequest registerUsageCallback(java.lang.String callingPackage, android.net.DataUsageRequest request, android.os.Messenger messenger, android.os.IBinder binder) throws android.os.RemoteException;
  /** Unregisters a callback on data usage. */
  public void unregisterUsageRequest(android.net.DataUsageRequest request) throws android.os.RemoteException;
  /** Get the uid stats information since boot */
  public long getUidStats(int uid, int type) throws android.os.RemoteException;
  /** Get the iface stats information since boot */
  public long getIfaceStats(java.lang.String iface, int type) throws android.os.RemoteException;
  /** Get the total network stats information since boot */
  public long getTotalStats(int type) throws android.os.RemoteException;
}
