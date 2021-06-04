/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.attention;
/**
 * Interface for a concrete implementation to provide to the AttentionManagerService.
 *
 * @hide
 */
public interface IAttentionService extends android.os.IInterface
{
  /** Default implementation for IAttentionService. */
  public static class Default implements android.service.attention.IAttentionService
  {
    @Override public void checkAttention(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void cancelAttentionCheck(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.attention.IAttentionService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.attention.IAttentionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.attention.IAttentionService interface,
     * generating a proxy if needed.
     */
    public static android.service.attention.IAttentionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.attention.IAttentionService))) {
        return ((android.service.attention.IAttentionService)iin);
      }
      return new android.service.attention.IAttentionService.Stub.Proxy(obj);
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
        case TRANSACTION_checkAttention:
        {
          data.enforceInterface(descriptor);
          android.service.attention.IAttentionCallback _arg0;
          _arg0 = android.service.attention.IAttentionCallback.Stub.asInterface(data.readStrongBinder());
          this.checkAttention(_arg0);
          return true;
        }
        case TRANSACTION_cancelAttentionCheck:
        {
          data.enforceInterface(descriptor);
          android.service.attention.IAttentionCallback _arg0;
          _arg0 = android.service.attention.IAttentionCallback.Stub.asInterface(data.readStrongBinder());
          this.cancelAttentionCheck(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.attention.IAttentionService
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
      @Override public void checkAttention(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkAttention, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().checkAttention(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelAttentionCheck(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAttentionCheck, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAttentionCheck(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.attention.IAttentionService sDefaultImpl;
    }
    static final int TRANSACTION_checkAttention = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_cancelAttentionCheck = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.service.attention.IAttentionService impl) {
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
    public static android.service.attention.IAttentionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void checkAttention(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException;
  public void cancelAttentionCheck(android.service.attention.IAttentionCallback callback) throws android.os.RemoteException;
}
