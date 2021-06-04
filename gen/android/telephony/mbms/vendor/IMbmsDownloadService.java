/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms.vendor;
/**
 * @hide
 */
public interface IMbmsDownloadService extends android.os.IInterface
{
  /** Default implementation for IMbmsDownloadService. */
  public static class Default implements android.telephony.mbms.vendor.IMbmsDownloadService
  {
    @Override public int initialize(int subId, android.telephony.mbms.IMbmsDownloadSessionCallback listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int requestUpdateFileServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int setTempFileRootDirectory(int subId, java.lang.String rootDirectoryPath) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int download(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int addStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int removeStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int addProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int removeProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.util.List<android.telephony.mbms.DownloadRequest> listPendingDownloads(int subscriptionId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int cancelDownload(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int requestDownloadState(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.FileInfo fileInfo) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int resetDownloadKnowledge(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void dispose(int subId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.vendor.IMbmsDownloadService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.vendor.IMbmsDownloadService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.vendor.IMbmsDownloadService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.vendor.IMbmsDownloadService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.vendor.IMbmsDownloadService))) {
        return ((android.telephony.mbms.vendor.IMbmsDownloadService)iin);
      }
      return new android.telephony.mbms.vendor.IMbmsDownloadService.Stub.Proxy(obj);
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
        case TRANSACTION_initialize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.mbms.IMbmsDownloadSessionCallback _arg1;
          _arg1 = android.telephony.mbms.IMbmsDownloadSessionCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.initialize(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestUpdateFileServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          int _result = this.requestUpdateFileServices(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setTempFileRootDirectory:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.setTempFileRootDirectory(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_download:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.download(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addStatusListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.IDownloadStatusListener _arg1;
          _arg1 = android.telephony.mbms.IDownloadStatusListener.Stub.asInterface(data.readStrongBinder());
          int _result = this.addStatusListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeStatusListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.IDownloadStatusListener _arg1;
          _arg1 = android.telephony.mbms.IDownloadStatusListener.Stub.asInterface(data.readStrongBinder());
          int _result = this.removeStatusListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addProgressListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.IDownloadProgressListener _arg1;
          _arg1 = android.telephony.mbms.IDownloadProgressListener.Stub.asInterface(data.readStrongBinder());
          int _result = this.addProgressListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeProgressListener:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.IDownloadProgressListener _arg1;
          _arg1 = android.telephony.mbms.IDownloadProgressListener.Stub.asInterface(data.readStrongBinder());
          int _result = this.removeProgressListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_listPendingDownloads:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.telephony.mbms.DownloadRequest> _result = this.listPendingDownloads(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_cancelDownload:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.cancelDownload(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestDownloadState:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.FileInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.mbms.FileInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.requestDownloadState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_resetDownloadKnowledge:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.resetDownloadKnowledge(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_dispose:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.dispose(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.vendor.IMbmsDownloadService
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
      @Override public int initialize(int subId, android.telephony.mbms.IMbmsDownloadSessionCallback listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_initialize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().initialize(subId, listener);
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
      @Override public int requestUpdateFileServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStringList(serviceClasses);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUpdateFileServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestUpdateFileServices(subId, serviceClasses);
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
      @Override public int setTempFileRootDirectory(int subId, java.lang.String rootDirectoryPath) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(rootDirectoryPath);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTempFileRootDirectory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setTempFileRootDirectory(subId, rootDirectoryPath);
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
      @Override public int download(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_download, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().download(downloadRequest);
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
      @Override public int addStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addStatusListener(downloadRequest, listener);
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
      @Override public int removeStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeStatusListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeStatusListener(downloadRequest, listener);
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
      @Override public int addProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addProgressListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addProgressListener(downloadRequest, listener);
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
      @Override public int removeProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeProgressListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeProgressListener(downloadRequest, listener);
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
      @Override public java.util.List<android.telephony.mbms.DownloadRequest> listPendingDownloads(int subscriptionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.mbms.DownloadRequest> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subscriptionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listPendingDownloads, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listPendingDownloads(subscriptionId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.mbms.DownloadRequest.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int cancelDownload(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelDownload, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().cancelDownload(downloadRequest);
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
      @Override public int requestDownloadState(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.FileInfo fileInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((fileInfo!=null)) {
            _data.writeInt(1);
            fileInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestDownloadState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestDownloadState(downloadRequest, fileInfo);
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
      @Override public int resetDownloadKnowledge(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((downloadRequest!=null)) {
            _data.writeInt(1);
            downloadRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetDownloadKnowledge, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().resetDownloadKnowledge(downloadRequest);
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
      @Override public void dispose(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispose, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispose(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.telephony.mbms.vendor.IMbmsDownloadService sDefaultImpl;
    }
    static final int TRANSACTION_initialize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_requestUpdateFileServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setTempFileRootDirectory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_download = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeStatusListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_addProgressListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_removeProgressListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_listPendingDownloads = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_cancelDownload = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_requestDownloadState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_resetDownloadKnowledge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_dispose = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.telephony.mbms.vendor.IMbmsDownloadService impl) {
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
    public static android.telephony.mbms.vendor.IMbmsDownloadService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int initialize(int subId, android.telephony.mbms.IMbmsDownloadSessionCallback listener) throws android.os.RemoteException;
  public int requestUpdateFileServices(int subId, java.util.List<java.lang.String> serviceClasses) throws android.os.RemoteException;
  public int setTempFileRootDirectory(int subId, java.lang.String rootDirectoryPath) throws android.os.RemoteException;
  public int download(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException;
  public int addStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException;
  public int removeStatusListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadStatusListener listener) throws android.os.RemoteException;
  public int addProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException;
  public int removeProgressListener(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.IDownloadProgressListener listener) throws android.os.RemoteException;
  public java.util.List<android.telephony.mbms.DownloadRequest> listPendingDownloads(int subscriptionId) throws android.os.RemoteException;
  public int cancelDownload(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException;
  public int requestDownloadState(android.telephony.mbms.DownloadRequest downloadRequest, android.telephony.mbms.FileInfo fileInfo) throws android.os.RemoteException;
  public int resetDownloadKnowledge(android.telephony.mbms.DownloadRequest downloadRequest) throws android.os.RemoteException;
  public void dispose(int subId) throws android.os.RemoteException;
}
