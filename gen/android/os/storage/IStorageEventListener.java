/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.storage;
/**
 * Callback class for receiving events from StorageManagerService.
 *
 * Don't change the existing transaction Ids as they could be used in the native code.
 * When adding a new method, assign the next available transaction id.
 *
 * @hide - Applications should use {@link android.os.storage.StorageEventListener} class for
 *         storage event callbacks.
 */
public interface IStorageEventListener extends android.os.IInterface
{
  /** Default implementation for IStorageEventListener. */
  public static class Default implements android.os.storage.IStorageEventListener
  {
    /**
         * Detection state of USB Mass Storage has changed
         *
         * @param available true if a UMS host is connected.
         */
    @Override public void onUsbMassStorageConnectionChanged(boolean connected) throws android.os.RemoteException
    {
    }
    /**
         * Storage state has changed.
         *
         * @param path The volume mount path.
         * @param oldState The old state of the volume.
         * @param newState The new state of the volume. Note: State is one of the
         *            values returned by Environment.getExternalStorageState()
         */
    @Override public void onStorageStateChanged(java.lang.String path, java.lang.String oldState, java.lang.String newState) throws android.os.RemoteException
    {
    }
    @Override public void onVolumeStateChanged(android.os.storage.VolumeInfo vol, int oldState, int newState) throws android.os.RemoteException
    {
    }
    @Override public void onVolumeRecordChanged(android.os.storage.VolumeRecord rec) throws android.os.RemoteException
    {
    }
    @Override public void onVolumeForgotten(java.lang.String fsUuid) throws android.os.RemoteException
    {
    }
    @Override public void onDiskScanned(android.os.storage.DiskInfo disk, int volumeCount) throws android.os.RemoteException
    {
    }
    @Override public void onDiskDestroyed(android.os.storage.DiskInfo disk) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.storage.IStorageEventListener
  {
    private static final java.lang.String DESCRIPTOR = "android.os.storage.IStorageEventListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.storage.IStorageEventListener interface,
     * generating a proxy if needed.
     */
    public static android.os.storage.IStorageEventListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.storage.IStorageEventListener))) {
        return ((android.os.storage.IStorageEventListener)iin);
      }
      return new android.os.storage.IStorageEventListener.Stub.Proxy(obj);
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
        case TRANSACTION_onUsbMassStorageConnectionChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onUsbMassStorageConnectionChanged(_arg0);
          return true;
        }
        case TRANSACTION_onStorageStateChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.onStorageStateChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onVolumeStateChanged:
        {
          data.enforceInterface(descriptor);
          android.os.storage.VolumeInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.storage.VolumeInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onVolumeStateChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onVolumeRecordChanged:
        {
          data.enforceInterface(descriptor);
          android.os.storage.VolumeRecord _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.storage.VolumeRecord.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onVolumeRecordChanged(_arg0);
          return true;
        }
        case TRANSACTION_onVolumeForgotten:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onVolumeForgotten(_arg0);
          return true;
        }
        case TRANSACTION_onDiskScanned:
        {
          data.enforceInterface(descriptor);
          android.os.storage.DiskInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.storage.DiskInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onDiskScanned(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDiskDestroyed:
        {
          data.enforceInterface(descriptor);
          android.os.storage.DiskInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.storage.DiskInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDiskDestroyed(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.storage.IStorageEventListener
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
           * Detection state of USB Mass Storage has changed
           *
           * @param available true if a UMS host is connected.
           */
      @Override public void onUsbMassStorageConnectionChanged(boolean connected) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((connected)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUsbMassStorageConnectionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUsbMassStorageConnectionChanged(connected);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Storage state has changed.
           *
           * @param path The volume mount path.
           * @param oldState The old state of the volume.
           * @param newState The new state of the volume. Note: State is one of the
           *            values returned by Environment.getExternalStorageState()
           */
      @Override public void onStorageStateChanged(java.lang.String path, java.lang.String oldState, java.lang.String newState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(path);
          _data.writeString(oldState);
          _data.writeString(newState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStorageStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStorageStateChanged(path, oldState, newState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVolumeStateChanged(android.os.storage.VolumeInfo vol, int oldState, int newState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((vol!=null)) {
            _data.writeInt(1);
            vol.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(oldState);
          _data.writeInt(newState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVolumeStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVolumeStateChanged(vol, oldState, newState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVolumeRecordChanged(android.os.storage.VolumeRecord rec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((rec!=null)) {
            _data.writeInt(1);
            rec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVolumeRecordChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVolumeRecordChanged(rec);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVolumeForgotten(java.lang.String fsUuid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(fsUuid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVolumeForgotten, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVolumeForgotten(fsUuid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDiskScanned(android.os.storage.DiskInfo disk, int volumeCount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((disk!=null)) {
            _data.writeInt(1);
            disk.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(volumeCount);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDiskScanned, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDiskScanned(disk, volumeCount);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDiskDestroyed(android.os.storage.DiskInfo disk) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((disk!=null)) {
            _data.writeInt(1);
            disk.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDiskDestroyed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDiskDestroyed(disk);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.storage.IStorageEventListener sDefaultImpl;
    }
    static final int TRANSACTION_onUsbMassStorageConnectionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onStorageStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onVolumeStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onVolumeRecordChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onVolumeForgotten = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onDiskScanned = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onDiskDestroyed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.os.storage.IStorageEventListener impl) {
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
    public static android.os.storage.IStorageEventListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Detection state of USB Mass Storage has changed
       *
       * @param available true if a UMS host is connected.
       */
  public void onUsbMassStorageConnectionChanged(boolean connected) throws android.os.RemoteException;
  /**
       * Storage state has changed.
       *
       * @param path The volume mount path.
       * @param oldState The old state of the volume.
       * @param newState The new state of the volume. Note: State is one of the
       *            values returned by Environment.getExternalStorageState()
       */
  public void onStorageStateChanged(java.lang.String path, java.lang.String oldState, java.lang.String newState) throws android.os.RemoteException;
  public void onVolumeStateChanged(android.os.storage.VolumeInfo vol, int oldState, int newState) throws android.os.RemoteException;
  public void onVolumeRecordChanged(android.os.storage.VolumeRecord rec) throws android.os.RemoteException;
  public void onVolumeForgotten(java.lang.String fsUuid) throws android.os.RemoteException;
  public void onDiskScanned(android.os.storage.DiskInfo disk, int volumeCount) throws android.os.RemoteException;
  public void onDiskDestroyed(android.os.storage.DiskInfo disk) throws android.os.RemoteException;
}
