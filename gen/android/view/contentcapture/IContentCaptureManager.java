/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.contentcapture;
/**
  * Interface between an app (ContentCaptureManager / ContentCaptureSession) and the system-server
  * implementation service (ContentCaptureManagerService).
  *
  * @hide
  */
public interface IContentCaptureManager extends android.os.IInterface
{
  /** Default implementation for IContentCaptureManager. */
  public static class Default implements android.view.contentcapture.IContentCaptureManager
  {
    /**
         * Starts a new session for the calling user running as part of the
         * app's activity identified by {@code activityToken}/{@code componentName}.
         *
         * @param sessionId Unique session id as provided by the app.
         * @param flags Meta flags that enable or disable content capture (see
         *     {@link IContentCaptureContext#flags}).
         */
    @Override public void startSession(android.os.IBinder activityToken, android.content.ComponentName componentName, int sessionId, int flags, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    /**
         * Marks the end of a session for the calling user identified by
         * the corresponding {@code startSession}'s {@code sessionId}.
         */
    @Override public void finishSession(int sessionId) throws android.os.RemoteException
    {
    }
    /**
         * Returns the content capture service's component name (if enabled and
         * connected).
         * @param Receiver of the content capture service's @{code ComponentName}
         *     provided {@code Bundle} with key "{@code EXTRA}".
         */
    @Override public void getServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    /**
         * Requests the removal of content capture data for the calling user.
         */
    @Override public void removeData(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException
    {
    }
    /**
         * Returns whether the content capture feature is enabled for the calling user.
         */
    @Override public void isContentCaptureFeatureEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    /**
         * Returns a ComponentName with the name of custom service activity, if defined.
         */
    @Override public void getServiceSettingsActivity(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list with the ContentCaptureConditions for the package (or null if not defined).
         */
    @Override public void getContentCaptureConditions(java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.contentcapture.IContentCaptureManager
  {
    private static final java.lang.String DESCRIPTOR = "android.view.contentcapture.IContentCaptureManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.contentcapture.IContentCaptureManager interface,
     * generating a proxy if needed.
     */
    public static android.view.contentcapture.IContentCaptureManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.contentcapture.IContentCaptureManager))) {
        return ((android.view.contentcapture.IContentCaptureManager)iin);
      }
      return new android.view.contentcapture.IContentCaptureManager.Stub.Proxy(obj);
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
        case TRANSACTION_startSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          com.android.internal.os.IResultReceiver _arg4;
          _arg4 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.startSession(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_finishSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.finishSession(_arg0);
          return true;
        }
        case TRANSACTION_getServiceComponentName:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getServiceComponentName(_arg0);
          return true;
        }
        case TRANSACTION_removeData:
        {
          data.enforceInterface(descriptor);
          android.view.contentcapture.DataRemovalRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.contentcapture.DataRemovalRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeData(_arg0);
          return true;
        }
        case TRANSACTION_isContentCaptureFeatureEnabled:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.isContentCaptureFeatureEnabled(_arg0);
          return true;
        }
        case TRANSACTION_getServiceSettingsActivity:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getServiceSettingsActivity(_arg0);
          return true;
        }
        case TRANSACTION_getContentCaptureConditions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.os.IResultReceiver _arg1;
          _arg1 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getContentCaptureConditions(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.contentcapture.IContentCaptureManager
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
      /**
           * Starts a new session for the calling user running as part of the
           * app's activity identified by {@code activityToken}/{@code componentName}.
           *
           * @param sessionId Unique session id as provided by the app.
           * @param flags Meta flags that enable or disable content capture (see
           *     {@link IContentCaptureContext#flags}).
           */
      @Override public void startSession(android.os.IBinder activityToken, android.content.ComponentName componentName, int sessionId, int flags, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sessionId);
          _data.writeInt(flags);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSession(activityToken, componentName, sessionId, flags, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Marks the end of a session for the calling user identified by
           * the corresponding {@code startSession}'s {@code sessionId}.
           */
      @Override public void finishSession(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSession(sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns the content capture service's component name (if enabled and
           * connected).
           * @param Receiver of the content capture service's @{code ComponentName}
           *     provided {@code Bundle} with key "{@code EXTRA}".
           */
      @Override public void getServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceComponentName, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getServiceComponentName(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Requests the removal of content capture data for the calling user.
           */
      @Override public void removeData(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeData(request);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns whether the content capture feature is enabled for the calling user.
           */
      @Override public void isContentCaptureFeatureEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isContentCaptureFeatureEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isContentCaptureFeatureEnabled(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns a ComponentName with the name of custom service activity, if defined.
           */
      @Override public void getServiceSettingsActivity(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceSettingsActivity, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getServiceSettingsActivity(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns a list with the ContentCaptureConditions for the package (or null if not defined).
           */
      @Override public void getContentCaptureConditions(java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContentCaptureConditions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getContentCaptureConditions(packageName, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.contentcapture.IContentCaptureManager sDefaultImpl;
    }
    static final int TRANSACTION_startSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_finishSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getServiceComponentName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_removeData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isContentCaptureFeatureEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getServiceSettingsActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getContentCaptureConditions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.view.contentcapture.IContentCaptureManager impl) {
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
    public static android.view.contentcapture.IContentCaptureManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Starts a new session for the calling user running as part of the
       * app's activity identified by {@code activityToken}/{@code componentName}.
       *
       * @param sessionId Unique session id as provided by the app.
       * @param flags Meta flags that enable or disable content capture (see
       *     {@link IContentCaptureContext#flags}).
       */
  public void startSession(android.os.IBinder activityToken, android.content.ComponentName componentName, int sessionId, int flags, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  /**
       * Marks the end of a session for the calling user identified by
       * the corresponding {@code startSession}'s {@code sessionId}.
       */
  public void finishSession(int sessionId) throws android.os.RemoteException;
  /**
       * Returns the content capture service's component name (if enabled and
       * connected).
       * @param Receiver of the content capture service's @{code ComponentName}
       *     provided {@code Bundle} with key "{@code EXTRA}".
       */
  public void getServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  /**
       * Requests the removal of content capture data for the calling user.
       */
  public void removeData(android.view.contentcapture.DataRemovalRequest request) throws android.os.RemoteException;
  /**
       * Returns whether the content capture feature is enabled for the calling user.
       */
  public void isContentCaptureFeatureEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  /**
       * Returns a ComponentName with the name of custom service activity, if defined.
       */
  public void getServiceSettingsActivity(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  /**
       * Returns a list with the ContentCaptureConditions for the package (or null if not defined).
       */
  public void getContentCaptureConditions(java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
}
