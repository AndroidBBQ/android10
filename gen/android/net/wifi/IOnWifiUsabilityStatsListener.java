/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for Wi-Fi usability stats listener.
 *
 * @hide
 */
public interface IOnWifiUsabilityStatsListener extends android.os.IInterface
{
  /** Default implementation for IOnWifiUsabilityStatsListener. */
  public static class Default implements android.net.wifi.IOnWifiUsabilityStatsListener
  {
    /**
         * Service to manager callback providing current Wi-Fi usability stats.
         *
         * @param seqNum The sequence number of stats, used to derive the timing of updated Wi-Fi
         *               usability statistics, set by framework and shall be incremented by one
         *               after each update.
         * @param isSameBssidAndFreq The flag to indicate whether the BSSID and the frequency of
         *                           network stays the same or not relative to the last update of
         *                           Wi-Fi usability stats.
         * @param stats The updated Wi-Fi usability statistics.
         */
    @Override public void onWifiUsabilityStats(int seqNum, boolean isSameBssidAndFreq, android.net.wifi.WifiUsabilityStatsEntry stats) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.IOnWifiUsabilityStatsListener
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.IOnWifiUsabilityStatsListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.IOnWifiUsabilityStatsListener interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.IOnWifiUsabilityStatsListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.IOnWifiUsabilityStatsListener))) {
        return ((android.net.wifi.IOnWifiUsabilityStatsListener)iin);
      }
      return new android.net.wifi.IOnWifiUsabilityStatsListener.Stub.Proxy(obj);
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
        case TRANSACTION_onWifiUsabilityStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.net.wifi.WifiUsabilityStatsEntry _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.wifi.WifiUsabilityStatsEntry.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onWifiUsabilityStats(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.IOnWifiUsabilityStatsListener
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
           * Service to manager callback providing current Wi-Fi usability stats.
           *
           * @param seqNum The sequence number of stats, used to derive the timing of updated Wi-Fi
           *               usability statistics, set by framework and shall be incremented by one
           *               after each update.
           * @param isSameBssidAndFreq The flag to indicate whether the BSSID and the frequency of
           *                           network stays the same or not relative to the last update of
           *                           Wi-Fi usability stats.
           * @param stats The updated Wi-Fi usability statistics.
           */
      @Override public void onWifiUsabilityStats(int seqNum, boolean isSameBssidAndFreq, android.net.wifi.WifiUsabilityStatsEntry stats) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seqNum);
          _data.writeInt(((isSameBssidAndFreq)?(1):(0)));
          if ((stats!=null)) {
            _data.writeInt(1);
            stats.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWifiUsabilityStats, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWifiUsabilityStats(seqNum, isSameBssidAndFreq, stats);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.IOnWifiUsabilityStatsListener sDefaultImpl;
    }
    static final int TRANSACTION_onWifiUsabilityStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.net.wifi.IOnWifiUsabilityStatsListener impl) {
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
    public static android.net.wifi.IOnWifiUsabilityStatsListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Service to manager callback providing current Wi-Fi usability stats.
       *
       * @param seqNum The sequence number of stats, used to derive the timing of updated Wi-Fi
       *               usability statistics, set by framework and shall be incremented by one
       *               after each update.
       * @param isSameBssidAndFreq The flag to indicate whether the BSSID and the frequency of
       *                           network stays the same or not relative to the last update of
       *                           Wi-Fi usability stats.
       * @param stats The updated Wi-Fi usability statistics.
       */
  public void onWifiUsabilityStats(int seqNum, boolean isSameBssidAndFreq, android.net.wifi.WifiUsabilityStatsEntry stats) throws android.os.RemoteException;
}
