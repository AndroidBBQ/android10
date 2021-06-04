/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content;
/**
 * @hide
 */
public interface IContentService extends android.os.IInterface
{
  /** Default implementation for IContentService. */
  public static class Default implements android.content.IContentService
  {
    @Override public void unregisterContentObserver(android.database.IContentObserver observer) throws android.os.RemoteException
    {
    }
    /**
         * Register a content observer tied to a specific user's view of the provider.
         * @param userHandle the user whose view of the provider is to be observed.  May be
         *     the calling user without requiring any permission, otherwise the caller needs to
         *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL and
         *     USER_CURRENT are properly handled.
         */
    @Override public void registerContentObserver(android.net.Uri uri, boolean notifyForDescendants, android.database.IContentObserver observer, int userHandle, int targetSdkVersion) throws android.os.RemoteException
    {
    }
    /**
         * Notify observers of a particular user's view of the provider.
         * @param userHandle the user whose view of the provider is to be notified.  May be
         *     the calling user without requiring any permission, otherwise the caller needs to
         *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL
         *     USER_CURRENT are properly interpreted.
         */
    @Override public void notifyChange(android.net.Uri uri, android.database.IContentObserver observer, boolean observerWantsSelfNotifications, int flags, int userHandle, int targetSdkVersion, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void requestSync(android.accounts.Account account, java.lang.String authority, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * Start a sync given a request.
         */
    @Override public void sync(android.content.SyncRequest request, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void syncAsUser(android.content.SyncRequest request, int userId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void cancelSync(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
    {
    }
    @Override public void cancelSyncAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
    {
    }
    /** Cancel a sync, providing information about the sync to be cancelled. */
    @Override public void cancelRequest(android.content.SyncRequest request) throws android.os.RemoteException
    {
    }
    /**
         * Check if the provider should be synced when a network tickle is received
         * @param providerName the provider whose setting we are querying
         * @return true if the provider should be synced when a network tickle is received
         */
    @Override public boolean getSyncAutomatically(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean getSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set whether or not the provider is synced when it receives a network tickle.
         *
         * @param providerName the provider whose behavior is being controlled
         * @param sync true if the provider should be synced when tickles are received for it
         */
    @Override public void setSyncAutomatically(android.accounts.Account account, java.lang.String providerName, boolean sync) throws android.os.RemoteException
    {
    }
    @Override public void setSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, boolean sync, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Get a list of periodic operations for a specified authority, or service.
         * @param account account for authority, must be null if cname is non-null.
         * @param providerName name of provider, must be null if cname is non-null.
         * @param cname component to identify sync service, must be null if account/providerName are
         * non-null.
         */
    @Override public java.util.List<android.content.PeriodicSync> getPeriodicSyncs(android.accounts.Account account, java.lang.String providerName, android.content.ComponentName cname) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set whether or not the provider is to be synced on a periodic basis.
         *
         * @param providerName the provider whose behavior is being controlled
         * @param pollFrequency the period that a sync should be performed, in seconds. If this is
         * zero or less then no periodic syncs will be performed.
         */
    @Override public void addPeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras, long pollFrequency) throws android.os.RemoteException
    {
    }
    /**
         * Set whether or not the provider is to be synced on a periodic basis.
         *
         * @param providerName the provider whose behavior is being controlled
         * @param pollFrequency the period that a sync should be performed, in seconds. If this is
         * zero or less then no periodic syncs will be performed.
         */
    @Override public void removePeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    /**
         * Check if this account/provider is syncable.
         * @return >0 if it is syncable, 0 if not, and <0 if the state isn't known yet.
         */
    @Override public int getIsSyncable(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set whether this account/provider is syncable.
         * @param syncable, >0 denotes syncable, 0 means not syncable, <0 means unknown
         */
    @Override public void setIsSyncable(android.accounts.Account account, java.lang.String providerName, int syncable) throws android.os.RemoteException
    {
    }
    @Override public void setIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int syncable, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setMasterSyncAutomatically(boolean flag) throws android.os.RemoteException
    {
    }
    @Override public void setMasterSyncAutomaticallyAsUser(boolean flag, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean getMasterSyncAutomatically() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean getMasterSyncAutomaticallyAsUser(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.content.SyncInfo> getCurrentSyncs() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.content.SyncInfo> getCurrentSyncsAsUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the types of the SyncAdapters that are registered with the system.
         * @return Returns the types of the SyncAdapters that are registered with the system.
         */
    @Override public android.content.SyncAdapterType[] getSyncAdapterTypes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.SyncAdapterType[] getSyncAdapterTypesAsUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getSyncAdapterPackagesForAuthorityAsUser(java.lang.String authority, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns true if there is currently a operation for the given account/authority or service
         * actively being processed.
         * @param account account for authority, must be null if cname is non-null.
         * @param providerName name of provider, must be null if cname is non-null.
         * @param cname component to identify sync service, must be null if account/providerName are
         * non-null.
         */
    @Override public boolean isSyncActive(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the status that matches the authority. If there are multiples accounts for
         * the authority, the one with the latest "lastSuccessTime" status is returned.
         * @param account account for authority, must be null if cname is non-null.
         * @param providerName name of provider, must be null if cname is non-null.
         * @param cname component to identify sync service, must be null if account/providerName are
         * non-null.
         */
    @Override public android.content.SyncStatusInfo getSyncStatus(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.SyncStatusInfo getSyncStatusAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return true if the pending status is true of any matching authorities.
         * @param account account for authority, must be null if cname is non-null.
         * @param providerName name of provider, must be null if cname is non-null.
         * @param cname component to identify sync service, must be null if account/providerName are
         * non-null.
         */
    @Override public boolean isSyncPending(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isSyncPendingAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void addStatusChangeListener(int mask, android.content.ISyncStatusObserver callback) throws android.os.RemoteException
    {
    }
    @Override public void removeStatusChangeListener(android.content.ISyncStatusObserver callback) throws android.os.RemoteException
    {
    }
    @Override public void putCache(java.lang.String packageName, android.net.Uri key, android.os.Bundle value, int userId) throws android.os.RemoteException
    {
    }
    @Override public android.os.Bundle getCache(java.lang.String packageName, android.net.Uri key, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void resetTodayStats() throws android.os.RemoteException
    {
    }
    @Override public void onDbCorruption(java.lang.String tag, java.lang.String message, java.lang.String stacktrace) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.IContentService
  {
    private static final java.lang.String DESCRIPTOR = "android.content.IContentService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.IContentService interface,
     * generating a proxy if needed.
     */
    public static android.content.IContentService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.IContentService))) {
        return ((android.content.IContentService)iin);
      }
      return new android.content.IContentService.Stub.Proxy(obj);
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
        case TRANSACTION_unregisterContentObserver:
        {
          data.enforceInterface(descriptor);
          android.database.IContentObserver _arg0;
          _arg0 = android.database.IContentObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterContentObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerContentObserver:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.database.IContentObserver _arg2;
          _arg2 = android.database.IContentObserver.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.registerContentObserver(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyChange:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.database.IContentObserver _arg1;
          _arg1 = android.database.IContentObserver.Stub.asInterface(data.readStrongBinder());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          java.lang.String _arg6;
          _arg6 = data.readString();
          this.notifyChange(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestSync:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.requestSync(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sync:
        {
          data.enforceInterface(descriptor);
          android.content.SyncRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.SyncRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.sync(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_syncAsUser:
        {
          data.enforceInterface(descriptor);
          android.content.SyncRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.SyncRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.syncAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelSync:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.cancelSync(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelSyncAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.cancelSyncAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelRequest:
        {
          data.enforceInterface(descriptor);
          android.content.SyncRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.SyncRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.cancelRequest(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSyncAutomatically:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.getSyncAutomatically(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSyncAutomaticallyAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.getSyncAutomaticallyAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setSyncAutomatically:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setSyncAutomatically(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSyncAutomaticallyAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.setSyncAutomaticallyAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPeriodicSyncs:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.util.List<android.content.PeriodicSync> _result = this.getPeriodicSyncs(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_addPeriodicSync:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          long _arg3;
          _arg3 = data.readLong();
          this.addPeriodicSync(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removePeriodicSync:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.removePeriodicSync(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getIsSyncable:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getIsSyncable(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getIsSyncableAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _result = this.getIsSyncableAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setIsSyncable:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.setIsSyncable(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setIsSyncableAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.setIsSyncableAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMasterSyncAutomatically:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setMasterSyncAutomatically(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMasterSyncAutomaticallyAsUser:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setMasterSyncAutomaticallyAsUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMasterSyncAutomatically:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getMasterSyncAutomatically();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getMasterSyncAutomaticallyAsUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getMasterSyncAutomaticallyAsUser(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentSyncs:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.content.SyncInfo> _result = this.getCurrentSyncs();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getCurrentSyncsAsUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.content.SyncInfo> _result = this.getCurrentSyncsAsUser(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getSyncAdapterTypes:
        {
          data.enforceInterface(descriptor);
          android.content.SyncAdapterType[] _result = this.getSyncAdapterTypes();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getSyncAdapterTypesAsUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.SyncAdapterType[] _result = this.getSyncAdapterTypesAsUser(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getSyncAdapterPackagesForAuthorityAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String[] _result = this.getSyncAdapterPackagesForAuthorityAsUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_isSyncActive:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.isSyncActive(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSyncStatus:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.SyncStatusInfo _result = this.getSyncStatus(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getSyncStatusAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.content.SyncStatusInfo _result = this.getSyncStatusAsUser(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_isSyncPending:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.isSyncPending(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSyncPendingAsUser:
        {
          data.enforceInterface(descriptor);
          android.accounts.Account _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accounts.Account.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.isSyncPendingAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addStatusChangeListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ISyncStatusObserver _arg1;
          _arg1 = android.content.ISyncStatusObserver.Stub.asInterface(data.readStrongBinder());
          this.addStatusChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeStatusChangeListener:
        {
          data.enforceInterface(descriptor);
          android.content.ISyncStatusObserver _arg0;
          _arg0 = android.content.ISyncStatusObserver.Stub.asInterface(data.readStrongBinder());
          this.removeStatusChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_putCache:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.putCache(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCache:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _result = this.getCache(_arg0, _arg1, _arg2);
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
        case TRANSACTION_resetTodayStats:
        {
          data.enforceInterface(descriptor);
          this.resetTodayStats();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onDbCorruption:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.onDbCorruption(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.IContentService
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
      @Override public void unregisterContentObserver(android.database.IContentObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterContentObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterContentObserver(observer);
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
           * Register a content observer tied to a specific user's view of the provider.
           * @param userHandle the user whose view of the provider is to be observed.  May be
           *     the calling user without requiring any permission, otherwise the caller needs to
           *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL and
           *     USER_CURRENT are properly handled.
           */
      @Override public void registerContentObserver(android.net.Uri uri, boolean notifyForDescendants, android.database.IContentObserver observer, int userHandle, int targetSdkVersion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((notifyForDescendants)?(1):(0)));
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(userHandle);
          _data.writeInt(targetSdkVersion);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerContentObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerContentObserver(uri, notifyForDescendants, observer, userHandle, targetSdkVersion);
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
           * Notify observers of a particular user's view of the provider.
           * @param userHandle the user whose view of the provider is to be notified.  May be
           *     the calling user without requiring any permission, otherwise the caller needs to
           *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL
           *     USER_CURRENT are properly interpreted.
           */
      @Override public void notifyChange(android.net.Uri uri, android.database.IContentObserver observer, boolean observerWantsSelfNotifications, int flags, int userHandle, int targetSdkVersion, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(((observerWantsSelfNotifications)?(1):(0)));
          _data.writeInt(flags);
          _data.writeInt(userHandle);
          _data.writeInt(targetSdkVersion);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyChange, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyChange(uri, observer, observerWantsSelfNotifications, flags, userHandle, targetSdkVersion, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestSync(android.accounts.Account account, java.lang.String authority, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestSync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestSync(account, authority, extras, callingPackage);
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
           * Start a sync given a request.
           */
      @Override public void sync(android.content.SyncRequest request, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sync(request, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void syncAsUser(android.content.SyncRequest request, int userId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_syncAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().syncAsUser(request, userId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelSync(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSync(account, authority, cname);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelSyncAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSyncAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSyncAsUser(account, authority, cname, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Cancel a sync, providing information about the sync to be cancelled. */
      @Override public void cancelRequest(android.content.SyncRequest request) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelRequest(request);
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
           * Check if the provider should be synced when a network tickle is received
           * @param providerName the provider whose setting we are querying
           * @return true if the provider should be synced when a network tickle is received
           */
      @Override public boolean getSyncAutomatically(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncAutomatically, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncAutomatically(account, providerName);
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
      @Override public boolean getSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncAutomaticallyAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncAutomaticallyAsUser(account, providerName, userId);
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
           * Set whether or not the provider is synced when it receives a network tickle.
           *
           * @param providerName the provider whose behavior is being controlled
           * @param sync true if the provider should be synced when tickles are received for it
           */
      @Override public void setSyncAutomatically(android.accounts.Account account, java.lang.String providerName, boolean sync) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(((sync)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSyncAutomatically, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSyncAutomatically(account, providerName, sync);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, boolean sync, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(((sync)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSyncAutomaticallyAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSyncAutomaticallyAsUser(account, providerName, sync, userId);
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
           * Get a list of periodic operations for a specified authority, or service.
           * @param account account for authority, must be null if cname is non-null.
           * @param providerName name of provider, must be null if cname is non-null.
           * @param cname component to identify sync service, must be null if account/providerName are
           * non-null.
           */
      @Override public java.util.List<android.content.PeriodicSync> getPeriodicSyncs(android.accounts.Account account, java.lang.String providerName, android.content.ComponentName cname) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.content.PeriodicSync> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPeriodicSyncs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPeriodicSyncs(account, providerName, cname);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.content.PeriodicSync.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set whether or not the provider is to be synced on a periodic basis.
           *
           * @param providerName the provider whose behavior is being controlled
           * @param pollFrequency the period that a sync should be performed, in seconds. If this is
           * zero or less then no periodic syncs will be performed.
           */
      @Override public void addPeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras, long pollFrequency) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(pollFrequency);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPeriodicSync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPeriodicSync(account, providerName, extras, pollFrequency);
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
           * Set whether or not the provider is to be synced on a periodic basis.
           *
           * @param providerName the provider whose behavior is being controlled
           * @param pollFrequency the period that a sync should be performed, in seconds. If this is
           * zero or less then no periodic syncs will be performed.
           */
      @Override public void removePeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePeriodicSync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removePeriodicSync(account, providerName, extras);
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
           * Check if this account/provider is syncable.
           * @return >0 if it is syncable, 0 if not, and <0 if the state isn't known yet.
           */
      @Override public int getIsSyncable(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIsSyncable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIsSyncable(account, providerName);
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
      @Override public int getIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIsSyncableAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIsSyncableAsUser(account, providerName, userId);
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
      /**
           * Set whether this account/provider is syncable.
           * @param syncable, >0 denotes syncable, 0 means not syncable, <0 means unknown
           */
      @Override public void setIsSyncable(android.accounts.Account account, java.lang.String providerName, int syncable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(syncable);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIsSyncable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIsSyncable(account, providerName, syncable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int syncable, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(providerName);
          _data.writeInt(syncable);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIsSyncableAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIsSyncableAsUser(account, providerName, syncable, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setMasterSyncAutomatically(boolean flag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((flag)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMasterSyncAutomatically, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMasterSyncAutomatically(flag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setMasterSyncAutomaticallyAsUser(boolean flag, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((flag)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMasterSyncAutomaticallyAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMasterSyncAutomaticallyAsUser(flag, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean getMasterSyncAutomatically() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMasterSyncAutomatically, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMasterSyncAutomatically();
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
      @Override public boolean getMasterSyncAutomaticallyAsUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMasterSyncAutomaticallyAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMasterSyncAutomaticallyAsUser(userId);
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
      @Override public java.util.List<android.content.SyncInfo> getCurrentSyncs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.content.SyncInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentSyncs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentSyncs();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.content.SyncInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.content.SyncInfo> getCurrentSyncsAsUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.content.SyncInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentSyncsAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentSyncsAsUser(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.content.SyncInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Returns the types of the SyncAdapters that are registered with the system.
           * @return Returns the types of the SyncAdapters that are registered with the system.
           */
      @Override public android.content.SyncAdapterType[] getSyncAdapterTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.SyncAdapterType[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncAdapterTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncAdapterTypes();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.content.SyncAdapterType.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.SyncAdapterType[] getSyncAdapterTypesAsUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.SyncAdapterType[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncAdapterTypesAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncAdapterTypesAsUser(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.content.SyncAdapterType.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getSyncAdapterPackagesForAuthorityAsUser(java.lang.String authority, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(authority);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncAdapterPackagesForAuthorityAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncAdapterPackagesForAuthorityAsUser(authority, userId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Returns true if there is currently a operation for the given account/authority or service
           * actively being processed.
           * @param account account for authority, must be null if cname is non-null.
           * @param providerName name of provider, must be null if cname is non-null.
           * @param cname component to identify sync service, must be null if account/providerName are
           * non-null.
           */
      @Override public boolean isSyncActive(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSyncActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSyncActive(account, authority, cname);
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
           * Returns the status that matches the authority. If there are multiples accounts for
           * the authority, the one with the latest "lastSuccessTime" status is returned.
           * @param account account for authority, must be null if cname is non-null.
           * @param providerName name of provider, must be null if cname is non-null.
           * @param cname component to identify sync service, must be null if account/providerName are
           * non-null.
           */
      @Override public android.content.SyncStatusInfo getSyncStatus(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.SyncStatusInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncStatus(account, authority, cname);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.SyncStatusInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.SyncStatusInfo getSyncStatusAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.SyncStatusInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSyncStatusAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSyncStatusAsUser(account, authority, cname, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.SyncStatusInfo.CREATOR.createFromParcel(_reply);
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
           * Return true if the pending status is true of any matching authorities.
           * @param account account for authority, must be null if cname is non-null.
           * @param providerName name of provider, must be null if cname is non-null.
           * @param cname component to identify sync service, must be null if account/providerName are
           * non-null.
           */
      @Override public boolean isSyncPending(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSyncPending, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSyncPending(account, authority, cname);
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
      @Override public boolean isSyncPendingAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(authority);
          if ((cname!=null)) {
            _data.writeInt(1);
            cname.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSyncPendingAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSyncPendingAsUser(account, authority, cname, userId);
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
      @Override public void addStatusChangeListener(int mask, android.content.ISyncStatusObserver callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mask);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addStatusChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addStatusChangeListener(mask, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeStatusChangeListener(android.content.ISyncStatusObserver callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeStatusChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeStatusChangeListener(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void putCache(java.lang.String packageName, android.net.Uri key, android.os.Bundle value, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((key!=null)) {
            _data.writeInt(1);
            key.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((value!=null)) {
            _data.writeInt(1);
            value.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_putCache, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().putCache(packageName, key, value, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Bundle getCache(java.lang.String packageName, android.net.Uri key, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((key!=null)) {
            _data.writeInt(1);
            key.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCache, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCache(packageName, key, userId);
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
      @Override public void resetTodayStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetTodayStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetTodayStats();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onDbCorruption(java.lang.String tag, java.lang.String message, java.lang.String stacktrace) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(tag);
          _data.writeString(message);
          _data.writeString(stacktrace);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDbCorruption, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDbCorruption(tag, message, stacktrace);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.content.IContentService sDefaultImpl;
    }
    static final int TRANSACTION_unregisterContentObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerContentObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_requestSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_sync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_syncAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_cancelSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_cancelSyncAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_cancelRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getSyncAutomatically = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getSyncAutomaticallyAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setSyncAutomatically = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setSyncAutomaticallyAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getPeriodicSyncs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_addPeriodicSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_removePeriodicSync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getIsSyncable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getIsSyncableAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setIsSyncable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setIsSyncableAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setMasterSyncAutomatically = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_setMasterSyncAutomaticallyAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getMasterSyncAutomatically = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_getMasterSyncAutomaticallyAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getCurrentSyncs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getCurrentSyncsAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getSyncAdapterTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getSyncAdapterTypesAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getSyncAdapterPackagesForAuthorityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_isSyncActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getSyncStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getSyncStatusAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_isSyncPending = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_isSyncPendingAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_addStatusChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_removeStatusChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_putCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_resetTodayStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_onDbCorruption = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    public static boolean setDefaultImpl(android.content.IContentService impl) {
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
    public static android.content.IContentService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void unregisterContentObserver(android.database.IContentObserver observer) throws android.os.RemoteException;
  /**
       * Register a content observer tied to a specific user's view of the provider.
       * @param userHandle the user whose view of the provider is to be observed.  May be
       *     the calling user without requiring any permission, otherwise the caller needs to
       *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL and
       *     USER_CURRENT are properly handled.
       */
  public void registerContentObserver(android.net.Uri uri, boolean notifyForDescendants, android.database.IContentObserver observer, int userHandle, int targetSdkVersion) throws android.os.RemoteException;
  /**
       * Notify observers of a particular user's view of the provider.
       * @param userHandle the user whose view of the provider is to be notified.  May be
       *     the calling user without requiring any permission, otherwise the caller needs to
       *     hold the INTERACT_ACROSS_USERS_FULL permission.  Pseudousers USER_ALL
       *     USER_CURRENT are properly interpreted.
       */
  public void notifyChange(android.net.Uri uri, android.database.IContentObserver observer, boolean observerWantsSelfNotifications, int flags, int userHandle, int targetSdkVersion, java.lang.String callingPackage) throws android.os.RemoteException;
  public void requestSync(android.accounts.Account account, java.lang.String authority, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Start a sync given a request.
       */
  public void sync(android.content.SyncRequest request, java.lang.String callingPackage) throws android.os.RemoteException;
  public void syncAsUser(android.content.SyncRequest request, int userId, java.lang.String callingPackage) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:64:1:64:25")
  public void cancelSync(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException;
  public void cancelSyncAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException;
  /** Cancel a sync, providing information about the sync to be cancelled. */
  public void cancelRequest(android.content.SyncRequest request) throws android.os.RemoteException;
  /**
       * Check if the provider should be synced when a network tickle is received
       * @param providerName the provider whose setting we are querying
       * @return true if the provider should be synced when a network tickle is received
       */
  public boolean getSyncAutomatically(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException;
  public boolean getSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException;
  /**
       * Set whether or not the provider is synced when it receives a network tickle.
       *
       * @param providerName the provider whose behavior is being controlled
       * @param sync true if the provider should be synced when tickles are received for it
       */
  public void setSyncAutomatically(android.accounts.Account account, java.lang.String providerName, boolean sync) throws android.os.RemoteException;
  public void setSyncAutomaticallyAsUser(android.accounts.Account account, java.lang.String providerName, boolean sync, int userId) throws android.os.RemoteException;
  /**
       * Get a list of periodic operations for a specified authority, or service.
       * @param account account for authority, must be null if cname is non-null.
       * @param providerName name of provider, must be null if cname is non-null.
       * @param cname component to identify sync service, must be null if account/providerName are
       * non-null.
       */
  public java.util.List<android.content.PeriodicSync> getPeriodicSyncs(android.accounts.Account account, java.lang.String providerName, android.content.ComponentName cname) throws android.os.RemoteException;
  /**
       * Set whether or not the provider is to be synced on a periodic basis.
       *
       * @param providerName the provider whose behavior is being controlled
       * @param pollFrequency the period that a sync should be performed, in seconds. If this is
       * zero or less then no periodic syncs will be performed.
       */
  public void addPeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras, long pollFrequency) throws android.os.RemoteException;
  /**
       * Set whether or not the provider is to be synced on a periodic basis.
       *
       * @param providerName the provider whose behavior is being controlled
       * @param pollFrequency the period that a sync should be performed, in seconds. If this is
       * zero or less then no periodic syncs will be performed.
       */
  public void removePeriodicSync(android.accounts.Account account, java.lang.String providerName, android.os.Bundle extras) throws android.os.RemoteException;
  /**
       * Check if this account/provider is syncable.
       * @return >0 if it is syncable, 0 if not, and <0 if the state isn't known yet.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:122:1:122:25")
  public int getIsSyncable(android.accounts.Account account, java.lang.String providerName) throws android.os.RemoteException;
  public int getIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int userId) throws android.os.RemoteException;
  /**
       * Set whether this account/provider is syncable.
       * @param syncable, >0 denotes syncable, 0 means not syncable, <0 means unknown
       */
  public void setIsSyncable(android.accounts.Account account, java.lang.String providerName, int syncable) throws android.os.RemoteException;
  public void setIsSyncableAsUser(android.accounts.Account account, java.lang.String providerName, int syncable, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:133:1:133:25")
  public void setMasterSyncAutomatically(boolean flag) throws android.os.RemoteException;
  public void setMasterSyncAutomaticallyAsUser(boolean flag, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:137:1:137:25")
  public boolean getMasterSyncAutomatically() throws android.os.RemoteException;
  public boolean getMasterSyncAutomaticallyAsUser(int userId) throws android.os.RemoteException;
  public java.util.List<android.content.SyncInfo> getCurrentSyncs() throws android.os.RemoteException;
  public java.util.List<android.content.SyncInfo> getCurrentSyncsAsUser(int userId) throws android.os.RemoteException;
  /**
       * Returns the types of the SyncAdapters that are registered with the system.
       * @return Returns the types of the SyncAdapters that are registered with the system.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:148:1:148:25")
  public android.content.SyncAdapterType[] getSyncAdapterTypes() throws android.os.RemoteException;
  public android.content.SyncAdapterType[] getSyncAdapterTypesAsUser(int userId) throws android.os.RemoteException;
  public java.lang.String[] getSyncAdapterPackagesForAuthorityAsUser(java.lang.String authority, int userId) throws android.os.RemoteException;
  /**
       * Returns true if there is currently a operation for the given account/authority or service
       * actively being processed.
       * @param account account for authority, must be null if cname is non-null.
       * @param providerName name of provider, must be null if cname is non-null.
       * @param cname component to identify sync service, must be null if account/providerName are
       * non-null.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/IContentService.aidl:162:1:162:25")
  public boolean isSyncActive(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException;
  /**
       * Returns the status that matches the authority. If there are multiples accounts for
       * the authority, the one with the latest "lastSuccessTime" status is returned.
       * @param account account for authority, must be null if cname is non-null.
       * @param providerName name of provider, must be null if cname is non-null.
       * @param cname component to identify sync service, must be null if account/providerName are
       * non-null.
       */
  public android.content.SyncStatusInfo getSyncStatus(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException;
  public android.content.SyncStatusInfo getSyncStatusAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException;
  /**
       * Return true if the pending status is true of any matching authorities.
       * @param account account for authority, must be null if cname is non-null.
       * @param providerName name of provider, must be null if cname is non-null.
       * @param cname component to identify sync service, must be null if account/providerName are
       * non-null.
       */
  public boolean isSyncPending(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname) throws android.os.RemoteException;
  public boolean isSyncPendingAsUser(android.accounts.Account account, java.lang.String authority, android.content.ComponentName cname, int userId) throws android.os.RemoteException;
  public void addStatusChangeListener(int mask, android.content.ISyncStatusObserver callback) throws android.os.RemoteException;
  public void removeStatusChangeListener(android.content.ISyncStatusObserver callback) throws android.os.RemoteException;
  public void putCache(java.lang.String packageName, android.net.Uri key, android.os.Bundle value, int userId) throws android.os.RemoteException;
  public android.os.Bundle getCache(java.lang.String packageName, android.net.Uri key, int userId) throws android.os.RemoteException;
  public void resetTodayStats() throws android.os.RemoteException;
  public void onDbCorruption(java.lang.String tag, java.lang.String message, java.lang.String stacktrace) throws android.os.RemoteException;
}
