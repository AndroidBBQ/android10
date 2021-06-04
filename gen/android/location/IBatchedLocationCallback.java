/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * {@hide}
 */
public interface IBatchedLocationCallback extends android.os.IInterface
{
  /** Default implementation for IBatchedLocationCallback. */
  public static class Default implements android.location.IBatchedLocationCallback
  {
    @Override public void onLocationBatch(java.util.List<android.location.Location> locations) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.IBatchedLocationCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.location.IBatchedLocationCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.IBatchedLocationCallback interface,
     * generating a proxy if needed.
     */
    public static android.location.IBatchedLocationCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.IBatchedLocationCallback))) {
        return ((android.location.IBatchedLocationCallback)iin);
      }
      return new android.location.IBatchedLocationCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onLocationBatch:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.location.Location> _arg0;
          _arg0 = data.createTypedArrayList(android.location.Location.CREATOR);
          this.onLocationBatch(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.IBatchedLocationCallback
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
      @Override public void onLocationBatch(java.util.List<android.location.Location> locations) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(locations);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLocationBatch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLocationBatch(locations);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.location.IBatchedLocationCallback sDefaultImpl;
    }
    static final int TRANSACTION_onLocationBatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.location.IBatchedLocationCallback impl) {
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
    public static android.location.IBatchedLocationCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onLocationBatch(java.util.List<android.location.Location> locations) throws android.os.RemoteException;
}
