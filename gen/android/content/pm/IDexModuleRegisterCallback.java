/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * Callback for registering a dex module with the Package Manager.
 *
 * @hide
 */
public interface IDexModuleRegisterCallback extends android.os.IInterface
{
  /** Default implementation for IDexModuleRegisterCallback. */
  public static class Default implements android.content.pm.IDexModuleRegisterCallback
  {
    @Override public void onDexModuleRegistered(java.lang.String dexModulePath, boolean success, java.lang.String message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IDexModuleRegisterCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IDexModuleRegisterCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IDexModuleRegisterCallback interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IDexModuleRegisterCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IDexModuleRegisterCallback))) {
        return ((android.content.pm.IDexModuleRegisterCallback)iin);
      }
      return new android.content.pm.IDexModuleRegisterCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onDexModuleRegistered:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.onDexModuleRegistered(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.IDexModuleRegisterCallback
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
      @Override public void onDexModuleRegistered(java.lang.String dexModulePath, boolean success, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(dexModulePath);
          _data.writeInt(((success)?(1):(0)));
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDexModuleRegistered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDexModuleRegistered(dexModulePath, success, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.pm.IDexModuleRegisterCallback sDefaultImpl;
    }
    static final int TRANSACTION_onDexModuleRegistered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.content.pm.IDexModuleRegisterCallback impl) {
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
    public static android.content.pm.IDexModuleRegisterCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDexModuleRegistered(java.lang.String dexModulePath, boolean success, java.lang.String message) throws android.os.RemoteException;
}
