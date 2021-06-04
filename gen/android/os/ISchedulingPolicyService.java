/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * Initially only provides requestPriority() below, but in longer term
 * other scheduling policy related services will be collected here.
 *
 * @hide
 */
public interface ISchedulingPolicyService extends android.os.IInterface
{
  /** Default implementation for ISchedulingPolicyService. */
  public static class Default implements android.os.ISchedulingPolicyService
  {
    /**
         * Move thread tid into appropriate cgroup and assign it priority prio.
         * The thread group leader of tid must be pid.
         * There may be restrictions on who can call this.
         */
    @Override public int requestPriority(int pid, int tid, int prio, boolean isForApp) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Move media.codec process between SP_FOREGROUND and SP_TOP_APP.
         * When 'enable' is 'true', server will attempt to move media.codec process
         * from SP_FOREGROUND into SP_TOP_APP cpuset. A valid 'client' must be
         * provided for the server to receive death notifications. When 'enable'
         * is 'false', server will attempt to move media.codec process back to
         * the original cpuset, and 'client' is ignored in this case.
         */
    @Override public int requestCpusetBoost(boolean enable, android.os.IBinder client) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.ISchedulingPolicyService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.ISchedulingPolicyService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.ISchedulingPolicyService interface,
     * generating a proxy if needed.
     */
    public static android.os.ISchedulingPolicyService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.ISchedulingPolicyService))) {
        return ((android.os.ISchedulingPolicyService)iin);
      }
      return new android.os.ISchedulingPolicyService.Stub.Proxy(obj);
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
        case TRANSACTION_requestPriority:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          int _result = this.requestPriority(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestCpusetBoost:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _result = this.requestCpusetBoost(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.ISchedulingPolicyService
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
           * Move thread tid into appropriate cgroup and assign it priority prio.
           * The thread group leader of tid must be pid.
           * There may be restrictions on who can call this.
           */
      @Override public int requestPriority(int pid, int tid, int prio, boolean isForApp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pid);
          _data.writeInt(tid);
          _data.writeInt(prio);
          _data.writeInt(((isForApp)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestPriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestPriority(pid, tid, prio, isForApp);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Move media.codec process between SP_FOREGROUND and SP_TOP_APP.
           * When 'enable' is 'true', server will attempt to move media.codec process
           * from SP_FOREGROUND into SP_TOP_APP cpuset. A valid 'client' must be
           * provided for the server to receive death notifications. When 'enable'
           * is 'false', server will attempt to move media.codec process back to
           * the original cpuset, and 'client' is ignored in this case.
           */
      @Override public int requestCpusetBoost(boolean enable, android.os.IBinder client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeStrongBinder(client);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestCpusetBoost, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestCpusetBoost(enable, client);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.ISchedulingPolicyService sDefaultImpl;
    }
    static final int TRANSACTION_requestPriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_requestCpusetBoost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.ISchedulingPolicyService impl) {
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
    public static android.os.ISchedulingPolicyService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Move thread tid into appropriate cgroup and assign it priority prio.
       * The thread group leader of tid must be pid.
       * There may be restrictions on who can call this.
       */
  public int requestPriority(int pid, int tid, int prio, boolean isForApp) throws android.os.RemoteException;
  /**
       * Move media.codec process between SP_FOREGROUND and SP_TOP_APP.
       * When 'enable' is 'true', server will attempt to move media.codec process
       * from SP_FOREGROUND into SP_TOP_APP cpuset. A valid 'client' must be
       * provided for the server to receive death notifications. When 'enable'
       * is 'false', server will attempt to move media.codec process back to
       * the original cpuset, and 'client' is ignored in this case.
       */
  public int requestCpusetBoost(boolean enable, android.os.IBinder client) throws android.os.RemoteException;
}
