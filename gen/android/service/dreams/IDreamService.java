/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.dreams;
/**
 * @hide
 */
public interface IDreamService extends android.os.IInterface
{
  /** Default implementation for IDreamService. */
  public static class Default implements android.service.dreams.IDreamService
  {
    @Override public void attach(android.os.IBinder windowToken, boolean canDoze, android.os.IRemoteCallback started) throws android.os.RemoteException
    {
    }
    @Override public void detach() throws android.os.RemoteException
    {
    }
    @Override public void wakeUp() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.dreams.IDreamService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.dreams.IDreamService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.dreams.IDreamService interface,
     * generating a proxy if needed.
     */
    public static android.service.dreams.IDreamService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.dreams.IDreamService))) {
        return ((android.service.dreams.IDreamService)iin);
      }
      return new android.service.dreams.IDreamService.Stub.Proxy(obj);
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
        case TRANSACTION_attach:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.os.IRemoteCallback _arg2;
          _arg2 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          this.attach(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_detach:
        {
          data.enforceInterface(descriptor);
          this.detach();
          return true;
        }
        case TRANSACTION_wakeUp:
        {
          data.enforceInterface(descriptor);
          this.wakeUp();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.dreams.IDreamService
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
      @Override public void attach(android.os.IBinder windowToken, boolean canDoze, android.os.IRemoteCallback started) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(windowToken);
          _data.writeInt(((canDoze)?(1):(0)));
          _data.writeStrongBinder((((started!=null))?(started.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_attach, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attach(windowToken, canDoze, started);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void detach() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_detach, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().detach();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void wakeUp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_wakeUp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().wakeUp();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.dreams.IDreamService sDefaultImpl;
    }
    static final int TRANSACTION_attach = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_detach = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_wakeUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.service.dreams.IDreamService impl) {
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
    public static android.service.dreams.IDreamService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void attach(android.os.IBinder windowToken, boolean canDoze, android.os.IRemoteCallback started) throws android.os.RemoteException;
  public void detach() throws android.os.RemoteException;
  public void wakeUp() throws android.os.RemoteException;
}
