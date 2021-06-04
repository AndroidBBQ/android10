/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.backup;
/**
 * Direct interface to the Backup Manager Service that applications invoke on.  The only
 * operation currently needed is a simple notification that the app has made changes to
 * data it wishes to back up, so the system should run a backup pass.
 *
 * Apps will use the {@link android.app.backup.BackupManager} class rather than going through
 * this Binder interface directly.
 * 
 * {@hide}
 */
public interface IBackupManager extends android.os.IInterface
{
  /** Default implementation for IBackupManager. */
  public static class Default implements android.app.backup.IBackupManager
  {
    /**
         * Tell the system service that the caller has made changes to its
         * data, and therefore needs to undergo an incremental backup pass.
         *
         * Any application can invoke this method for its own package, but
         * only callers who hold the android.permission.BACKUP permission
         * may invoke it for arbitrary packages.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the caller has made changes to its data.
         */
    @Override public void dataChangedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.dataChangedForUser} for the calling user id.
         */
    @Override public void dataChanged(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Erase all backed-up data for the given package from the given storage
         * destination.
         *
         * Any application can invoke this method for its own package, but
         * only callers who hold the android.permission.BACKUP permission
         * may invoke it for arbitrary packages.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which backup data should be erased.
         */
    @Override public void clearBackupDataForUser(int userId, java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.clearBackupDataForUser} for the calling user id.
         */
    @Override public void clearBackupData(java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Run an initialize operation on the given transports.  This will wipe all data from
         * the backing data store and establish a clean starting point for all backup
         * operations.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the given transports should be initialized.
         */
    @Override public void initializeTransportsForUser(int userId, java.lang.String[] transportNames, android.app.backup.IBackupObserver observer) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the Backup Manager Service that an agent has become available.  This
         * method is only invoked by the Activity Manager.
         *
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which an agent has become available.
         */
    @Override public void agentConnectedForUser(int userId, java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.agentConnected} for the calling user id.
         */
    @Override public void agentConnected(java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException
    {
    }
    /**
         * Notify the Backup Manager Service that an agent has unexpectedly gone away.
         * This method is only invoked by the Activity Manager.
         *
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which an agent has unexpectedly gone away.
         */
    @Override public void agentDisconnectedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.agentDisconnected} for the calling user id.
         */
    @Override public void agentDisconnected(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Notify the Backup Manager Service that an application being installed will
         * need a data-restore pass.  This method is only invoked by the Package Manager.
         *
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the application will need a data-restore pass.
         */
    @Override public void restoreAtInstallForUser(int userId, java.lang.String packageName, int token) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.restoreAtInstallForUser} for the calling user id.
         */
    @Override public void restoreAtInstall(java.lang.String packageName, int token) throws android.os.RemoteException
    {
    }
    /**
         * Enable/disable the backup service entirely.  When disabled, no backup
         * or restore operations will take place.  Data-changed notifications will
         * still be observed and collected, however, so that changes made while the
         * mechanism was disabled will still be backed up properly if it is enabled
         * at some point in the future.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which backup service should be enabled/disabled.
         */
    @Override public void setBackupEnabledForUser(int userId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.setBackupEnabledForUser} for the calling user id.
         */
    @Override public void setBackupEnabled(boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * Enable/disable automatic restore of application data at install time.  When
         * enabled, installation of any package will involve the Backup Manager.  If data
         * exists for the newly-installed package, either from the device's current [enabled]
         * backup dataset or from the restore set used in the last wholesale restore operation,
         * that data will be supplied to the new package's restore agent before the package
         * is made generally available for launch.
         *
         * <p>Callers must hold  the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which automatic restore should be enabled/disabled.
         * @param doAutoRestore When true, enables the automatic app-data restore facility.  When
         *   false, this facility will be disabled.
         */
    @Override public void setAutoRestoreForUser(int userId, boolean doAutoRestore) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.setAutoRestoreForUser} for the calling user id.
         */
    @Override public void setAutoRestore(boolean doAutoRestore) throws android.os.RemoteException
    {
    }
    /**
         * Report whether the backup mechanism is currently enabled.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the backup service status should be reported.
         */
    @Override public boolean isBackupEnabledForUser(int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * {@link android.app.backup.IBackupManager.isBackupEnabledForUser} for the calling user id.
         */
    @Override public boolean isBackupEnabled() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set the device's backup password.  Returns {@code true} if the password was set
         * successfully, {@code false} otherwise.  Typically a failure means that an incorrect
         * current password was supplied.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         */
    @Override public boolean setBackupPassword(java.lang.String currentPw, java.lang.String newPw) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Reports whether a backup password is currently set.  If not, then a null or empty
         * "current password" argument should be passed to setBackupPassword().
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         */
    @Override public boolean hasBackupPassword() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Schedule an immediate backup attempt for all pending updates.  This is
         * primarily intended for transports to use when they detect a suitable
         * opportunity for doing a backup pass.  If there are no pending updates to
         * be sent, no action will be taken.  Even if some updates are pending, the
         * transport will still be asked to confirm via the usual requestBackupTime()
         * method.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which an immediate backup should be scheduled.
         */
    @Override public void backupNowForUser(int userId) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.backupNowForUser} for the calling user id.
         */
    @Override public void backupNow() throws android.os.RemoteException
    {
    }
    /**
         * Write a backup of the given package to the supplied file descriptor.
         * The fd may be a socket or other non-seekable destination.  If no package names
         * are supplied, then every application on the device will be backed up to the output.
         * Currently only used by the 'adb backup' command.
         *
         * <p>This method is <i>synchronous</i> -- it does not return until the backup has
         * completed.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If the {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which backup should be performed.
         * @param fd The file descriptor to which a 'tar' file stream is to be written.
         * @param includeApks If <code>true</code>, the resulting tar stream will include the
         *     application .apk files themselves as well as their data.
         * @param includeObbs If <code>true</code>, the resulting tar stream will include any
         *     application expansion (OBB) files themselves belonging to each application.
         * @param includeShared If <code>true</code>, the resulting tar stream will include
         *     the contents of the device's shared storage (SD card or equivalent).
         * @param allApps If <code>true</code>, the resulting tar stream will include all
         *     installed applications' data, not just those named in the <code>packageNames</code>
         *     parameter.
         * @param allIncludesSystem If {@code true}, then {@code allApps} will be interpreted
         *     as including packages pre-installed as part of the system. If {@code false},
         *     then setting {@code allApps} to {@code true} will mean only that all 3rd-party
         *     applications will be included in the dataset.
         * @param doKeyValue If {@code true}, also packages supporting key-value backup will be backed
         *     up. If {@code false}, key-value packages will be skipped.
         * @param packageNames The package names of the apps whose data (and optionally .apk files)
         *     are to be backed up.  The <code>allApps</code> parameter supersedes this.
         */
    @Override public void adbBackup(int userId, android.os.ParcelFileDescriptor fd, boolean includeApks, boolean includeObbs, boolean includeShared, boolean doWidgets, boolean allApps, boolean allIncludesSystem, boolean doCompress, boolean doKeyValue, java.lang.String[] packageNames) throws android.os.RemoteException
    {
    }
    /**
         * Perform a full-dataset backup of the given applications via the currently active
         * transport.
         *
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the full-dataset backup should be performed.
         * @param packageNames The package names of the apps whose data are to be backed up.
         */
    @Override public void fullTransportBackupForUser(int userId, java.lang.String[] packageNames) throws android.os.RemoteException
    {
    }
    /**
         * Restore device content from the data stream passed through the given socket.  The
         * data stream must be in the format emitted by adbBackup().
         * Currently only used by the 'adb restore' command.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If the {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL.
         *
         * @param userId User id for which restore should be performed.
         */
    @Override public void adbRestore(int userId, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
    }
    /**
         * Confirm that the requested full backup/restore operation can proceed.  The system will
         * not actually perform the operation described to fullBackup() / fullRestore() unless the
         * UI calls back into the Backup Manager to confirm, passing the correct token.  At
         * the same time, the UI supplies a callback Binder for progress notifications during
         * the operation.
         *
         * <p>The password passed by the confirming entity must match the saved backup or
         * full-device encryption password in order to perform a backup.  If a password is
         * supplied for restore, it must match the password used when creating the full
         * backup dataset being used for restore.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the requested backup/restore operation can proceed.
         */
    @Override public void acknowledgeFullBackupOrRestoreForUser(int userId, int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.acknowledgeFullBackupOrRestoreForUser} for the
         * calling user id.
         */
    @Override public void acknowledgeFullBackupOrRestore(int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException
    {
    }
    /**
         * Update the attributes of the transport identified by {@code transportComponent}. If the
         * specified transport has not been bound at least once (for registration), this call will be
         * ignored. Only the host process of the transport can change its description, otherwise a
         * {@link SecurityException} will be thrown.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the attributes of the transport should be updated.
         * @param transportComponent The identity of the transport being described.
         * @param name A {@link String} with the new name for the transport. This is NOT for
         *     identification. MUST NOT be {@code null}.
         * @param configurationIntent An {@link Intent} that can be passed to
         *     {@link Context#startActivity} in order to launch the transport's configuration UI. It may
         *     be {@code null} if the transport does not offer any user-facing configuration UI.
         * @param currentDestinationString A {@link String} describing the destination to which the
         *     transport is currently sending data. MUST NOT be {@code null}.
         * @param dataManagementIntent An {@link Intent} that can be passed to
         *     {@link Context#startActivity} in order to launch the transport's data-management UI. It
         *     may be {@code null} if the transport does not offer any user-facing data
         *     management UI.
         * @param dataManagementLabel A {@link CharSequence} to be used as the label for the transport's
         *     data management affordance. This MUST be {@code null} when dataManagementIntent is {@code
         *     null} and MUST NOT be {@code null} when dataManagementIntent is not {@code null}.
         * @throws SecurityException If the UID of the calling process differs from the package UID of
         *     {@code transportComponent} or if the caller does NOT have BACKUP permission.
         */
    @Override public void updateTransportAttributesForUser(int userId, android.content.ComponentName transportComponent, java.lang.String name, android.content.Intent configurationIntent, java.lang.String currentDestinationString, android.content.Intent dataManagementIntent, java.lang.CharSequence dataManagementLabel) throws android.os.RemoteException
    {
    }
    /**
         * Identify the currently selected transport.  Callers must hold the
         * android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the currently selected transport should be identified.
         */
    @Override public java.lang.String getCurrentTransportForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.getCurrentTransportForUser} for the calling user id.
         */
    @Override public java.lang.String getCurrentTransport() throws android.os.RemoteException
    {
      return null;
    }
    /**
          * Returns the {@link ComponentName} of the host service of the selected transport or {@code
          * null} if no transport selected or if the transport selected is not registered.  Callers must
          * hold the android.permission.BACKUP permission to use this method.
          * If {@code userId} is different from the calling user id, then the caller must hold the
          * android.permission.INTERACT_ACROSS_USERS_FULL permission.
          *
          * @param userId User id for which the currently selected transport should be identified.
          */
    @Override public android.content.ComponentName getCurrentTransportComponentForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Request a list of all available backup transports' names.  Callers must
         * hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which all available backup transports' names should be listed.
         */
    @Override public java.lang.String[] listAllTransportsForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.listAllTransportsForUser} for the calling user id.
         */
    @Override public java.lang.String[] listAllTransports() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which all available backup transports should be listed.
         */
    @Override public android.content.ComponentName[] listAllTransportComponentsForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Retrieve the list of whitelisted transport components.  Callers do </i>not</i> need
         * any special permission.
         *
         * @return The names of all whitelisted transport components defined by the system.
         */
    @Override public java.lang.String[] getTransportWhitelist() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Specify the current backup transport.  Callers must hold the
         * android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the transport should be selected.
         * @param transport The name of the transport to select.  This should be one
         * of {@link BackupManager.TRANSPORT_GOOGLE} or {@link BackupManager.TRANSPORT_ADB}.
         * @return The name of the previously selected transport.  If the given transport
         *   name is not one of the currently available transports, no change is made to
         *   the current transport setting and the method returns null.
         */
    @Override public java.lang.String selectBackupTransportForUser(int userId, java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.selectBackupTransportForUser} for the calling user
         * id.
         */
    @Override public java.lang.String selectBackupTransport(java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Specify the current backup transport and get notified when the transport is ready to be used.
         * This method is async because BackupManager might need to bind to the specified transport
         * which is in a separate process.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the transport should be selected.
         * @param transport ComponentName of the service hosting the transport. This is different from
         *                  the transport's name that is returned by {@link BackupTransport#name()}.
         * @param listener A listener object to get a callback on the transport being selected.
         */
    @Override public void selectBackupTransportAsyncForUser(int userId, android.content.ComponentName transport, android.app.backup.ISelectBackupTransportCallback listener) throws android.os.RemoteException
    {
    }
    /**
         * Get the configuration Intent, if any, from the given transport.  Callers must
         * hold the android.permission.BACKUP permission in order to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the configuration Intent should be reported.
         * @param transport The name of the transport to query.
         * @return An Intent to use with Activity#startActivity() to bring up the configuration
         *   UI supplied by the transport.  If the transport has no configuration UI, it should
         *   return {@code null} here.
         */
    @Override public android.content.Intent getConfigurationIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.getConfigurationIntentForUser} for the calling user
         * id.
         */
    @Override public android.content.Intent getConfigurationIntent(java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the destination string supplied by the given transport.  Callers must
         * hold the android.permission.BACKUP permission in order to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the transport destination string should be reported.
         * @param transport The name of the transport to query.
         * @return A string describing the current backup destination.  This string is used
         *   verbatim by the Settings UI as the summary text of the "configure..." item.
         */
    @Override public java.lang.String getDestinationStringForUser(int userId, java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.getDestinationStringForUser} for the calling user
         * id.
         */
    @Override public java.lang.String getDestinationString(java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the manage-data UI intent, if any, from the given transport.  Callers must
         * hold the android.permission.BACKUP permission in order to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the manage-data UI intent should be reported.
         */
    @Override public android.content.Intent getDataManagementIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * {@link android.app.backup.IBackupManager.getDataManagementIntentForUser} for the calling user
         * id.
         */
    @Override public android.content.Intent getDataManagementIntent(java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the manage-data menu label, if any, from the given transport.  Callers must
         * hold the android.permission.BACKUP permission in order to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the manage-data menu label should be reported.
         */
    @Override public java.lang.CharSequence getDataManagementLabelForUser(int userId, java.lang.String transport) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Begin a restore session.  Either or both of packageName and transportID
         * may be null.  If packageName is non-null, then only the given package will be
         * considered for restore.  If transportID is null, then the restore will use
         * the current active transport.
         * <p>
         * This method requires the android.permission.BACKUP permission <i>except</i>
         * when transportID is null and packageName is the name of the caller's own
         * package.  In that case, the restore session returned is suitable for supporting
         * the BackupManager.requestRestore() functionality via RestoreSession.restorePackage()
         * without requiring the app to hold any special permission.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which a restore session should be begun.
         * @param packageName The name of the single package for which a restore will
         *        be requested.  May be null, in which case all packages in the restore
         *        set can be restored.
         * @param transportID The name of the transport to use for the restore operation.
         *        May be null, in which case the current active transport is used.
         * @return An interface to the restore session, or null on error.
         */
    @Override public android.app.backup.IRestoreSession beginRestoreSessionForUser(int userId, java.lang.String packageName, java.lang.String transportID) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Notify the backup manager that a BackupAgent has completed the operation
         * corresponding to the given token and user id.
         *
         * @param userId User id for which the operation has been completed.
         * @param token The transaction token passed to the BackupAgent method being
         *        invoked.
         * @param result In the case of a full backup measure operation, the estimated
         *        total file size that would result from the operation. Unused in all other
         *        cases.
         */
    @Override public void opCompleteForUser(int userId, int token, long result) throws android.os.RemoteException
    {
    }
    /**
         * Notify the backup manager that a BackupAgent has completed the operation
         * corresponding to the given token.
         *
         * @param token The transaction token passed to the BackupAgent method being
         *        invoked.
         * @param result In the case of a full backup measure operation, the estimated
         *        total file size that would result from the operation. Unused in all other
         *        cases.
         */
    @Override public void opComplete(int token, long result) throws android.os.RemoteException
    {
    }
    /**
         * Make the device's backup and restore machinery (in)active.  When it is inactive,
         * the device will not perform any backup operations, nor will it deliver data for
         * restore, although clients can still safely call BackupManager methods.
         *
         * @param whichUser User handle of the defined user whose backup active state
         *     is to be adjusted.
         * @param makeActive {@code true} when backup services are to be made active;
         *     {@code false} otherwise.
         */
    @Override public void setBackupServiceActive(int whichUser, boolean makeActive) throws android.os.RemoteException
    {
    }
    /**
         * Queries the activity status of backup service as set by {@link #setBackupServiceActive}.
         * @param whichUser User handle of the defined user whose backup active state
         *     is being queried.
         */
    @Override public boolean isBackupServiceActive(int whichUser) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Ask the framework which dataset, if any, the given package's data would be
         * restored from if we were to install it right now.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which this operation should be performed.
         * @param packageName The name of the package whose most-suitable dataset we
         *     wish to look up
         * @return The dataset token from which a restore should be attempted, or zero if
         *     no suitable data is available.
         */
    @Override public long getAvailableRestoreTokenForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0L;
    }
    /**
         * Ask the framework whether this app is eligible for backup.
         *
         * <p>If you are calling this method multiple times, you should instead use
         * {@link #filterAppsEligibleForBackup(String[])} to save resources.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which this operation should be performed.
         * @param packageName The name of the package.
         * @return Whether this app is eligible for backup.
         */
    @Override public boolean isAppEligibleForBackupForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Filter the packages that are eligible for backup and return the result.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which the filter should be performed.
         * @param packages The list of packages to filter.
         * @return The packages eligible for backup.
         */
    @Override public java.lang.String[] filterAppsEligibleForBackupForUser(int userId, java.lang.String[] packages) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Request an immediate backup, providing an observer to which results of the backup operation
         * will be published. The Android backup system will decide for each package whether it will
         * be full app data backup or key/value-pair-based backup.
         *
         * <p>If this method returns zero (meaning success), the OS will attempt to backup all provided
         * packages using the remote transport.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which an immediate backup should be requested.

         * @param observer The {@link BackupObserver} to receive callbacks during the backup
         * operation.
         *
         * @param monitor the {@link BackupManagerMonitor} to receive callbacks about important events
         * during the backup operation.
         *
         * @param flags {@link BackupManager#FLAG_NON_INCREMENTAL_BACKUP}.
         *
         * @return Zero on success; nonzero on error.
         */
    @Override public int requestBackupForUser(int userId, java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * {@link android.app.backup.IBackupManager.requestBackupForUser} for the calling user id.
         */
    @Override public int requestBackup(java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Cancel all running backups. After this call returns, no currently running backups will
         * interact with the selected transport.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         * If {@code userId} is different from the calling user id, then the caller must hold the
         * android.permission.INTERACT_ACROSS_USERS_FULL permission.
         *
         * @param userId User id for which backups should be cancelled.
         */
    @Override public void cancelBackupsForUser(int userId) throws android.os.RemoteException
    {
    }
    /**
         * {@link android.app.backup.IBackupManager.cancelBackups} for the calling user id.
         */
    @Override public void cancelBackups() throws android.os.RemoteException
    {
    }
    /**
         * Returns a {@link UserHandle} for the user that has {@code ancestralSerialNumber} as the serial
         * number of the it's ancestral work profile.
         *
         * <p> The ancestral work profile is set by {@link #setAncestralSerialNumber(long)}
         * and it corresponds to the profile that was used to restore to the callers profile.
         */
    @Override public android.os.UserHandle getUserForAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the ancestral work profile for the calling user.
         *
         * <p> The ancestral work profile corresponds to the profile that was used to restore to the
         * callers profile.
         *
         * <p>Callers must hold the android.permission.BACKUP permission to use this method.
         */
    @Override public void setAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.backup.IBackupManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.backup.IBackupManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.backup.IBackupManager interface,
     * generating a proxy if needed.
     */
    public static android.app.backup.IBackupManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.backup.IBackupManager))) {
        return ((android.app.backup.IBackupManager)iin);
      }
      return new android.app.backup.IBackupManager.Stub.Proxy(obj);
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
        case TRANSACTION_dataChangedForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.dataChangedForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dataChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.dataChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearBackupDataForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.clearBackupDataForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearBackupData:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.clearBackupData(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_initializeTransportsForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          android.app.backup.IBackupObserver _arg2;
          _arg2 = android.app.backup.IBackupObserver.Stub.asInterface(data.readStrongBinder());
          this.initializeTransportsForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_agentConnectedForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          this.agentConnectedForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_agentConnected:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.agentConnected(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_agentDisconnectedForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.agentDisconnectedForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_agentDisconnected:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.agentDisconnected(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restoreAtInstallForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.restoreAtInstallForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restoreAtInstall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.restoreAtInstall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBackupEnabledForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setBackupEnabledForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBackupEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setBackupEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAutoRestoreForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setAutoRestoreForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAutoRestore:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setAutoRestore(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBackupEnabledForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isBackupEnabledForUser(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isBackupEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isBackupEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setBackupPassword:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.setBackupPassword(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hasBackupPassword:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasBackupPassword();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_backupNowForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.backupNowForUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_backupNow:
        {
          data.enforceInterface(descriptor);
          this.backupNow();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_adbBackup:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          boolean _arg7;
          _arg7 = (0!=data.readInt());
          boolean _arg8;
          _arg8 = (0!=data.readInt());
          boolean _arg9;
          _arg9 = (0!=data.readInt());
          java.lang.String[] _arg10;
          _arg10 = data.createStringArray();
          this.adbBackup(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_fullTransportBackupForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.fullTransportBackupForUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_adbRestore:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.adbRestore(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acknowledgeFullBackupOrRestoreForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.app.backup.IFullBackupRestoreObserver _arg5;
          _arg5 = android.app.backup.IFullBackupRestoreObserver.Stub.asInterface(data.readStrongBinder());
          this.acknowledgeFullBackupOrRestoreForUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acknowledgeFullBackupOrRestore:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.backup.IFullBackupRestoreObserver _arg4;
          _arg4 = android.app.backup.IFullBackupRestoreObserver.Stub.asInterface(data.readStrongBinder());
          this.acknowledgeFullBackupOrRestore(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateTransportAttributesForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.Intent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.content.Intent _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          java.lang.CharSequence _arg6;
          if (0!=data.readInt()) {
            _arg6 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.updateTransportAttributesForUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentTransportForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getCurrentTransportForUser(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCurrentTransport:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getCurrentTransport();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCurrentTransportComponentForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _result = this.getCurrentTransportComponentForUser(_arg0);
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
        case TRANSACTION_listAllTransportsForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _result = this.listAllTransportsForUser(_arg0);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_listAllTransports:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.listAllTransports();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_listAllTransportComponentsForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName[] _result = this.listAllTransportComponentsForUser(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getTransportWhitelist:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getTransportWhitelist();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_selectBackupTransportForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.selectBackupTransportForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_selectBackupTransport:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.selectBackupTransport(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_selectBackupTransportAsyncForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.backup.ISelectBackupTransportCallback _arg2;
          _arg2 = android.app.backup.ISelectBackupTransportCallback.Stub.asInterface(data.readStrongBinder());
          this.selectBackupTransportAsyncForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getConfigurationIntentForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent _result = this.getConfigurationIntentForUser(_arg0, _arg1);
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
        case TRANSACTION_getConfigurationIntent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.Intent _result = this.getConfigurationIntent(_arg0);
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
        case TRANSACTION_getDestinationStringForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getDestinationStringForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getDestinationString:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.getDestinationString(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getDataManagementIntentForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent _result = this.getDataManagementIntentForUser(_arg0, _arg1);
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
        case TRANSACTION_getDataManagementIntent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.Intent _result = this.getDataManagementIntent(_arg0);
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
        case TRANSACTION_getDataManagementLabelForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.CharSequence _result = this.getDataManagementLabelForUser(_arg0, _arg1);
          reply.writeNoException();
          if (_result!=null) {
            reply.writeInt(1);
            android.text.TextUtils.writeToParcel(_result, reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_beginRestoreSessionForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.backup.IRestoreSession _result = this.beginRestoreSessionForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_opCompleteForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          this.opCompleteForUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_opComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.opComplete(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBackupServiceActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setBackupServiceActive(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBackupServiceActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isBackupServiceActive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAvailableRestoreTokenForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getAvailableRestoreTokenForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_isAppEligibleForBackupForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isAppEligibleForBackupForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_filterAppsEligibleForBackupForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          java.lang.String[] _result = this.filterAppsEligibleForBackupForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_requestBackupForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          android.app.backup.IBackupObserver _arg2;
          _arg2 = android.app.backup.IBackupObserver.Stub.asInterface(data.readStrongBinder());
          android.app.backup.IBackupManagerMonitor _arg3;
          _arg3 = android.app.backup.IBackupManagerMonitor.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          int _result = this.requestBackupForUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestBackup:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          android.app.backup.IBackupObserver _arg1;
          _arg1 = android.app.backup.IBackupObserver.Stub.asInterface(data.readStrongBinder());
          android.app.backup.IBackupManagerMonitor _arg2;
          _arg2 = android.app.backup.IBackupManagerMonitor.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.requestBackup(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_cancelBackupsForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.cancelBackupsForUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelBackups:
        {
          data.enforceInterface(descriptor);
          this.cancelBackups();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUserForAncestralSerialNumber:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.UserHandle _result = this.getUserForAncestralSerialNumber(_arg0);
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
        case TRANSACTION_setAncestralSerialNumber:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setAncestralSerialNumber(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.backup.IBackupManager
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
           * Tell the system service that the caller has made changes to its
           * data, and therefore needs to undergo an incremental backup pass.
           *
           * Any application can invoke this method for its own package, but
           * only callers who hold the android.permission.BACKUP permission
           * may invoke it for arbitrary packages.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the caller has made changes to its data.
           */
      @Override public void dataChangedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dataChangedForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dataChangedForUser(userId, packageName);
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
           * {@link android.app.backup.IBackupManager.dataChangedForUser} for the calling user id.
           */
      @Override public void dataChanged(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dataChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dataChanged(packageName);
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
           * Erase all backed-up data for the given package from the given storage
           * destination.
           *
           * Any application can invoke this method for its own package, but
           * only callers who hold the android.permission.BACKUP permission
           * may invoke it for arbitrary packages.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which backup data should be erased.
           */
      @Override public void clearBackupDataForUser(int userId, java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transportName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearBackupDataForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearBackupDataForUser(userId, transportName, packageName);
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
           * {@link android.app.backup.IBackupManager.clearBackupDataForUser} for the calling user id.
           */
      @Override public void clearBackupData(java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transportName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearBackupData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearBackupData(transportName, packageName);
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
           * Run an initialize operation on the given transports.  This will wipe all data from
           * the backing data store and establish a clean starting point for all backup
           * operations.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the given transports should be initialized.
           */
      @Override public void initializeTransportsForUser(int userId, java.lang.String[] transportNames, android.app.backup.IBackupObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStringArray(transportNames);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_initializeTransportsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initializeTransportsForUser(userId, transportNames, observer);
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
           * Notifies the Backup Manager Service that an agent has become available.  This
           * method is only invoked by the Activity Manager.
           *
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which an agent has become available.
           */
      @Override public void agentConnectedForUser(int userId, java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          _data.writeStrongBinder(agent);
          boolean _status = mRemote.transact(Stub.TRANSACTION_agentConnectedForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().agentConnectedForUser(userId, packageName, agent);
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
           * {@link android.app.backup.IBackupManager.agentConnected} for the calling user id.
           */
      @Override public void agentConnected(java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder(agent);
          boolean _status = mRemote.transact(Stub.TRANSACTION_agentConnected, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().agentConnected(packageName, agent);
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
           * Notify the Backup Manager Service that an agent has unexpectedly gone away.
           * This method is only invoked by the Activity Manager.
           *
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which an agent has unexpectedly gone away.
           */
      @Override public void agentDisconnectedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_agentDisconnectedForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().agentDisconnectedForUser(userId, packageName);
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
           * {@link android.app.backup.IBackupManager.agentDisconnected} for the calling user id.
           */
      @Override public void agentDisconnected(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_agentDisconnected, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().agentDisconnected(packageName);
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
           * Notify the Backup Manager Service that an application being installed will
           * need a data-restore pass.  This method is only invoked by the Package Manager.
           *
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the application will need a data-restore pass.
           */
      @Override public void restoreAtInstallForUser(int userId, java.lang.String packageName, int token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          _data.writeInt(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreAtInstallForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreAtInstallForUser(userId, packageName, token);
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
           * {@link android.app.backup.IBackupManager.restoreAtInstallForUser} for the calling user id.
           */
      @Override public void restoreAtInstall(java.lang.String packageName, int token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreAtInstall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreAtInstall(packageName, token);
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
           * Enable/disable the backup service entirely.  When disabled, no backup
           * or restore operations will take place.  Data-changed notifications will
           * still be observed and collected, however, so that changes made while the
           * mechanism was disabled will still be backed up properly if it is enabled
           * at some point in the future.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which backup service should be enabled/disabled.
           */
      @Override public void setBackupEnabledForUser(int userId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBackupEnabledForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBackupEnabledForUser(userId, isEnabled);
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
           * {@link android.app.backup.IBackupManager.setBackupEnabledForUser} for the calling user id.
           */
      @Override public void setBackupEnabled(boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBackupEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBackupEnabled(isEnabled);
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
           * Enable/disable automatic restore of application data at install time.  When
           * enabled, installation of any package will involve the Backup Manager.  If data
           * exists for the newly-installed package, either from the device's current [enabled]
           * backup dataset or from the restore set used in the last wholesale restore operation,
           * that data will be supplied to the new package's restore agent before the package
           * is made generally available for launch.
           *
           * <p>Callers must hold  the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which automatic restore should be enabled/disabled.
           * @param doAutoRestore When true, enables the automatic app-data restore facility.  When
           *   false, this facility will be disabled.
           */
      @Override public void setAutoRestoreForUser(int userId, boolean doAutoRestore) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((doAutoRestore)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAutoRestoreForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAutoRestoreForUser(userId, doAutoRestore);
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
           * {@link android.app.backup.IBackupManager.setAutoRestoreForUser} for the calling user id.
           */
      @Override public void setAutoRestore(boolean doAutoRestore) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((doAutoRestore)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAutoRestore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAutoRestore(doAutoRestore);
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
           * Report whether the backup mechanism is currently enabled.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the backup service status should be reported.
           */
      @Override public boolean isBackupEnabledForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBackupEnabledForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBackupEnabledForUser(userId);
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
           * {@link android.app.backup.IBackupManager.isBackupEnabledForUser} for the calling user id.
           */
      @Override public boolean isBackupEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBackupEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBackupEnabled();
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
           * Set the device's backup password.  Returns {@code true} if the password was set
           * successfully, {@code false} otherwise.  Typically a failure means that an incorrect
           * current password was supplied.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           */
      @Override public boolean setBackupPassword(java.lang.String currentPw, java.lang.String newPw) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(currentPw);
          _data.writeString(newPw);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBackupPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setBackupPassword(currentPw, newPw);
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
           * Reports whether a backup password is currently set.  If not, then a null or empty
           * "current password" argument should be passed to setBackupPassword().
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           */
      @Override public boolean hasBackupPassword() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasBackupPassword, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasBackupPassword();
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
           * Schedule an immediate backup attempt for all pending updates.  This is
           * primarily intended for transports to use when they detect a suitable
           * opportunity for doing a backup pass.  If there are no pending updates to
           * be sent, no action will be taken.  Even if some updates are pending, the
           * transport will still be asked to confirm via the usual requestBackupTime()
           * method.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which an immediate backup should be scheduled.
           */
      @Override public void backupNowForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_backupNowForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().backupNowForUser(userId);
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
           * {@link android.app.backup.IBackupManager.backupNowForUser} for the calling user id.
           */
      @Override public void backupNow() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_backupNow, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().backupNow();
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
           * Write a backup of the given package to the supplied file descriptor.
           * The fd may be a socket or other non-seekable destination.  If no package names
           * are supplied, then every application on the device will be backed up to the output.
           * Currently only used by the 'adb backup' command.
           *
           * <p>This method is <i>synchronous</i> -- it does not return until the backup has
           * completed.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If the {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which backup should be performed.
           * @param fd The file descriptor to which a 'tar' file stream is to be written.
           * @param includeApks If <code>true</code>, the resulting tar stream will include the
           *     application .apk files themselves as well as their data.
           * @param includeObbs If <code>true</code>, the resulting tar stream will include any
           *     application expansion (OBB) files themselves belonging to each application.
           * @param includeShared If <code>true</code>, the resulting tar stream will include
           *     the contents of the device's shared storage (SD card or equivalent).
           * @param allApps If <code>true</code>, the resulting tar stream will include all
           *     installed applications' data, not just those named in the <code>packageNames</code>
           *     parameter.
           * @param allIncludesSystem If {@code true}, then {@code allApps} will be interpreted
           *     as including packages pre-installed as part of the system. If {@code false},
           *     then setting {@code allApps} to {@code true} will mean only that all 3rd-party
           *     applications will be included in the dataset.
           * @param doKeyValue If {@code true}, also packages supporting key-value backup will be backed
           *     up. If {@code false}, key-value packages will be skipped.
           * @param packageNames The package names of the apps whose data (and optionally .apk files)
           *     are to be backed up.  The <code>allApps</code> parameter supersedes this.
           */
      @Override public void adbBackup(int userId, android.os.ParcelFileDescriptor fd, boolean includeApks, boolean includeObbs, boolean includeShared, boolean doWidgets, boolean allApps, boolean allIncludesSystem, boolean doCompress, boolean doKeyValue, java.lang.String[] packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((includeApks)?(1):(0)));
          _data.writeInt(((includeObbs)?(1):(0)));
          _data.writeInt(((includeShared)?(1):(0)));
          _data.writeInt(((doWidgets)?(1):(0)));
          _data.writeInt(((allApps)?(1):(0)));
          _data.writeInt(((allIncludesSystem)?(1):(0)));
          _data.writeInt(((doCompress)?(1):(0)));
          _data.writeInt(((doKeyValue)?(1):(0)));
          _data.writeStringArray(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adbBackup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adbBackup(userId, fd, includeApks, includeObbs, includeShared, doWidgets, allApps, allIncludesSystem, doCompress, doKeyValue, packageNames);
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
           * Perform a full-dataset backup of the given applications via the currently active
           * transport.
           *
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the full-dataset backup should be performed.
           * @param packageNames The package names of the apps whose data are to be backed up.
           */
      @Override public void fullTransportBackupForUser(int userId, java.lang.String[] packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStringArray(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_fullTransportBackupForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().fullTransportBackupForUser(userId, packageNames);
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
           * Restore device content from the data stream passed through the given socket.  The
           * data stream must be in the format emitted by adbBackup().
           * Currently only used by the 'adb restore' command.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If the {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL.
           *
           * @param userId User id for which restore should be performed.
           */
      @Override public void adbRestore(int userId, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_adbRestore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adbRestore(userId, fd);
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
           * Confirm that the requested full backup/restore operation can proceed.  The system will
           * not actually perform the operation described to fullBackup() / fullRestore() unless the
           * UI calls back into the Backup Manager to confirm, passing the correct token.  At
           * the same time, the UI supplies a callback Binder for progress notifications during
           * the operation.
           *
           * <p>The password passed by the confirming entity must match the saved backup or
           * full-device encryption password in order to perform a backup.  If a password is
           * supplied for restore, it must match the password used when creating the full
           * backup dataset being used for restore.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the requested backup/restore operation can proceed.
           */
      @Override public void acknowledgeFullBackupOrRestoreForUser(int userId, int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(token);
          _data.writeInt(((allow)?(1):(0)));
          _data.writeString(curPassword);
          _data.writeString(encryptionPassword);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_acknowledgeFullBackupOrRestoreForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acknowledgeFullBackupOrRestoreForUser(userId, token, allow, curPassword, encryptionPassword, observer);
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
           * {@link android.app.backup.IBackupManager.acknowledgeFullBackupOrRestoreForUser} for the
           * calling user id.
           */
      @Override public void acknowledgeFullBackupOrRestore(int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(((allow)?(1):(0)));
          _data.writeString(curPassword);
          _data.writeString(encryptionPassword);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_acknowledgeFullBackupOrRestore, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acknowledgeFullBackupOrRestore(token, allow, curPassword, encryptionPassword, observer);
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
           * Update the attributes of the transport identified by {@code transportComponent}. If the
           * specified transport has not been bound at least once (for registration), this call will be
           * ignored. Only the host process of the transport can change its description, otherwise a
           * {@link SecurityException} will be thrown.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the attributes of the transport should be updated.
           * @param transportComponent The identity of the transport being described.
           * @param name A {@link String} with the new name for the transport. This is NOT for
           *     identification. MUST NOT be {@code null}.
           * @param configurationIntent An {@link Intent} that can be passed to
           *     {@link Context#startActivity} in order to launch the transport's configuration UI. It may
           *     be {@code null} if the transport does not offer any user-facing configuration UI.
           * @param currentDestinationString A {@link String} describing the destination to which the
           *     transport is currently sending data. MUST NOT be {@code null}.
           * @param dataManagementIntent An {@link Intent} that can be passed to
           *     {@link Context#startActivity} in order to launch the transport's data-management UI. It
           *     may be {@code null} if the transport does not offer any user-facing data
           *     management UI.
           * @param dataManagementLabel A {@link CharSequence} to be used as the label for the transport's
           *     data management affordance. This MUST be {@code null} when dataManagementIntent is {@code
           *     null} and MUST NOT be {@code null} when dataManagementIntent is not {@code null}.
           * @throws SecurityException If the UID of the calling process differs from the package UID of
           *     {@code transportComponent} or if the caller does NOT have BACKUP permission.
           */
      @Override public void updateTransportAttributesForUser(int userId, android.content.ComponentName transportComponent, java.lang.String name, android.content.Intent configurationIntent, java.lang.String currentDestinationString, android.content.Intent dataManagementIntent, java.lang.CharSequence dataManagementLabel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          if ((transportComponent!=null)) {
            _data.writeInt(1);
            transportComponent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(name);
          if ((configurationIntent!=null)) {
            _data.writeInt(1);
            configurationIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(currentDestinationString);
          if ((dataManagementIntent!=null)) {
            _data.writeInt(1);
            dataManagementIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if (dataManagementLabel!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(dataManagementLabel, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateTransportAttributesForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateTransportAttributesForUser(userId, transportComponent, name, configurationIntent, currentDestinationString, dataManagementIntent, dataManagementLabel);
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
           * Identify the currently selected transport.  Callers must hold the
           * android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the currently selected transport should be identified.
           */
      @Override public java.lang.String getCurrentTransportForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTransportForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTransportForUser(userId);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * {@link android.app.backup.IBackupManager.getCurrentTransportForUser} for the calling user id.
           */
      @Override public java.lang.String getCurrentTransport() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTransport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTransport();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Returns the {@link ComponentName} of the host service of the selected transport or {@code
            * null} if no transport selected or if the transport selected is not registered.  Callers must
            * hold the android.permission.BACKUP permission to use this method.
            * If {@code userId} is different from the calling user id, then the caller must hold the
            * android.permission.INTERACT_ACROSS_USERS_FULL permission.
            *
            * @param userId User id for which the currently selected transport should be identified.
            */
      @Override public android.content.ComponentName getCurrentTransportComponentForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTransportComponentForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTransportComponentForUser(userId);
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
      /**
           * Request a list of all available backup transports' names.  Callers must
           * hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which all available backup transports' names should be listed.
           */
      @Override public java.lang.String[] listAllTransportsForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listAllTransportsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listAllTransportsForUser(userId);
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
           * {@link android.app.backup.IBackupManager.listAllTransportsForUser} for the calling user id.
           */
      @Override public java.lang.String[] listAllTransports() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listAllTransports, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listAllTransports();
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
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which all available backup transports should be listed.
           */
      @Override public android.content.ComponentName[] listAllTransportComponentsForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listAllTransportComponentsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listAllTransportComponentsForUser(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.content.ComponentName.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Retrieve the list of whitelisted transport components.  Callers do </i>not</i> need
           * any special permission.
           *
           * @return The names of all whitelisted transport components defined by the system.
           */
      @Override public java.lang.String[] getTransportWhitelist() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTransportWhitelist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTransportWhitelist();
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
           * Specify the current backup transport.  Callers must hold the
           * android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the transport should be selected.
           * @param transport The name of the transport to select.  This should be one
           * of {@link BackupManager.TRANSPORT_GOOGLE} or {@link BackupManager.TRANSPORT_ADB}.
           * @return The name of the previously selected transport.  If the given transport
           *   name is not one of the currently available transports, no change is made to
           *   the current transport setting and the method returns null.
           */
      @Override public java.lang.String selectBackupTransportForUser(int userId, java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_selectBackupTransportForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().selectBackupTransportForUser(userId, transport);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * {@link android.app.backup.IBackupManager.selectBackupTransportForUser} for the calling user
           * id.
           */
      @Override public java.lang.String selectBackupTransport(java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_selectBackupTransport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().selectBackupTransport(transport);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Specify the current backup transport and get notified when the transport is ready to be used.
           * This method is async because BackupManager might need to bind to the specified transport
           * which is in a separate process.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the transport should be selected.
           * @param transport ComponentName of the service hosting the transport. This is different from
           *                  the transport's name that is returned by {@link BackupTransport#name()}.
           * @param listener A listener object to get a callback on the transport being selected.
           */
      @Override public void selectBackupTransportAsyncForUser(int userId, android.content.ComponentName transport, android.app.backup.ISelectBackupTransportCallback listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          if ((transport!=null)) {
            _data.writeInt(1);
            transport.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_selectBackupTransportAsyncForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().selectBackupTransportAsyncForUser(userId, transport, listener);
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
           * Get the configuration Intent, if any, from the given transport.  Callers must
           * hold the android.permission.BACKUP permission in order to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the configuration Intent should be reported.
           * @param transport The name of the transport to query.
           * @return An Intent to use with Activity#startActivity() to bring up the configuration
           *   UI supplied by the transport.  If the transport has no configuration UI, it should
           *   return {@code null} here.
           */
      @Override public android.content.Intent getConfigurationIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigurationIntentForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigurationIntentForUser(userId, transport);
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
      /**
           * {@link android.app.backup.IBackupManager.getConfigurationIntentForUser} for the calling user
           * id.
           */
      @Override public android.content.Intent getConfigurationIntent(java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfigurationIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfigurationIntent(transport);
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
      /**
           * Get the destination string supplied by the given transport.  Callers must
           * hold the android.permission.BACKUP permission in order to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the transport destination string should be reported.
           * @param transport The name of the transport to query.
           * @return A string describing the current backup destination.  This string is used
           *   verbatim by the Settings UI as the summary text of the "configure..." item.
           */
      @Override public java.lang.String getDestinationStringForUser(int userId, java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDestinationStringForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDestinationStringForUser(userId, transport);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * {@link android.app.backup.IBackupManager.getDestinationStringForUser} for the calling user
           * id.
           */
      @Override public java.lang.String getDestinationString(java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDestinationString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDestinationString(transport);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the manage-data UI intent, if any, from the given transport.  Callers must
           * hold the android.permission.BACKUP permission in order to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the manage-data UI intent should be reported.
           */
      @Override public android.content.Intent getDataManagementIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataManagementIntentForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataManagementIntentForUser(userId, transport);
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
      /**
           * {@link android.app.backup.IBackupManager.getDataManagementIntentForUser} for the calling user
           * id.
           */
      @Override public android.content.Intent getDataManagementIntent(java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataManagementIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataManagementIntent(transport);
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
      /**
           * Get the manage-data menu label, if any, from the given transport.  Callers must
           * hold the android.permission.BACKUP permission in order to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the manage-data menu label should be reported.
           */
      @Override public java.lang.CharSequence getDataManagementLabelForUser(int userId, java.lang.String transport) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.CharSequence _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(transport);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataManagementLabelForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataManagementLabelForUser(userId, transport);
          }
          _reply.readException();
          if (0!=_reply.readInt()) {
            _result = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(_reply);
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
           * Begin a restore session.  Either or both of packageName and transportID
           * may be null.  If packageName is non-null, then only the given package will be
           * considered for restore.  If transportID is null, then the restore will use
           * the current active transport.
           * <p>
           * This method requires the android.permission.BACKUP permission <i>except</i>
           * when transportID is null and packageName is the name of the caller's own
           * package.  In that case, the restore session returned is suitable for supporting
           * the BackupManager.requestRestore() functionality via RestoreSession.restorePackage()
           * without requiring the app to hold any special permission.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which a restore session should be begun.
           * @param packageName The name of the single package for which a restore will
           *        be requested.  May be null, in which case all packages in the restore
           *        set can be restored.
           * @param transportID The name of the transport to use for the restore operation.
           *        May be null, in which case the current active transport is used.
           * @return An interface to the restore session, or null on error.
           */
      @Override public android.app.backup.IRestoreSession beginRestoreSessionForUser(int userId, java.lang.String packageName, java.lang.String transportID) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.backup.IRestoreSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          _data.writeString(transportID);
          boolean _status = mRemote.transact(Stub.TRANSACTION_beginRestoreSessionForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().beginRestoreSessionForUser(userId, packageName, transportID);
          }
          _reply.readException();
          _result = android.app.backup.IRestoreSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Notify the backup manager that a BackupAgent has completed the operation
           * corresponding to the given token and user id.
           *
           * @param userId User id for which the operation has been completed.
           * @param token The transaction token passed to the BackupAgent method being
           *        invoked.
           * @param result In the case of a full backup measure operation, the estimated
           *        total file size that would result from the operation. Unused in all other
           *        cases.
           */
      @Override public void opCompleteForUser(int userId, int token, long result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(token);
          _data.writeLong(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_opCompleteForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().opCompleteForUser(userId, token, result);
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
           * Notify the backup manager that a BackupAgent has completed the operation
           * corresponding to the given token.
           *
           * @param token The transaction token passed to the BackupAgent method being
           *        invoked.
           * @param result In the case of a full backup measure operation, the estimated
           *        total file size that would result from the operation. Unused in all other
           *        cases.
           */
      @Override public void opComplete(int token, long result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeLong(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_opComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().opComplete(token, result);
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
           * Make the device's backup and restore machinery (in)active.  When it is inactive,
           * the device will not perform any backup operations, nor will it deliver data for
           * restore, although clients can still safely call BackupManager methods.
           *
           * @param whichUser User handle of the defined user whose backup active state
           *     is to be adjusted.
           * @param makeActive {@code true} when backup services are to be made active;
           *     {@code false} otherwise.
           */
      @Override public void setBackupServiceActive(int whichUser, boolean makeActive) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(whichUser);
          _data.writeInt(((makeActive)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBackupServiceActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBackupServiceActive(whichUser, makeActive);
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
           * Queries the activity status of backup service as set by {@link #setBackupServiceActive}.
           * @param whichUser User handle of the defined user whose backup active state
           *     is being queried.
           */
      @Override public boolean isBackupServiceActive(int whichUser) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(whichUser);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBackupServiceActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBackupServiceActive(whichUser);
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
           * Ask the framework which dataset, if any, the given package's data would be
           * restored from if we were to install it right now.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which this operation should be performed.
           * @param packageName The name of the package whose most-suitable dataset we
           *     wish to look up
           * @return The dataset token from which a restore should be attempted, or zero if
           *     no suitable data is available.
           */
      @Override public long getAvailableRestoreTokenForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableRestoreTokenForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableRestoreTokenForUser(userId, packageName);
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
      /**
           * Ask the framework whether this app is eligible for backup.
           *
           * <p>If you are calling this method multiple times, you should instead use
           * {@link #filterAppsEligibleForBackup(String[])} to save resources.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which this operation should be performed.
           * @param packageName The name of the package.
           * @return Whether this app is eligible for backup.
           */
      @Override public boolean isAppEligibleForBackupForUser(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAppEligibleForBackupForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAppEligibleForBackupForUser(userId, packageName);
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
           * Filter the packages that are eligible for backup and return the result.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which the filter should be performed.
           * @param packages The list of packages to filter.
           * @return The packages eligible for backup.
           */
      @Override public java.lang.String[] filterAppsEligibleForBackupForUser(int userId, java.lang.String[] packages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStringArray(packages);
          boolean _status = mRemote.transact(Stub.TRANSACTION_filterAppsEligibleForBackupForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().filterAppsEligibleForBackupForUser(userId, packages);
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
           * Request an immediate backup, providing an observer to which results of the backup operation
           * will be published. The Android backup system will decide for each package whether it will
           * be full app data backup or key/value-pair-based backup.
           *
           * <p>If this method returns zero (meaning success), the OS will attempt to backup all provided
           * packages using the remote transport.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which an immediate backup should be requested.

           * @param observer The {@link BackupObserver} to receive callbacks during the backup
           * operation.
           *
           * @param monitor the {@link BackupManagerMonitor} to receive callbacks about important events
           * during the backup operation.
           *
           * @param flags {@link BackupManager#FLAG_NON_INCREMENTAL_BACKUP}.
           *
           * @return Zero on success; nonzero on error.
           */
      @Override public int requestBackupForUser(int userId, java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStringArray(packages);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeStrongBinder((((monitor!=null))?(monitor.asBinder()):(null)));
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBackupForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestBackupForUser(userId, packages, observer, monitor, flags);
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
           * {@link android.app.backup.IBackupManager.requestBackupForUser} for the calling user id.
           */
      @Override public int requestBackup(java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(packages);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeStrongBinder((((monitor!=null))?(monitor.asBinder()):(null)));
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBackup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestBackup(packages, observer, monitor, flags);
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
           * Cancel all running backups. After this call returns, no currently running backups will
           * interact with the selected transport.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           * If {@code userId} is different from the calling user id, then the caller must hold the
           * android.permission.INTERACT_ACROSS_USERS_FULL permission.
           *
           * @param userId User id for which backups should be cancelled.
           */
      @Override public void cancelBackupsForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelBackupsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelBackupsForUser(userId);
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
           * {@link android.app.backup.IBackupManager.cancelBackups} for the calling user id.
           */
      @Override public void cancelBackups() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelBackups, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelBackups();
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
           * Returns a {@link UserHandle} for the user that has {@code ancestralSerialNumber} as the serial
           * number of the it's ancestral work profile.
           *
           * <p> The ancestral work profile is set by {@link #setAncestralSerialNumber(long)}
           * and it corresponds to the profile that was used to restore to the callers profile.
           */
      @Override public android.os.UserHandle getUserForAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.UserHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(ancestralSerialNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserForAncestralSerialNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUserForAncestralSerialNumber(ancestralSerialNumber);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.UserHandle.CREATOR.createFromParcel(_reply);
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
           * Sets the ancestral work profile for the calling user.
           *
           * <p> The ancestral work profile corresponds to the profile that was used to restore to the
           * callers profile.
           *
           * <p>Callers must hold the android.permission.BACKUP permission to use this method.
           */
      @Override public void setAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(ancestralSerialNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAncestralSerialNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAncestralSerialNumber(ancestralSerialNumber);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.backup.IBackupManager sDefaultImpl;
    }
    static final int TRANSACTION_dataChangedForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_dataChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_clearBackupDataForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_clearBackupData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_initializeTransportsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_agentConnectedForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_agentConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_agentDisconnectedForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_agentDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_restoreAtInstallForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_restoreAtInstall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setBackupEnabledForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setBackupEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setAutoRestoreForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setAutoRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_isBackupEnabledForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_isBackupEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setBackupPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_hasBackupPassword = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_backupNowForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_backupNow = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_adbBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_fullTransportBackupForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_adbRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_acknowledgeFullBackupOrRestoreForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_acknowledgeFullBackupOrRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_updateTransportAttributesForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getCurrentTransportForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getCurrentTransport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getCurrentTransportComponentForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_listAllTransportsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_listAllTransports = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_listAllTransportComponentsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getTransportWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_selectBackupTransportForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_selectBackupTransport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_selectBackupTransportAsyncForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getConfigurationIntentForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_getConfigurationIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_getDestinationStringForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_getDestinationString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getDataManagementIntentForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getDataManagementIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_getDataManagementLabelForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_beginRestoreSessionForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_opCompleteForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_opComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_setBackupServiceActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_isBackupServiceActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_getAvailableRestoreTokenForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_isAppEligibleForBackupForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_filterAppsEligibleForBackupForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_requestBackupForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_requestBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_cancelBackupsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_cancelBackups = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_getUserForAncestralSerialNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_setAncestralSerialNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    public static boolean setDefaultImpl(android.app.backup.IBackupManager impl) {
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
    public static android.app.backup.IBackupManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Tell the system service that the caller has made changes to its
       * data, and therefore needs to undergo an incremental backup pass.
       *
       * Any application can invoke this method for its own package, but
       * only callers who hold the android.permission.BACKUP permission
       * may invoke it for arbitrary packages.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the caller has made changes to its data.
       */
  public void dataChangedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.dataChangedForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:57:1:57:25")
  public void dataChanged(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Erase all backed-up data for the given package from the given storage
       * destination.
       *
       * Any application can invoke this method for its own package, but
       * only callers who hold the android.permission.BACKUP permission
       * may invoke it for arbitrary packages.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which backup data should be erased.
       */
  public void clearBackupDataForUser(int userId, java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.clearBackupDataForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:77:1:77:25")
  public void clearBackupData(java.lang.String transportName, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Run an initialize operation on the given transports.  This will wipe all data from
       * the backing data store and establish a clean starting point for all backup
       * operations.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the given transports should be initialized.
       */
  public void initializeTransportsForUser(int userId, java.lang.String[] transportNames, android.app.backup.IBackupObserver observer) throws android.os.RemoteException;
  /**
       * Notifies the Backup Manager Service that an agent has become available.  This
       * method is only invoked by the Activity Manager.
       *
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which an agent has become available.
       */
  public void agentConnectedForUser(int userId, java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.agentConnected} for the calling user id.
       */
  public void agentConnected(java.lang.String packageName, android.os.IBinder agent) throws android.os.RemoteException;
  /**
       * Notify the Backup Manager Service that an agent has unexpectedly gone away.
       * This method is only invoked by the Activity Manager.
       *
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which an agent has unexpectedly gone away.
       */
  public void agentDisconnectedForUser(int userId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.agentDisconnected} for the calling user id.
       */
  public void agentDisconnected(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Notify the Backup Manager Service that an application being installed will
       * need a data-restore pass.  This method is only invoked by the Package Manager.
       *
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the application will need a data-restore pass.
       */
  public void restoreAtInstallForUser(int userId, java.lang.String packageName, int token) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.restoreAtInstallForUser} for the calling user id.
       */
  public void restoreAtInstall(java.lang.String packageName, int token) throws android.os.RemoteException;
  /**
       * Enable/disable the backup service entirely.  When disabled, no backup
       * or restore operations will take place.  Data-changed notifications will
       * still be observed and collected, however, so that changes made while the
       * mechanism was disabled will still be backed up properly if it is enabled
       * at some point in the future.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which backup service should be enabled/disabled.
       */
  public void setBackupEnabledForUser(int userId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.setBackupEnabledForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:160:1:160:25")
  public void setBackupEnabled(boolean isEnabled) throws android.os.RemoteException;
  /**
       * Enable/disable automatic restore of application data at install time.  When
       * enabled, installation of any package will involve the Backup Manager.  If data
       * exists for the newly-installed package, either from the device's current [enabled]
       * backup dataset or from the restore set used in the last wholesale restore operation,
       * that data will be supplied to the new package's restore agent before the package
       * is made generally available for launch.
       *
       * <p>Callers must hold  the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which automatic restore should be enabled/disabled.
       * @param doAutoRestore When true, enables the automatic app-data restore facility.  When
       *   false, this facility will be disabled.
       */
  public void setAutoRestoreForUser(int userId, boolean doAutoRestore) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.setAutoRestoreForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:184:1:184:25")
  public void setAutoRestore(boolean doAutoRestore) throws android.os.RemoteException;
  /**
       * Report whether the backup mechanism is currently enabled.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the backup service status should be reported.
       */
  public boolean isBackupEnabledForUser(int userId) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.isBackupEnabledForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:201:1:201:25")
  public boolean isBackupEnabled() throws android.os.RemoteException;
  /**
       * Set the device's backup password.  Returns {@code true} if the password was set
       * successfully, {@code false} otherwise.  Typically a failure means that an incorrect
       * current password was supplied.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       */
  public boolean setBackupPassword(java.lang.String currentPw, java.lang.String newPw) throws android.os.RemoteException;
  /**
       * Reports whether a backup password is currently set.  If not, then a null or empty
       * "current password" argument should be passed to setBackupPassword().
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       */
  public boolean hasBackupPassword() throws android.os.RemoteException;
  /**
       * Schedule an immediate backup attempt for all pending updates.  This is
       * primarily intended for transports to use when they detect a suitable
       * opportunity for doing a backup pass.  If there are no pending updates to
       * be sent, no action will be taken.  Even if some updates are pending, the
       * transport will still be asked to confirm via the usual requestBackupTime()
       * method.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which an immediate backup should be scheduled.
       */
  public void backupNowForUser(int userId) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.backupNowForUser} for the calling user id.
       */
  public void backupNow() throws android.os.RemoteException;
  /**
       * Write a backup of the given package to the supplied file descriptor.
       * The fd may be a socket or other non-seekable destination.  If no package names
       * are supplied, then every application on the device will be backed up to the output.
       * Currently only used by the 'adb backup' command.
       *
       * <p>This method is <i>synchronous</i> -- it does not return until the backup has
       * completed.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If the {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which backup should be performed.
       * @param fd The file descriptor to which a 'tar' file stream is to be written.
       * @param includeApks If <code>true</code>, the resulting tar stream will include the
       *     application .apk files themselves as well as their data.
       * @param includeObbs If <code>true</code>, the resulting tar stream will include any
       *     application expansion (OBB) files themselves belonging to each application.
       * @param includeShared If <code>true</code>, the resulting tar stream will include
       *     the contents of the device's shared storage (SD card or equivalent).
       * @param allApps If <code>true</code>, the resulting tar stream will include all
       *     installed applications' data, not just those named in the <code>packageNames</code>
       *     parameter.
       * @param allIncludesSystem If {@code true}, then {@code allApps} will be interpreted
       *     as including packages pre-installed as part of the system. If {@code false},
       *     then setting {@code allApps} to {@code true} will mean only that all 3rd-party
       *     applications will be included in the dataset.
       * @param doKeyValue If {@code true}, also packages supporting key-value backup will be backed
       *     up. If {@code false}, key-value packages will be skipped.
       * @param packageNames The package names of the apps whose data (and optionally .apk files)
       *     are to be backed up.  The <code>allApps</code> parameter supersedes this.
       */
  public void adbBackup(int userId, android.os.ParcelFileDescriptor fd, boolean includeApks, boolean includeObbs, boolean includeShared, boolean doWidgets, boolean allApps, boolean allIncludesSystem, boolean doCompress, boolean doKeyValue, java.lang.String[] packageNames) throws android.os.RemoteException;
  /**
       * Perform a full-dataset backup of the given applications via the currently active
       * transport.
       *
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the full-dataset backup should be performed.
       * @param packageNames The package names of the apps whose data are to be backed up.
       */
  public void fullTransportBackupForUser(int userId, java.lang.String[] packageNames) throws android.os.RemoteException;
  /**
       * Restore device content from the data stream passed through the given socket.  The
       * data stream must be in the format emitted by adbBackup().
       * Currently only used by the 'adb restore' command.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If the {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL.
       *
       * @param userId User id for which restore should be performed.
       */
  public void adbRestore(int userId, android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  /**
       * Confirm that the requested full backup/restore operation can proceed.  The system will
       * not actually perform the operation described to fullBackup() / fullRestore() unless the
       * UI calls back into the Backup Manager to confirm, passing the correct token.  At
       * the same time, the UI supplies a callback Binder for progress notifications during
       * the operation.
       *
       * <p>The password passed by the confirming entity must match the saved backup or
       * full-device encryption password in order to perform a backup.  If a password is
       * supplied for restore, it must match the password used when creating the full
       * backup dataset being used for restore.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the requested backup/restore operation can proceed.
       */
  public void acknowledgeFullBackupOrRestoreForUser(int userId, int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.acknowledgeFullBackupOrRestoreForUser} for the
       * calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:330:1:330:25")
  public void acknowledgeFullBackupOrRestore(int token, boolean allow, java.lang.String curPassword, java.lang.String encryptionPassword, android.app.backup.IFullBackupRestoreObserver observer) throws android.os.RemoteException;
  /**
       * Update the attributes of the transport identified by {@code transportComponent}. If the
       * specified transport has not been bound at least once (for registration), this call will be
       * ignored. Only the host process of the transport can change its description, otherwise a
       * {@link SecurityException} will be thrown.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the attributes of the transport should be updated.
       * @param transportComponent The identity of the transport being described.
       * @param name A {@link String} with the new name for the transport. This is NOT for
       *     identification. MUST NOT be {@code null}.
       * @param configurationIntent An {@link Intent} that can be passed to
       *     {@link Context#startActivity} in order to launch the transport's configuration UI. It may
       *     be {@code null} if the transport does not offer any user-facing configuration UI.
       * @param currentDestinationString A {@link String} describing the destination to which the
       *     transport is currently sending data. MUST NOT be {@code null}.
       * @param dataManagementIntent An {@link Intent} that can be passed to
       *     {@link Context#startActivity} in order to launch the transport's data-management UI. It
       *     may be {@code null} if the transport does not offer any user-facing data
       *     management UI.
       * @param dataManagementLabel A {@link CharSequence} to be used as the label for the transport's
       *     data management affordance. This MUST be {@code null} when dataManagementIntent is {@code
       *     null} and MUST NOT be {@code null} when dataManagementIntent is not {@code null}.
       * @throws SecurityException If the UID of the calling process differs from the package UID of
       *     {@code transportComponent} or if the caller does NOT have BACKUP permission.
       */
  public void updateTransportAttributesForUser(int userId, android.content.ComponentName transportComponent, java.lang.String name, android.content.Intent configurationIntent, java.lang.String currentDestinationString, android.content.Intent dataManagementIntent, java.lang.CharSequence dataManagementLabel) throws android.os.RemoteException;
  /**
       * Identify the currently selected transport.  Callers must hold the
       * android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the currently selected transport should be identified.
       */
  public java.lang.String getCurrentTransportForUser(int userId) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.getCurrentTransportForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:380:1:380:25")
  public java.lang.String getCurrentTransport() throws android.os.RemoteException;
  /**
        * Returns the {@link ComponentName} of the host service of the selected transport or {@code
        * null} if no transport selected or if the transport selected is not registered.  Callers must
        * hold the android.permission.BACKUP permission to use this method.
        * If {@code userId} is different from the calling user id, then the caller must hold the
        * android.permission.INTERACT_ACROSS_USERS_FULL permission.
        *
        * @param userId User id for which the currently selected transport should be identified.
        */
  public android.content.ComponentName getCurrentTransportComponentForUser(int userId) throws android.os.RemoteException;
  /**
       * Request a list of all available backup transports' names.  Callers must
       * hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which all available backup transports' names should be listed.
       */
  public java.lang.String[] listAllTransportsForUser(int userId) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.listAllTransportsForUser} for the calling user id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:407:1:407:25")
  public java.lang.String[] listAllTransports() throws android.os.RemoteException;
  /**
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which all available backup transports should be listed.
       */
  public android.content.ComponentName[] listAllTransportComponentsForUser(int userId) throws android.os.RemoteException;
  /**
       * Retrieve the list of whitelisted transport components.  Callers do </i>not</i> need
       * any special permission.
       *
       * @return The names of all whitelisted transport components defined by the system.
       */
  public java.lang.String[] getTransportWhitelist() throws android.os.RemoteException;
  /**
       * Specify the current backup transport.  Callers must hold the
       * android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the transport should be selected.
       * @param transport The name of the transport to select.  This should be one
       * of {@link BackupManager.TRANSPORT_GOOGLE} or {@link BackupManager.TRANSPORT_ADB}.
       * @return The name of the previously selected transport.  If the given transport
       *   name is not one of the currently available transports, no change is made to
       *   the current transport setting and the method returns null.
       */
  public java.lang.String selectBackupTransportForUser(int userId, java.lang.String transport) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.selectBackupTransportForUser} for the calling user
       * id.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:445:1:445:25")
  public java.lang.String selectBackupTransport(java.lang.String transport) throws android.os.RemoteException;
  /**
       * Specify the current backup transport and get notified when the transport is ready to be used.
       * This method is async because BackupManager might need to bind to the specified transport
       * which is in a separate process.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the transport should be selected.
       * @param transport ComponentName of the service hosting the transport. This is different from
       *                  the transport's name that is returned by {@link BackupTransport#name()}.
       * @param listener A listener object to get a callback on the transport being selected.
       */
  public void selectBackupTransportAsyncForUser(int userId, android.content.ComponentName transport, android.app.backup.ISelectBackupTransportCallback listener) throws android.os.RemoteException;
  /**
       * Get the configuration Intent, if any, from the given transport.  Callers must
       * hold the android.permission.BACKUP permission in order to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the configuration Intent should be reported.
       * @param transport The name of the transport to query.
       * @return An Intent to use with Activity#startActivity() to bring up the configuration
       *   UI supplied by the transport.  If the transport has no configuration UI, it should
       *   return {@code null} here.
       */
  public android.content.Intent getConfigurationIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.getConfigurationIntentForUser} for the calling user
       * id.
       */
  public android.content.Intent getConfigurationIntent(java.lang.String transport) throws android.os.RemoteException;
  /**
       * Get the destination string supplied by the given transport.  Callers must
       * hold the android.permission.BACKUP permission in order to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the transport destination string should be reported.
       * @param transport The name of the transport to query.
       * @return A string describing the current backup destination.  This string is used
       *   verbatim by the Settings UI as the summary text of the "configure..." item.
       */
  public java.lang.String getDestinationStringForUser(int userId, java.lang.String transport) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.getDestinationStringForUser} for the calling user
       * id.
       */
  public java.lang.String getDestinationString(java.lang.String transport) throws android.os.RemoteException;
  /**
       * Get the manage-data UI intent, if any, from the given transport.  Callers must
       * hold the android.permission.BACKUP permission in order to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the manage-data UI intent should be reported.
       */
  public android.content.Intent getDataManagementIntentForUser(int userId, java.lang.String transport) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.getDataManagementIntentForUser} for the calling user
       * id.
       */
  public android.content.Intent getDataManagementIntent(java.lang.String transport) throws android.os.RemoteException;
  /**
       * Get the manage-data menu label, if any, from the given transport.  Callers must
       * hold the android.permission.BACKUP permission in order to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the manage-data menu label should be reported.
       */
  public java.lang.CharSequence getDataManagementLabelForUser(int userId, java.lang.String transport) throws android.os.RemoteException;
  /**
       * Begin a restore session.  Either or both of packageName and transportID
       * may be null.  If packageName is non-null, then only the given package will be
       * considered for restore.  If transportID is null, then the restore will use
       * the current active transport.
       * <p>
       * This method requires the android.permission.BACKUP permission <i>except</i>
       * when transportID is null and packageName is the name of the caller's own
       * package.  In that case, the restore session returned is suitable for supporting
       * the BackupManager.requestRestore() functionality via RestoreSession.restorePackage()
       * without requiring the app to hold any special permission.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which a restore session should be begun.
       * @param packageName The name of the single package for which a restore will
       *        be requested.  May be null, in which case all packages in the restore
       *        set can be restored.
       * @param transportID The name of the transport to use for the restore operation.
       *        May be null, in which case the current active transport is used.
       * @return An interface to the restore session, or null on error.
       */
  public android.app.backup.IRestoreSession beginRestoreSessionForUser(int userId, java.lang.String packageName, java.lang.String transportID) throws android.os.RemoteException;
  /**
       * Notify the backup manager that a BackupAgent has completed the operation
       * corresponding to the given token and user id.
       *
       * @param userId User id for which the operation has been completed.
       * @param token The transaction token passed to the BackupAgent method being
       *        invoked.
       * @param result In the case of a full backup measure operation, the estimated
       *        total file size that would result from the operation. Unused in all other
       *        cases.
       */
  public void opCompleteForUser(int userId, int token, long result) throws android.os.RemoteException;
  /**
       * Notify the backup manager that a BackupAgent has completed the operation
       * corresponding to the given token.
       *
       * @param token The transaction token passed to the BackupAgent method being
       *        invoked.
       * @param result In the case of a full backup measure operation, the estimated
       *        total file size that would result from the operation. Unused in all other
       *        cases.
       */
  public void opComplete(int token, long result) throws android.os.RemoteException;
  /**
       * Make the device's backup and restore machinery (in)active.  When it is inactive,
       * the device will not perform any backup operations, nor will it deliver data for
       * restore, although clients can still safely call BackupManager methods.
       *
       * @param whichUser User handle of the defined user whose backup active state
       *     is to be adjusted.
       * @param makeActive {@code true} when backup services are to be made active;
       *     {@code false} otherwise.
       */
  public void setBackupServiceActive(int whichUser, boolean makeActive) throws android.os.RemoteException;
  /**
       * Queries the activity status of backup service as set by {@link #setBackupServiceActive}.
       * @param whichUser User handle of the defined user whose backup active state
       *     is being queried.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/backup/IBackupManager.aidl:596:1:596:25")
  public boolean isBackupServiceActive(int whichUser) throws android.os.RemoteException;
  /**
       * Ask the framework which dataset, if any, the given package's data would be
       * restored from if we were to install it right now.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which this operation should be performed.
       * @param packageName The name of the package whose most-suitable dataset we
       *     wish to look up
       * @return The dataset token from which a restore should be attempted, or zero if
       *     no suitable data is available.
       */
  public long getAvailableRestoreTokenForUser(int userId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Ask the framework whether this app is eligible for backup.
       *
       * <p>If you are calling this method multiple times, you should instead use
       * {@link #filterAppsEligibleForBackup(String[])} to save resources.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which this operation should be performed.
       * @param packageName The name of the package.
       * @return Whether this app is eligible for backup.
       */
  public boolean isAppEligibleForBackupForUser(int userId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Filter the packages that are eligible for backup and return the result.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which the filter should be performed.
       * @param packages The list of packages to filter.
       * @return The packages eligible for backup.
       */
  public java.lang.String[] filterAppsEligibleForBackupForUser(int userId, java.lang.String[] packages) throws android.os.RemoteException;
  /**
       * Request an immediate backup, providing an observer to which results of the backup operation
       * will be published. The Android backup system will decide for each package whether it will
       * be full app data backup or key/value-pair-based backup.
       *
       * <p>If this method returns zero (meaning success), the OS will attempt to backup all provided
       * packages using the remote transport.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which an immediate backup should be requested.

       * @param observer The {@link BackupObserver} to receive callbacks during the backup
       * operation.
       *
       * @param monitor the {@link BackupManagerMonitor} to receive callbacks about important events
       * during the backup operation.
       *
       * @param flags {@link BackupManager#FLAG_NON_INCREMENTAL_BACKUP}.
       *
       * @return Zero on success; nonzero on error.
       */
  public int requestBackupForUser(int userId, java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.requestBackupForUser} for the calling user id.
       */
  public int requestBackup(java.lang.String[] packages, android.app.backup.IBackupObserver observer, android.app.backup.IBackupManagerMonitor monitor, int flags) throws android.os.RemoteException;
  /**
       * Cancel all running backups. After this call returns, no currently running backups will
       * interact with the selected transport.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       * If {@code userId} is different from the calling user id, then the caller must hold the
       * android.permission.INTERACT_ACROSS_USERS_FULL permission.
       *
       * @param userId User id for which backups should be cancelled.
       */
  public void cancelBackupsForUser(int userId) throws android.os.RemoteException;
  /**
       * {@link android.app.backup.IBackupManager.cancelBackups} for the calling user id.
       */
  public void cancelBackups() throws android.os.RemoteException;
  /**
       * Returns a {@link UserHandle} for the user that has {@code ancestralSerialNumber} as the serial
       * number of the it's ancestral work profile.
       *
       * <p> The ancestral work profile is set by {@link #setAncestralSerialNumber(long)}
       * and it corresponds to the profile that was used to restore to the callers profile.
       */
  public android.os.UserHandle getUserForAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException;
  /**
       * Sets the ancestral work profile for the calling user.
       *
       * <p> The ancestral work profile corresponds to the profile that was used to restore to the
       * callers profile.
       *
       * <p>Callers must hold the android.permission.BACKUP permission to use this method.
       */
  public void setAncestralSerialNumber(long ancestralSerialNumber) throws android.os.RemoteException;
}
