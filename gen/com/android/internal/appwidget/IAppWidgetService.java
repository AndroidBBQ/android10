/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.appwidget;
/** {@hide} */
public interface IAppWidgetService extends android.os.IInterface
{
  /** Default implementation for IAppWidgetService. */
  public static class Default implements com.android.internal.appwidget.IAppWidgetService
  {
    //
    // for AppWidgetHost
    //

    @Override public android.content.pm.ParceledListSlice startListening(com.android.internal.appwidget.IAppWidgetHost host, java.lang.String callingPackage, int hostId, int[] appWidgetIds) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void stopListening(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
    {
    }
    @Override public int allocateAppWidgetId(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void deleteAppWidgetId(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
    {
    }
    @Override public void deleteHost(java.lang.String packageName, int hostId) throws android.os.RemoteException
    {
    }
    @Override public void deleteAllHosts() throws android.os.RemoteException
    {
    }
    @Override public android.widget.RemoteViews getAppWidgetViews(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int[] getAppWidgetIdsForHost(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.IntentSender createAppWidgetConfigIntentSender(java.lang.String callingPackage, int appWidgetId, int intentFlags) throws android.os.RemoteException
    {
      return null;
    }
    //
    // for AppWidgetManager
    //

    @Override public void updateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException
    {
    }
    @Override public void updateAppWidgetOptions(java.lang.String callingPackage, int appWidgetId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public android.os.Bundle getAppWidgetOptions(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void partiallyUpdateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException
    {
    }
    @Override public void updateAppWidgetProvider(android.content.ComponentName provider, android.widget.RemoteViews views) throws android.os.RemoteException
    {
    }
    @Override public void updateAppWidgetProviderInfo(android.content.ComponentName provider, java.lang.String metadataKey) throws android.os.RemoteException
    {
    }
    @Override public void notifyAppWidgetViewDataChanged(java.lang.String packageName, int[] appWidgetIds, int viewId) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getInstalledProvidersForProfile(int categoryFilter, int profileId, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.appwidget.AppWidgetProviderInfo getAppWidgetInfo(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean hasBindAppWidgetPermission(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setBindAppWidgetPermission(java.lang.String packageName, int userId, boolean permission) throws android.os.RemoteException
    {
    }
    @Override public boolean bindAppWidgetId(java.lang.String callingPackage, int appWidgetId, int providerProfileId, android.content.ComponentName providerComponent, android.os.Bundle options) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean bindRemoteViewsService(java.lang.String callingPackage, int appWidgetId, android.content.Intent intent, android.app.IApplicationThread caller, android.os.IBinder token, android.app.IServiceConnection connection, int flags) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int[] getAppWidgetIds(android.content.ComponentName providerComponent) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isBoundWidgetPackage(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean requestPinAppWidget(java.lang.String packageName, android.content.ComponentName providerComponent, android.os.Bundle extras, android.content.IntentSender resultIntent) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isRequestPinAppWidgetSupported() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.appwidget.IAppWidgetService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.appwidget.IAppWidgetService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.appwidget.IAppWidgetService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.appwidget.IAppWidgetService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.appwidget.IAppWidgetService))) {
        return ((com.android.internal.appwidget.IAppWidgetService)iin);
      }
      return new com.android.internal.appwidget.IAppWidgetService.Stub.Proxy(obj);
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
        case TRANSACTION_startListening:
        {
          data.enforceInterface(descriptor);
          com.android.internal.appwidget.IAppWidgetHost _arg0;
          _arg0 = com.android.internal.appwidget.IAppWidgetHost.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int[] _arg3;
          _arg3 = data.createIntArray();
          android.content.pm.ParceledListSlice _result = this.startListening(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_stopListening:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.stopListening(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_allocateAppWidgetId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.allocateAppWidgetId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_deleteAppWidgetId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.deleteAppWidgetId(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deleteHost:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.deleteHost(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deleteAllHosts:
        {
          data.enforceInterface(descriptor);
          this.deleteAllHosts();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAppWidgetViews:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.widget.RemoteViews _result = this.getAppWidgetViews(_arg0, _arg1);
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
        case TRANSACTION_getAppWidgetIdsForHost:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int[] _result = this.getAppWidgetIdsForHost(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_createAppWidgetConfigIntentSender:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.content.IntentSender _result = this.createAppWidgetConfigIntentSender(_arg0, _arg1, _arg2);
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
        case TRANSACTION_updateAppWidgetIds:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int[] _arg1;
          _arg1 = data.createIntArray();
          android.widget.RemoteViews _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.widget.RemoteViews.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.updateAppWidgetIds(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateAppWidgetOptions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.updateAppWidgetOptions(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAppWidgetOptions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _result = this.getAppWidgetOptions(_arg0, _arg1);
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
        case TRANSACTION_partiallyUpdateAppWidgetIds:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int[] _arg1;
          _arg1 = data.createIntArray();
          android.widget.RemoteViews _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.widget.RemoteViews.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.partiallyUpdateAppWidgetIds(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateAppWidgetProvider:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.widget.RemoteViews _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.widget.RemoteViews.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updateAppWidgetProvider(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateAppWidgetProviderInfo:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.updateAppWidgetProviderInfo(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyAppWidgetViewDataChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int[] _arg1;
          _arg1 = data.createIntArray();
          int _arg2;
          _arg2 = data.readInt();
          this.notifyAppWidgetViewDataChanged(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInstalledProvidersForProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.pm.ParceledListSlice _result = this.getInstalledProvidersForProfile(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getAppWidgetInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.appwidget.AppWidgetProviderInfo _result = this.getAppWidgetInfo(_arg0, _arg1);
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
        case TRANSACTION_hasBindAppWidgetPermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.hasBindAppWidgetPermission(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setBindAppWidgetPermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setBindAppWidgetPermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_bindAppWidgetId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.content.ComponentName _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _result = this.bindAppWidgetId(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_bindRemoteViewsService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.app.IApplicationThread _arg3;
          _arg3 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg4;
          _arg4 = data.readStrongBinder();
          android.app.IServiceConnection _arg5;
          _arg5 = android.app.IServiceConnection.Stub.asInterface(data.readStrongBinder());
          int _arg6;
          _arg6 = data.readInt();
          boolean _result = this.bindRemoteViewsService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAppWidgetIds:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int[] _result = this.getAppWidgetIds(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_isBoundWidgetPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isBoundWidgetPackage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestPinAppWidget:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.IntentSender _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.IntentSender.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _result = this.requestPinAppWidget(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRequestPinAppWidgetSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isRequestPinAppWidgetSupported();
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
    private static class Proxy implements com.android.internal.appwidget.IAppWidgetService
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
      //
      // for AppWidgetHost
      //

      @Override public android.content.pm.ParceledListSlice startListening(com.android.internal.appwidget.IAppWidgetHost host, java.lang.String callingPackage, int hostId, int[] appWidgetIds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((host!=null))?(host.asBinder()):(null)));
          _data.writeString(callingPackage);
          _data.writeInt(hostId);
          _data.writeIntArray(appWidgetIds);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startListening, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startListening(host, callingPackage, hostId, appWidgetIds);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      @Override public void stopListening(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(hostId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopListening, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopListening(callingPackage, hostId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int allocateAppWidgetId(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(hostId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_allocateAppWidgetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().allocateAppWidgetId(callingPackage, hostId);
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
      @Override public void deleteAppWidgetId(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteAppWidgetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteAppWidgetId(callingPackage, appWidgetId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deleteHost(java.lang.String packageName, int hostId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(hostId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteHost, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteHost(packageName, hostId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deleteAllHosts() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteAllHosts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteAllHosts();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.widget.RemoteViews getAppWidgetViews(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.widget.RemoteViews _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppWidgetViews, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppWidgetViews(callingPackage, appWidgetId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.widget.RemoteViews.CREATOR.createFromParcel(_reply);
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
      @Override public int[] getAppWidgetIdsForHost(java.lang.String callingPackage, int hostId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(hostId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppWidgetIdsForHost, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppWidgetIdsForHost(callingPackage, hostId);
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
      @Override public android.content.IntentSender createAppWidgetConfigIntentSender(java.lang.String callingPackage, int appWidgetId, int intentFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.IntentSender _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          _data.writeInt(intentFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createAppWidgetConfigIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createAppWidgetConfigIntentSender(callingPackage, appWidgetId, intentFlags);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.IntentSender.CREATOR.createFromParcel(_reply);
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
      //
      // for AppWidgetManager
      //

      @Override public void updateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeIntArray(appWidgetIds);
          if ((views!=null)) {
            _data.writeInt(1);
            views.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppWidgetIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppWidgetIds(callingPackage, appWidgetIds, views);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateAppWidgetOptions(java.lang.String callingPackage, int appWidgetId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppWidgetOptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppWidgetOptions(callingPackage, appWidgetId, extras);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Bundle getAppWidgetOptions(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppWidgetOptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppWidgetOptions(callingPackage, appWidgetId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
      @Override public void partiallyUpdateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeIntArray(appWidgetIds);
          if ((views!=null)) {
            _data.writeInt(1);
            views.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_partiallyUpdateAppWidgetIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().partiallyUpdateAppWidgetIds(callingPackage, appWidgetIds, views);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateAppWidgetProvider(android.content.ComponentName provider, android.widget.RemoteViews views) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provider!=null)) {
            _data.writeInt(1);
            provider.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((views!=null)) {
            _data.writeInt(1);
            views.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppWidgetProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppWidgetProvider(provider, views);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateAppWidgetProviderInfo(android.content.ComponentName provider, java.lang.String metadataKey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provider!=null)) {
            _data.writeInt(1);
            provider.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(metadataKey);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppWidgetProviderInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppWidgetProviderInfo(provider, metadataKey);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyAppWidgetViewDataChanged(java.lang.String packageName, int[] appWidgetIds, int viewId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeIntArray(appWidgetIds);
          _data.writeInt(viewId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAppWidgetViewDataChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAppWidgetViewDataChanged(packageName, appWidgetIds, viewId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getInstalledProvidersForProfile(int categoryFilter, int profileId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(categoryFilter);
          _data.writeInt(profileId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInstalledProvidersForProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInstalledProvidersForProfile(categoryFilter, profileId, packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
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
      @Override public android.appwidget.AppWidgetProviderInfo getAppWidgetInfo(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.appwidget.AppWidgetProviderInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppWidgetInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppWidgetInfo(callingPackage, appWidgetId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.appwidget.AppWidgetProviderInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean hasBindAppWidgetPermission(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasBindAppWidgetPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasBindAppWidgetPermission(packageName, userId);
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
      @Override public void setBindAppWidgetPermission(java.lang.String packageName, int userId, boolean permission) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          _data.writeInt(((permission)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBindAppWidgetPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBindAppWidgetPermission(packageName, userId, permission);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean bindAppWidgetId(java.lang.String callingPackage, int appWidgetId, int providerProfileId, android.content.ComponentName providerComponent, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          _data.writeInt(providerProfileId);
          if ((providerComponent!=null)) {
            _data.writeInt(1);
            providerComponent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindAppWidgetId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bindAppWidgetId(callingPackage, appWidgetId, providerProfileId, providerComponent, options);
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
      @Override public boolean bindRemoteViewsService(java.lang.String callingPackage, int appWidgetId, android.content.Intent intent, android.app.IApplicationThread caller, android.os.IBinder token, android.app.IServiceConnection connection, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(appWidgetId);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindRemoteViewsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bindRemoteViewsService(callingPackage, appWidgetId, intent, caller, token, connection, flags);
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
      @Override public int[] getAppWidgetIds(android.content.ComponentName providerComponent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((providerComponent!=null)) {
            _data.writeInt(1);
            providerComponent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppWidgetIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppWidgetIds(providerComponent);
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
      @Override public boolean isBoundWidgetPackage(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBoundWidgetPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBoundWidgetPackage(packageName, userId);
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
      @Override public boolean requestPinAppWidget(java.lang.String packageName, android.content.ComponentName providerComponent, android.os.Bundle extras, android.content.IntentSender resultIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((providerComponent!=null)) {
            _data.writeInt(1);
            providerComponent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((resultIntent!=null)) {
            _data.writeInt(1);
            resultIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestPinAppWidget, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestPinAppWidget(packageName, providerComponent, extras, resultIntent);
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
      @Override public boolean isRequestPinAppWidgetSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRequestPinAppWidgetSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRequestPinAppWidgetSupported();
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
      public static com.android.internal.appwidget.IAppWidgetService sDefaultImpl;
    }
    static final int TRANSACTION_startListening = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopListening = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_allocateAppWidgetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_deleteAppWidgetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_deleteHost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_deleteAllHosts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getAppWidgetViews = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getAppWidgetIdsForHost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_createAppWidgetConfigIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_updateAppWidgetIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_updateAppWidgetOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getAppWidgetOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_partiallyUpdateAppWidgetIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_updateAppWidgetProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_updateAppWidgetProviderInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_notifyAppWidgetViewDataChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getInstalledProvidersForProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getAppWidgetInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_hasBindAppWidgetPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setBindAppWidgetPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_bindAppWidgetId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_bindRemoteViewsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getAppWidgetIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_isBoundWidgetPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_requestPinAppWidget = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_isRequestPinAppWidgetSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    public static boolean setDefaultImpl(com.android.internal.appwidget.IAppWidgetService impl) {
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
    public static com.android.internal.appwidget.IAppWidgetService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  //
  // for AppWidgetHost
  //

  public android.content.pm.ParceledListSlice startListening(com.android.internal.appwidget.IAppWidgetHost host, java.lang.String callingPackage, int hostId, int[] appWidgetIds) throws android.os.RemoteException;
  public void stopListening(java.lang.String callingPackage, int hostId) throws android.os.RemoteException;
  public int allocateAppWidgetId(java.lang.String callingPackage, int hostId) throws android.os.RemoteException;
  public void deleteAppWidgetId(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException;
  public void deleteHost(java.lang.String packageName, int hostId) throws android.os.RemoteException;
  public void deleteAllHosts() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/appwidget/IAppWidgetService.aidl:45:1:45:25")
  public android.widget.RemoteViews getAppWidgetViews(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException;
  public int[] getAppWidgetIdsForHost(java.lang.String callingPackage, int hostId) throws android.os.RemoteException;
  public android.content.IntentSender createAppWidgetConfigIntentSender(java.lang.String callingPackage, int appWidgetId, int intentFlags) throws android.os.RemoteException;
  //
  // for AppWidgetManager
  //

  public void updateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException;
  public void updateAppWidgetOptions(java.lang.String callingPackage, int appWidgetId, android.os.Bundle extras) throws android.os.RemoteException;
  public android.os.Bundle getAppWidgetOptions(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException;
  public void partiallyUpdateAppWidgetIds(java.lang.String callingPackage, int[] appWidgetIds, android.widget.RemoteViews views) throws android.os.RemoteException;
  public void updateAppWidgetProvider(android.content.ComponentName provider, android.widget.RemoteViews views) throws android.os.RemoteException;
  public void updateAppWidgetProviderInfo(android.content.ComponentName provider, java.lang.String metadataKey) throws android.os.RemoteException;
  public void notifyAppWidgetViewDataChanged(java.lang.String packageName, int[] appWidgetIds, int viewId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getInstalledProvidersForProfile(int categoryFilter, int profileId, java.lang.String packageName) throws android.os.RemoteException;
  public android.appwidget.AppWidgetProviderInfo getAppWidgetInfo(java.lang.String callingPackage, int appWidgetId) throws android.os.RemoteException;
  public boolean hasBindAppWidgetPermission(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public void setBindAppWidgetPermission(java.lang.String packageName, int userId, boolean permission) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/appwidget/IAppWidgetService.aidl:67:1:67:25")
  public boolean bindAppWidgetId(java.lang.String callingPackage, int appWidgetId, int providerProfileId, android.content.ComponentName providerComponent, android.os.Bundle options) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/appwidget/IAppWidgetService.aidl:70:1:70:25")
  public boolean bindRemoteViewsService(java.lang.String callingPackage, int appWidgetId, android.content.Intent intent, android.app.IApplicationThread caller, android.os.IBinder token, android.app.IServiceConnection connection, int flags) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/appwidget/IAppWidgetService.aidl:74:1:74:25")
  public int[] getAppWidgetIds(android.content.ComponentName providerComponent) throws android.os.RemoteException;
  public boolean isBoundWidgetPackage(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public boolean requestPinAppWidget(java.lang.String packageName, android.content.ComponentName providerComponent, android.os.Bundle extras, android.content.IntentSender resultIntent) throws android.os.RemoteException;
  public boolean isRequestPinAppWidgetSupported() throws android.os.RemoteException;
}
