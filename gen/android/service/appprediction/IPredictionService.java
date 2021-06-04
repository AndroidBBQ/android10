/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.appprediction;
/**
 * Interface from the system to a prediction service.
 *
 * @hide
 */
public interface IPredictionService extends android.os.IInterface
{
  /** Default implementation for IPredictionService. */
  public static class Default implements android.service.appprediction.IPredictionService
  {
    @Override public void onCreatePredictionSession(android.app.prediction.AppPredictionContext context, android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
    {
    }
    @Override public void notifyAppTargetEvent(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.AppTargetEvent event) throws android.os.RemoteException
    {
    }
    @Override public void notifyLaunchLocationShown(android.app.prediction.AppPredictionSessionId sessionId, java.lang.String launchLocation, android.content.pm.ParceledListSlice targetIds) throws android.os.RemoteException
    {
    }
    @Override public void sortAppTargets(android.app.prediction.AppPredictionSessionId sessionId, android.content.pm.ParceledListSlice targets, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void registerPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void requestPredictionUpdate(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
    {
    }
    @Override public void onDestroyPredictionSession(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.appprediction.IPredictionService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.appprediction.IPredictionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.appprediction.IPredictionService interface,
     * generating a proxy if needed.
     */
    public static android.service.appprediction.IPredictionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.appprediction.IPredictionService))) {
        return ((android.service.appprediction.IPredictionService)iin);
      }
      return new android.service.appprediction.IPredictionService.Stub.Proxy(obj);
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
        case TRANSACTION_onCreatePredictionSession:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionContext _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionContext.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.prediction.AppPredictionSessionId _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onCreatePredictionSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyAppTargetEvent:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.prediction.AppTargetEvent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.prediction.AppTargetEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyAppTargetEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyLaunchLocationShown:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.pm.ParceledListSlice _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.notifyLaunchLocationShown(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_sortAppTargets:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.prediction.IPredictionCallback _arg2;
          _arg2 = android.app.prediction.IPredictionCallback.Stub.asInterface(data.readStrongBinder());
          this.sortAppTargets(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_registerPredictionUpdates:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.prediction.IPredictionCallback _arg1;
          _arg1 = android.app.prediction.IPredictionCallback.Stub.asInterface(data.readStrongBinder());
          this.registerPredictionUpdates(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unregisterPredictionUpdates:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.prediction.IPredictionCallback _arg1;
          _arg1 = android.app.prediction.IPredictionCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterPredictionUpdates(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_requestPredictionUpdate:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.requestPredictionUpdate(_arg0);
          return true;
        }
        case TRANSACTION_onDestroyPredictionSession:
        {
          data.enforceInterface(descriptor);
          android.app.prediction.AppPredictionSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.prediction.AppPredictionSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDestroyPredictionSession(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.appprediction.IPredictionService
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
      @Override public void onCreatePredictionSession(android.app.prediction.AppPredictionContext context, android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
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
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCreatePredictionSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCreatePredictionSession(context, sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyAppTargetEvent(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.AppTargetEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAppTargetEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAppTargetEvent(sessionId, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyLaunchLocationShown(android.app.prediction.AppPredictionSessionId sessionId, java.lang.String launchLocation, android.content.pm.ParceledListSlice targetIds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(launchLocation);
          if ((targetIds!=null)) {
            _data.writeInt(1);
            targetIds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyLaunchLocationShown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyLaunchLocationShown(sessionId, launchLocation, targetIds);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sortAppTargets(android.app.prediction.AppPredictionSessionId sessionId, android.content.pm.ParceledListSlice targets, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((targets!=null)) {
            _data.writeInt(1);
            targets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sortAppTargets, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sortAppTargets(sessionId, targets, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void registerPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPredictionUpdates, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPredictionUpdates(sessionId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterPredictionUpdates, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterPredictionUpdates(sessionId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestPredictionUpdate(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestPredictionUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestPredictionUpdate(sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDestroyPredictionSession(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDestroyPredictionSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDestroyPredictionSession(sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.appprediction.IPredictionService sDefaultImpl;
    }
    static final int TRANSACTION_onCreatePredictionSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_notifyAppTargetEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyLaunchLocationShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_sortAppTargets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_registerPredictionUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_unregisterPredictionUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_requestPredictionUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onDestroyPredictionSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.service.appprediction.IPredictionService impl) {
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
    public static android.service.appprediction.IPredictionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCreatePredictionSession(android.app.prediction.AppPredictionContext context, android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException;
  public void notifyAppTargetEvent(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.AppTargetEvent event) throws android.os.RemoteException;
  public void notifyLaunchLocationShown(android.app.prediction.AppPredictionSessionId sessionId, java.lang.String launchLocation, android.content.pm.ParceledListSlice targetIds) throws android.os.RemoteException;
  public void sortAppTargets(android.app.prediction.AppPredictionSessionId sessionId, android.content.pm.ParceledListSlice targets, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException;
  public void registerPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException;
  public void unregisterPredictionUpdates(android.app.prediction.AppPredictionSessionId sessionId, android.app.prediction.IPredictionCallback callback) throws android.os.RemoteException;
  public void requestPredictionUpdate(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException;
  public void onDestroyPredictionSession(android.app.prediction.AppPredictionSessionId sessionId) throws android.os.RemoteException;
}
