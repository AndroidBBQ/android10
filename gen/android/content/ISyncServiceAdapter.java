/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content;
/**
 * Interface to define an anonymous service that is extended by developers
 * in order to perform anonymous syncs (syncs without an Account or Content
 * Provider specified). See {@link android.content.AbstractThreadedSyncAdapter}.
 * {@hide}
 */
public interface ISyncServiceAdapter extends android.os.IInterface
{
  /** Default implementation for ISyncServiceAdapter. */
  public static class Default implements android.content.ISyncServiceAdapter
  {
    /**
         * Initiate a sync. SyncAdapter-specific parameters may be specified in
         * extras, which is guaranteed to not be null.
         *
         * @param syncContext the ISyncContext used to indicate the progress of the sync. When
         *   the sync is finished (successfully or not) ISyncContext.onFinished() must be called.
         * @param extras SyncAdapter-specific parameters.
         *
         */
    @Override public void startSync(android.content.ISyncContext syncContext, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    /**
         * Cancel the currently ongoing sync.
         */
    @Override public void cancelSync(android.content.ISyncContext syncContext) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.ISyncServiceAdapter
  {
    private static final java.lang.String DESCRIPTOR = "android.content.ISyncServiceAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.ISyncServiceAdapter interface,
     * generating a proxy if needed.
     */
    public static android.content.ISyncServiceAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.ISyncServiceAdapter))) {
        return ((android.content.ISyncServiceAdapter)iin);
      }
      return new android.content.ISyncServiceAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_startSync:
        {
          data.enforceInterface(descriptor);
          android.content.ISyncContext _arg0;
          _arg0 = android.content.ISyncContext.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.startSync(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_cancelSync:
        {
          data.enforceInterface(descriptor);
          android.content.ISyncContext _arg0;
          _arg0 = android.content.ISyncContext.Stub.asInterface(data.readStrongBinder());
          this.cancelSync(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.ISyncServiceAdapter
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
           * Initiate a sync. SyncAdapter-specific parameters may be specified in
           * extras, which is guaranteed to not be null.
           *
           * @param syncContext the ISyncContext used to indicate the progress of the sync. When
           *   the sync is finished (successfully or not) ISyncContext.onFinished() must be called.
           * @param extras SyncAdapter-specific parameters.
           *
           */
      @Override public void startSync(android.content.ISyncContext syncContext, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((syncContext!=null))?(syncContext.asBinder()):(null)));
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSync, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSync(syncContext, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Cancel the currently ongoing sync.
           */
      @Override public void cancelSync(android.content.ISyncContext syncContext) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((syncContext!=null))?(syncContext.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSync, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSync(syncContext);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.ISyncServiceAdapter sDefaultImpl;
    }
    static final int TRANSACTION_startSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_cancelSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.content.ISyncServiceAdapter impl) {
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
    public static android.content.ISyncServiceAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Initiate a sync. SyncAdapter-specific parameters may be specified in
       * extras, which is guaranteed to not be null.
       *
       * @param syncContext the ISyncContext used to indicate the progress of the sync. When
       *   the sync is finished (successfully or not) ISyncContext.onFinished() must be called.
       * @param extras SyncAdapter-specific parameters.
       *
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/ISyncServiceAdapter.aidl:38:1:38:25")
  public void startSync(android.content.ISyncContext syncContext, android.os.Bundle extras) throws android.os.RemoteException;
  /**
       * Cancel the currently ongoing sync.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/ISyncServiceAdapter.aidl:44:1:44:25")
  public void cancelSync(android.content.ISyncContext syncContext) throws android.os.RemoteException;
}
