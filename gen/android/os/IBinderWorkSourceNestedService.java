/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
public interface IBinderWorkSourceNestedService extends android.os.IInterface
{
  /** Default implementation for IBinderWorkSourceNestedService. */
  public static class Default implements android.os.IBinderWorkSourceNestedService
  {
    @Override public int[] nestedCallWithWorkSourceToSet(int uidToBlame) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int[] nestedCall() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IBinderWorkSourceNestedService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IBinderWorkSourceNestedService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IBinderWorkSourceNestedService interface,
     * generating a proxy if needed.
     */
    public static android.os.IBinderWorkSourceNestedService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IBinderWorkSourceNestedService))) {
        return ((android.os.IBinderWorkSourceNestedService)iin);
      }
      return new android.os.IBinderWorkSourceNestedService.Stub.Proxy(obj);
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
        case TRANSACTION_nestedCallWithWorkSourceToSet:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _result = this.nestedCallWithWorkSourceToSet(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_nestedCall:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.nestedCall();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IBinderWorkSourceNestedService
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
      @Override public int[] nestedCallWithWorkSourceToSet(int uidToBlame) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uidToBlame);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nestedCallWithWorkSourceToSet, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nestedCallWithWorkSourceToSet(uidToBlame);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int[] nestedCall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nestedCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nestedCall();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.IBinderWorkSourceNestedService sDefaultImpl;
    }
    static final int TRANSACTION_nestedCallWithWorkSourceToSet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_nestedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.IBinderWorkSourceNestedService impl) {
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
    public static android.os.IBinderWorkSourceNestedService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int[] nestedCallWithWorkSourceToSet(int uidToBlame) throws android.os.RemoteException;
  public int[] nestedCall() throws android.os.RemoteException;
}
