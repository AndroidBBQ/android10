/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.security.cts.activity;
public interface ISecureRandomService extends android.os.IInterface
{
  /** Default implementation for ISecureRandomService. */
  public static class Default implements android.security.cts.activity.ISecureRandomService
  {
    @Override public int getRandomBytesAndPid(byte[] randomBytes) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.security.cts.activity.ISecureRandomService
  {
    private static final java.lang.String DESCRIPTOR = "android.security.cts.activity.ISecureRandomService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.security.cts.activity.ISecureRandomService interface,
     * generating a proxy if needed.
     */
    public static android.security.cts.activity.ISecureRandomService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.security.cts.activity.ISecureRandomService))) {
        return ((android.security.cts.activity.ISecureRandomService)iin);
      }
      return new android.security.cts.activity.ISecureRandomService.Stub.Proxy(obj);
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
        case TRANSACTION_getRandomBytesAndPid:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _result = this.getRandomBytesAndPid(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          reply.writeByteArray(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.security.cts.activity.ISecureRandomService
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
      @Override public int getRandomBytesAndPid(byte[] randomBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(randomBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRandomBytesAndPid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRandomBytesAndPid(randomBytes);
          }
          _reply.readException();
          _result = _reply.readInt();
          _reply.readByteArray(randomBytes);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.security.cts.activity.ISecureRandomService sDefaultImpl;
    }
    static final int TRANSACTION_getRandomBytesAndPid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.security.cts.activity.ISecureRandomService impl) {
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
    public static android.security.cts.activity.ISecureRandomService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int getRandomBytesAndPid(byte[] randomBytes) throws android.os.RemoteException;
}
