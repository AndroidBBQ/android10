/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * {@hide}
 */
public interface IOnAppsChangedListener extends android.os.IInterface
{
  /** Default implementation for IOnAppsChangedListener. */
  public static class Default implements android.content.pm.IOnAppsChangedListener
  {
    @Override public void onPackageRemoved(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void onPackageAdded(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void onPackageChanged(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void onPackagesAvailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException
    {
    }
    @Override public void onPackagesUnavailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException
    {
    }
    @Override public void onPackagesSuspended(android.os.UserHandle user, java.lang.String[] packageNames, android.os.Bundle launcherExtras) throws android.os.RemoteException
    {
    }
    @Override public void onPackagesUnsuspended(android.os.UserHandle user, java.lang.String[] packageNames) throws android.os.RemoteException
    {
    }
    @Override public void onShortcutChanged(android.os.UserHandle user, java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IOnAppsChangedListener
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IOnAppsChangedListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IOnAppsChangedListener interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IOnAppsChangedListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IOnAppsChangedListener))) {
        return ((android.content.pm.IOnAppsChangedListener)iin);
      }
      return new android.content.pm.IOnAppsChangedListener.Stub.Proxy(obj);
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
        case TRANSACTION_onPackageRemoved:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onPackageRemoved(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPackageAdded:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onPackageAdded(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPackageChanged:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onPackageChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPackagesAvailable:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onPackagesAvailable(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onPackagesUnavailable:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onPackagesUnavailable(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onPackagesSuspended:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onPackagesSuspended(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onPackagesUnsuspended:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.onPackagesUnsuspended(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onShortcutChanged:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
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
          this.onShortcutChanged(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.IOnAppsChangedListener
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
      @Override public void onPackageRemoved(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackageRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackageRemoved(user, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackageAdded(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackageAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackageAdded(user, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackageChanged(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackageChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackageChanged(user, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackagesAvailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(packageNames);
          _data.writeInt(((replacing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackagesAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackagesAvailable(user, packageNames, replacing);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackagesUnavailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(packageNames);
          _data.writeInt(((replacing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackagesUnavailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackagesUnavailable(user, packageNames, replacing);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackagesSuspended(android.os.UserHandle user, java.lang.String[] packageNames, android.os.Bundle launcherExtras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(packageNames);
          if ((launcherExtras!=null)) {
            _data.writeInt(1);
            launcherExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackagesSuspended, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackagesSuspended(user, packageNames, launcherExtras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPackagesUnsuspended(android.os.UserHandle user, java.lang.String[] packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPackagesUnsuspended, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPackagesUnsuspended(user, packageNames);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onShortcutChanged(android.os.UserHandle user, java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          if ((shortcuts!=null)) {
            _data.writeInt(1);
            shortcuts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShortcutChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShortcutChanged(user, packageName, shortcuts);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.pm.IOnAppsChangedListener sDefaultImpl;
    }
    static final int TRANSACTION_onPackageRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onPackageAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onPackageChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onPackagesAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onPackagesUnavailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onPackagesSuspended = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onPackagesUnsuspended = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onShortcutChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.content.pm.IOnAppsChangedListener impl) {
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
    public static android.content.pm.IOnAppsChangedListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onPackageRemoved(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException;
  public void onPackageAdded(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException;
  public void onPackageChanged(android.os.UserHandle user, java.lang.String packageName) throws android.os.RemoteException;
  public void onPackagesAvailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException;
  public void onPackagesUnavailable(android.os.UserHandle user, java.lang.String[] packageNames, boolean replacing) throws android.os.RemoteException;
  public void onPackagesSuspended(android.os.UserHandle user, java.lang.String[] packageNames, android.os.Bundle launcherExtras) throws android.os.RemoteException;
  public void onPackagesUnsuspended(android.os.UserHandle user, java.lang.String[] packageNames) throws android.os.RemoteException;
  public void onShortcutChanged(android.os.UserHandle user, java.lang.String packageName, android.content.pm.ParceledListSlice shortcuts) throws android.os.RemoteException;
}
