/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * The service responsible for answering network recommendation requests.
 * @hide
 */
public interface INetworkRecommendationProvider extends android.os.IInterface
{
  /** Default implementation for INetworkRecommendationProvider. */
  public static class Default implements android.net.INetworkRecommendationProvider
  {
    /**
         * Request scoring for networks.
         *
         * Implementations should use {@link NetworkScoreManager#updateScores(ScoredNetwork[])} to
         * respond to score requests.
         *
         * @param networks an array of {@link NetworkKey}s to score
         * @hide
         */
    @Override public void requestScores(android.net.NetworkKey[] networks) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkRecommendationProvider
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkRecommendationProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkRecommendationProvider interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkRecommendationProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkRecommendationProvider))) {
        return ((android.net.INetworkRecommendationProvider)iin);
      }
      return new android.net.INetworkRecommendationProvider.Stub.Proxy(obj);
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
        case TRANSACTION_requestScores:
        {
          data.enforceInterface(descriptor);
          android.net.NetworkKey[] _arg0;
          _arg0 = data.createTypedArray(android.net.NetworkKey.CREATOR);
          this.requestScores(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkRecommendationProvider
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
           * Request scoring for networks.
           *
           * Implementations should use {@link NetworkScoreManager#updateScores(ScoredNetwork[])} to
           * respond to score requests.
           *
           * @param networks an array of {@link NetworkKey}s to score
           * @hide
           */
      @Override public void requestScores(android.net.NetworkKey[] networks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(networks, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestScores, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestScores(networks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkRecommendationProvider sDefaultImpl;
    }
    static final int TRANSACTION_requestScores = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.net.INetworkRecommendationProvider impl) {
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
    public static android.net.INetworkRecommendationProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Request scoring for networks.
       *
       * Implementations should use {@link NetworkScoreManager#updateScores(ScoredNetwork[])} to
       * respond to score requests.
       *
       * @param networks an array of {@link NetworkKey}s to score
       * @hide
       */
  public void requestScores(android.net.NetworkKey[] networks) throws android.os.RemoteException;
}
