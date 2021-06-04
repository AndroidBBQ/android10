/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * API for package data change related callbacks from the Package Manager.
 * Some usage scenarios include deletion of cache directory, generate
 * statistics related to code, data, cache usage(TODO)
 * {@hide}
 */
public interface IPackageStatsObserver extends android.os.IInterface
{
  /** Default implementation for IPackageStatsObserver. */
  public static class Default implements android.content.pm.IPackageStatsObserver
  {
    @Override public void onGetStatsCompleted(android.content.pm.PackageStats pStats, boolean succeeded) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.IPackageStatsObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.IPackageStatsObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.IPackageStatsObserver interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.IPackageStatsObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.IPackageStatsObserver))) {
        return ((android.content.pm.IPackageStatsObserver)iin);
      }
      return new android.content.pm.IPackageStatsObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onGetStatsCompleted:
        {
          data.enforceInterface(descriptor);
          android.content.pm.PackageStats _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.PackageStats.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onGetStatsCompleted(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.IPackageStatsObserver
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
      @Override public void onGetStatsCompleted(android.content.pm.PackageStats pStats, boolean succeeded) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pStats!=null)) {
            _data.writeInt(1);
            pStats.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((succeeded)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetStatsCompleted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetStatsCompleted(pStats, succeeded);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.pm.IPackageStatsObserver sDefaultImpl;
    }
    static final int TRANSACTION_onGetStatsCompleted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.content.pm.IPackageStatsObserver impl) {
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
    public static android.content.pm.IPackageStatsObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/pm/IPackageStatsObserver.aidl:29:1:29:25")
  public void onGetStatsCompleted(android.content.pm.PackageStats pStats, boolean succeeded) throws android.os.RemoteException;
}
