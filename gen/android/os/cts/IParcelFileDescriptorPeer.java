/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.cts;
public interface IParcelFileDescriptorPeer extends android.os.IInterface
{
  /** Default implementation for IParcelFileDescriptorPeer. */
  public static class Default implements android.os.cts.IParcelFileDescriptorPeer
  {
    @Override public void setPeer(android.os.cts.IParcelFileDescriptorPeer peer) throws android.os.RemoteException
    {
    }
    /* Setup internal local and remote FDs */
    @Override public void setupReadPipe() throws android.os.RemoteException
    {
    }
    @Override public void setupWritePipe() throws android.os.RemoteException
    {
    }
    @Override public void setupSocket() throws android.os.RemoteException
    {
    }
    @Override public void setupFile() throws android.os.RemoteException
    {
    }
    @Override public android.os.ParcelFileDescriptor get() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void set(android.os.ParcelFileDescriptor pfd) throws android.os.RemoteException
    {
    }
    /* Ask this peer to get their remote FD from another */
    @Override public void doGet() throws android.os.RemoteException
    {
    }
    /* Ask this peer to set their remote FD to another */
    @Override public void doSet() throws android.os.RemoteException
    {
    }
    @Override public int read() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void write(int oneByte) throws android.os.RemoteException
    {
    }
    @Override public void close() throws android.os.RemoteException
    {
    }
    @Override public void closeWithError(java.lang.String msg) throws android.os.RemoteException
    {
    }
    @Override public void detachFd() throws android.os.RemoteException
    {
    }
    @Override public void leak() throws android.os.RemoteException
    {
    }
    @Override public void crash() throws android.os.RemoteException
    {
    }
    @Override public java.lang.String checkError() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String checkListener() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.cts.IParcelFileDescriptorPeer
  {
    private static final java.lang.String DESCRIPTOR = "android.os.cts.IParcelFileDescriptorPeer";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.cts.IParcelFileDescriptorPeer interface,
     * generating a proxy if needed.
     */
    public static android.os.cts.IParcelFileDescriptorPeer asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.cts.IParcelFileDescriptorPeer))) {
        return ((android.os.cts.IParcelFileDescriptorPeer)iin);
      }
      return new android.os.cts.IParcelFileDescriptorPeer.Stub.Proxy(obj);
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
        case TRANSACTION_setPeer:
        {
          data.enforceInterface(descriptor);
          android.os.cts.IParcelFileDescriptorPeer _arg0;
          _arg0 = android.os.cts.IParcelFileDescriptorPeer.Stub.asInterface(data.readStrongBinder());
          this.setPeer(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setupReadPipe:
        {
          data.enforceInterface(descriptor);
          this.setupReadPipe();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setupWritePipe:
        {
          data.enforceInterface(descriptor);
          this.setupWritePipe();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setupSocket:
        {
          data.enforceInterface(descriptor);
          this.setupSocket();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setupFile:
        {
          data.enforceInterface(descriptor);
          this.setupFile();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_get:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _result = this.get();
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
        case TRANSACTION_set:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.set(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_doGet:
        {
          data.enforceInterface(descriptor);
          this.doGet();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_doSet:
        {
          data.enforceInterface(descriptor);
          this.doSet();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_read:
        {
          data.enforceInterface(descriptor);
          int _result = this.read();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_write:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.write(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeWithError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.closeWithError(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_detachFd:
        {
          data.enforceInterface(descriptor);
          this.detachFd();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_leak:
        {
          data.enforceInterface(descriptor);
          this.leak();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_crash:
        {
          data.enforceInterface(descriptor);
          this.crash();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkError:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.checkError();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_checkListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.checkListener();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.cts.IParcelFileDescriptorPeer
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
      @Override public void setPeer(android.os.cts.IParcelFileDescriptorPeer peer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((peer!=null))?(peer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPeer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPeer(peer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Setup internal local and remote FDs */
      @Override public void setupReadPipe() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupReadPipe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupReadPipe();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setupWritePipe() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupWritePipe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupWritePipe();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setupSocket() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupSocket, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupSocket();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setupFile() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupFile();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.ParcelFileDescriptor get() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
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
      @Override public void set(android.os.ParcelFileDescriptor pfd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pfd!=null)) {
            _data.writeInt(1);
            pfd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_set, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().set(pfd);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Ask this peer to get their remote FD from another */
      @Override public void doGet() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doGet, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doGet();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Ask this peer to set their remote FD to another */
      @Override public void doSet() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doSet, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doSet();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int read() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_read, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().read();
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
      @Override public void write(int oneByte) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(oneByte);
          boolean _status = mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().write(oneByte);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      @Override public void closeWithError(java.lang.String msg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(msg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeWithError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeWithError(msg);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void detachFd() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_detachFd, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().detachFd();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void leak() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_leak, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().leak();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void crash() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_crash, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().crash();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String checkError() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkError, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkError();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String checkListener() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkListener();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.cts.IParcelFileDescriptorPeer sDefaultImpl;
    }
    static final int TRANSACTION_setPeer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setupReadPipe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setupWritePipe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setupSocket = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setupFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_get = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_set = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_doGet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_doSet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_read = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_closeWithError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_detachFd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_leak = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_crash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_checkError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_checkListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    public static boolean setDefaultImpl(android.os.cts.IParcelFileDescriptorPeer impl) {
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
    public static android.os.cts.IParcelFileDescriptorPeer getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setPeer(android.os.cts.IParcelFileDescriptorPeer peer) throws android.os.RemoteException;
  /* Setup internal local and remote FDs */
  public void setupReadPipe() throws android.os.RemoteException;
  public void setupWritePipe() throws android.os.RemoteException;
  public void setupSocket() throws android.os.RemoteException;
  public void setupFile() throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor get() throws android.os.RemoteException;
  public void set(android.os.ParcelFileDescriptor pfd) throws android.os.RemoteException;
  /* Ask this peer to get their remote FD from another */
  public void doGet() throws android.os.RemoteException;
  /* Ask this peer to set their remote FD to another */
  public void doSet() throws android.os.RemoteException;
  public int read() throws android.os.RemoteException;
  public void write(int oneByte) throws android.os.RemoteException;
  public void close() throws android.os.RemoteException;
  public void closeWithError(java.lang.String msg) throws android.os.RemoteException;
  public void detachFd() throws android.os.RemoteException;
  public void leak() throws android.os.RemoteException;
  public void crash() throws android.os.RemoteException;
  public java.lang.String checkError() throws android.os.RemoteException;
  public java.lang.String checkListener() throws android.os.RemoteException;
}
