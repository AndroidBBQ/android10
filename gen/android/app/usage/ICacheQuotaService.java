/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.usage;
/** {@hide} */
public interface ICacheQuotaService extends android.os.IInterface
{
  /** Default implementation for ICacheQuotaService. */
  public static class Default implements android.app.usage.ICacheQuotaService
  {
    @Override public void computeCacheQuotaHints(android.os.RemoteCallback callback, java.util.List<android.app.usage.CacheQuotaHint> requests) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.usage.ICacheQuotaService
  {
    private static final java.lang.String DESCRIPTOR = "android.app.usage.ICacheQuotaService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.usage.ICacheQuotaService interface,
     * generating a proxy if needed.
     */
    public static android.app.usage.ICacheQuotaService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.usage.ICacheQuotaService))) {
        return ((android.app.usage.ICacheQuotaService)iin);
      }
      return new android.app.usage.ICacheQuotaService.Stub.Proxy(obj);
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
        case TRANSACTION_computeCacheQuotaHints:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.util.List<android.app.usage.CacheQuotaHint> _arg1;
          _arg1 = data.createTypedArrayList(android.app.usage.CacheQuotaHint.CREATOR);
          this.computeCacheQuotaHints(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.usage.ICacheQuotaService
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
      @Override public void computeCacheQuotaHints(android.os.RemoteCallback callback, java.util.List<android.app.usage.CacheQuotaHint> requests) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedList(requests);
          boolean _status = mRemote.transact(Stub.TRANSACTION_computeCacheQuotaHints, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().computeCacheQuotaHints(callback, requests);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.usage.ICacheQuotaService sDefaultImpl;
    }
    static final int TRANSACTION_computeCacheQuotaHints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.usage.ICacheQuotaService impl) {
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
    public static android.app.usage.ICacheQuotaService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void computeCacheQuotaHints(android.os.RemoteCallback callback, java.util.List<android.app.usage.CacheQuotaHint> requests) throws android.os.RemoteException;
}
