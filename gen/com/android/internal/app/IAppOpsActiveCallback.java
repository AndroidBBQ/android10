/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
// Iterface to observe op active changes

public interface IAppOpsActiveCallback extends android.os.IInterface
{
  /** Default implementation for IAppOpsActiveCallback. */
  public static class Default implements com.android.internal.app.IAppOpsActiveCallback
  {
    @Override public void opActiveChanged(int op, int uid, java.lang.String packageName, boolean active) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IAppOpsActiveCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IAppOpsActiveCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IAppOpsActiveCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IAppOpsActiveCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IAppOpsActiveCallback))) {
        return ((com.android.internal.app.IAppOpsActiveCallback)iin);
      }
      return new com.android.internal.app.IAppOpsActiveCallback.Stub.Proxy(obj);
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
        case TRANSACTION_opActiveChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.opActiveChanged(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IAppOpsActiveCallback
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
      @Override public void opActiveChanged(int op, int uid, java.lang.String packageName, boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(op);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_opActiveChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().opActiveChanged(op, uid, packageName, active);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.app.IAppOpsActiveCallback sDefaultImpl;
    }
    static final int TRANSACTION_opActiveChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.app.IAppOpsActiveCallback impl) {
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
    public static com.android.internal.app.IAppOpsActiveCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void opActiveChanged(int op, int uid, java.lang.String packageName, boolean active) throws android.os.RemoteException;
}
