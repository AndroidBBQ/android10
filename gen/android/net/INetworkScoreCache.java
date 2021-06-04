/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * A service which stores a subset of scored networks from the active network scorer.
 *
 * <p>To be implemented by network subsystems (e.g. Wi-Fi). NetworkScoreService will propagate
 * scores down to each subsystem depending on the network type. Implementations may register for
 * a given network type by calling NetworkScoreManager.registerNetworkSubsystem.
 *
 * <p>A proper implementation should throw SecurityException whenever the caller is not privileged.
 * It may request scores by calling NetworkScoreManager#requestScores(NetworkKey[]); a call to
 * updateScores may follow but may not depending on the active scorer's implementation, and in
 * general this method may be called at any time.
 *
 * <p>Implementations should also override dump() so that "adb shell dumpsys network_score" includes
 * the current scores for each network for debugging purposes.
 * @hide
 */
public interface INetworkScoreCache extends android.os.IInterface
{
  /** Default implementation for INetworkScoreCache. */
  public static class Default implements android.net.INetworkScoreCache
  {
    @Override public void updateScores(java.util.List<android.net.ScoredNetwork> networks) throws android.os.RemoteException
    {
    }
    @Override public void clearScores() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkScoreCache
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkScoreCache";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkScoreCache interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkScoreCache asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkScoreCache))) {
        return ((android.net.INetworkScoreCache)iin);
      }
      return new android.net.INetworkScoreCache.Stub.Proxy(obj);
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
        case TRANSACTION_updateScores:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.ScoredNetwork> _arg0;
          _arg0 = data.createTypedArrayList(android.net.ScoredNetwork.CREATOR);
          this.updateScores(_arg0);
          return true;
        }
        case TRANSACTION_clearScores:
        {
          data.enforceInterface(descriptor);
          this.clearScores();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkScoreCache
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
      @Override public void updateScores(java.util.List<android.net.ScoredNetwork> networks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(networks);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateScores, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateScores(networks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clearScores() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearScores, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearScores();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkScoreCache sDefaultImpl;
    }
    static final int TRANSACTION_updateScores = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_clearScores = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.INetworkScoreCache impl) {
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
    public static android.net.INetworkScoreCache getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void updateScores(java.util.List<android.net.ScoredNetwork> networks) throws android.os.RemoteException;
  public void clearScores() throws android.os.RemoteException;
}
