/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.util;
public interface IRemoteMemoryIntArray extends android.os.IInterface
{
  /** Default implementation for IRemoteMemoryIntArray. */
  public static class Default implements android.util.IRemoteMemoryIntArray
  {
    @Override public android.util.MemoryIntArray peekInstance() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void create(int size) throws android.os.RemoteException
    {
    }
    @Override public boolean isWritable() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int get(int index) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void set(int index, int value) throws android.os.RemoteException
    {
    }
    @Override public int size() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void close() throws android.os.RemoteException
    {
    }
    @Override public boolean isClosed() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void accessLastElementInRemoteProcess(android.util.MemoryIntArray array) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.util.IRemoteMemoryIntArray
  {
    private static final java.lang.String DESCRIPTOR = "android.util.IRemoteMemoryIntArray";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.util.IRemoteMemoryIntArray interface,
     * generating a proxy if needed.
     */
    public static android.util.IRemoteMemoryIntArray asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.util.IRemoteMemoryIntArray))) {
        return ((android.util.IRemoteMemoryIntArray)iin);
      }
      return new android.util.IRemoteMemoryIntArray.Stub.Proxy(obj);
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
        case TRANSACTION_peekInstance:
        {
          data.enforceInterface(descriptor);
          android.util.MemoryIntArray _result = this.peekInstance();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_create:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.create(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isWritable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isWritable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_get:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.get(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_set:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.set(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_size:
        {
          data.enforceInterface(descriptor);
          int _result = this.size();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isClosed:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isClosed();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_accessLastElementInRemoteProcess:
        {
          data.enforceInterface(descriptor);
          android.util.MemoryIntArray _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.util.MemoryIntArray.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.accessLastElementInRemoteProcess(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.util.IRemoteMemoryIntArray
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
      @Override public android.util.MemoryIntArray peekInstance() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.util.MemoryIntArray _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_peekInstance, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().peekInstance();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.util.MemoryIntArray.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void create(int size) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(size);
          boolean _status = mRemote.transact(Stub.TRANSACTION_create, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().create(size);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isWritable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isWritable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isWritable();
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
      @Override public int get(int index) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(index);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get(index);
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
      @Override public void set(int index, int value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(index);
          _data.writeInt(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_set, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().set(index, value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int size() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_size, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().size();
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
      @Override public void close() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isClosed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isClosed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isClosed();
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
      @Override public void accessLastElementInRemoteProcess(android.util.MemoryIntArray array) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((array!=null)) {
            _data.writeInt(1);
            array.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_accessLastElementInRemoteProcess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().accessLastElementInRemoteProcess(array);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.util.IRemoteMemoryIntArray sDefaultImpl;
    }
    static final int TRANSACTION_peekInstance = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_create = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isWritable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_get = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_set = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_size = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isClosed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_accessLastElementInRemoteProcess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.util.IRemoteMemoryIntArray impl) {
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
    public static android.util.IRemoteMemoryIntArray getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.util.MemoryIntArray peekInstance() throws android.os.RemoteException;
  public void create(int size) throws android.os.RemoteException;
  public boolean isWritable() throws android.os.RemoteException;
  public int get(int index) throws android.os.RemoteException;
  public void set(int index, int value) throws android.os.RemoteException;
  public int size() throws android.os.RemoteException;
  public void close() throws android.os.RemoteException;
  public boolean isClosed() throws android.os.RemoteException;
  public void accessLastElementInRemoteProcess(android.util.MemoryIntArray array) throws android.os.RemoteException;
}
