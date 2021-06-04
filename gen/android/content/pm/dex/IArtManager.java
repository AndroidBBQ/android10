/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm.dex;
/**
 * A system service that provides access to runtime and compiler artifacts.
 *
 * @hide
 */
public interface IArtManager extends android.os.IInterface
{
  /** Default implementation for IArtManager. */
  public static class Default implements android.content.pm.dex.IArtManager
  {
    /**
         * Snapshots a runtime profile according to the {@code profileType} parameter.
         *
         * If {@code profileType} is {@link ArtManager#PROFILE_APPS} the method will snapshot
         * the profile for for an apk belonging to the package {@code packageName}.
         * The apk is identified by {@code codePath}.
         *
         * If {@code profileType} is {@code ArtManager.PROFILE_BOOT_IMAGE} the method will snapshot
         * the profile for the boot image. In this case {@code codePath can be null}. The parameters
         * {@code packageName} and {@code codePath} are ignored.
         *
         * The calling process must have {@code android.permission.READ_RUNTIME_PROFILE} permission.
         *
         * The result will be posted on the {@code executor} using the given {@code callback}.
         * The profile will be available as a read-only {@link android.os.ParcelFileDescriptor}.
         *
         * This method will throw {@link IllegalStateException} if
         * {@link ArtManager#isRuntimeProfilingEnabled(int)} does not return true for the given
         * {@code profileType}.
         */
    @Override public void snapshotRuntimeProfile(int profileType, java.lang.String packageName, java.lang.String codePath, android.content.pm.dex.ISnapshotRuntimeProfileCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
           * Returns true if runtime profiles are enabled for the given type, false otherwise.
           * The type can be can be either {@code ArtManager.PROFILE_APPS}
           * or {@code ArtManager.PROFILE_BOOT_IMAGE}.
           *
           * @param profileType
           */
    @Override public boolean isRuntimeProfilingEnabled(int profileType, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.dex.IArtManager
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.dex.IArtManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.dex.IArtManager interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.dex.IArtManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.dex.IArtManager))) {
        return ((android.content.pm.dex.IArtManager)iin);
      }
      return new android.content.pm.dex.IArtManager.Stub.Proxy(obj);
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
        case TRANSACTION_snapshotRuntimeProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.pm.dex.ISnapshotRuntimeProfileCallback _arg3;
          _arg3 = android.content.pm.dex.ISnapshotRuntimeProfileCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.snapshotRuntimeProfile(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isRuntimeProfilingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isRuntimeProfilingEnabled(_arg0, _arg1);
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
    private static class Proxy implements android.content.pm.dex.IArtManager
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
           * Snapshots a runtime profile according to the {@code profileType} parameter.
           *
           * If {@code profileType} is {@link ArtManager#PROFILE_APPS} the method will snapshot
           * the profile for for an apk belonging to the package {@code packageName}.
           * The apk is identified by {@code codePath}.
           *
           * If {@code profileType} is {@code ArtManager.PROFILE_BOOT_IMAGE} the method will snapshot
           * the profile for the boot image. In this case {@code codePath can be null}. The parameters
           * {@code packageName} and {@code codePath} are ignored.
           *
           * The calling process must have {@code android.permission.READ_RUNTIME_PROFILE} permission.
           *
           * The result will be posted on the {@code executor} using the given {@code callback}.
           * The profile will be available as a read-only {@link android.os.ParcelFileDescriptor}.
           *
           * This method will throw {@link IllegalStateException} if
           * {@link ArtManager#isRuntimeProfilingEnabled(int)} does not return true for the given
           * {@code profileType}.
           */
      @Override public void snapshotRuntimeProfile(int profileType, java.lang.String packageName, java.lang.String codePath, android.content.pm.dex.ISnapshotRuntimeProfileCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profileType);
          _data.writeString(packageName);
          _data.writeString(codePath);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_snapshotRuntimeProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().snapshotRuntimeProfile(profileType, packageName, codePath, callback, callingPackage);
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
             * Returns true if runtime profiles are enabled for the given type, false otherwise.
             * The type can be can be either {@code ArtManager.PROFILE_APPS}
             * or {@code ArtManager.PROFILE_BOOT_IMAGE}.
             *
             * @param profileType
             */
      @Override public boolean isRuntimeProfilingEnabled(int profileType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profileType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRuntimeProfilingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRuntimeProfilingEnabled(profileType, callingPackage);
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
      public static android.content.pm.dex.IArtManager sDefaultImpl;
    }
    static final int TRANSACTION_snapshotRuntimeProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isRuntimeProfilingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.content.pm.dex.IArtManager impl) {
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
    public static android.content.pm.dex.IArtManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Snapshots a runtime profile according to the {@code profileType} parameter.
       *
       * If {@code profileType} is {@link ArtManager#PROFILE_APPS} the method will snapshot
       * the profile for for an apk belonging to the package {@code packageName}.
       * The apk is identified by {@code codePath}.
       *
       * If {@code profileType} is {@code ArtManager.PROFILE_BOOT_IMAGE} the method will snapshot
       * the profile for the boot image. In this case {@code codePath can be null}. The parameters
       * {@code packageName} and {@code codePath} are ignored.
       *
       * The calling process must have {@code android.permission.READ_RUNTIME_PROFILE} permission.
       *
       * The result will be posted on the {@code executor} using the given {@code callback}.
       * The profile will be available as a read-only {@link android.os.ParcelFileDescriptor}.
       *
       * This method will throw {@link IllegalStateException} if
       * {@link ArtManager#isRuntimeProfilingEnabled(int)} does not return true for the given
       * {@code profileType}.
       */
  public void snapshotRuntimeProfile(int profileType, java.lang.String packageName, java.lang.String codePath, android.content.pm.dex.ISnapshotRuntimeProfileCallback callback, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
         * Returns true if runtime profiles are enabled for the given type, false otherwise.
         * The type can be can be either {@code ArtManager.PROFILE_APPS}
         * or {@code ArtManager.PROFILE_BOOT_IMAGE}.
         *
         * @param profileType
         */
  public boolean isRuntimeProfilingEnabled(int profileType, java.lang.String callingPackage) throws android.os.RemoteException;
}
