/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
// Iterface to observe op note/checks of ops

public interface IAppOpsNotedCallback extends android.os.IInterface
{
  /** Default implementation for IAppOpsNotedCallback. */
  public static class Default implements com.android.internal.app.IAppOpsNotedCallback
  {
    @Override public void opNoted(int op, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IAppOpsNotedCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IAppOpsNotedCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IAppOpsNotedCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IAppOpsNotedCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IAppOpsNotedCallback))) {
        return ((com.android.internal.app.IAppOpsNotedCallback)iin);
      }
      return new com.android.internal.app.IAppOpsNotedCallback.Stub.Proxy(obj);
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
        case TRANSACTION_opNoted:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.opNoted(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IAppOpsNotedCallback
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
      @Override public void opNoted(int op, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(op);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_opNoted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().opNoted(op, uid, packageName, mode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.app.IAppOpsNotedCallback sDefaultImpl;
    }
    static final int TRANSACTION_opNoted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.app.IAppOpsNotedCallback impl) {
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
    public static com.android.internal.app.IAppOpsNotedCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void opNoted(int op, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException;
}
