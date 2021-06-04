/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.backup;
/**
 * Callback class for receiving important events during backup/restore operations.
 * These callbacks will run on the binder thread.
 *
 * @hide
 */
public interface IBackupManagerMonitor extends android.os.IInterface
{
  /** Default implementation for IBackupManagerMonitor. */
  public static class Default implements android.app.backup.IBackupManagerMonitor
  {
    /**
       * This method will be called each time something important happens on BackupManager.
       *
       * @param event bundle will contain data about event, like package name, package version etc.
       */
    @Override public void onEvent(android.os.Bundle event) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.backup.IBackupManagerMonitor
  {
    private static final java.lang.String DESCRIPTOR = "android.app.backup.IBackupManagerMonitor";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.backup.IBackupManagerMonitor interface,
     * generating a proxy if needed.
     */
    public static android.app.backup.IBackupManagerMonitor asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.backup.IBackupManagerMonitor))) {
        return ((android.app.backup.IBackupManagerMonitor)iin);
      }
      return new android.app.backup.IBackupManagerMonitor.Stub.Proxy(obj);
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
        case TRANSACTION_onEvent:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onEvent(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.backup.IBackupManagerMonitor
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
         * This method will be called each time something important happens on BackupManager.
         *
         * @param event bundle will contain data about event, like package name, package version etc.
         */
      @Override public void onEvent(android.os.Bundle event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEvent(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.backup.IBackupManagerMonitor sDefaultImpl;
    }
    static final int TRANSACTION_onEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.backup.IBackupManagerMonitor impl) {
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
    public static android.app.backup.IBackupManagerMonitor getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
     * This method will be called each time something important happens on BackupManager.
     *
     * @param event bundle will contain data about event, like package name, package version etc.
     */
  public void onEvent(android.os.Bundle event) throws android.os.RemoteException;
}
