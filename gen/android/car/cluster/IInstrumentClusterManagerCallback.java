/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.cluster;
/**
 * Interface from Car Service to {@link android.car.cluster.CarInstrumentClusterManager}
 *
 * @deprecated CarInstrumentClusterManager is deprecated
 * @hide
 */
public interface IInstrumentClusterManagerCallback extends android.os.IInterface
{
  /** Default implementation for IInstrumentClusterManagerCallback. */
  public static class Default implements android.car.cluster.IInstrumentClusterManagerCallback
  {
    /**
         * Notifies manager about changes in the cluster activity state.
         *
         * @param category cluster activity category to which this state applies,
         *        see {@link android.car.cluster.CarInstrumentClusterManager} for details.
         * @param clusterActivityState is a {@link Bundle} object,
         *        see {@link android.car.cluster.ClusterActivityState} for how to construct the bundle.
         */
    @Override public void setClusterActivityState(java.lang.String category, android.os.Bundle clusterActivityState) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.cluster.IInstrumentClusterManagerCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.cluster.IInstrumentClusterManagerCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.cluster.IInstrumentClusterManagerCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.cluster.IInstrumentClusterManagerCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.cluster.IInstrumentClusterManagerCallback))) {
        return ((android.car.cluster.IInstrumentClusterManagerCallback)iin);
      }
      return new android.car.cluster.IInstrumentClusterManagerCallback.Stub.Proxy(obj);
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
        case TRANSACTION_setClusterActivityState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setClusterActivityState(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.cluster.IInstrumentClusterManagerCallback
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
           * Notifies manager about changes in the cluster activity state.
           *
           * @param category cluster activity category to which this state applies,
           *        see {@link android.car.cluster.CarInstrumentClusterManager} for details.
           * @param clusterActivityState is a {@link Bundle} object,
           *        see {@link android.car.cluster.ClusterActivityState} for how to construct the bundle.
           */
      @Override public void setClusterActivityState(java.lang.String category, android.os.Bundle clusterActivityState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(category);
          if ((clusterActivityState!=null)) {
            _data.writeInt(1);
            clusterActivityState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setClusterActivityState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setClusterActivityState(category, clusterActivityState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.cluster.IInstrumentClusterManagerCallback sDefaultImpl;
    }
    static final int TRANSACTION_setClusterActivityState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.cluster.IInstrumentClusterManagerCallback impl) {
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
    public static android.car.cluster.IInstrumentClusterManagerCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies manager about changes in the cluster activity state.
       *
       * @param category cluster activity category to which this state applies,
       *        see {@link android.car.cluster.CarInstrumentClusterManager} for details.
       * @param clusterActivityState is a {@link Bundle} object,
       *        see {@link android.car.cluster.ClusterActivityState} for how to construct the bundle.
       */
  public void setClusterActivityState(java.lang.String category, android.os.Bundle clusterActivityState) throws android.os.RemoteException;
}
