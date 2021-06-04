/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** @hide */
public interface ISearchManager extends android.os.IInterface
{
  /** Default implementation for ISearchManager. */
  public static class Default implements android.app.ISearchManager
  {
    @Override public android.app.SearchableInfo getSearchableInfo(android.content.ComponentName launchActivity) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.app.SearchableInfo> getSearchablesInGlobalSearch() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.content.pm.ResolveInfo> getGlobalSearchActivities() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.ComponentName getGlobalSearchActivity() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.ComponentName getWebSearchActivity() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void launchAssist(android.os.Bundle args) throws android.os.RemoteException
    {
    }
    @Override public boolean launchLegacyAssist(java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.ISearchManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.ISearchManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.ISearchManager interface,
     * generating a proxy if needed.
     */
    public static android.app.ISearchManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.ISearchManager))) {
        return ((android.app.ISearchManager)iin);
      }
      return new android.app.ISearchManager.Stub.Proxy(obj);
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
        case TRANSACTION_getSearchableInfo:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.SearchableInfo _result = this.getSearchableInfo(_arg0);
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
        case TRANSACTION_getSearchablesInGlobalSearch:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.app.SearchableInfo> _result = this.getSearchablesInGlobalSearch();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getGlobalSearchActivities:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.content.pm.ResolveInfo> _result = this.getGlobalSearchActivities();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getGlobalSearchActivity:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _result = this.getGlobalSearchActivity();
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
        case TRANSACTION_getWebSearchActivity:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _result = this.getWebSearchActivity();
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
        case TRANSACTION_launchAssist:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.launchAssist(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_launchLegacyAssist:
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
          boolean _result = this.launchLegacyAssist(_arg0, _arg1, _arg2);
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
    private static class Proxy implements android.app.ISearchManager
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
      @Override public android.app.SearchableInfo getSearchableInfo(android.content.ComponentName launchActivity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.SearchableInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((launchActivity!=null)) {
            _data.writeInt(1);
            launchActivity.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSearchableInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSearchableInfo(launchActivity);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.SearchableInfo.CREATOR.createFromParcel(_reply);
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
      @Override public java.util.List<android.app.SearchableInfo> getSearchablesInGlobalSearch() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.SearchableInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSearchablesInGlobalSearch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSearchablesInGlobalSearch();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.SearchableInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.content.pm.ResolveInfo> getGlobalSearchActivities() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.content.pm.ResolveInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGlobalSearchActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGlobalSearchActivities();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.content.pm.ResolveInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.ComponentName getGlobalSearchActivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGlobalSearchActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGlobalSearchActivity();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.ComponentName.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.ComponentName getWebSearchActivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWebSearchActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWebSearchActivity();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.ComponentName.CREATOR.createFromParcel(_reply);
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
      @Override public void launchAssist(android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchAssist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().launchAssist(args);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean launchLegacyAssist(java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(hint);
          _data.writeInt(userHandle);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchLegacyAssist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().launchLegacyAssist(hint, userHandle, args);
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
      public static android.app.ISearchManager sDefaultImpl;
    }
    static final int TRANSACTION_getSearchableInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getSearchablesInGlobalSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getGlobalSearchActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getGlobalSearchActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getWebSearchActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_launchAssist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_launchLegacyAssist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.app.ISearchManager impl) {
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
    public static android.app.ISearchManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.app.SearchableInfo getSearchableInfo(android.content.ComponentName launchActivity) throws android.os.RemoteException;
  public java.util.List<android.app.SearchableInfo> getSearchablesInGlobalSearch() throws android.os.RemoteException;
  public java.util.List<android.content.pm.ResolveInfo> getGlobalSearchActivities() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/ISearchManager.aidl:31:1:31:24")
  public android.content.ComponentName getGlobalSearchActivity() throws android.os.RemoteException;
  public android.content.ComponentName getWebSearchActivity() throws android.os.RemoteException;
  public void launchAssist(android.os.Bundle args) throws android.os.RemoteException;
  public boolean launchLegacyAssist(java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException;
}
