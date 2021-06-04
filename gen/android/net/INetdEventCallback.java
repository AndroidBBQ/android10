/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** {@hide} */
public interface INetdEventCallback extends android.os.IInterface
{
  /** Default implementation for INetdEventCallback. */
  public static class Default implements android.net.INetdEventCallback
  {
    /**
         * Reports a single DNS lookup function call.
         * This method must not block or perform long-running operations.
         *
         * @param netId the ID of the network the lookup was performed on.
         * @param eventType one of the EVENT_* constants in {@link INetdEventListener}.
         * @param returnCode the return value of the query, may vary based on {@code eventType}. See
         *        {@code getaddrinfo()}, {@code gethostbyaddr()} and {@code gethostbyname()} section in
         *        bionic/libc/include/netdb.h.
         * @param hostname the name that was looked up.
         * @param ipAddresses (possibly a subset of) the IP addresses returned.
         *        At most {@link #DNS_REPORTED_IP_ADDRESSES_LIMIT} addresses are logged.
         * @param ipAddressesCount the number of IP addresses returned. May be different from the length
         *        of ipAddresses if there were too many addresses to log.
         * @param timestamp the timestamp at which the query was reported by netd.
         * @param uid the UID of the application that performed the query.
         */
    @Override public void onDnsEvent(int netId, int eventType, int returnCode, java.lang.String hostname, java.lang.String[] ipAddresses, int ipAddressesCount, long timestamp, int uid) throws android.os.RemoteException
    {
    }
    /**
         * Represents adding or removing a NAT64 prefix.
         * This method must not block or perform long-running operations.
         *
         * @param netId the ID of the network the prefix was performed on.
         * @param added true if the NAT64 prefix was added, or false if the NAT64 prefix was removed.
         *        There is only one prefix at a time for each netId. If a prefix is added, it replaces
         *        the previous-added prefix.
         * @param prefixString the detected NAT64 prefix as a string literal.
         * @param prefixLength the prefix length associated with this NAT64 prefix.
         */
    @Override public void onNat64PrefixEvent(int netId, boolean added, java.lang.String prefixString, int prefixLength) throws android.os.RemoteException
    {
    }
    /**
         * Represents a private DNS validation success or failure.
         * This method must not block or perform long-running operations.
         *
         * @param netId the ID of the network the validation was performed on.
         * @param ipAddress the IP address for which validation was performed.
         * @param hostname the hostname for which validation was performed.
         * @param validated whether or not validation was successful.
         */
    @Override public void onPrivateDnsValidationEvent(int netId, java.lang.String ipAddress, java.lang.String hostname, boolean validated) throws android.os.RemoteException
    {
    }
    /**
         * Reports a single connect library call.
         * This method must not block or perform long-running operations.
         *
         * @param ipAddr destination IP address.
         * @param port destination port number.
         * @param timestamp the timestamp at which the call was reported by netd.
         * @param uid the UID of the application that performed the connection.
         */
    @Override public void onConnectEvent(java.lang.String ipAddr, int port, long timestamp, int uid) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetdEventCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetdEventCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetdEventCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.INetdEventCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetdEventCallback))) {
        return ((android.net.INetdEventCallback)iin);
      }
      return new android.net.INetdEventCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onDnsEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String[] _arg4;
          _arg4 = data.createStringArray();
          int _arg5;
          _arg5 = data.readInt();
          long _arg6;
          _arg6 = data.readLong();
          int _arg7;
          _arg7 = data.readInt();
          this.onDnsEvent(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_onNat64PrefixEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.onNat64PrefixEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onPrivateDnsValidationEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.onPrivateDnsValidationEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onConnectEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          int _arg3;
          _arg3 = data.readInt();
          this.onConnectEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetdEventCallback
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
           * Reports a single DNS lookup function call.
           * This method must not block or perform long-running operations.
           *
           * @param netId the ID of the network the lookup was performed on.
           * @param eventType one of the EVENT_* constants in {@link INetdEventListener}.
           * @param returnCode the return value of the query, may vary based on {@code eventType}. See
           *        {@code getaddrinfo()}, {@code gethostbyaddr()} and {@code gethostbyname()} section in
           *        bionic/libc/include/netdb.h.
           * @param hostname the name that was looked up.
           * @param ipAddresses (possibly a subset of) the IP addresses returned.
           *        At most {@link #DNS_REPORTED_IP_ADDRESSES_LIMIT} addresses are logged.
           * @param ipAddressesCount the number of IP addresses returned. May be different from the length
           *        of ipAddresses if there were too many addresses to log.
           * @param timestamp the timestamp at which the query was reported by netd.
           * @param uid the UID of the application that performed the query.
           */
      @Override public void onDnsEvent(int netId, int eventType, int returnCode, java.lang.String hostname, java.lang.String[] ipAddresses, int ipAddressesCount, long timestamp, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeInt(eventType);
          _data.writeInt(returnCode);
          _data.writeString(hostname);
          _data.writeStringArray(ipAddresses);
          _data.writeInt(ipAddressesCount);
          _data.writeLong(timestamp);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDnsEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDnsEvent(netId, eventType, returnCode, hostname, ipAddresses, ipAddressesCount, timestamp, uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Represents adding or removing a NAT64 prefix.
           * This method must not block or perform long-running operations.
           *
           * @param netId the ID of the network the prefix was performed on.
           * @param added true if the NAT64 prefix was added, or false if the NAT64 prefix was removed.
           *        There is only one prefix at a time for each netId. If a prefix is added, it replaces
           *        the previous-added prefix.
           * @param prefixString the detected NAT64 prefix as a string literal.
           * @param prefixLength the prefix length associated with this NAT64 prefix.
           */
      @Override public void onNat64PrefixEvent(int netId, boolean added, java.lang.String prefixString, int prefixLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeInt(((added)?(1):(0)));
          _data.writeString(prefixString);
          _data.writeInt(prefixLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNat64PrefixEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNat64PrefixEvent(netId, added, prefixString, prefixLength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Represents a private DNS validation success or failure.
           * This method must not block or perform long-running operations.
           *
           * @param netId the ID of the network the validation was performed on.
           * @param ipAddress the IP address for which validation was performed.
           * @param hostname the hostname for which validation was performed.
           * @param validated whether or not validation was successful.
           */
      @Override public void onPrivateDnsValidationEvent(int netId, java.lang.String ipAddress, java.lang.String hostname, boolean validated) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          _data.writeString(ipAddress);
          _data.writeString(hostname);
          _data.writeInt(((validated)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrivateDnsValidationEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrivateDnsValidationEvent(netId, ipAddress, hostname, validated);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Reports a single connect library call.
           * This method must not block or perform long-running operations.
           *
           * @param ipAddr destination IP address.
           * @param port destination port number.
           * @param timestamp the timestamp at which the call was reported by netd.
           * @param uid the UID of the application that performed the connection.
           */
      @Override public void onConnectEvent(java.lang.String ipAddr, int port, long timestamp, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(ipAddr);
          _data.writeInt(port);
          _data.writeLong(timestamp);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectEvent(ipAddr, port, timestamp, uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetdEventCallback sDefaultImpl;
    }
    static final int TRANSACTION_onDnsEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onNat64PrefixEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onPrivateDnsValidationEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onConnectEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.net.INetdEventCallback impl) {
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
    public static android.net.INetdEventCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Possible addNetdEventCallback callers.

  public static final int CALLBACK_CALLER_CONNECTIVITY_SERVICE = 0;
  public static final int CALLBACK_CALLER_DEVICE_POLICY = 1;
  public static final int CALLBACK_CALLER_NETWORK_WATCHLIST = 2;
  /**
       * Reports a single DNS lookup function call.
       * This method must not block or perform long-running operations.
       *
       * @param netId the ID of the network the lookup was performed on.
       * @param eventType one of the EVENT_* constants in {@link INetdEventListener}.
       * @param returnCode the return value of the query, may vary based on {@code eventType}. See
       *        {@code getaddrinfo()}, {@code gethostbyaddr()} and {@code gethostbyname()} section in
       *        bionic/libc/include/netdb.h.
       * @param hostname the name that was looked up.
       * @param ipAddresses (possibly a subset of) the IP addresses returned.
       *        At most {@link #DNS_REPORTED_IP_ADDRESSES_LIMIT} addresses are logged.
       * @param ipAddressesCount the number of IP addresses returned. May be different from the length
       *        of ipAddresses if there were too many addresses to log.
       * @param timestamp the timestamp at which the query was reported by netd.
       * @param uid the UID of the application that performed the query.
       */
  public void onDnsEvent(int netId, int eventType, int returnCode, java.lang.String hostname, java.lang.String[] ipAddresses, int ipAddressesCount, long timestamp, int uid) throws android.os.RemoteException;
  /**
       * Represents adding or removing a NAT64 prefix.
       * This method must not block or perform long-running operations.
       *
       * @param netId the ID of the network the prefix was performed on.
       * @param added true if the NAT64 prefix was added, or false if the NAT64 prefix was removed.
       *        There is only one prefix at a time for each netId. If a prefix is added, it replaces
       *        the previous-added prefix.
       * @param prefixString the detected NAT64 prefix as a string literal.
       * @param prefixLength the prefix length associated with this NAT64 prefix.
       */
  public void onNat64PrefixEvent(int netId, boolean added, java.lang.String prefixString, int prefixLength) throws android.os.RemoteException;
  /**
       * Represents a private DNS validation success or failure.
       * This method must not block or perform long-running operations.
       *
       * @param netId the ID of the network the validation was performed on.
       * @param ipAddress the IP address for which validation was performed.
       * @param hostname the hostname for which validation was performed.
       * @param validated whether or not validation was successful.
       */
  public void onPrivateDnsValidationEvent(int netId, java.lang.String ipAddress, java.lang.String hostname, boolean validated) throws android.os.RemoteException;
  /**
       * Reports a single connect library call.
       * This method must not block or perform long-running operations.
       *
       * @param ipAddr destination IP address.
       * @param port destination port number.
       * @param timestamp the timestamp at which the call was reported by netd.
       * @param uid the UID of the application that performed the connection.
       */
  public void onConnectEvent(java.lang.String ipAddr, int port, long timestamp, int uid) throws android.os.RemoteException;
}
