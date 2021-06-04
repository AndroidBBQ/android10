/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.resolver;
/**
 * @hide
 */
public interface IResolverRankerResult extends android.os.IInterface
{
  /** Default implementation for IResolverRankerResult. */
  public static class Default implements android.service.resolver.IResolverRankerResult
  {
    @Override public void sendResult(java.util.List<android.service.resolver.ResolverTarget> results) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.resolver.IResolverRankerResult
  {
    private static final java.lang.String DESCRIPTOR = "android.service.resolver.IResolverRankerResult";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.resolver.IResolverRankerResult interface,
     * generating a proxy if needed.
     */
    public static android.service.resolver.IResolverRankerResult asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.resolver.IResolverRankerResult))) {
        return ((android.service.resolver.IResolverRankerResult)iin);
      }
      return new android.service.resolver.IResolverRankerResult.Stub.Proxy(obj);
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
        case TRANSACTION_sendResult:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.service.resolver.ResolverTarget> _arg0;
          _arg0 = data.createTypedArrayList(android.service.resolver.ResolverTarget.CREATOR);
          this.sendResult(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.resolver.IResolverRankerResult
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
      @Override public void sendResult(java.util.List<android.service.resolver.ResolverTarget> results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(results);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendResult(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.resolver.IResolverRankerResult sDefaultImpl;
    }
    static final int TRANSACTION_sendResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.resolver.IResolverRankerResult impl) {
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
    public static android.service.resolver.IResolverRankerResult getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void sendResult(java.util.List<android.service.resolver.ResolverTarget> results) throws android.os.RemoteException;
}
