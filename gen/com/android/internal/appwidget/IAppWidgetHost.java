/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.appwidget;
/** {@hide} */
public interface IAppWidgetHost extends android.os.IInterface
{
  /** Default implementation for IAppWidgetHost. */
  public static class Default implements com.android.internal.appwidget.IAppWidgetHost
  {
    @Override public void updateAppWidget(int appWidgetId, android.widget.RemoteViews views) throws android.os.RemoteException
    {
    }
    @Override public void providerChanged(int appWidgetId, android.appwidget.AppWidgetProviderInfo info) throws android.os.RemoteException
    {
    }
    @Override public void providersChanged() throws android.os.RemoteException
    {
    }
    @Override public void viewDataChanged(int appWidgetId, int viewId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.appwidget.IAppWidgetHost
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.appwidget.IAppWidgetHost";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.appwidget.IAppWidgetHost interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.appwidget.IAppWidgetHost asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.appwidget.IAppWidgetHost))) {
        return ((com.android.internal.appwidget.IAppWidgetHost)iin);
      }
      return new com.android.internal.appwidget.IAppWidgetHost.Stub.Proxy(obj);
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
        case TRANSACTION_updateAppWidget:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.widget.RemoteViews _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.widget.RemoteViews.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updateAppWidget(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_providerChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.appwidget.AppWidgetProviderInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.appwidget.AppWidgetProviderInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.providerChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_providersChanged:
        {
          data.enforceInterface(descriptor);
          this.providersChanged();
          return true;
        }
        case TRANSACTION_viewDataChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.viewDataChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.appwidget.IAppWidgetHost
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
      @Override public void updateAppWidget(int appWidgetId, android.widget.RemoteViews views) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appWidgetId);
          if ((views!=null)) {
            _data.writeInt(1);
            views.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAppWidget, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAppWidget(appWidgetId, views);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void providerChanged(int appWidgetId, android.appwidget.AppWidgetProviderInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appWidgetId);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_providerChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().providerChanged(appWidgetId, info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void providersChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_providersChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().providersChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void viewDataChanged(int appWidgetId, int viewId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appWidgetId);
          _data.writeInt(viewId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_viewDataChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().viewDataChanged(appWidgetId, viewId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.appwidget.IAppWidgetHost sDefaultImpl;
    }
    static final int TRANSACTION_updateAppWidget = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_providerChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_providersChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_viewDataChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.appwidget.IAppWidgetHost impl) {
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
    public static com.android.internal.appwidget.IAppWidgetHost getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void updateAppWidget(int appWidgetId, android.widget.RemoteViews views) throws android.os.RemoteException;
  public void providerChanged(int appWidgetId, android.appwidget.AppWidgetProviderInfo info) throws android.os.RemoteException;
  public void providersChanged() throws android.os.RemoteException;
  public void viewDataChanged(int appWidgetId, int viewId) throws android.os.RemoteException;
}
