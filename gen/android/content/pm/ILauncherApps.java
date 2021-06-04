/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * {@hide}
 */
public interface ILauncherApps extends android.os.IInterface
{
  /** Default implementation for ILauncherApps. */
  public static class Default implements android.content.pm.ILauncherApps
  {
    @Override public void addOnAppsChangedListener(java.lang.String callingPackage, android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeOnAppsChangedListener(android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getLauncherActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ActivityInfo resolveActivity(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void startSessionDetailsActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.pm.PackageInstaller.SessionInfo sessionInfo, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
    {
    }
    @Override public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
    {
    }
    @Override public void showAppDetailsAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
    {
    }
    @Override public boolean isPackageEnabled(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.os.Bundle getSuspendedPackageLauncherExtras(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isActivityEnabled(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.pm.ApplicationInfo getApplicationInfo(java.lang.String callingPackage, java.lang.String packageName, int flags, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.LauncherApps.AppUsageLimit getAppUsageLimit(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getShortcuts(java.lang.String callingPackage, long changedSince, java.lang.String packageName, java.util.List shortcutIds, android.content.ComponentName componentName, int flags, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void pinShortcuts(java.lang.String callingPackage, java.lang.String packageName, java.util.List<java.lang.String> shortcutIds, android.os.UserHandle user) throws android.os.RemoteException
    {
    }
    @Override public boolean startShortcut(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, android.graphics.Rect sourceBounds, android.os.Bundle startActivityOptions, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getShortcutIconResId(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.os.ParcelFileDescriptor getShortcutIconFd(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean hasShortcutHostPermission(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean shouldHideFromSuggestions(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.pm.ParceledListSlice getShortcutConfigActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.IntentSender getShortcutConfigActivityIntent(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
    {
      return null;
    }
    // Unregister is performed using package installer

    @Override public void registerPackageInstallerCallback(java.lang.String callingPackage, android.content.pm.IPackageInstallerCallback callback) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getAllSessions(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.ILauncherApps
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.ILauncherApps";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.ILauncherApps interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.ILauncherApps asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.ILauncherApps))) {
        return ((android.content.pm.ILauncherApps)iin);
      }
      return new android.content.pm.ILauncherApps.Stub.Proxy(obj);
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
        case TRANSACTION_addOnAppsChangedListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.IOnAppsChangedListener _arg1;
          _arg1 = android.content.pm.IOnAppsChangedListener.Stub.asInterface(data.readStrongBinder());
          this.addOnAppsChangedListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOnAppsChangedListener:
        {
          data.enforceInterface(descriptor);
          android.content.pm.IOnAppsChangedListener _arg0;
          _arg0 = android.content.pm.IOnAppsChangedListener.Stub.asInterface(data.readStrongBinder());
          this.removeOnAppsChangedListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLauncherActivities:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.pm.ParceledListSlice _result = this.getLauncherActivities(_arg0, _arg1, _arg2);
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
        case TRANSACTION_resolveActivity:
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
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.pm.ActivityInfo _result = this.resolveActivity(_arg0, _arg1, _arg2);
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
        case TRANSACTION_startSessionDetailsActivityAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.pm.PackageInstaller.SessionInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.pm.PackageInstaller.SessionInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          android.os.UserHandle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.startSessionDetailsActivityAsUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startActivityAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          android.os.UserHandle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.startActivityAsUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showAppDetailsAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          android.os.UserHandle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.showAppDetailsAsUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isPackageEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.isPackageEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSuspendedPackageLauncherExtras:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.UserHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _result = this.getSuspendedPackageLauncherExtras(_arg0, _arg1);
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
        case TRANSACTION_isActivityEnabled:
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
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.isActivityEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getApplicationInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.os.UserHandle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.content.pm.ApplicationInfo _result = this.getApplicationInfo(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_getAppUsageLimit:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.pm.LauncherApps.AppUsageLimit _result = this.getAppUsageLimit(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.util.List _arg3;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg3 = data.readArrayList(cl);
          android.content.ComponentName _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          int _arg5;
          _arg5 = data.readInt();
          android.os.UserHandle _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          android.content.pm.ParceledListSlice _result = this.getShortcuts(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
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
        case TRANSACTION_pinShortcuts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<java.lang.String> _arg2;
          _arg2 = data.createStringArrayList();
          android.os.UserHandle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.pinShortcuts(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startShortcut:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          int _arg5;
          _arg5 = data.readInt();
          boolean _result = this.startShortcut(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getShortcutIconResId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.getShortcutIconResId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getShortcutIconFd:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.os.ParcelFileDescriptor _result = this.getShortcutIconFd(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_hasShortcutHostPermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.hasShortcutHostPermission(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_shouldHideFromSuggestions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.UserHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.shouldHideFromSuggestions(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getShortcutConfigActivities:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.pm.ParceledListSlice _result = this.getShortcutConfigActivities(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getShortcutConfigActivityIntent:
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
          android.os.UserHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.IntentSender _result = this.getShortcutConfigActivityIntent(_arg0, _arg1, _arg2);
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
        case TRANSACTION_registerPackageInstallerCallback:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.IPackageInstallerCallback _arg1;
          _arg1 = android.content.pm.IPackageInstallerCallback.Stub.asInterface(data.readStrongBinder());
          this.registerPackageInstallerCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAllSessions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ParceledListSlice _result = this.getAllSessions(_arg0);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.ILauncherApps
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
      @Override public void addOnAppsChangedListener(java.lang.String callingPackage, android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnAppsChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnAppsChangedListener(callingPackage, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOnAppsChangedListener(android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOnAppsChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOnAppsChangedListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getLauncherActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLauncherActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLauncherActivities(callingPackage, packageName, user);
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
      @Override public android.content.pm.ActivityInfo resolveActivity(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ActivityInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resolveActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().resolveActivity(callingPackage, component, user);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ActivityInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void startSessionDetailsActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.pm.PackageInstaller.SessionInfo sessionInfo, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sourceBounds!=null)) {
            _data.writeInt(1);
            sourceBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((opts!=null)) {
            _data.writeInt(1);
            opts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSessionDetailsActivityAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSessionDetailsActivityAsUser(caller, callingPackage, sessionInfo, sourceBounds, opts, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sourceBounds!=null)) {
            _data.writeInt(1);
            sourceBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((opts!=null)) {
            _data.writeInt(1);
            opts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startActivityAsUser(caller, callingPackage, component, sourceBounds, opts, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showAppDetailsAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sourceBounds!=null)) {
            _data.writeInt(1);
            sourceBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((opts!=null)) {
            _data.writeInt(1);
            opts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_showAppDetailsAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showAppDetailsAsUser(caller, callingPackage, component, sourceBounds, opts, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isPackageEnabled(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isPackageEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isPackageEnabled(callingPackage, packageName, user);
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
      @Override public android.os.Bundle getSuspendedPackageLauncherExtras(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSuspendedPackageLauncherExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSuspendedPackageLauncherExtras(packageName, user);
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
      @Override public boolean isActivityEnabled(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActivityEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActivityEnabled(callingPackage, component, user);
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
      @Override public android.content.pm.ApplicationInfo getApplicationInfo(java.lang.String callingPackage, java.lang.String packageName, int flags, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ApplicationInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          _data.writeInt(flags);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getApplicationInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getApplicationInfo(callingPackage, packageName, flags, user);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.pm.LauncherApps.AppUsageLimit getAppUsageLimit(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.LauncherApps.AppUsageLimit _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppUsageLimit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppUsageLimit(callingPackage, packageName, user);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.LauncherApps.AppUsageLimit.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.pm.ParceledListSlice getShortcuts(java.lang.String callingPackage, long changedSince, java.lang.String packageName, java.util.List shortcutIds, android.content.ComponentName componentName, int flags, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeLong(changedSince);
          _data.writeString(packageName);
          _data.writeList(shortcutIds);
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShortcuts(callingPackage, changedSince, packageName, shortcutIds, componentName, flags, user);
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
      @Override public void pinShortcuts(java.lang.String callingPackage, java.lang.String packageName, java.util.List<java.lang.String> shortcutIds, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          _data.writeStringList(shortcutIds);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_pinShortcuts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pinShortcuts(callingPackage, packageName, shortcutIds, user);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean startShortcut(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, android.graphics.Rect sourceBounds, android.os.Bundle startActivityOptions, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          _data.writeString(id);
          if ((sourceBounds!=null)) {
            _data.writeInt(1);
            sourceBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((startActivityOptions!=null)) {
            _data.writeInt(1);
            startActivityOptions.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startShortcut, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startShortcut(callingPackage, packageName, id, sourceBounds, startActivityOptions, userId);
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
      @Override public int getShortcutIconResId(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          _data.writeString(id);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShortcutIconResId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShortcutIconResId(callingPackage, packageName, id, userId);
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
      @Override public android.os.ParcelFileDescriptor getShortcutIconFd(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          _data.writeString(id);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShortcutIconFd, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShortcutIconFd(callingPackage, packageName, id, userId);
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
      @Override public boolean hasShortcutHostPermission(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasShortcutHostPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasShortcutHostPermission(callingPackage);
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
      @Override public boolean shouldHideFromSuggestions(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldHideFromSuggestions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldHideFromSuggestions(packageName, user);
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
      @Override public android.content.pm.ParceledListSlice getShortcutConfigActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShortcutConfigActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShortcutConfigActivities(callingPackage, packageName, user);
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
      @Override public android.content.IntentSender getShortcutConfigActivityIntent(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.IntentSender _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShortcutConfigActivityIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShortcutConfigActivityIntent(callingPackage, component, user);
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
      // Unregister is performed using package installer

      @Override public void registerPackageInstallerCallback(java.lang.String callingPackage, android.content.pm.IPackageInstallerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPackageInstallerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPackageInstallerCallback(callingPackage, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getAllSessions(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllSessions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllSessions(callingPackage);
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
      public static android.content.pm.ILauncherApps sDefaultImpl;
    }
    static final int TRANSACTION_addOnAppsChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeOnAppsChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getLauncherActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_resolveActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startSessionDetailsActivityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_startActivityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_showAppDetailsAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_isPackageEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getSuspendedPackageLauncherExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_isActivityEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getApplicationInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getAppUsageLimit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_pinShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_startShortcut = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getShortcutIconResId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getShortcutIconFd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_hasShortcutHostPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_shouldHideFromSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getShortcutConfigActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getShortcutConfigActivityIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_registerPackageInstallerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getAllSessions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    public static boolean setDefaultImpl(android.content.pm.ILauncherApps impl) {
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
    public static android.content.pm.ILauncherApps getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addOnAppsChangedListener(java.lang.String callingPackage, android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException;
  public void removeOnAppsChangedListener(android.content.pm.IOnAppsChangedListener listener) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getLauncherActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.pm.ActivityInfo resolveActivity(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException;
  public void startSessionDetailsActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.pm.PackageInstaller.SessionInfo sessionInfo, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException;
  public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException;
  public void showAppDetailsAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, android.graphics.Rect sourceBounds, android.os.Bundle opts, android.os.UserHandle user) throws android.os.RemoteException;
  public boolean isPackageEnabled(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public android.os.Bundle getSuspendedPackageLauncherExtras(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public boolean isActivityEnabled(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.pm.ApplicationInfo getApplicationInfo(java.lang.String callingPackage, java.lang.String packageName, int flags, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.pm.LauncherApps.AppUsageLimit getAppUsageLimit(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getShortcuts(java.lang.String callingPackage, long changedSince, java.lang.String packageName, java.util.List shortcutIds, android.content.ComponentName componentName, int flags, android.os.UserHandle user) throws android.os.RemoteException;
  public void pinShortcuts(java.lang.String callingPackage, java.lang.String packageName, java.util.List<java.lang.String> shortcutIds, android.os.UserHandle user) throws android.os.RemoteException;
  public boolean startShortcut(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, android.graphics.Rect sourceBounds, android.os.Bundle startActivityOptions, int userId) throws android.os.RemoteException;
  public int getShortcutIconResId(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor getShortcutIconFd(java.lang.String callingPackage, java.lang.String packageName, java.lang.String id, int userId) throws android.os.RemoteException;
  public boolean hasShortcutHostPermission(java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean shouldHideFromSuggestions(java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getShortcutConfigActivities(java.lang.String callingPackage, java.lang.String packageName, android.os.UserHandle user) throws android.os.RemoteException;
  public android.content.IntentSender getShortcutConfigActivityIntent(java.lang.String callingPackage, android.content.ComponentName component, android.os.UserHandle user) throws android.os.RemoteException;
  // Unregister is performed using package installer

  public void registerPackageInstallerCallback(java.lang.String callingPackage, android.content.pm.IPackageInstallerCallback callback) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getAllSessions(java.lang.String callingPackage) throws android.os.RemoteException;
}
