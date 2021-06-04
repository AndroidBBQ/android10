/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.security;
/**
 * Used by the {@code KeyChainActivity} to return alias for {@link KeyStore#chooseAlias}.
 *
 * @hide
 */
public interface IKeyChainAliasCallback extends android.os.IInterface
{
  /** Default implementation for IKeyChainAliasCallback. */
  public static class Default implements android.security.IKeyChainAliasCallback
  {
    @Override public void alias(java.lang.String alias) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.security.IKeyChainAliasCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.security.IKeyChainAliasCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.security.IKeyChainAliasCallback interface,
     * generating a proxy if needed.
     */
    public static android.security.IKeyChainAliasCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.security.IKeyChainAliasCallback))) {
        return ((android.security.IKeyChainAliasCallback)iin);
      }
      return new android.security.IKeyChainAliasCallback.Stub.Proxy(obj);
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
        case TRANSACTION_alias:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.alias(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.security.IKeyChainAliasCallback
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
      @Override public void alias(java.lang.String alias) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(alias);
          boolean _status = mRemote.transact(Stub.TRANSACTION_alias, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().alias(alias);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.security.IKeyChainAliasCallback sDefaultImpl;
    }
    static final int TRANSACTION_alias = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.security.IKeyChainAliasCallback impl) {
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
    public static android.security.IKeyChainAliasCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void alias(java.lang.String alias) throws android.os.RemoteException;
}
