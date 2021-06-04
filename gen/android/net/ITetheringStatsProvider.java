/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Interface for NetworkManagementService to query tethering statistics and set data limits.
 *
 * TODO: this does not really need to be an interface since Tethering runs in the same process
 * as NetworkManagementService. Consider refactoring Tethering to use direct access to
 * NetworkManagementService instead of using INetworkManagementService, and then deleting this
 * interface.
 *
 * @hide
 */
public interface ITetheringStatsProvider extends android.os.IInterface
{
  /** Default implementation for ITetheringStatsProvider. */
  public static class Default implements android.net.ITetheringStatsProvider
  {
    // Returns cumulative statistics for all tethering sessions since boot, on all upstreams.
    // @code {how} is one of the NetworkStats.STATS_PER_* constants. If {@code how} is
    // {@code STATS_PER_IFACE}, the provider should not include any traffic that is already
    // counted by kernel interface counters.

    @Override public android.net.NetworkStats getTetherStats(int how) throws android.os.RemoteException
    {
      return null;
    }
    // Sets the interface quota for the specified upstream interface. This is defined as the number
    // of bytes, starting from zero and counting from now, after which data should stop being
    // forwarded to/from the specified upstream. A value of QUOTA_UNLIMITED means there is no limit.

    @Override public void setInterfaceQuota(java.lang.String iface, long quotaBytes) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ITetheringStatsProvider
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ITetheringStatsProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ITetheringStatsProvider interface,
     * generating a proxy if needed.
     */
    public static android.net.ITetheringStatsProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ITetheringStatsProvider))) {
        return ((android.net.ITetheringStatsProvider)iin);
      }
      return new android.net.ITetheringStatsProvider.Stub.Proxy(obj);
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
        case TRANSACTION_getTetherStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.NetworkStats _result = this.getTetherStats(_arg0);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ITetheringStatsProvider
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
      // Returns cumulative statistics for all tethering sessions since boot, on all upstreams.
      // @code {how} is one of the NetworkStats.STATS_PER_* constants. If {@code how} is
      // {@code STATS_PER_IFACE}, the provider should not include any traffic that is already
      // counted by kernel interface counters.

      @Override public android.net.NetworkStats getTetherStats(int how) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(how);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherStats(how);
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
      // Sets the interface quota for the specified upstream interface. This is defined as the number
      // of bytes, starting from zero and counting from now, after which data should stop being
      // forwarded to/from the specified upstream. A value of QUOTA_UNLIMITED means there is no limit.

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
      public static android.net.ITetheringStatsProvider sDefaultImpl;
    }
    static final int TRANSACTION_getTetherStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setInterfaceQuota = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.ITetheringStatsProvider impl) {
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
    public static android.net.ITetheringStatsProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Indicates that no data usage limit is set.

  public static final int QUOTA_UNLIMITED = -1;
  // Returns cumulative statistics for all tethering sessions since boot, on all upstreams.
  // @code {how} is one of the NetworkStats.STATS_PER_* constants. If {@code how} is
  // {@code STATS_PER_IFACE}, the provider should not include any traffic that is already
  // counted by kernel interface counters.

  public android.net.NetworkStats getTetherStats(int how) throws android.os.RemoteException;
  // Sets the interface quota for the specified upstream interface. This is defined as the number
  // of bytes, starting from zero and counting from now, after which data should stop being
  // forwarded to/from the specified upstream. A value of QUOTA_UNLIMITED means there is no limit.

  public void setInterfaceQuota(java.lang.String iface, long quotaBytes) throws android.os.RemoteException;
}
