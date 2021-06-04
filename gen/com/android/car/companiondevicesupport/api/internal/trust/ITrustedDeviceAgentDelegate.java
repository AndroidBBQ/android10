/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.trust;
/** Delegate for TrustAgent operations. */
public interface ITrustedDeviceAgentDelegate extends android.os.IInterface
{
  /** Default implementation for ITrustedDeviceAgentDelegate. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate
  {
    /** Add escrow token for user. */
    @Override public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException
    {
    }
    /** Unlock user with token and handle. */
    @Override public void unlockUserWithToken(byte[] token, long handle, int userId) throws android.os.RemoteException
    {
    }
    /** Remove the escrow token associated with handle for user. */
    @Override public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate))) {
        return ((com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate.Stub.Proxy(obj);
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
        case TRANSACTION_addEscrowToken:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          this.addEscrowToken(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unlockUserWithToken:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.unlockUserWithToken(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_removeEscrowToken:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.removeEscrowToken(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate
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
      /** Add escrow token for user. */
      @Override public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addEscrowToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addEscrowToken(token, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Unlock user with token and handle. */
      @Override public void unlockUserWithToken(byte[] token, long handle, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          _data.writeLong(handle);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unlockUserWithToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unlockUserWithToken(token, handle, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Remove the escrow token associated with handle for user. */
      @Override public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeEscrowToken, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeEscrowToken(handle, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate sDefaultImpl;
    }
    static final int TRANSACTION_addEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unlockUserWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Add escrow token for user. */
  public void addEscrowToken(byte[] token, int userId) throws android.os.RemoteException;
  /** Unlock user with token and handle. */
  public void unlockUserWithToken(byte[] token, long handle, int userId) throws android.os.RemoteException;
  /** Remove the escrow token associated with handle for user. */
  public void removeEscrowToken(long handle, int userId) throws android.os.RemoteException;
}
