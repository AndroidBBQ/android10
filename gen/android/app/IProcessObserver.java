/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** {@hide} */
public interface IProcessObserver extends android.os.IInterface
{
  /** Default implementation for IProcessObserver. */
  public static class Default implements android.app.IProcessObserver
  {
    @Override public void onForegroundActivitiesChanged(int pid, int uid, boolean foregroundActivities) throws android.os.RemoteException
    {
    }
    @Override public void onForegroundServicesChanged(int pid, int uid, int serviceTypes) throws android.os.RemoteException
    {
    }
    @Override public void onProcessDied(int pid, int uid) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IProcessObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IProcessObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IProcessObserver interface,
     * generating a proxy if needed.
     */
    public static android.app.IProcessObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IProcessObserver))) {
        return ((android.app.IProcessObserver)iin);
      }
      return new android.app.IProcessObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onForegroundActivitiesChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onForegroundActivitiesChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onForegroundServicesChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onForegroundServicesChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onProcessDied:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onProcessDied(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IProcessObserver
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
      @Override public void onForegroundActivitiesChanged(int pid, int uid, boolean foregroundActivities) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeInt(((foregroundActivities)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onForegroundActivitiesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onForegroundActivitiesChanged(pid, uid, foregroundActivities);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onForegroundServicesChanged(int pid, int uid, int serviceTypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeInt(serviceTypes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onForegroundServicesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onForegroundServicesChanged(pid, uid, serviceTypes);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onProcessDied(int pid, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pid);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProcessDied, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProcessDied(pid, uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IProcessObserver sDefaultImpl;
    }
    static final int TRANSACTION_onForegroundActivitiesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onForegroundServicesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onProcessDied = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.app.IProcessObserver impl) {
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
    public static android.app.IProcessObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onForegroundActivitiesChanged(int pid, int uid, boolean foregroundActivities) throws android.os.RemoteException;
  public void onForegroundServicesChanged(int pid, int uid, int serviceTypes) throws android.os.RemoteException;
  public void onProcessDied(int pid, int uid) throws android.os.RemoteException;
}
