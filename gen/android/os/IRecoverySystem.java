/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/** @hide */
public interface IRecoverySystem extends android.os.IInterface
{
  /** Default implementation for IRecoverySystem. */
  public static class Default implements android.os.IRecoverySystem
  {
    @Override public boolean uncrypt(java.lang.String packageFile, android.os.IRecoverySystemProgressListener listener) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setupBcb(java.lang.String command) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean clearBcb() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void rebootRecoveryWithCommand(java.lang.String command) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IRecoverySystem
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IRecoverySystem";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IRecoverySystem interface,
     * generating a proxy if needed.
     */
    public static android.os.IRecoverySystem asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IRecoverySystem))) {
        return ((android.os.IRecoverySystem)iin);
      }
      return new android.os.IRecoverySystem.Stub.Proxy(obj);
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
        case TRANSACTION_uncrypt:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IRecoverySystemProgressListener _arg1;
          _arg1 = android.os.IRecoverySystemProgressListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.uncrypt(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setupBcb:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.setupBcb(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_clearBcb:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.clearBcb();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_rebootRecoveryWithCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.rebootRecoveryWithCommand(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IRecoverySystem
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
      @Override public boolean uncrypt(java.lang.String packageFile, android.os.IRecoverySystemProgressListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageFile);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_uncrypt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().uncrypt(packageFile, listener);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean setupBcb(java.lang.String command) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(command);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupBcb, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setupBcb(command);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean clearBcb() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearBcb, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearBcb();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void rebootRecoveryWithCommand(java.lang.String command) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(command);
          boolean _status = mRemote.transact(Stub.TRANSACTION_rebootRecoveryWithCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rebootRecoveryWithCommand(command);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IRecoverySystem sDefaultImpl;
    }
    static final int TRANSACTION_uncrypt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setupBcb = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_clearBcb = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_rebootRecoveryWithCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.os.IRecoverySystem impl) {
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
    public static android.os.IRecoverySystem getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean uncrypt(java.lang.String packageFile, android.os.IRecoverySystemProgressListener listener) throws android.os.RemoteException;
  public boolean setupBcb(java.lang.String command) throws android.os.RemoteException;
  public boolean clearBcb() throws android.os.RemoteException;
  public void rebootRecoveryWithCommand(java.lang.String command) throws android.os.RemoteException;
}
