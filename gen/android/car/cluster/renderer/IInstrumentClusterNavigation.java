/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.cluster.renderer;
/**
 * Binder API for Instrument Cluster Navigation. This represents a direct communication channel
 * from navigation applications to the cluster vendor implementation.
 *
 * @hide
 */
public interface IInstrumentClusterNavigation extends android.os.IInterface
{
  /** Default implementation for IInstrumentClusterNavigation. */
  public static class Default implements android.car.cluster.renderer.IInstrumentClusterNavigation
  {
    /**
         * Called when there is a change on the navigation state.
         *
         * @param bundle {@link android.os.Bundle} containing the description of the navigation state
         *               change. This information can be parsed using
         *               <a href="https://developer.android.com/reference/androidx/car/cluster/navigation/NavigationState.html#toParcelable()">
         *               androidx.car.cluster.navigation.NavigationState#fromParcelable(Parcelable)</a>
         */
    @Override public void onNavigationStateChanged(android.os.Bundle bundle) throws android.os.RemoteException
    {
    }
    /**
         * Returns attributes of instrument cluster for navigation.
         */
    @Override public android.car.navigation.CarNavigationInstrumentCluster getInstrumentClusterInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.cluster.renderer.IInstrumentClusterNavigation
  {
    private static final java.lang.String DESCRIPTOR = "android.car.cluster.renderer.IInstrumentClusterNavigation";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.cluster.renderer.IInstrumentClusterNavigation interface,
     * generating a proxy if needed.
     */
    public static android.car.cluster.renderer.IInstrumentClusterNavigation asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.cluster.renderer.IInstrumentClusterNavigation))) {
        return ((android.car.cluster.renderer.IInstrumentClusterNavigation)iin);
      }
      return new android.car.cluster.renderer.IInstrumentClusterNavigation.Stub.Proxy(obj);
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
        case TRANSACTION_onNavigationStateChanged:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onNavigationStateChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInstrumentClusterInfo:
        {
          data.enforceInterface(descriptor);
          android.car.navigation.CarNavigationInstrumentCluster _result = this.getInstrumentClusterInfo();
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.cluster.renderer.IInstrumentClusterNavigation
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
           * Called when there is a change on the navigation state.
           *
           * @param bundle {@link android.os.Bundle} containing the description of the navigation state
           *               change. This information can be parsed using
           *               <a href="https://developer.android.com/reference/androidx/car/cluster/navigation/NavigationState.html#toParcelable()">
           *               androidx.car.cluster.navigation.NavigationState#fromParcelable(Parcelable)</a>
           */
      @Override public void onNavigationStateChanged(android.os.Bundle bundle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((bundle!=null)) {
            _data.writeInt(1);
            bundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNavigationStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNavigationStateChanged(bundle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Returns attributes of instrument cluster for navigation.
           */
      @Override public android.car.navigation.CarNavigationInstrumentCluster getInstrumentClusterInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.navigation.CarNavigationInstrumentCluster _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInstrumentClusterInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInstrumentClusterInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.navigation.CarNavigationInstrumentCluster.CREATOR.createFromParcel(_reply);
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
      public static android.car.cluster.renderer.IInstrumentClusterNavigation sDefaultImpl;
    }
    static final int TRANSACTION_onNavigationStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getInstrumentClusterInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.cluster.renderer.IInstrumentClusterNavigation impl) {
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
    public static android.car.cluster.renderer.IInstrumentClusterNavigation getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when there is a change on the navigation state.
       *
       * @param bundle {@link android.os.Bundle} containing the description of the navigation state
       *               change. This information can be parsed using
       *               <a href="https://developer.android.com/reference/androidx/car/cluster/navigation/NavigationState.html#toParcelable()">
       *               androidx.car.cluster.navigation.NavigationState#fromParcelable(Parcelable)</a>
       */
  public void onNavigationStateChanged(android.os.Bundle bundle) throws android.os.RemoteException;
  /**
       * Returns attributes of instrument cluster for navigation.
       */
  public android.car.navigation.CarNavigationInstrumentCluster getInstrumentClusterInfo() throws android.os.RemoteException;
}
