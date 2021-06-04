/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * {@hide}
 */
public interface IShortcutService extends android.os.IInterface
{
  /** Default implementation for IShortcutService. */
  public static class Default implements android.content.pm.IShortcutService
  {
    @Override public boolean setDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.pm.ParceledListSlice getDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getManifestShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean addDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeDynamicShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeAllDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getPinnedShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean updateShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean requestPinShortcut(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, android.content.IntentSender resultIntent, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.Intent createShortcutResultIntent(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void disableShortcuts(java.lang.String packageName, java.util.List shortcutIds, java.lang.CharSequence disabledMessage, int disabledMessageResId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void enableShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException
    {
    }
    @Override public int getMaxShortcutCountPerActivity(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getRemainingCallCount(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public long getRateLimitResetTime(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public int getIconMaxDimensions(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void reportShortcutUsed(java.lang.String packageName, java.lang.String shortcutId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void resetThrottling() throws android.os.RemoteException
    {
    }
    // system only API for developer opsions

    @Override public void onApplicationActive(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    // system only API for sysUI

    @Override public byte[] getBackupPayload(int user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void applyRestore(byte[] payload, int user) throws android.os.RemoteException
    {
    }
    @Override public boolean isRequestPinItemSupported(int user, int requestType) throws android.os.RemoteException
    {
      return false;
    }
    // System API used by framework's ShareSheet (ChooserActivity)

    @Override public android.content.pm.ParceledListSlice getShareTargets(java.lang.String packageName, android.content.IntentFilter filter, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean hasShareTargets(java.lang.String packageName, java.lang.String packageToCheck, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IShortcutService
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IShortcutService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IShortcutService interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IShortcutService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IShortcutService))) {
        return ((android.content.pm.IShortcutService)iin);
      }
      return new android.content.pm.IShortcutService.Stub.Proxy(obj);
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
        case TRANSACTION_setDynamicShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setDynamicShortcuts(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getDynamicShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getDynamicShortcuts(_arg0, _arg1);
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
        case TRANSACTION_getManifestShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getManifestShortcuts(_arg0, _arg1);
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
        case TRANSACTION_addDynamicShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.addDynamicShortcuts(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeDynamicShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List _arg1;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg1 = data.readArrayList(cl);
          int _arg2;
          _arg2 = data.readInt();
          this.removeDynamicShortcuts(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAllDynamicShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.removeAllDynamicShortcuts(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPinnedShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getPinnedShortcuts(_arg0, _arg1);
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
        case TRANSACTION_updateShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.updateShortcuts(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestPinShortcut:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ShortcutInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ShortcutInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.content.IntentSender _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.IntentSender.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.requestPinShortcut(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createShortcutResultIntent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ShortcutInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ShortcutInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.content.Intent _result = this.createShortcutResultIntent(_arg0, _arg1, _arg2);
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
        case TRANSACTION_disableShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List _arg1;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg1 = data.readArrayList(cl);
          java.lang.CharSequence _arg2;
          if (0!=data.readInt()) {
            _arg2 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.disableShortcuts(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List _arg1;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg1 = data.readArrayList(cl);
          int _arg2;
          _arg2 = data.readInt();
          this.enableShortcuts(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMaxShortcutCountPerActivity:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getMaxShortcutCountPerActivity(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getRemainingCallCount:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getRemainingCallCount(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getRateLimitResetTime:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _result = this.getRateLimitResetTime(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getIconMaxDimensions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getIconMaxDimensions(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_reportShortcutUsed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.reportShortcutUsed(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resetThrottling:
        {
          data.enforceInterface(descriptor);
          this.resetThrottling();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onApplicationActive:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onApplicationActive(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getBackupPayload:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _result = this.getBackupPayload(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_applyRestore:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          this.applyRestore(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isRequestPinItemSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isRequestPinItemSupported(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getShareTargets:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.IntentFilter _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.IntentFilter.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getShareTargets(_arg0, _arg1, _arg2);
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
        case TRANSACTION_hasShareTargets:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.hasShareTargets(_arg0, _arg1, _arg2);
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
    private static class Proxy implements android.content.pm.IShortcutService
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
      @Override public boolean setDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((shortcutInfoList!=null)) {
            _data.writeInt(1);
            shortcutInfoList.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDynamicShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDynamicShortcuts(packageName, shortcutInfoList, userId);
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
      @Override public android.content.pm.ParceledListSlice getDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDynamicShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDynamicShortcuts(packageName, userId);
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
      @Override public android.content.pm.ParceledListSlice getManifestShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getManifestShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getManifestShortcuts(packageName, userId);
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
      @Override public boolean addDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((shortcutInfoList!=null)) {
            _data.writeInt(1);
            shortcutInfoList.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addDynamicShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addDynamicShortcuts(packageName, shortcutInfoList, userId);
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
      @Override public void removeDynamicShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeList(shortcutIds);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeDynamicShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeDynamicShortcuts(packageName, shortcutIds, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeAllDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAllDynamicShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAllDynamicShortcuts(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getPinnedShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPinnedShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPinnedShortcuts(packageName, userId);
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
      @Override public boolean updateShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((shortcuts!=null)) {
            _data.writeInt(1);
            shortcuts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateShortcuts(packageName, shortcuts, userId);
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
      @Override public boolean requestPinShortcut(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, android.content.IntentSender resultIntent, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((shortcut!=null)) {
            _data.writeInt(1);
            shortcut.writeToParcel(_data, 0);
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
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestPinShortcut, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestPinShortcut(packageName, shortcut, resultIntent, userId);
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
      @Override public android.content.Intent createShortcutResultIntent(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((shortcut!=null)) {
            _data.writeInt(1);
            shortcut.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createShortcutResultIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createShortcutResultIntent(packageName, shortcut, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.Intent.CREATOR.createFromParcel(_reply);
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
      @Override public void disableShortcuts(java.lang.String packageName, java.util.List shortcutIds, java.lang.CharSequence disabledMessage, int disabledMessageResId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeList(shortcutIds);
          if (disabledMessage!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(disabledMessage, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(disabledMessageResId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableShortcuts(packageName, shortcutIds, disabledMessage, disabledMessageResId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void enableShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeList(shortcutIds);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableShortcuts(packageName, shortcutIds, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMaxShortcutCountPerActivity(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMaxShortcutCountPerActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMaxShortcutCountPerActivity(packageName, userId);
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
      @Override public int getRemainingCallCount(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRemainingCallCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRemainingCallCount(packageName, userId);
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
      @Override public long getRateLimitResetTime(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRateLimitResetTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRateLimitResetTime(packageName, userId);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getIconMaxDimensions(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIconMaxDimensions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIconMaxDimensions(packageName, userId);
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
      @Override public void reportShortcutUsed(java.lang.String packageName, java.lang.String shortcutId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(shortcutId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportShortcutUsed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportShortcutUsed(packageName, shortcutId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resetThrottling() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetThrottling, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetThrottling();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // system only API for developer opsions

      @Override public void onApplicationActive(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onApplicationActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onApplicationActive(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // system only API for sysUI

      @Override public byte[] getBackupPayload(int user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(user);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBackupPayload, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBackupPayload(user);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void applyRestore(byte[] payload, int user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(payload);
          _data.writeInt(user);
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyRestore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyRestore(payload, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isRequestPinItemSupported(int user, int requestType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(user);
          _data.writeInt(requestType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRequestPinItemSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRequestPinItemSupported(user, requestType);
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
      // System API used by framework's ShareSheet (ChooserActivity)

      @Override public android.content.pm.ParceledListSlice getShareTargets(java.lang.String packageName, android.content.IntentFilter filter, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((filter!=null)) {
            _data.writeInt(1);
            filter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShareTargets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShareTargets(packageName, filter, userId);
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
      @Override public boolean hasShareTargets(java.lang.String packageName, java.lang.String packageToCheck, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(packageToCheck);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasShareTargets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasShareTargets(packageName, packageToCheck, userId);
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
      public static android.content.pm.IShortcutService sDefaultImpl;
    }
    static final int TRANSACTION_setDynamicShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getDynamicShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getManifestShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_addDynamicShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_removeDynamicShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeAllDynamicShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getPinnedShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_updateShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_requestPinShortcut = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_createShortcutResultIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_disableShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_enableShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getMaxShortcutCountPerActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getRemainingCallCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getRateLimitResetTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getIconMaxDimensions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_reportShortcutUsed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_resetThrottling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_onApplicationActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getBackupPayload = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_applyRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_isRequestPinItemSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getShareTargets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_hasShareTargets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    public static boolean setDefaultImpl(android.content.pm.IShortcutService impl) {
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
    public static android.content.pm.IShortcutService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean setDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getManifestShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public boolean addDynamicShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcutInfoList, int userId) throws android.os.RemoteException;
  public void removeDynamicShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException;
  public void removeAllDynamicShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getPinnedShortcuts(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public boolean updateShortcuts(java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts, int userId) throws android.os.RemoteException;
  public boolean requestPinShortcut(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, android.content.IntentSender resultIntent, int userId) throws android.os.RemoteException;
  public android.content.Intent createShortcutResultIntent(java.lang.String packageName, android.content.pm.ShortcutInfo shortcut, int userId) throws android.os.RemoteException;
  public void disableShortcuts(java.lang.String packageName, java.util.List shortcutIds, java.lang.CharSequence disabledMessage, int disabledMessageResId, int userId) throws android.os.RemoteException;
  public void enableShortcuts(java.lang.String packageName, java.util.List shortcutIds, int userId) throws android.os.RemoteException;
  public int getMaxShortcutCountPerActivity(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public int getRemainingCallCount(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public long getRateLimitResetTime(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public int getIconMaxDimensions(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public void reportShortcutUsed(java.lang.String packageName, java.lang.String shortcutId, int userId) throws android.os.RemoteException;
  public void resetThrottling() throws android.os.RemoteException;
  // system only API for developer opsions

  public void onApplicationActive(java.lang.String packageName, int userId) throws android.os.RemoteException;
  // system only API for sysUI

  public byte[] getBackupPayload(int user) throws android.os.RemoteException;
  public void applyRestore(byte[] payload, int user) throws android.os.RemoteException;
  public boolean isRequestPinItemSupported(int user, int requestType) throws android.os.RemoteException;
  // System API used by framework's ShareSheet (ChooserActivity)

  public android.content.pm.ParceledListSlice getShareTargets(java.lang.String packageName, android.content.IntentFilter filter, int userId) throws android.os.RemoteException;
  public boolean hasShareTargets(java.lang.String packageName, java.lang.String packageToCheck, int userId) throws android.os.RemoteException;
}
