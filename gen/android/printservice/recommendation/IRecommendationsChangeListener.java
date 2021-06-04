/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.printservice.recommendation;
/**
 * Interface for observing changes of the print service recommendations.
 *
 * @hide
 */
public interface IRecommendationsChangeListener extends android.os.IInterface
{
  /** Default implementation for IRecommendationsChangeListener. */
  public static class Default implements android.printservice.recommendation.IRecommendationsChangeListener
  {
    @Override public void onRecommendationsChanged() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.printservice.recommendation.IRecommendationsChangeListener
  {
    private static final java.lang.String DESCRIPTOR = "android.printservice.recommendation.IRecommendationsChangeListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.printservice.recommendation.IRecommendationsChangeListener interface,
     * generating a proxy if needed.
     */
    public static android.printservice.recommendation.IRecommendationsChangeListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.printservice.recommendation.IRecommendationsChangeListener))) {
        return ((android.printservice.recommendation.IRecommendationsChangeListener)iin);
      }
      return new android.printservice.recommendation.IRecommendationsChangeListener.Stub.Proxy(obj);
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
        case TRANSACTION_onRecommendationsChanged:
        {
          data.enforceInterface(descriptor);
          this.onRecommendationsChanged();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.printservice.recommendation.IRecommendationsChangeListener
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
      @Override public void onRecommendationsChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRecommendationsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRecommendationsChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.printservice.recommendation.IRecommendationsChangeListener sDefaultImpl;
    }
    static final int TRANSACTION_onRecommendationsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.printservice.recommendation.IRecommendationsChangeListener impl) {
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
    public static android.printservice.recommendation.IRecommendationsChangeListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onRecommendationsChanged() throws android.os.RemoteException;
}
