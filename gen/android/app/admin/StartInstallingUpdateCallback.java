/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.admin;
/**
* Callback used between {@link DevicePolicyManager} and {@link DevicePolicyManagerService} to
* indicate that starting installing an update is finished.
* {@hide}
*/
public interface StartInstallingUpdateCallback extends android.os.IInterface
{
  /** Default implementation for StartInstallingUpdateCallback. */
  public static class Default implements android.app.admin.StartInstallingUpdateCallback
  {
    @Override public void onStartInstallingUpdateError(int errorCode, java.lang.String errorMessage) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.admin.StartInstallingUpdateCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.app.admin.StartInstallingUpdateCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.admin.StartInstallingUpdateCallback interface,
     * generating a proxy if needed.
     */
    public static android.app.admin.StartInstallingUpdateCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.admin.StartInstallingUpdateCallback))) {
        return ((android.app.admin.StartInstallingUpdateCallback)iin);
      }
      return new android.app.admin.StartInstallingUpdateCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onStartInstallingUpdateError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onStartInstallingUpdateError(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.admin.StartInstallingUpdateCallback
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
      @Override public void onStartInstallingUpdateError(int errorCode, java.lang.String errorMessage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errorCode);
          _data.writeString(errorMessage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartInstallingUpdateError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartInstallingUpdateError(errorCode, errorMessage);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.admin.StartInstallingUpdateCallback sDefaultImpl;
    }
    static final int TRANSACTION_onStartInstallingUpdateError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.admin.StartInstallingUpdateCallback impl) {
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
    public static android.app.admin.StartInstallingUpdateCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onStartInstallingUpdateError(int errorCode, java.lang.String errorMessage) throws android.os.RemoteException;
}
