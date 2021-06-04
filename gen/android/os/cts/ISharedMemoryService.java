/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.cts;
public interface ISharedMemoryService extends android.os.IInterface
{
  /** Default implementation for ISharedMemoryService. */
  public static class Default implements android.os.cts.ISharedMemoryService
  {
    @Override public void setup(android.os.SharedMemory memory, int prot) throws android.os.RemoteException
    {
    }
    @Override public byte read(int index) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void write(int index, byte value) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.cts.ISharedMemoryService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.cts.ISharedMemoryService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.cts.ISharedMemoryService interface,
     * generating a proxy if needed.
     */
    public static android.os.cts.ISharedMemoryService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.cts.ISharedMemoryService))) {
        return ((android.os.cts.ISharedMemoryService)iin);
      }
      return new android.os.cts.ISharedMemoryService.Stub.Proxy(obj);
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
        case TRANSACTION_setup:
        {
          data.enforceInterface(descriptor);
          android.os.SharedMemory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.SharedMemory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.setup(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_read:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte _result = this.read(_arg0);
          reply.writeNoException();
          reply.writeByte(_result);
          return true;
        }
        case TRANSACTION_write:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte _arg1;
          _arg1 = data.readByte();
          this.write(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.cts.ISharedMemoryService
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
      @Override public void setup(android.os.SharedMemory memory, int prot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((memory!=null)) {
            _data.writeInt(1);
            memory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(prot);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setup(memory, prot);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public byte read(int index) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(index);
          boolean _status = mRemote.transact(Stub.TRANSACTION_read, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().read(index);
          }
          _reply.readException();
          _result = _reply.readByte();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void write(int index, byte value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(index);
          _data.writeByte(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().write(index, value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.cts.ISharedMemoryService sDefaultImpl;
    }
    static final int TRANSACTION_setup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_read = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.os.cts.ISharedMemoryService impl) {
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
    public static android.os.cts.ISharedMemoryService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setup(android.os.SharedMemory memory, int prot) throws android.os.RemoteException;
  public byte read(int index) throws android.os.RemoteException;
  public void write(int index, byte value) throws android.os.RemoteException;
}
