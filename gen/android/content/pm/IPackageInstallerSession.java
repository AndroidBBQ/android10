/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/** {@hide} */
public interface IPackageInstallerSession extends android.os.IInterface
{
  /** Default implementation for IPackageInstallerSession. */
  public static class Default implements android.content.pm.IPackageInstallerSession
  {
    @Override public void setClientProgress(float progress) throws android.os.RemoteException
    {
    }
    @Override public void addClientProgress(float progress) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String[] getNames() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.ParcelFileDescriptor openWrite(java.lang.String name, long offsetBytes, long lengthBytes) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.ParcelFileDescriptor openRead(java.lang.String name) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void write(java.lang.String name, long offsetBytes, long lengthBytes, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
    }
    @Override public void removeSplit(java.lang.String splitName) throws android.os.RemoteException
    {
    }
    @Override public void close() throws android.os.RemoteException
    {
    }
    @Override public void commit(android.content.IntentSender statusReceiver, boolean forTransferred) throws android.os.RemoteException
    {
    }
    @Override public void transfer(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void abandon() throws android.os.RemoteException
    {
    }
    @Override public boolean isMultiPackage() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int[] getChildSessionIds() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addChildSessionId(int sessionId) throws android.os.RemoteException
    {
    }
    @Override public void removeChildSessionId(int sessionId) throws android.os.RemoteException
    {
    }
    @Override public int getParentSessionId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isStaged() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IPackageInstallerSession
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IPackageInstallerSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IPackageInstallerSession interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IPackageInstallerSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IPackageInstallerSession))) {
        return ((android.content.pm.IPackageInstallerSession)iin);
      }
      return new android.content.pm.IPackageInstallerSession.Stub.Proxy(obj);
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
        case TRANSACTION_setClientProgress:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setClientProgress(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addClientProgress:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.addClientProgress(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNames:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getNames();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_openWrite:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          android.os.ParcelFileDescriptor _result = this.openWrite(_arg0, _arg1, _arg2);
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
        case TRANSACTION_openRead:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _result = this.openRead(_arg0);
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
        case TRANSACTION_write:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          android.os.ParcelFileDescriptor _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.write(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeSplit:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeSplit(_arg0);
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
        case TRANSACTION_commit:
        {
          data.enforceInterface(descriptor);
          android.content.IntentSender _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.IntentSender.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.commit(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_transfer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.transfer(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_abandon:
        {
          data.enforceInterface(descriptor);
          this.abandon();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isMultiPackage:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isMultiPackage();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getChildSessionIds:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getChildSessionIds();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_addChildSessionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.addChildSessionId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeChildSessionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeChildSessionId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getParentSessionId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getParentSessionId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isStaged:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isStaged();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.IPackageInstallerSession
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
      @Override public void setClientProgress(float progress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(progress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setClientProgress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setClientProgress(progress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addClientProgress(float progress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(progress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClientProgress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addClientProgress(progress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String[] getNames() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNames, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNames();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.ParcelFileDescriptor openWrite(java.lang.String name, long offsetBytes, long lengthBytes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeLong(offsetBytes);
          _data.writeLong(lengthBytes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openWrite, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openWrite(name, offsetBytes, lengthBytes);
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
      @Override public android.os.ParcelFileDescriptor openRead(java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openRead, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openRead(name);
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
      @Override public void write(java.lang.String name, long offsetBytes, long lengthBytes, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeLong(offsetBytes);
          _data.writeLong(lengthBytes);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().write(name, offsetBytes, lengthBytes, fd);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeSplit(java.lang.String splitName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(splitName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSplit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSplit(splitName);
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
      @Override public void commit(android.content.IntentSender statusReceiver, boolean forTransferred) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((statusReceiver!=null)) {
            _data.writeInt(1);
            statusReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((forTransferred)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_commit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commit(statusReceiver, forTransferred);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void transfer(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_transfer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().transfer(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void abandon() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_abandon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().abandon();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isMultiPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMultiPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMultiPackage();
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
      @Override public int[] getChildSessionIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getChildSessionIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getChildSessionIds();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void addChildSessionId(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addChildSessionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addChildSessionId(sessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeChildSessionId(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeChildSessionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeChildSessionId(sessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getParentSessionId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getParentSessionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getParentSessionId();
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
      @Override public boolean isStaged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isStaged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isStaged();
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
      public static android.content.pm.IPackageInstallerSession sDefaultImpl;
    }
    static final int TRANSACTION_setClientProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addClientProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getNames = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_openWrite = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_openRead = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeSplit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_commit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_transfer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_abandon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_isMultiPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getChildSessionIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_addChildSessionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_removeChildSessionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getParentSessionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_isStaged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    public static boolean setDefaultImpl(android.content.pm.IPackageInstallerSession impl) {
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
    public static android.content.pm.IPackageInstallerSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setClientProgress(float progress) throws android.os.RemoteException;
  public void addClientProgress(float progress) throws android.os.RemoteException;
  public java.lang.String[] getNames() throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor openWrite(java.lang.String name, long offsetBytes, long lengthBytes) throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor openRead(java.lang.String name) throws android.os.RemoteException;
  public void write(java.lang.String name, long offsetBytes, long lengthBytes, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  public void removeSplit(java.lang.String splitName) throws android.os.RemoteException;
  public void close() throws android.os.RemoteException;
  public void commit(android.content.IntentSender statusReceiver, boolean forTransferred) throws android.os.RemoteException;
  public void transfer(java.lang.String packageName) throws android.os.RemoteException;
  public void abandon() throws android.os.RemoteException;
  public boolean isMultiPackage() throws android.os.RemoteException;
  public int[] getChildSessionIds() throws android.os.RemoteException;
  public void addChildSessionId(int sessionId) throws android.os.RemoteException;
  public void removeChildSessionId(int sessionId) throws android.os.RemoteException;
  public int getParentSessionId() throws android.os.RemoteException;
  public boolean isStaged() throws android.os.RemoteException;
}
