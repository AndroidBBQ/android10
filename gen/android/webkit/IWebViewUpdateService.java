/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.webkit;
/**
 * Private service to wait for the updatable WebView to be ready for use.
 * @hide
 */
public interface IWebViewUpdateService extends android.os.IInterface
{
  /** Default implementation for IWebViewUpdateService. */
  public static class Default implements android.webkit.IWebViewUpdateService
  {
    /**
         * Used by the relro file creator to notify the service that it's done.
         */
    @Override public void notifyRelroCreationCompleted() throws android.os.RemoteException
    {
    }
    /**
         * Used by WebViewFactory to block loading of WebView code until
         * preparations are complete. Returns the package used as WebView provider.
         */
    @Override public android.webkit.WebViewProviderResponse waitForAndGetProvider() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * DevelopmentSettings uses this to notify WebViewUpdateService that a new provider has been
         * selected by the user. Returns the provider we end up switching to, this could be different to
         * the one passed as argument to this method since the Dev Setting calling this method could be
         * stale. I.e. the Dev setting could be letting the user choose uninstalled/disabled packages,
         * it would then try to update the provider to such a package while in reality the update
         * service would switch to another one.
         */
    @Override public java.lang.String changeProviderAndSetting(java.lang.String newProvider) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * DevelopmentSettings uses this to get the current available WebView
         * providers (to display as choices to the user).
         */
    @Override public android.webkit.WebViewProviderInfo[] getValidWebViewPackages() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Fetch all packages that could potentially implement WebView.
         */
    @Override public android.webkit.WebViewProviderInfo[] getAllWebViewPackages() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Used by DevelopmentSetting to get the name of the WebView provider currently in use.
         */
    @Override public java.lang.String getCurrentWebViewPackageName() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Used by public API for debugging purposes.
         */
    @Override public android.content.pm.PackageInfo getCurrentWebViewPackage() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Used by Settings to determine whether multiprocess is enabled.
         */
    @Override public boolean isMultiProcessEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Used by Settings to enable/disable multiprocess.
         */
    @Override public void enableMultiProcess(boolean enable) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.webkit.IWebViewUpdateService
  {
    private static final java.lang.String DESCRIPTOR = "android.webkit.IWebViewUpdateService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.webkit.IWebViewUpdateService interface,
     * generating a proxy if needed.
     */
    public static android.webkit.IWebViewUpdateService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.webkit.IWebViewUpdateService))) {
        return ((android.webkit.IWebViewUpdateService)iin);
      }
      return new android.webkit.IWebViewUpdateService.Stub.Proxy(obj);
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
        case TRANSACTION_notifyRelroCreationCompleted:
        {
          data.enforceInterface(descriptor);
          this.notifyRelroCreationCompleted();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_waitForAndGetProvider:
        {
          data.enforceInterface(descriptor);
          android.webkit.WebViewProviderResponse _result = this.waitForAndGetProvider();
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
        case TRANSACTION_changeProviderAndSetting:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.changeProviderAndSetting(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getValidWebViewPackages:
        {
          data.enforceInterface(descriptor);
          android.webkit.WebViewProviderInfo[] _result = this.getValidWebViewPackages();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getAllWebViewPackages:
        {
          data.enforceInterface(descriptor);
          android.webkit.WebViewProviderInfo[] _result = this.getAllWebViewPackages();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getCurrentWebViewPackageName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getCurrentWebViewPackageName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCurrentWebViewPackage:
        {
          data.enforceInterface(descriptor);
          android.content.pm.PackageInfo _result = this.getCurrentWebViewPackage();
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
        case TRANSACTION_isMultiProcessEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isMultiProcessEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableMultiProcess:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.enableMultiProcess(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.webkit.IWebViewUpdateService
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
           * Used by the relro file creator to notify the service that it's done.
           */
      @Override public void notifyRelroCreationCompleted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyRelroCreationCompleted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyRelroCreationCompleted();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Used by WebViewFactory to block loading of WebView code until
           * preparations are complete. Returns the package used as WebView provider.
           */
      @Override public android.webkit.WebViewProviderResponse waitForAndGetProvider() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.webkit.WebViewProviderResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_waitForAndGetProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().waitForAndGetProvider();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.webkit.WebViewProviderResponse.CREATOR.createFromParcel(_reply);
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
      /**
           * DevelopmentSettings uses this to notify WebViewUpdateService that a new provider has been
           * selected by the user. Returns the provider we end up switching to, this could be different to
           * the one passed as argument to this method since the Dev Setting calling this method could be
           * stale. I.e. the Dev setting could be letting the user choose uninstalled/disabled packages,
           * it would then try to update the provider to such a package while in reality the update
           * service would switch to another one.
           */
      @Override public java.lang.String changeProviderAndSetting(java.lang.String newProvider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(newProvider);
          boolean _status = mRemote.transact(Stub.TRANSACTION_changeProviderAndSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().changeProviderAndSetting(newProvider);
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
      /**
           * DevelopmentSettings uses this to get the current available WebView
           * providers (to display as choices to the user).
           */
      @Override public android.webkit.WebViewProviderInfo[] getValidWebViewPackages() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.webkit.WebViewProviderInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getValidWebViewPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getValidWebViewPackages();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.webkit.WebViewProviderInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Fetch all packages that could potentially implement WebView.
           */
      @Override public android.webkit.WebViewProviderInfo[] getAllWebViewPackages() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.webkit.WebViewProviderInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllWebViewPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllWebViewPackages();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.webkit.WebViewProviderInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Used by DevelopmentSetting to get the name of the WebView provider currently in use.
           */
      @Override public java.lang.String getCurrentWebViewPackageName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentWebViewPackageName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentWebViewPackageName();
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
      /**
           * Used by public API for debugging purposes.
           */
      @Override public android.content.pm.PackageInfo getCurrentWebViewPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.PackageInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentWebViewPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentWebViewPackage();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.PackageInfo.CREATOR.createFromParcel(_reply);
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
      /**
           * Used by Settings to determine whether multiprocess is enabled.
           */
      @Override public boolean isMultiProcessEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMultiProcessEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMultiProcessEnabled();
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
      /**
           * Used by Settings to enable/disable multiprocess.
           */
      @Override public void enableMultiProcess(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableMultiProcess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableMultiProcess(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.webkit.IWebViewUpdateService sDefaultImpl;
    }
    static final int TRANSACTION_notifyRelroCreationCompleted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_waitForAndGetProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_changeProviderAndSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getValidWebViewPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getAllWebViewPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getCurrentWebViewPackageName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getCurrentWebViewPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isMultiProcessEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_enableMultiProcess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.webkit.IWebViewUpdateService impl) {
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
    public static android.webkit.IWebViewUpdateService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Used by the relro file creator to notify the service that it's done.
       */
  public void notifyRelroCreationCompleted() throws android.os.RemoteException;
  /**
       * Used by WebViewFactory to block loading of WebView code until
       * preparations are complete. Returns the package used as WebView provider.
       */
  public android.webkit.WebViewProviderResponse waitForAndGetProvider() throws android.os.RemoteException;
  /**
       * DevelopmentSettings uses this to notify WebViewUpdateService that a new provider has been
       * selected by the user. Returns the provider we end up switching to, this could be different to
       * the one passed as argument to this method since the Dev Setting calling this method could be
       * stale. I.e. the Dev setting could be letting the user choose uninstalled/disabled packages,
       * it would then try to update the provider to such a package while in reality the update
       * service would switch to another one.
       */
  public java.lang.String changeProviderAndSetting(java.lang.String newProvider) throws android.os.RemoteException;
  /**
       * DevelopmentSettings uses this to get the current available WebView
       * providers (to display as choices to the user).
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/webkit/IWebViewUpdateService.aidl:54:1:54:25")
  public android.webkit.WebViewProviderInfo[] getValidWebViewPackages() throws android.os.RemoteException;
  /**
       * Fetch all packages that could potentially implement WebView.
       */
  public android.webkit.WebViewProviderInfo[] getAllWebViewPackages() throws android.os.RemoteException;
  /**
       * Used by DevelopmentSetting to get the name of the WebView provider currently in use.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/webkit/IWebViewUpdateService.aidl:65:1:65:25")
  public java.lang.String getCurrentWebViewPackageName() throws android.os.RemoteException;
  /**
       * Used by public API for debugging purposes.
       */
  public android.content.pm.PackageInfo getCurrentWebViewPackage() throws android.os.RemoteException;
  /**
       * Used by Settings to determine whether multiprocess is enabled.
       */
  public boolean isMultiProcessEnabled() throws android.os.RemoteException;
  /**
       * Used by Settings to enable/disable multiprocess.
       */
  public void enableMultiProcess(boolean enable) throws android.os.RemoteException;
}
