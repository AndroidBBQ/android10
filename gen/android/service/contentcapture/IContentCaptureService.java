/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.contentcapture;
/**
 * Interface from the system to a Content Capture service.
 *
 * @hide
 */
public interface IContentCaptureService extends android.os.IInterface
{
  /** Default implementation for IContentCaptureService. */
  public static class Default implements android.service.contentcapture.IContentCaptureService
  {
    @Override public void onConnected(android.os.IBinder callback, boolean verbose, boolean debug) throws android.os.RemoteException
    {
    }
    @Override public void onDisconnected() throws android.os.RemoteException
    {
    }
    @Override public void onSessionStarted(android.view.contentcapture.ContentCaptureContext context, int sessionId, int uid, com.android.internal.os.IResultReceiver clientReceiver, int initialState) throws android.os.RemoteException
    {
    }
    @Override public void onSessionFinished(int sessionId) throws android.os.RemoteException
    {
    }
    @Override public void onActivitySnapshot(int sessionId, android.service.contentcapture.SnapshotData snapshotData) throws android.os.RemoteException
    {
    }
    @Override public void onDataRemovalRequest(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException
    {
    }
    @Override public void onActivityEvent(android.service.contentcapture.ActivityEvent event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.contentcapture.IContentCaptureService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.contentcapture.IContentCaptureService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.contentcapture.IContentCaptureService interface,
     * generating a proxy if needed.
     */
    public static android.service.contentcapture.IContentCaptureService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.contentcapture.IContentCaptureService))) {
        return ((android.service.contentcapture.IContentCaptureService)iin);
      }
      return new android.service.contentcapture.IContentCaptureService.Stub.Proxy(obj);
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
        case TRANSACTION_onConnected:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onConnected(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onDisconnected:
        {
          data.enforceInterface(descriptor);
          this.onDisconnected();
          return true;
        }
        case TRANSACTION_onSessionStarted:
        {
          data.enforceInterface(descriptor);
          android.view.contentcapture.ContentCaptureContext _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.contentcapture.ContentCaptureContext.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.os.IResultReceiver _arg3;
          _arg3 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          this.onSessionStarted(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onSessionFinished:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSessionFinished(_arg0);
          return true;
        }
        case TRANSACTION_onActivitySnapshot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.service.contentcapture.SnapshotData _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.service.contentcapture.SnapshotData.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onActivitySnapshot(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDataRemovalRequest:
        {
          data.enforceInterface(descriptor);
          android.view.contentcapture.DataRemovalRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.contentcapture.DataRemovalRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDataRemovalRequest(_arg0);
          return true;
        }
        case TRANSACTION_onActivityEvent:
        {
          data.enforceInterface(descriptor);
          android.service.contentcapture.ActivityEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.contentcapture.ActivityEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onActivityEvent(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.contentcapture.IContentCaptureService
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
      @Override public void onConnected(android.os.IBinder callback, boolean verbose, boolean debug) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(callback);
          _data.writeInt(((verbose)?(1):(0)));
          _data.writeInt(((debug)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnected(callback, verbose, debug);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDisconnected() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDisconnected();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionStarted(android.view.contentcapture.ContentCaptureContext context, int sessionId, int uid, com.android.internal.os.IResultReceiver clientReceiver, int initialState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((context!=null)) {
            _data.writeInt(1);
            context.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sessionId);
          _data.writeInt(uid);
          _data.writeStrongBinder((((clientReceiver!=null))?(clientReceiver.asBinder()):(null)));
          _data.writeInt(initialState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionStarted(context, sessionId, uid, clientReceiver, initialState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSessionFinished(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionFinished(sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onActivitySnapshot(int sessionId, android.service.contentcapture.SnapshotData snapshotData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          if ((snapshotData!=null)) {
            _data.writeInt(1);
            snapshotData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActivitySnapshot, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActivitySnapshot(sessionId, snapshotData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataRemovalRequest(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataRemovalRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataRemovalRequest(request);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onActivityEvent(android.service.contentcapture.ActivityEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActivityEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActivityEvent(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.contentcapture.IContentCaptureService sDefaultImpl;
    }
    static final int TRANSACTION_onConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSessionStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSessionFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onActivitySnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onDataRemovalRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onActivityEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.service.contentcapture.IContentCaptureService impl) {
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
    public static android.service.contentcapture.IContentCaptureService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onConnected(android.os.IBinder callback, boolean verbose, boolean debug) throws android.os.RemoteException;
  public void onDisconnected() throws android.os.RemoteException;
  public void onSessionStarted(android.view.contentcapture.ContentCaptureContext context, int sessionId, int uid, com.android.internal.os.IResultReceiver clientReceiver, int initialState) throws android.os.RemoteException;
  public void onSessionFinished(int sessionId) throws android.os.RemoteException;
  public void onActivitySnapshot(int sessionId, android.service.contentcapture.SnapshotData snapshotData) throws android.os.RemoteException;
  public void onDataRemovalRequest(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException;
  public void onActivityEvent(android.service.contentcapture.ActivityEvent event) throws android.os.RemoteException;
}
