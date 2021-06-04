/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * System private API for talking with the activity manager service.  This
 * provides calls from the application back to the activity manager.
 *
 * {@hide}
 */
public interface IActivityManager extends android.os.IInterface
{
  /** Default implementation for IActivityManager. */
  public static class Default implements android.app.IActivityManager
  {
    // WARNING: when these transactions are updated, check if they are any callers on the native
    // side. If so, make sure they are using the correct transaction ids and arguments.
    // If a transaction which will also be used on the native side is being inserted, add it to
    // below block of transactions.
    // Since these transactions are also called from native code, these must be kept in sync with
    // the ones in frameworks/native/libs/binder/include/binder/IActivityManager.h
    // =============== Beginning of transactions used on native side as well ======================

    @Override public android.os.ParcelFileDescriptor openContentUri(java.lang.String uriString) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerUidObserver(android.app.IUidObserver observer, int which, int cutpoint, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void unregisterUidObserver(android.app.IUidObserver observer) throws android.os.RemoteException
    {
    }
    @Override public boolean isUidActive(int uid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getUidProcessState(int uid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    // =============== End of transactions used on native side as well ============================
    // Special low-level communication with activity manager.

    @Override public void handleApplicationCrash(android.os.IBinder app, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException
    {
    }
    @Override public int startActivity(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void unhandledBack() throws android.os.RemoteException
    {
    }
    @Override public boolean finishActivity(android.os.IBinder token, int code, android.content.Intent data, int finishTask) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.Intent registerReceiver(android.app.IApplicationThread caller, java.lang.String callerPackage, android.content.IIntentReceiver receiver, android.content.IntentFilter filter, java.lang.String requiredPermission, int userId, int flags) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void unregisterReceiver(android.content.IIntentReceiver receiver) throws android.os.RemoteException
    {
    }
    @Override public int broadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver resultTo, int resultCode, java.lang.String resultData, android.os.Bundle map, java.lang.String[] requiredPermissions, int appOp, android.os.Bundle options, boolean serialized, boolean sticky, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void unbroadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, int userId) throws android.os.RemoteException
    {
    }
    @Override public void finishReceiver(android.os.IBinder who, int resultCode, java.lang.String resultData, android.os.Bundle map, boolean abortBroadcast, int flags) throws android.os.RemoteException
    {
    }
    @Override public void attachApplication(android.app.IApplicationThread app, long startSeq) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.app.ActivityManager.RunningTaskInfo> getTasks(int maxNum) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.app.ActivityManager.RunningTaskInfo> getFilteredTasks(int maxNum, int ignoreActivityType, int ignoreWindowingMode) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void moveTaskToFront(android.app.IApplicationThread caller, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public int getTaskForActivity(android.os.IBinder token, boolean onlyRoot) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.app.ContentProviderHolder getContentProvider(android.app.IApplicationThread caller, java.lang.String callingPackage, java.lang.String name, int userId, boolean stable) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void publishContentProviders(android.app.IApplicationThread caller, java.util.List<android.app.ContentProviderHolder> providers) throws android.os.RemoteException
    {
    }
    @Override public boolean refContentProvider(android.os.IBinder connection, int stableDelta, int unstableDelta) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.app.PendingIntent getRunningServiceControlPanel(android.content.ComponentName service) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.ComponentName startService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, boolean requireForeground, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int stopService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    // Currently keeping old bindService because it is on the greylist

    @Override public int bindService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int bindIsolatedService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String instanceName, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void updateServiceGroup(android.app.IServiceConnection connection, int group, int importance) throws android.os.RemoteException
    {
    }
    @Override public boolean unbindService(android.app.IServiceConnection connection) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void publishService(android.os.IBinder token, android.content.Intent intent, android.os.IBinder service) throws android.os.RemoteException
    {
    }
    @Override public void setDebugApp(java.lang.String packageName, boolean waitForDebugger, boolean persistent) throws android.os.RemoteException
    {
    }
    @Override public void setAgentApp(java.lang.String packageName, java.lang.String agent) throws android.os.RemoteException
    {
    }
    @Override public void setAlwaysFinish(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean startInstrumentation(android.content.ComponentName className, java.lang.String profileFile, int flags, android.os.Bundle arguments, android.app.IInstrumentationWatcher watcher, android.app.IUiAutomationConnection connection, int userId, java.lang.String abiOverride) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void addInstrumentationResults(android.app.IApplicationThread target, android.os.Bundle results) throws android.os.RemoteException
    {
    }
    @Override public void finishInstrumentation(android.app.IApplicationThread target, int resultCode, android.os.Bundle results) throws android.os.RemoteException
    {
    }
    /**
         * @return A copy of global {@link Configuration}, contains general settings for the entire
         *         system. Corresponds to the configuration of the default display.
         * @throws RemoteException
         */
    @Override public android.content.res.Configuration getConfiguration() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Updates global configuration and applies changes to the entire system.
         * @param values Update values for global configuration. If null is passed it will request the
         *               Window Manager to compute new config for the default display.
         * @throws RemoteException
         * @return Returns true if the configuration was updated.
         */
    @Override public boolean updateConfiguration(android.content.res.Configuration values) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean stopServiceToken(android.content.ComponentName className, android.os.IBinder token, int startId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setProcessLimit(int max) throws android.os.RemoteException
    {
    }
    @Override public int getProcessLimit() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int checkPermission(java.lang.String permission, int pid, int uid) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int checkUriPermission(android.net.Uri uri, int pid, int uid, int mode, int userId, android.os.IBinder callerToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void grantUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException
    {
    }
    @Override public void revokeUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setActivityController(android.app.IActivityController watcher, boolean imAMonkey) throws android.os.RemoteException
    {
    }
    @Override public void showWaitingForDebugger(android.app.IApplicationThread who, boolean waiting) throws android.os.RemoteException
    {
    }
    /*
         * This will deliver the specified signal to all the persistent processes. Currently only
         * SIGUSR1 is delivered. All others are ignored.
         */
    @Override public void signalPersistentProcesses(int signal) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getRecentTasks(int maxNum, int flags, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void serviceDoneExecuting(android.os.IBinder token, int type, int startId, int res) throws android.os.RemoteException
    {
    }
    @Override public android.content.IIntentSender getIntentSender(int type, java.lang.String packageName, android.os.IBinder token, java.lang.String resultWho, int requestCode, android.content.Intent[] intents, java.lang.String[] resolvedTypes, int flags, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void cancelIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getPackageForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException
    {
    }
    @Override public void unregisterIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException
    {
    }
    @Override public void enterSafeMode() throws android.os.RemoteException
    {
    }
    @Override public void noteWakeupAlarm(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String sourcePkg, java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public void removeContentProvider(android.os.IBinder connection, boolean stable) throws android.os.RemoteException
    {
    }
    @Override public void setRequestedOrientation(android.os.IBinder token, int requestedOrientation) throws android.os.RemoteException
    {
    }
    @Override public void unbindFinished(android.os.IBinder token, android.content.Intent service, boolean doRebind) throws android.os.RemoteException
    {
    }
    @Override public void setProcessImportant(android.os.IBinder token, int pid, boolean isForeground, java.lang.String reason) throws android.os.RemoteException
    {
    }
    @Override public void setServiceForeground(android.content.ComponentName className, android.os.IBinder token, int id, android.app.Notification notification, int flags, int foregroundServiceType) throws android.os.RemoteException
    {
    }
    @Override public int getForegroundServiceType(android.content.ComponentName className, android.os.IBinder token) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean moveActivityTaskToBack(android.os.IBinder token, boolean nonRoot) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void getMemoryInfo(android.app.ActivityManager.MemoryInfo outInfo) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.app.ActivityManager.ProcessErrorStateInfo> getProcessesInErrorState() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean clearApplicationUserData(java.lang.String packageName, boolean keepState, android.content.pm.IPackageDataObserver observer, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void forceStopPackage(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean killPids(int[] pids, java.lang.String reason, boolean secure) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<android.app.ActivityManager.RunningServiceInfo> getServices(int maxNum, int flags) throws android.os.RemoteException
    {
      return null;
    }
    // Retrieve running application processes in the system

    @Override public java.util.List<android.app.ActivityManager.RunningAppProcessInfo> getRunningAppProcesses() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.IBinder peekService(android.content.Intent service, java.lang.String resolvedType, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    // Turn on/off profiling in a particular process.

    @Override public boolean profileControl(java.lang.String process, int userId, boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean shutdown(int timeout) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void stopAppSwitches() throws android.os.RemoteException
    {
    }
    @Override public void resumeAppSwitches() throws android.os.RemoteException
    {
    }
    @Override public boolean bindBackupAgent(java.lang.String packageName, int backupRestoreMode, int targetUserId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void backupAgentCreated(java.lang.String packageName, android.os.IBinder agent, int userId) throws android.os.RemoteException
    {
    }
    @Override public void unbindBackupAgent(android.content.pm.ApplicationInfo appInfo) throws android.os.RemoteException
    {
    }
    @Override public int getUidForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int handleIncomingUser(int callingPid, int callingUid, int userId, boolean allowAll, boolean requireFull, java.lang.String name, java.lang.String callerPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void addPackageDependency(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void killApplication(java.lang.String pkg, int appId, int userId, java.lang.String reason) throws android.os.RemoteException
    {
    }
    @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
    {
    }
    @Override public android.os.Debug.MemoryInfo[] getProcessMemoryInfo(int[] pids) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void killApplicationProcess(java.lang.String processName, int uid) throws android.os.RemoteException
    {
    }
    // Special low-level communication with activity manager.

    @Override public boolean handleApplicationWtf(android.os.IBinder app, java.lang.String tag, boolean system, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void killBackgroundProcesses(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean isUserAMonkey() throws android.os.RemoteException
    {
      return false;
    }
    // Retrieve info of applications installed on external media that are currently
    // running.

    @Override public java.util.List<android.content.pm.ApplicationInfo> getRunningExternalApplications() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void finishHeavyWeightApp() throws android.os.RemoteException
    {
    }
    // A StrictMode violation to be handled.

    @Override public void handleApplicationStrictModeViolation(android.os.IBinder app, int penaltyMask, android.os.StrictMode.ViolationInfo crashInfo) throws android.os.RemoteException
    {
    }
    @Override public boolean isTopActivityImmersive() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void crashApplication(int uid, int initialPid, java.lang.String packageName, int userId, java.lang.String message, boolean force) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getProviderMimeType(android.net.Uri uri, int userId) throws android.os.RemoteException
    {
      return null;
    }
    // Cause the specified process to dump the specified heap.

    @Override public boolean dumpHeap(java.lang.String process, int userId, boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isUserRunning(int userid, int flags) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setPackageScreenCompatMode(java.lang.String packageName, int mode) throws android.os.RemoteException
    {
    }
    @Override public boolean switchUser(int userid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeTask(int taskId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void registerProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException
    {
    }
    @Override public void unregisterProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException
    {
    }
    @Override public boolean isIntentSenderTargetedToPackage(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void updatePersistentConfiguration(android.content.res.Configuration values) throws android.os.RemoteException
    {
    }
    @Override public long[] getProcessPss(int[] pids) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void showBootMessage(java.lang.CharSequence msg, boolean always) throws android.os.RemoteException
    {
    }
    @Override public void killAllBackgroundProcesses() throws android.os.RemoteException
    {
    }
    @Override public android.app.ContentProviderHolder getContentProviderExternal(java.lang.String name, int userId, android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException
    {
      return null;
    }
    /** @deprecated - Use {@link #removeContentProviderExternalAsUser} which takes a user ID. */
    @Override public void removeContentProviderExternal(java.lang.String name, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void removeContentProviderExternalAsUser(java.lang.String name, android.os.IBinder token, int userId) throws android.os.RemoteException
    {
    }
    // Get memory information about the calling process.

    @Override public void getMyMemoryState(android.app.ActivityManager.RunningAppProcessInfo outInfo) throws android.os.RemoteException
    {
    }
    @Override public boolean killProcessesBelowForeground(java.lang.String reason) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.content.pm.UserInfo getCurrentUser() throws android.os.RemoteException
    {
      return null;
    }
    // This is not public because you need to be very careful in how you
    // manage your activity to make sure it is always the uid you expect.

    @Override public int getLaunchedFromUid(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void unstableProviderDied(android.os.IBinder connection) throws android.os.RemoteException
    {
    }
    @Override public boolean isIntentSenderAnActivity(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isIntentSenderAForegroundService(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isIntentSenderABroadcast(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int stopUser(int userid, boolean force, android.app.IStopUserCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void registerUserSwitchObserver(android.app.IUserSwitchObserver observer, java.lang.String name) throws android.os.RemoteException
    {
    }
    @Override public void unregisterUserSwitchObserver(android.app.IUserSwitchObserver observer) throws android.os.RemoteException
    {
    }
    @Override public int[] getRunningUserIds() throws android.os.RemoteException
    {
      return null;
    }
    // Request a heap dump for the system server.

    @Override public void requestSystemServerHeapDump() throws android.os.RemoteException
    {
    }
    // Deprecated - This method is only used by a few internal components and it will soon be
    // replaced by a proper bug report API (which will be restricted to a few, pre-defined apps).
    // No new code should be calling it.

    @Override public void requestBugReport(int bugreportType) throws android.os.RemoteException
    {
    }
    /**
         *  Takes a telephony bug report and notifies the user with the title and description
         *  that are passed to this API as parameters
         *
         *  @param shareTitle should be a valid legible string less than 50 chars long
         *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
         *
         *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
         *          paremeters cannot be encoding to an UTF-8 charset.
         */
    @Override public void requestTelephonyBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException
    {
    }
    /**
         *  Deprecated - This method is only used by Wifi, and it will soon be replaced by a proper
         *  bug report API.
         *
         *  Takes a minimal bugreport of Wifi-related state.
         *
         *  @param shareTitle should be a valid legible string less than 50 chars long
         *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
         *
         *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
         *          parameters cannot be encoding to an UTF-8 charset.
         */
    @Override public void requestWifiBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException
    {
    }
    @Override public android.content.Intent getIntentForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
    {
      return null;
    }
    // This is not public because you need to be very careful in how you
    // manage your activity to make sure it is always the uid you expect.

    @Override public java.lang.String getLaunchedFromPackage(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void killUid(int appId, int userId, java.lang.String reason) throws android.os.RemoteException
    {
    }
    @Override public void setUserIsMonkey(boolean monkey) throws android.os.RemoteException
    {
    }
    @Override public void hang(android.os.IBinder who, boolean allowRestart) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.app.ActivityManager.StackInfo> getAllStackInfos() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void moveTaskToStack(int taskId, int stackId, boolean toTop) throws android.os.RemoteException
    {
    }
    /**
         * Resizes the input stack id to the given bounds.
         *
         * @param stackId Id of the stack to resize.
         * @param bounds Bounds to resize the stack to or {@code null} for fullscreen.
         * @param allowResizeInDockedMode True if the resize should be allowed when the docked stack is
         *                                active.
         * @param preserveWindows True if the windows of activities contained in the stack should be
         *                        preserved.
         * @param animate True if the stack resize should be animated.
         * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
         *                          default animation duration should be used.
         * @throws RemoteException
         */
    @Override public void resizeStack(int stackId, android.graphics.Rect bounds, boolean allowResizeInDockedMode, boolean preserveWindows, boolean animate, int animationDuration) throws android.os.RemoteException
    {
    }
    @Override public void setFocusedStack(int stackId) throws android.os.RemoteException
    {
    }
    @Override public android.app.ActivityManager.StackInfo getFocusedStackInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void restart() throws android.os.RemoteException
    {
    }
    @Override public void performIdleMaintenance() throws android.os.RemoteException
    {
    }
    @Override public void appNotRespondingViaProvider(android.os.IBinder connection) throws android.os.RemoteException
    {
    }
    @Override public android.graphics.Rect getTaskBounds(int taskId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setProcessMemoryTrimLevel(java.lang.String process, int uid, int level) throws android.os.RemoteException
    {
      return false;
    }
    // Start of L transactions

    @Override public java.lang.String getTagForIntentSender(android.content.IIntentSender sender, java.lang.String prefix) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean startUserInBackground(int userid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isInLockTaskMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startRecentsActivity(android.content.Intent intent, android.app.IAssistDataReceiver assistDataReceiver, android.view.IRecentsAnimationRunner recentsAnimationRunner) throws android.os.RemoteException
    {
    }
    @Override public void cancelRecentsAnimation(boolean restoreHomeStackPosition) throws android.os.RemoteException
    {
    }
    @Override public int startActivityFromRecents(int taskId, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void startSystemLockTaskMode(int taskId) throws android.os.RemoteException
    {
    }
    @Override public boolean isTopOfTask(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void bootAnimationComplete() throws android.os.RemoteException
    {
    }
    @Override public int checkPermissionWithToken(java.lang.String permission, int pid, int uid, android.os.IBinder callerToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void registerTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
    {
    }
    @Override public void notifyCleartextNetwork(int uid, byte[] firstPacket) throws android.os.RemoteException
    {
    }
    @Override public void setTaskResizeable(int taskId, int resizeableMode) throws android.os.RemoteException
    {
    }
    @Override public void resizeTask(int taskId, android.graphics.Rect bounds, int resizeMode) throws android.os.RemoteException
    {
    }
    @Override public int getLockTaskModeState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setDumpHeapDebugLimit(java.lang.String processName, int uid, long maxMemSize, java.lang.String reportPackage) throws android.os.RemoteException
    {
    }
    @Override public void dumpHeapFinished(java.lang.String path) throws android.os.RemoteException
    {
    }
    @Override public void updateLockTaskPackages(int userId, java.lang.String[] packages) throws android.os.RemoteException
    {
    }
    @Override public void noteAlarmStart(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public void noteAlarmFinish(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public int getPackageProcessState(java.lang.String packageName, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void updateDeviceOwner(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    // Start of N transactions
    // Start Binder transaction tracking for all applications.

    @Override public boolean startBinderTracking() throws android.os.RemoteException
    {
      return false;
    }
    // Stop Binder transaction tracking for all applications and dump trace data to the given file
    // descriptor.

    @Override public boolean stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Try to place task to provided position. The final position might be different depending on
         * current user and stacks state. The task will be moved to target stack if it's currently in
         * different stack.
         */
    @Override public void positionTaskInStack(int taskId, int stackId, int position) throws android.os.RemoteException
    {
    }
    @Override public void suppressResizeConfigChanges(boolean suppress) throws android.os.RemoteException
    {
    }
    @Override public boolean moveTopActivityToPinnedStack(int stackId, android.graphics.Rect bounds) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isAppStartModeDisabled(int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean unlockUser(int userid, byte[] token, byte[] secret, android.os.IProgressListener listener) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void killPackageDependents(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Resizes the docked stack, and all other stacks as the result of the dock stack bounds change.
         *
         * @param dockedBounds The bounds for the docked stack.
         * @param tempDockedTaskBounds The temporary bounds for the tasks in the docked stack, which
         *                             might be different from the stack bounds to allow more
         *                             flexibility while resizing, or {@code null} if they should be the
         *                             same as the stack bounds.
         * @param tempDockedTaskInsetBounds The temporary bounds for the tasks to calculate the insets.
         *                                  When resizing, we usually "freeze" the layout of a task. To
         *                                  achieve that, we also need to "freeze" the insets, which
         *                                  gets achieved by changing task bounds but not bounds used
         *                                  to calculate the insets in this transient state
         * @param tempOtherTaskBounds The temporary bounds for the tasks in all other stacks, or
         *                            {@code null} if they should be the same as the stack bounds.
         * @param tempOtherTaskInsetBounds Like {@code tempDockedTaskInsetBounds}, but for the other
         *                                 stacks.
         * @throws RemoteException
         */
    @Override public void resizeDockedStack(android.graphics.Rect dockedBounds, android.graphics.Rect tempDockedTaskBounds, android.graphics.Rect tempDockedTaskInsetBounds, android.graphics.Rect tempOtherTaskBounds, android.graphics.Rect tempOtherTaskInsetBounds) throws android.os.RemoteException
    {
    }
    @Override public void removeStack(int stackId) throws android.os.RemoteException
    {
    }
    @Override public void makePackageIdle(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    @Override public int getMemoryTrimLevel() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isVrModePackageEnabled(android.content.ComponentName packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyLockedProfile(int userId) throws android.os.RemoteException
    {
    }
    @Override public void startConfirmDeviceCredentialIntent(android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void sendIdleJobTrigger() throws android.os.RemoteException
    {
    }
    @Override public int sendIntentSender(android.content.IIntentSender target, android.os.IBinder whitelistToken, int code, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isBackgroundRestricted(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    // Start of N MR1 transactions

    @Override public void setRenderThread(int tid) throws android.os.RemoteException
    {
    }
    /**
         * Lets activity manager know whether the calling process is currently showing "top-level" UI
         * that is not an activity, i.e. windows on the screen the user is currently interacting with.
         *
         * <p>This flag can only be set for persistent processes.
         *
         * @param hasTopUi Whether the calling process has "top-level" UI.
         */
    @Override public void setHasTopUi(boolean hasTopUi) throws android.os.RemoteException
    {
    }
    // Start of O transactions

    @Override public int restartUserInBackground(int userId) throws android.os.RemoteException
    {
      return 0;
    }
    /** Cancels the window transitions for the given task. */
    @Override public void cancelTaskWindowTransition(int taskId) throws android.os.RemoteException
    {
    }
    /**
         * @param taskId the id of the task to retrieve the sAutoapshots for
         * @param reducedResolution if set, if the snapshot needs to be loaded from disk, this will load
         *                          a reduced resolution of it, which is much faster
         * @return a graphic buffer representing a screenshot of a task
         */
    @Override public android.app.ActivityManager.TaskSnapshot getTaskSnapshot(int taskId, boolean reducedResolution) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void scheduleApplicationInfoChanged(java.util.List<java.lang.String> packageNames, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setPersistentVrThread(int tid) throws android.os.RemoteException
    {
    }
    @Override public void waitForNetworkStateUpdate(long procStateSeq) throws android.os.RemoteException
    {
    }
    /**
         * Add a bare uid to the background restrictions whitelist.  Only the system uid may call this.
         */
    @Override public void backgroundWhitelistUid(int uid) throws android.os.RemoteException
    {
    }
    // Start of P transactions
    /**
         *  Similar to {@link #startUserInBackground(int userId), but with a listener to report
         *  user unlock progress.
         */
    @Override public boolean startUserInBackgroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Method for the shell UID to start deletating its permission identity to an
         * active instrumenation. The shell can delegate permissions only to one active
         * instrumentation at a time. An active instrumentation is one running and
         * started from the shell.
         */
    @Override public void startDelegateShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException
    {
    }
    /**
         * Method for the shell UID to stop deletating its permission identity to an
         * active instrumenation. An active instrumentation is one running and
         * started from the shell.
         */
    @Override public void stopDelegateShellPermissionIdentity() throws android.os.RemoteException
    {
    }
    /** Returns a file descriptor that'll be closed when the system server process dies. */
    @Override public android.os.ParcelFileDescriptor getLifeMonitor() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Start user, if it us not already running, and bring it to foreground.
         * unlockProgressListener can be null if monitoring progress is not necessary.
         */
    @Override public boolean startUserInForegroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IActivityManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IActivityManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IActivityManager interface,
     * generating a proxy if needed.
     */
    public static android.app.IActivityManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IActivityManager))) {
        return ((android.app.IActivityManager)iin);
      }
      return new android.app.IActivityManager.Stub.Proxy(obj);
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
        case TRANSACTION_openContentUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _result = this.openContentUri(_arg0);
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
        case TRANSACTION_registerUidObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IUidObserver _arg0;
          _arg0 = android.app.IUidObserver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.registerUidObserver(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterUidObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IUidObserver _arg0;
          _arg0 = android.app.IUidObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterUidObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isUidActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isUidActive(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getUidProcessState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getUidProcessState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_handleApplicationCrash:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.app.ApplicationErrorReport.ParcelableCrashInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.ApplicationErrorReport.ParcelableCrashInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.handleApplicationCrash(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startActivity:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.IBinder _arg4;
          _arg4 = data.readStrongBinder();
          java.lang.String _arg5;
          _arg5 = data.readString();
          int _arg6;
          _arg6 = data.readInt();
          int _arg7;
          _arg7 = data.readInt();
          android.app.ProfilerInfo _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.app.ProfilerInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          android.os.Bundle _arg9;
          if ((0!=data.readInt())) {
            _arg9 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg9 = null;
          }
          int _result = this.startActivity(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unhandledBack:
        {
          data.enforceInterface(descriptor);
          this.unhandledBack();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finishActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.finishActivity(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerReceiver:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.IIntentReceiver _arg2;
          _arg2 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          android.content.IntentFilter _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.IntentFilter.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          android.content.Intent _result = this.registerReceiver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
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
        case TRANSACTION_unregisterReceiver:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentReceiver _arg0;
          _arg0 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          this.unregisterReceiver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_broadcastIntent:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.IIntentReceiver _arg3;
          _arg3 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          java.lang.String _arg5;
          _arg5 = data.readString();
          android.os.Bundle _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          java.lang.String[] _arg7;
          _arg7 = data.createStringArray();
          int _arg8;
          _arg8 = data.readInt();
          android.os.Bundle _arg9;
          if ((0!=data.readInt())) {
            _arg9 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg9 = null;
          }
          boolean _arg10;
          _arg10 = (0!=data.readInt());
          boolean _arg11;
          _arg11 = (0!=data.readInt());
          int _arg12;
          _arg12 = data.readInt();
          int _result = this.broadcastIntent(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11, _arg12);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unbroadcastIntent:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.unbroadcastIntent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finishReceiver:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          int _arg5;
          _arg5 = data.readInt();
          this.finishReceiver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_attachApplication:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          long _arg1;
          _arg1 = data.readLong();
          this.attachApplication(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTasks:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.app.ActivityManager.RunningTaskInfo> _result = this.getTasks(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getFilteredTasks:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.util.List<android.app.ActivityManager.RunningTaskInfo> _result = this.getFilteredTasks(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_moveTaskToFront:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.moveTaskToFront(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTaskForActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _result = this.getTaskForActivity(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getContentProvider:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.app.ContentProviderHolder _result = this.getContentProvider(_arg0, _arg1, _arg2, _arg3, _arg4);
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
        case TRANSACTION_publishContentProviders:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.util.List<android.app.ContentProviderHolder> _arg1;
          _arg1 = data.createTypedArrayList(android.app.ContentProviderHolder.CREATOR);
          this.publishContentProviders(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_refContentProvider:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.refContentProvider(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getRunningServiceControlPanel:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.PendingIntent _result = this.getRunningServiceControlPanel(_arg0);
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
        case TRANSACTION_startService:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _arg5;
          _arg5 = data.readInt();
          android.content.ComponentName _result = this.startService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
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
        case TRANSACTION_stopService:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.stopService(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_bindService:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.IServiceConnection _arg4;
          _arg4 = android.app.IServiceConnection.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          java.lang.String _arg6;
          _arg6 = data.readString();
          int _arg7;
          _arg7 = data.readInt();
          int _result = this.bindService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_bindIsolatedService:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.IServiceConnection _arg4;
          _arg4 = android.app.IServiceConnection.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          java.lang.String _arg6;
          _arg6 = data.readString();
          java.lang.String _arg7;
          _arg7 = data.readString();
          int _arg8;
          _arg8 = data.readInt();
          int _result = this.bindIsolatedService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateServiceGroup:
        {
          data.enforceInterface(descriptor);
          android.app.IServiceConnection _arg0;
          _arg0 = android.app.IServiceConnection.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.updateServiceGroup(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unbindService:
        {
          data.enforceInterface(descriptor);
          android.app.IServiceConnection _arg0;
          _arg0 = android.app.IServiceConnection.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unbindService(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_publishService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          this.publishService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDebugApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setDebugApp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAgentApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setAgentApp(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAlwaysFinish:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setAlwaysFinish(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startInstrumentation:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.app.IInstrumentationWatcher _arg4;
          _arg4 = android.app.IInstrumentationWatcher.Stub.asInterface(data.readStrongBinder());
          android.app.IUiAutomationConnection _arg5;
          _arg5 = android.app.IUiAutomationConnection.Stub.asInterface(data.readStrongBinder());
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          boolean _result = this.startInstrumentation(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addInstrumentationResults:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addInstrumentationResults(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finishInstrumentation:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.finishInstrumentation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getConfiguration:
        {
          data.enforceInterface(descriptor);
          android.content.res.Configuration _result = this.getConfiguration();
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
        case TRANSACTION_updateConfiguration:
        {
          data.enforceInterface(descriptor);
          android.content.res.Configuration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.res.Configuration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.updateConfiguration(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopServiceToken:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.stopServiceToken(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setProcessLimit:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setProcessLimit(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProcessLimit:
        {
          data.enforceInterface(descriptor);
          int _result = this.getProcessLimit();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkPermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _result = this.checkPermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkUriPermission:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          android.os.IBinder _arg5;
          _arg5 = data.readStrongBinder();
          int _result = this.checkUriPermission(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_grantUriPermission:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.grantUriPermission(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_revokeUriPermission:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.revokeUriPermission(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setActivityController:
        {
          data.enforceInterface(descriptor);
          android.app.IActivityController _arg0;
          _arg0 = android.app.IActivityController.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setActivityController(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showWaitingForDebugger:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.showWaitingForDebugger(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_signalPersistentProcesses:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.signalPersistentProcesses(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRecentTasks:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getRecentTasks(_arg0, _arg1, _arg2);
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
        case TRANSACTION_serviceDoneExecuting:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.serviceDoneExecuting(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getIntentSender:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          android.content.Intent[] _arg5;
          _arg5 = data.createTypedArray(android.content.Intent.CREATOR);
          java.lang.String[] _arg6;
          _arg6 = data.createStringArray();
          int _arg7;
          _arg7 = data.readInt();
          android.os.Bundle _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          int _arg9;
          _arg9 = data.readInt();
          android.content.IIntentSender _result = this.getIntentSender(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_cancelIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          this.cancelIntentSender(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackageForIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          java.lang.String _result = this.getPackageForIntentSender(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_registerIntentSenderCancelListener:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          com.android.internal.os.IResultReceiver _arg1;
          _arg1 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.registerIntentSenderCancelListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterIntentSenderCancelListener:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          com.android.internal.os.IResultReceiver _arg1;
          _arg1 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.unregisterIntentSenderCancelListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enterSafeMode:
        {
          data.enforceInterface(descriptor);
          this.enterSafeMode();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_noteWakeupAlarm:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.noteWakeupAlarm(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeContentProvider:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.removeContentProvider(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setRequestedOrientation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.setRequestedOrientation(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unbindFinished:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.unbindFinished(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setProcessImportant:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setProcessImportant(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setServiceForeground:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          android.app.Notification _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.Notification.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          this.setServiceForeground(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getForegroundServiceType:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _result = this.getForegroundServiceType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_moveActivityTaskToBack:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.moveActivityTaskToBack(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getMemoryInfo:
        {
          data.enforceInterface(descriptor);
          android.app.ActivityManager.MemoryInfo _arg0;
          _arg0 = new android.app.ActivityManager.MemoryInfo();
          this.getMemoryInfo(_arg0);
          reply.writeNoException();
          if ((_arg0!=null)) {
            reply.writeInt(1);
            _arg0.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getProcessesInErrorState:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.app.ActivityManager.ProcessErrorStateInfo> _result = this.getProcessesInErrorState();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_clearApplicationUserData:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.content.pm.IPackageDataObserver _arg2;
          _arg2 = android.content.pm.IPackageDataObserver.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.clearApplicationUserData(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_forceStopPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.forceStopPackage(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_killPids:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _result = this.killPids(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.app.ActivityManager.RunningServiceInfo> _result = this.getServices(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getRunningAppProcesses:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.app.ActivityManager.RunningAppProcessInfo> _result = this.getRunningAppProcesses();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_peekService:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.IBinder _result = this.peekService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_profileControl:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.app.ProfilerInfo _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.ProfilerInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          boolean _result = this.profileControl(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.shutdown(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopAppSwitches:
        {
          data.enforceInterface(descriptor);
          this.stopAppSwitches();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resumeAppSwitches:
        {
          data.enforceInterface(descriptor);
          this.resumeAppSwitches();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_bindBackupAgent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.bindBackupAgent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_backupAgentCreated:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          this.backupAgentCreated(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unbindBackupAgent:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ApplicationInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.unbindBackupAgent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUidForIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          int _result = this.getUidForIntentSender(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_handleIncomingUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          java.lang.String _arg5;
          _arg5 = data.readString();
          java.lang.String _arg6;
          _arg6 = data.readString();
          int _result = this.handleIncomingUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addPackageDependency:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.addPackageDependency(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_killApplication:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.killApplication(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeSystemDialogs:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.closeSystemDialogs(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProcessMemoryInfo:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          android.os.Debug.MemoryInfo[] _result = this.getProcessMemoryInfo(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_killApplicationProcess:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.killApplicationProcess(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handleApplicationWtf:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.app.ApplicationErrorReport.ParcelableCrashInfo _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.ApplicationErrorReport.ParcelableCrashInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _result = this.handleApplicationWtf(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_killBackgroundProcesses:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.killBackgroundProcesses(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isUserAMonkey:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isUserAMonkey();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getRunningExternalApplications:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.content.pm.ApplicationInfo> _result = this.getRunningExternalApplications();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_finishHeavyWeightApp:
        {
          data.enforceInterface(descriptor);
          this.finishHeavyWeightApp();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handleApplicationStrictModeViolation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          android.os.StrictMode.ViolationInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.StrictMode.ViolationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.handleApplicationStrictModeViolation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTopActivityImmersive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isTopActivityImmersive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_crashApplication:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.crashApplication(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProviderMimeType:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _result = this.getProviderMimeType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_dumpHeap:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          java.lang.String _arg5;
          _arg5 = data.readString();
          android.os.ParcelFileDescriptor _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          android.os.RemoteCallback _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          boolean _result = this.dumpHeap(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isUserRunning:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isUserRunning(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPackageScreenCompatMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setPackageScreenCompatMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_switchUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.switchUser(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.removeTask(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerProcessObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IProcessObserver _arg0;
          _arg0 = android.app.IProcessObserver.Stub.asInterface(data.readStrongBinder());
          this.registerProcessObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterProcessObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IProcessObserver _arg0;
          _arg0 = android.app.IProcessObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterProcessObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isIntentSenderTargetedToPackage:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isIntentSenderTargetedToPackage(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updatePersistentConfiguration:
        {
          data.enforceInterface(descriptor);
          android.content.res.Configuration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.res.Configuration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updatePersistentConfiguration(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getProcessPss:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          long[] _result = this.getProcessPss(_arg0);
          reply.writeNoException();
          reply.writeLongArray(_result);
          return true;
        }
        case TRANSACTION_showBootMessage:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.showBootMessage(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_killAllBackgroundProcesses:
        {
          data.enforceInterface(descriptor);
          this.killAllBackgroundProcesses();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getContentProviderExternal:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.ContentProviderHolder _result = this.getContentProviderExternal(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_removeContentProviderExternal:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.removeContentProviderExternal(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeContentProviderExternalAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          this.removeContentProviderExternalAsUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMyMemoryState:
        {
          data.enforceInterface(descriptor);
          android.app.ActivityManager.RunningAppProcessInfo _arg0;
          _arg0 = new android.app.ActivityManager.RunningAppProcessInfo();
          this.getMyMemoryState(_arg0);
          reply.writeNoException();
          if ((_arg0!=null)) {
            reply.writeInt(1);
            _arg0.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_killProcessesBelowForeground:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.killProcessesBelowForeground(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentUser:
        {
          data.enforceInterface(descriptor);
          android.content.pm.UserInfo _result = this.getCurrentUser();
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
        case TRANSACTION_getLaunchedFromUid:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.getLaunchedFromUid(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unstableProviderDied:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.unstableProviderDied(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isIntentSenderAnActivity:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isIntentSenderAnActivity(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isIntentSenderAForegroundService:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isIntentSenderAForegroundService(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isIntentSenderABroadcast:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.isIntentSenderABroadcast(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startActivityAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.IBinder _arg4;
          _arg4 = data.readStrongBinder();
          java.lang.String _arg5;
          _arg5 = data.readString();
          int _arg6;
          _arg6 = data.readInt();
          int _arg7;
          _arg7 = data.readInt();
          android.app.ProfilerInfo _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.app.ProfilerInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          android.os.Bundle _arg9;
          if ((0!=data.readInt())) {
            _arg9 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg9 = null;
          }
          int _arg10;
          _arg10 = data.readInt();
          int _result = this.startActivityAsUser(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.app.IStopUserCallback _arg2;
          _arg2 = android.app.IStopUserCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.stopUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerUserSwitchObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IUserSwitchObserver _arg0;
          _arg0 = android.app.IUserSwitchObserver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.registerUserSwitchObserver(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterUserSwitchObserver:
        {
          data.enforceInterface(descriptor);
          android.app.IUserSwitchObserver _arg0;
          _arg0 = android.app.IUserSwitchObserver.Stub.asInterface(data.readStrongBinder());
          this.unregisterUserSwitchObserver(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRunningUserIds:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getRunningUserIds();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_requestSystemServerHeapDump:
        {
          data.enforceInterface(descriptor);
          this.requestSystemServerHeapDump();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestBugReport:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.requestBugReport(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestTelephonyBugReport:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.requestTelephonyBugReport(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestWifiBugReport:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.requestWifiBugReport(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getIntentForIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _result = this.getIntentForIntentSender(_arg0);
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
        case TRANSACTION_getLaunchedFromPackage:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _result = this.getLaunchedFromPackage(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_killUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.killUid(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUserIsMonkey:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setUserIsMonkey(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hang:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.hang(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAllStackInfos:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.app.ActivityManager.StackInfo> _result = this.getAllStackInfos();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_moveTaskToStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.moveTaskToStack(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resizeStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          int _arg5;
          _arg5 = data.readInt();
          this.resizeStack(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFocusedStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setFocusedStack(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFocusedStackInfo:
        {
          data.enforceInterface(descriptor);
          android.app.ActivityManager.StackInfo _result = this.getFocusedStackInfo();
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
        case TRANSACTION_restart:
        {
          data.enforceInterface(descriptor);
          this.restart();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_performIdleMaintenance:
        {
          data.enforceInterface(descriptor);
          this.performIdleMaintenance();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_appNotRespondingViaProvider:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.appNotRespondingViaProvider(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTaskBounds:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Rect _result = this.getTaskBounds(_arg0);
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
        case TRANSACTION_setProcessMemoryTrimLevel:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setProcessMemoryTrimLevel(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getTagForIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getTagForIntentSender(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_startUserInBackground:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.startUserInBackground(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isInLockTaskMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInLockTaskMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startRecentsActivity:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.IAssistDataReceiver _arg1;
          _arg1 = android.app.IAssistDataReceiver.Stub.asInterface(data.readStrongBinder());
          android.view.IRecentsAnimationRunner _arg2;
          _arg2 = android.view.IRecentsAnimationRunner.Stub.asInterface(data.readStrongBinder());
          this.startRecentsActivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelRecentsAnimation:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.cancelRecentsAnimation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startActivityFromRecents:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.startActivityFromRecents(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startSystemLockTaskMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.startSystemLockTaskMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTopOfTask:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.isTopOfTask(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_bootAnimationComplete:
        {
          data.enforceInterface(descriptor);
          this.bootAnimationComplete();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkPermissionWithToken:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          int _result = this.checkPermissionWithToken(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerTaskStackListener:
        {
          data.enforceInterface(descriptor);
          android.app.ITaskStackListener _arg0;
          _arg0 = android.app.ITaskStackListener.Stub.asInterface(data.readStrongBinder());
          this.registerTaskStackListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTaskStackListener:
        {
          data.enforceInterface(descriptor);
          android.app.ITaskStackListener _arg0;
          _arg0 = android.app.ITaskStackListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterTaskStackListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCleartextNetwork:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.notifyCleartextNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTaskResizeable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setTaskResizeable(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resizeTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.resizeTask(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLockTaskModeState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getLockTaskModeState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDumpHeapDebugLimit:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setDumpHeapDebugLimit(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dumpHeapFinished:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.dumpHeapFinished(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateLockTaskPackages:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.updateLockTaskPackages(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_noteAlarmStart:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.noteAlarmStart(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_noteAlarmFinish:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.noteAlarmFinish(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackageProcessState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getPackageProcessState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateDeviceOwner:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.updateDeviceOwner(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startBinderTracking:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.startBinderTracking();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopBinderTrackingAndDump:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.stopBinderTrackingAndDump(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_positionTaskInStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.positionTaskInStack(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_suppressResizeConfigChanges:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.suppressResizeConfigChanges(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_moveTopActivityToPinnedStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.moveTopActivityToPinnedStack(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isAppStartModeDisabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isAppStartModeDisabled(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unlockUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          android.os.IProgressListener _arg3;
          _arg3 = android.os.IProgressListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unlockUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_killPackageDependents:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.killPackageDependents(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resizeDockedStack:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
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
          android.graphics.Rect _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.resizeDockedStack(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeStack(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_makePackageIdle:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.makePackageIdle(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMemoryTrimLevel:
        {
          data.enforceInterface(descriptor);
          int _result = this.getMemoryTrimLevel();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isVrModePackageEnabled:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isVrModePackageEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyLockedProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyLockedProfile(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startConfirmDeviceCredentialIntent:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.startConfirmDeviceCredentialIntent(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendIdleJobTrigger:
        {
          data.enforceInterface(descriptor);
          this.sendIdleJobTrigger();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendIntentSender:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentSender _arg0;
          _arg0 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          android.content.Intent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.content.IIntentReceiver _arg5;
          _arg5 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg6;
          _arg6 = data.readString();
          android.os.Bundle _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          int _result = this.sendIntentSender(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isBackgroundRestricted:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isBackgroundRestricted(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setRenderThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setRenderThread(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setHasTopUi:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setHasTopUi(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restartUserInBackground:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.restartUserInBackground(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_cancelTaskWindowTransition:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.cancelTaskWindowTransition(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTaskSnapshot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.app.ActivityManager.TaskSnapshot _result = this.getTaskSnapshot(_arg0, _arg1);
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
        case TRANSACTION_scheduleApplicationInfoChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          int _arg1;
          _arg1 = data.readInt();
          this.scheduleApplicationInfoChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPersistentVrThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setPersistentVrThread(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_waitForNetworkStateUpdate:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.waitForNetworkStateUpdate(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_backgroundWhitelistUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.backgroundWhitelistUid(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startUserInBackgroundWithListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IProgressListener _arg1;
          _arg1 = android.os.IProgressListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.startUserInBackgroundWithListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startDelegateShellPermissionIdentity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.startDelegateShellPermissionIdentity(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopDelegateShellPermissionIdentity:
        {
          data.enforceInterface(descriptor);
          this.stopDelegateShellPermissionIdentity();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLifeMonitor:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _result = this.getLifeMonitor();
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
        case TRANSACTION_startUserInForegroundWithListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IProgressListener _arg1;
          _arg1 = android.os.IProgressListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.startUserInForegroundWithListener(_arg0, _arg1);
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
    private static class Proxy implements android.app.IActivityManager
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
      // WARNING: when these transactions are updated, check if they are any callers on the native
      // side. If so, make sure they are using the correct transaction ids and arguments.
      // If a transaction which will also be used on the native side is being inserted, add it to
      // below block of transactions.
      // Since these transactions are also called from native code, these must be kept in sync with
      // the ones in frameworks/native/libs/binder/include/binder/IActivityManager.h
      // =============== Beginning of transactions used on native side as well ======================

      @Override public android.os.ParcelFileDescriptor openContentUri(java.lang.String uriString) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uriString);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openContentUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openContentUri(uriString);
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
      @Override public void registerUidObserver(android.app.IUidObserver observer, int which, int cutpoint, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(which);
          _data.writeInt(cutpoint);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUidObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerUidObserver(observer, which, cutpoint, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterUidObserver(android.app.IUidObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUidObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUidObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isUidActive(int uid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUidActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUidActive(uid, callingPackage);
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
      @Override public int getUidProcessState(int uid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUidProcessState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUidProcessState(uid, callingPackage);
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
      // =============== End of transactions used on native side as well ============================
      // Special low-level communication with activity manager.

      @Override public void handleApplicationCrash(android.os.IBinder app, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(app);
          if ((crashInfo!=null)) {
            _data.writeInt(1);
            crashInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleApplicationCrash, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleApplicationCrash(app, crashInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int startActivity(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder(resultTo);
          _data.writeString(resultWho);
          _data.writeInt(requestCode);
          _data.writeInt(flags);
          if ((profilerInfo!=null)) {
            _data.writeInt(1);
            profilerInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivity(caller, callingPackage, intent, resolvedType, resultTo, resultWho, requestCode, flags, profilerInfo, options);
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
      @Override public void unhandledBack() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unhandledBack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unhandledBack();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean finishActivity(android.os.IBinder token, int code, android.content.Intent data, int finishTask) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(code);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(finishTask);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().finishActivity(token, code, data, finishTask);
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
      @Override public android.content.Intent registerReceiver(android.app.IApplicationThread caller, java.lang.String callerPackage, android.content.IIntentReceiver receiver, android.content.IntentFilter filter, java.lang.String requiredPermission, int userId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callerPackage);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          if ((filter!=null)) {
            _data.writeInt(1);
            filter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(requiredPermission);
          _data.writeInt(userId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerReceiver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerReceiver(caller, callerPackage, receiver, filter, requiredPermission, userId, flags);
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
      @Override public void unregisterReceiver(android.content.IIntentReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterReceiver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterReceiver(receiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int broadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver resultTo, int resultCode, java.lang.String resultData, android.os.Bundle map, java.lang.String[] requiredPermissions, int appOp, android.os.Bundle options, boolean serialized, boolean sticky, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder((((resultTo!=null))?(resultTo.asBinder()):(null)));
          _data.writeInt(resultCode);
          _data.writeString(resultData);
          if ((map!=null)) {
            _data.writeInt(1);
            map.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(requiredPermissions);
          _data.writeInt(appOp);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((serialized)?(1):(0)));
          _data.writeInt(((sticky)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_broadcastIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().broadcastIntent(caller, intent, resolvedType, resultTo, resultCode, resultData, map, requiredPermissions, appOp, options, serialized, sticky, userId);
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
      @Override public void unbroadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unbroadcastIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unbroadcastIntent(caller, intent, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finishReceiver(android.os.IBinder who, int resultCode, java.lang.String resultData, android.os.Bundle map, boolean abortBroadcast, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(who);
          _data.writeInt(resultCode);
          _data.writeString(resultData);
          if ((map!=null)) {
            _data.writeInt(1);
            map.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((abortBroadcast)?(1):(0)));
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishReceiver, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishReceiver(who, resultCode, resultData, map, abortBroadcast, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void attachApplication(android.app.IApplicationThread app, long startSeq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((app!=null))?(app.asBinder()):(null)));
          _data.writeLong(startSeq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_attachApplication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attachApplication(app, startSeq);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.app.ActivityManager.RunningTaskInfo> getTasks(int maxNum) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.RunningTaskInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(maxNum);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTasks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTasks(maxNum);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.RunningTaskInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.app.ActivityManager.RunningTaskInfo> getFilteredTasks(int maxNum, int ignoreActivityType, int ignoreWindowingMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.RunningTaskInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(maxNum);
          _data.writeInt(ignoreActivityType);
          _data.writeInt(ignoreWindowingMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFilteredTasks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFilteredTasks(maxNum, ignoreActivityType, ignoreWindowingMode);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.RunningTaskInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void moveTaskToFront(android.app.IApplicationThread caller, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          _data.writeInt(task);
          _data.writeInt(flags);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveTaskToFront, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().moveTaskToFront(caller, callingPackage, task, flags, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getTaskForActivity(android.os.IBinder token, boolean onlyRoot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((onlyRoot)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTaskForActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTaskForActivity(token, onlyRoot);
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
      @Override public android.app.ContentProviderHolder getContentProvider(android.app.IApplicationThread caller, java.lang.String callingPackage, java.lang.String name, int userId, boolean stable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ContentProviderHolder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          _data.writeString(name);
          _data.writeInt(userId);
          _data.writeInt(((stable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContentProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContentProvider(caller, callingPackage, name, userId, stable);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ContentProviderHolder.CREATOR.createFromParcel(_reply);
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
      @Override public void publishContentProviders(android.app.IApplicationThread caller, java.util.List<android.app.ContentProviderHolder> providers) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeTypedList(providers);
          boolean _status = mRemote.transact(Stub.TRANSACTION_publishContentProviders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().publishContentProviders(caller, providers);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean refContentProvider(android.os.IBinder connection, int stableDelta, int unstableDelta) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(connection);
          _data.writeInt(stableDelta);
          _data.writeInt(unstableDelta);
          boolean _status = mRemote.transact(Stub.TRANSACTION_refContentProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().refContentProvider(connection, stableDelta, unstableDelta);
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
      @Override public android.app.PendingIntent getRunningServiceControlPanel(android.content.ComponentName service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.PendingIntent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRunningServiceControlPanel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRunningServiceControlPanel(service);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.PendingIntent.CREATOR.createFromParcel(_reply);
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
      @Override public android.content.ComponentName startService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, boolean requireForeground, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeInt(((requireForeground)?(1):(0)));
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startService(caller, service, resolvedType, requireForeground, callingPackage, userId);
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
      @Override public int stopService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopService(caller, service, resolvedType, userId);
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
      // Currently keeping old bindService because it is on the greylist

      @Override public int bindService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeStrongBinder(token);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bindService(caller, token, service, resolvedType, connection, flags, callingPackage, userId);
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
      @Override public int bindIsolatedService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String instanceName, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeStrongBinder(token);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeString(instanceName);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindIsolatedService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bindIsolatedService(caller, token, service, resolvedType, connection, flags, instanceName, callingPackage, userId);
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
      @Override public void updateServiceGroup(android.app.IServiceConnection connection, int group, int importance) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(group);
          _data.writeInt(importance);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateServiceGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateServiceGroup(connection, group, importance);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean unbindService(android.app.IServiceConnection connection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unbindService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unbindService(connection);
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
      @Override public void publishService(android.os.IBinder token, android.content.Intent intent, android.os.IBinder service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(service);
          boolean _status = mRemote.transact(Stub.TRANSACTION_publishService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().publishService(token, intent, service);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setDebugApp(java.lang.String packageName, boolean waitForDebugger, boolean persistent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((waitForDebugger)?(1):(0)));
          _data.writeInt(((persistent)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDebugApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDebugApp(packageName, waitForDebugger, persistent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAgentApp(java.lang.String packageName, java.lang.String agent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(agent);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAgentApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAgentApp(packageName, agent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAlwaysFinish(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAlwaysFinish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAlwaysFinish(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean startInstrumentation(android.content.ComponentName className, java.lang.String profileFile, int flags, android.os.Bundle arguments, android.app.IInstrumentationWatcher watcher, android.app.IUiAutomationConnection connection, int userId, java.lang.String abiOverride) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((className!=null)) {
            _data.writeInt(1);
            className.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(profileFile);
          _data.writeInt(flags);
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((watcher!=null))?(watcher.asBinder()):(null)));
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(userId);
          _data.writeString(abiOverride);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInstrumentation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startInstrumentation(className, profileFile, flags, arguments, watcher, connection, userId, abiOverride);
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
      @Override public void addInstrumentationResults(android.app.IApplicationThread target, android.os.Bundle results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((target!=null))?(target.asBinder()):(null)));
          if ((results!=null)) {
            _data.writeInt(1);
            results.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addInstrumentationResults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addInstrumentationResults(target, results);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finishInstrumentation(android.app.IApplicationThread target, int resultCode, android.os.Bundle results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((target!=null))?(target.asBinder()):(null)));
          _data.writeInt(resultCode);
          if ((results!=null)) {
            _data.writeInt(1);
            results.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishInstrumentation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishInstrumentation(target, resultCode, results);
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
           * @return A copy of global {@link Configuration}, contains general settings for the entire
           *         system. Corresponds to the configuration of the default display.
           * @throws RemoteException
           */
      @Override public android.content.res.Configuration getConfiguration() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.res.Configuration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfiguration();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.res.Configuration.CREATOR.createFromParcel(_reply);
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
           * Updates global configuration and applies changes to the entire system.
           * @param values Update values for global configuration. If null is passed it will request the
           *               Window Manager to compute new config for the default display.
           * @throws RemoteException
           * @return Returns true if the configuration was updated.
           */
      @Override public boolean updateConfiguration(android.content.res.Configuration values) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((values!=null)) {
            _data.writeInt(1);
            values.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateConfiguration(values);
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
      @Override public boolean stopServiceToken(android.content.ComponentName className, android.os.IBinder token, int startId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((className!=null)) {
            _data.writeInt(1);
            className.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          _data.writeInt(startId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopServiceToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopServiceToken(className, token, startId);
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
      @Override public void setProcessLimit(int max) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(max);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProcessLimit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProcessLimit(max);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getProcessLimit() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProcessLimit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProcessLimit();
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
      @Override public int checkPermission(java.lang.String permission, int pid, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(permission);
          _data.writeInt(pid);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkPermission(permission, pid, uid);
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
      @Override public int checkUriPermission(android.net.Uri uri, int pid, int uid, int mode, int userId, android.os.IBinder callerToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeInt(mode);
          _data.writeInt(userId);
          _data.writeStrongBinder(callerToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkUriPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkUriPermission(uri, pid, uid, mode, userId, callerToken);
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
      @Override public void grantUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(targetPkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(mode);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantUriPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantUriPermission(caller, targetPkg, uri, mode, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void revokeUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(targetPkg);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(mode);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeUriPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeUriPermission(caller, targetPkg, uri, mode, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setActivityController(android.app.IActivityController watcher, boolean imAMonkey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((watcher!=null))?(watcher.asBinder()):(null)));
          _data.writeInt(((imAMonkey)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActivityController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActivityController(watcher, imAMonkey);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showWaitingForDebugger(android.app.IApplicationThread who, boolean waiting) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((who!=null))?(who.asBinder()):(null)));
          _data.writeInt(((waiting)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showWaitingForDebugger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showWaitingForDebugger(who, waiting);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /*
           * This will deliver the specified signal to all the persistent processes. Currently only
           * SIGUSR1 is delivered. All others are ignored.
           */
      @Override public void signalPersistentProcesses(int signal) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(signal);
          boolean _status = mRemote.transact(Stub.TRANSACTION_signalPersistentProcesses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().signalPersistentProcesses(signal);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getRecentTasks(int maxNum, int flags, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(maxNum);
          _data.writeInt(flags);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRecentTasks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRecentTasks(maxNum, flags, userId);
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
      @Override public void serviceDoneExecuting(android.os.IBinder token, int type, int startId, int res) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(type);
          _data.writeInt(startId);
          _data.writeInt(res);
          boolean _status = mRemote.transact(Stub.TRANSACTION_serviceDoneExecuting, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().serviceDoneExecuting(token, type, startId, res);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public android.content.IIntentSender getIntentSender(int type, java.lang.String packageName, android.os.IBinder token, java.lang.String resultWho, int requestCode, android.content.Intent[] intents, java.lang.String[] resolvedTypes, int flags, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.IIntentSender _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(type);
          _data.writeString(packageName);
          _data.writeStrongBinder(token);
          _data.writeString(resultWho);
          _data.writeInt(requestCode);
          _data.writeTypedArray(intents, 0);
          _data.writeStringArray(resolvedTypes);
          _data.writeInt(flags);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIntentSender(type, packageName, token, resultWho, requestCode, intents, resolvedTypes, flags, options, userId);
          }
          _reply.readException();
          _result = android.content.IIntentSender.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void cancelIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelIntentSender(sender);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getPackageForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageForIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageForIntentSender(sender);
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
      @Override public void registerIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerIntentSenderCancelListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerIntentSenderCancelListener(sender, receiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterIntentSenderCancelListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterIntentSenderCancelListener(sender, receiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void enterSafeMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enterSafeMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enterSafeMode();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void noteWakeupAlarm(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String sourcePkg, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sourceUid);
          _data.writeString(sourcePkg);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_noteWakeupAlarm, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().noteWakeupAlarm(sender, workSource, sourceUid, sourcePkg, tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeContentProvider(android.os.IBinder connection, boolean stable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(connection);
          _data.writeInt(((stable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeContentProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeContentProvider(connection, stable);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRequestedOrientation(android.os.IBinder token, int requestedOrientation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(requestedOrientation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRequestedOrientation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRequestedOrientation(token, requestedOrientation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unbindFinished(android.os.IBinder token, android.content.Intent service, boolean doRebind) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((doRebind)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unbindFinished, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unbindFinished(token, service, doRebind);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setProcessImportant(android.os.IBinder token, int pid, boolean isForeground, java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(pid);
          _data.writeInt(((isForeground)?(1):(0)));
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProcessImportant, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProcessImportant(token, pid, isForeground, reason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setServiceForeground(android.content.ComponentName className, android.os.IBinder token, int id, android.app.Notification notification, int flags, int foregroundServiceType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((className!=null)) {
            _data.writeInt(1);
            className.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          _data.writeInt(id);
          if ((notification!=null)) {
            _data.writeInt(1);
            notification.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          _data.writeInt(foregroundServiceType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setServiceForeground, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setServiceForeground(className, token, id, notification, flags, foregroundServiceType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getForegroundServiceType(android.content.ComponentName className, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((className!=null)) {
            _data.writeInt(1);
            className.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getForegroundServiceType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getForegroundServiceType(className, token);
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
      @Override public boolean moveActivityTaskToBack(android.os.IBinder token, boolean nonRoot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((nonRoot)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveActivityTaskToBack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().moveActivityTaskToBack(token, nonRoot);
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
      @Override public void getMemoryInfo(android.app.ActivityManager.MemoryInfo outInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMemoryInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getMemoryInfo(outInfo);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            outInfo.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.app.ActivityManager.ProcessErrorStateInfo> getProcessesInErrorState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.ProcessErrorStateInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProcessesInErrorState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProcessesInErrorState();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.ProcessErrorStateInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean clearApplicationUserData(java.lang.String packageName, boolean keepState, android.content.pm.IPackageDataObserver observer, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((keepState)?(1):(0)));
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearApplicationUserData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearApplicationUserData(packageName, keepState, observer, userId);
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
      @Override public void forceStopPackage(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceStopPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceStopPackage(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean killPids(int[] pids, java.lang.String reason, boolean secure) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(pids);
          _data.writeString(reason);
          _data.writeInt(((secure)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_killPids, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().killPids(pids, reason, secure);
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
      @Override public java.util.List<android.app.ActivityManager.RunningServiceInfo> getServices(int maxNum, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.RunningServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(maxNum);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServices(maxNum, flags);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.RunningServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Retrieve running application processes in the system

      @Override public java.util.List<android.app.ActivityManager.RunningAppProcessInfo> getRunningAppProcesses() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.RunningAppProcessInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRunningAppProcesses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRunningAppProcesses();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.RunningAppProcessInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.IBinder peekService(android.content.Intent service, java.lang.String resolvedType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_peekService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().peekService(service, resolvedType, callingPackage);
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Turn on/off profiling in a particular process.

      @Override public boolean profileControl(java.lang.String process, int userId, boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(process);
          _data.writeInt(userId);
          _data.writeInt(((start)?(1):(0)));
          if ((profilerInfo!=null)) {
            _data.writeInt(1);
            profilerInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(profileType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_profileControl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().profileControl(process, userId, start, profilerInfo, profileType);
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
      @Override public boolean shutdown(int timeout) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeout);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shutdown(timeout);
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
      @Override public void stopAppSwitches() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopAppSwitches, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopAppSwitches();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resumeAppSwitches() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resumeAppSwitches, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resumeAppSwitches();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean bindBackupAgent(java.lang.String packageName, int backupRestoreMode, int targetUserId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(backupRestoreMode);
          _data.writeInt(targetUserId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindBackupAgent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bindBackupAgent(packageName, backupRestoreMode, targetUserId);
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
      @Override public void backupAgentCreated(java.lang.String packageName, android.os.IBinder agent, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeStrongBinder(agent);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_backupAgentCreated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().backupAgentCreated(packageName, agent, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unbindBackupAgent(android.content.pm.ApplicationInfo appInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((appInfo!=null)) {
            _data.writeInt(1);
            appInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unbindBackupAgent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unbindBackupAgent(appInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getUidForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUidForIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUidForIntentSender(sender);
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
      @Override public int handleIncomingUser(int callingPid, int callingUid, int userId, boolean allowAll, boolean requireFull, java.lang.String name, java.lang.String callerPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callingPid);
          _data.writeInt(callingUid);
          _data.writeInt(userId);
          _data.writeInt(((allowAll)?(1):(0)));
          _data.writeInt(((requireFull)?(1):(0)));
          _data.writeString(name);
          _data.writeString(callerPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleIncomingUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handleIncomingUser(callingPid, callingUid, userId, allowAll, requireFull, name, callerPackage);
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
      @Override public void addPackageDependency(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPackageDependency, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPackageDependency(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void killApplication(java.lang.String pkg, int appId, int userId, java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeInt(appId);
          _data.writeInt(userId);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killApplication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killApplication(pkg, appId, userId, reason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSystemDialogs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSystemDialogs(reason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Debug.MemoryInfo[] getProcessMemoryInfo(int[] pids) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Debug.MemoryInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(pids);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProcessMemoryInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProcessMemoryInfo(pids);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.os.Debug.MemoryInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void killApplicationProcess(java.lang.String processName, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(processName);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killApplicationProcess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killApplicationProcess(processName, uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Special low-level communication with activity manager.

      @Override public boolean handleApplicationWtf(android.os.IBinder app, java.lang.String tag, boolean system, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(app);
          _data.writeString(tag);
          _data.writeInt(((system)?(1):(0)));
          if ((crashInfo!=null)) {
            _data.writeInt(1);
            crashInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleApplicationWtf, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handleApplicationWtf(app, tag, system, crashInfo);
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
      @Override public void killBackgroundProcesses(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killBackgroundProcesses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killBackgroundProcesses(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isUserAMonkey() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUserAMonkey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUserAMonkey();
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
      // Retrieve info of applications installed on external media that are currently
      // running.

      @Override public java.util.List<android.content.pm.ApplicationInfo> getRunningExternalApplications() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.content.pm.ApplicationInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRunningExternalApplications, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRunningExternalApplications();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.content.pm.ApplicationInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void finishHeavyWeightApp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishHeavyWeightApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishHeavyWeightApp();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // A StrictMode violation to be handled.

      @Override public void handleApplicationStrictModeViolation(android.os.IBinder app, int penaltyMask, android.os.StrictMode.ViolationInfo crashInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(app);
          _data.writeInt(penaltyMask);
          if ((crashInfo!=null)) {
            _data.writeInt(1);
            crashInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleApplicationStrictModeViolation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleApplicationStrictModeViolation(app, penaltyMask, crashInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isTopActivityImmersive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTopActivityImmersive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTopActivityImmersive();
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
      @Override public void crashApplication(int uid, int initialPid, java.lang.String packageName, int userId, java.lang.String message, boolean force) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(initialPid);
          _data.writeString(packageName);
          _data.writeInt(userId);
          _data.writeString(message);
          _data.writeInt(((force)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_crashApplication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().crashApplication(uid, initialPid, packageName, userId, message, force);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getProviderMimeType(android.net.Uri uri, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProviderMimeType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProviderMimeType(uri, userId);
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
      // Cause the specified process to dump the specified heap.

      @Override public boolean dumpHeap(java.lang.String process, int userId, boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(process);
          _data.writeInt(userId);
          _data.writeInt(((managed)?(1):(0)));
          _data.writeInt(((mallocInfo)?(1):(0)));
          _data.writeInt(((runGc)?(1):(0)));
          _data.writeString(path);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((finishCallback!=null)) {
            _data.writeInt(1);
            finishCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpHeap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().dumpHeap(process, userId, managed, mallocInfo, runGc, path, fd, finishCallback);
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
      @Override public boolean isUserRunning(int userid, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUserRunning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUserRunning(userid, flags);
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
      @Override public void setPackageScreenCompatMode(java.lang.String packageName, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPackageScreenCompatMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPackageScreenCompatMode(packageName, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean switchUser(int userid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().switchUser(userid);
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
      @Override public boolean removeTask(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeTask(taskId);
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
      @Override public void registerProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerProcessObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerProcessObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterProcessObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterProcessObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isIntentSenderTargetedToPackage(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isIntentSenderTargetedToPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isIntentSenderTargetedToPackage(sender);
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
      @Override public void updatePersistentConfiguration(android.content.res.Configuration values) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((values!=null)) {
            _data.writeInt(1);
            values.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePersistentConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updatePersistentConfiguration(values);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long[] getProcessPss(int[] pids) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(pids);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProcessPss, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProcessPss(pids);
          }
          _reply.readException();
          _result = _reply.createLongArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void showBootMessage(java.lang.CharSequence msg, boolean always) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (msg!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(msg, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((always)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_showBootMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showBootMessage(msg, always);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void killAllBackgroundProcesses() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killAllBackgroundProcesses, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killAllBackgroundProcesses();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.app.ContentProviderHolder getContentProviderExternal(java.lang.String name, int userId, android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ContentProviderHolder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeInt(userId);
          _data.writeStrongBinder(token);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContentProviderExternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContentProviderExternal(name, userId, token, tag);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ContentProviderHolder.CREATOR.createFromParcel(_reply);
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
      /** @deprecated - Use {@link #removeContentProviderExternalAsUser} which takes a user ID. */
      @Override public void removeContentProviderExternal(java.lang.String name, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeContentProviderExternal, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeContentProviderExternal(name, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeContentProviderExternalAsUser(java.lang.String name, android.os.IBinder token, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeStrongBinder(token);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeContentProviderExternalAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeContentProviderExternalAsUser(name, token, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get memory information about the calling process.

      @Override public void getMyMemoryState(android.app.ActivityManager.RunningAppProcessInfo outInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMyMemoryState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getMyMemoryState(outInfo);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            outInfo.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean killProcessesBelowForeground(java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killProcessesBelowForeground, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().killProcessesBelowForeground(reason);
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
      @Override public android.content.pm.UserInfo getCurrentUser() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.UserInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentUser();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.UserInfo.CREATOR.createFromParcel(_reply);
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
      // This is not public because you need to be very careful in how you
      // manage your activity to make sure it is always the uid you expect.

      @Override public int getLaunchedFromUid(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLaunchedFromUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLaunchedFromUid(activityToken);
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
      @Override public void unstableProviderDied(android.os.IBinder connection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(connection);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unstableProviderDied, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unstableProviderDied(connection);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isIntentSenderAnActivity(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isIntentSenderAnActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isIntentSenderAnActivity(sender);
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
      @Override public boolean isIntentSenderAForegroundService(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isIntentSenderAForegroundService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isIntentSenderAForegroundService(sender);
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
      @Override public boolean isIntentSenderABroadcast(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isIntentSenderABroadcast, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isIntentSenderABroadcast(sender);
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
      @Override public int startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder(resultTo);
          _data.writeString(resultWho);
          _data.writeInt(requestCode);
          _data.writeInt(flags);
          if ((profilerInfo!=null)) {
            _data.writeInt(1);
            profilerInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityAsUser(caller, callingPackage, intent, resolvedType, resultTo, resultWho, requestCode, flags, profilerInfo, options, userId);
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
      @Override public int stopUser(int userid, boolean force, android.app.IStopUserCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          _data.writeInt(((force)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopUser(userid, force, callback);
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
      @Override public void registerUserSwitchObserver(android.app.IUserSwitchObserver observer, java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUserSwitchObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerUserSwitchObserver(observer, name);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterUserSwitchObserver(android.app.IUserSwitchObserver observer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUserSwitchObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUserSwitchObserver(observer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getRunningUserIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRunningUserIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRunningUserIds();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Request a heap dump for the system server.

      @Override public void requestSystemServerHeapDump() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestSystemServerHeapDump, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestSystemServerHeapDump();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Deprecated - This method is only used by a few internal components and it will soon be
      // replaced by a proper bug report API (which will be restricted to a few, pre-defined apps).
      // No new code should be calling it.

      @Override public void requestBugReport(int bugreportType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(bugreportType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestBugReport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestBugReport(bugreportType);
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
           *  Takes a telephony bug report and notifies the user with the title and description
           *  that are passed to this API as parameters
           *
           *  @param shareTitle should be a valid legible string less than 50 chars long
           *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
           *
           *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
           *          paremeters cannot be encoding to an UTF-8 charset.
           */
      @Override public void requestTelephonyBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(shareTitle);
          _data.writeString(shareDescription);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestTelephonyBugReport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestTelephonyBugReport(shareTitle, shareDescription);
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
           *  Deprecated - This method is only used by Wifi, and it will soon be replaced by a proper
           *  bug report API.
           *
           *  Takes a minimal bugreport of Wifi-related state.
           *
           *  @param shareTitle should be a valid legible string less than 50 chars long
           *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
           *
           *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
           *          parameters cannot be encoding to an UTF-8 charset.
           */
      @Override public void requestWifiBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(shareTitle);
          _data.writeString(shareDescription);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestWifiBugReport, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestWifiBugReport(shareTitle, shareDescription);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.Intent getIntentForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIntentForIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIntentForIntentSender(sender);
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
      // This is not public because you need to be very careful in how you
      // manage your activity to make sure it is always the uid you expect.

      @Override public java.lang.String getLaunchedFromPackage(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLaunchedFromPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLaunchedFromPackage(activityToken);
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
      @Override public void killUid(int appId, int userId, java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appId);
          _data.writeInt(userId);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killUid(appId, userId, reason);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setUserIsMonkey(boolean monkey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((monkey)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserIsMonkey, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserIsMonkey(monkey);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void hang(android.os.IBinder who, boolean allowRestart) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(who);
          _data.writeInt(((allowRestart)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_hang, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hang(who, allowRestart);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.app.ActivityManager.StackInfo> getAllStackInfos() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.ActivityManager.StackInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllStackInfos, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllStackInfos();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.ActivityManager.StackInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void moveTaskToStack(int taskId, int stackId, boolean toTop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(stackId);
          _data.writeInt(((toTop)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveTaskToStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().moveTaskToStack(taskId, stackId, toTop);
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
           * Resizes the input stack id to the given bounds.
           *
           * @param stackId Id of the stack to resize.
           * @param bounds Bounds to resize the stack to or {@code null} for fullscreen.
           * @param allowResizeInDockedMode True if the resize should be allowed when the docked stack is
           *                                active.
           * @param preserveWindows True if the windows of activities contained in the stack should be
           *                        preserved.
           * @param animate True if the stack resize should be animated.
           * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
           *                          default animation duration should be used.
           * @throws RemoteException
           */
      @Override public void resizeStack(int stackId, android.graphics.Rect bounds, boolean allowResizeInDockedMode, boolean preserveWindows, boolean animate, int animationDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((allowResizeInDockedMode)?(1):(0)));
          _data.writeInt(((preserveWindows)?(1):(0)));
          _data.writeInt(((animate)?(1):(0)));
          _data.writeInt(animationDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resizeStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resizeStack(stackId, bounds, allowResizeInDockedMode, preserveWindows, animate, animationDuration);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFocusedStack(int stackId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFocusedStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFocusedStack(stackId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.app.ActivityManager.StackInfo getFocusedStackInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ActivityManager.StackInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFocusedStackInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFocusedStackInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ActivityManager.StackInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void restart() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restart();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void performIdleMaintenance() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performIdleMaintenance, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performIdleMaintenance();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void appNotRespondingViaProvider(android.os.IBinder connection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(connection);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appNotRespondingViaProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appNotRespondingViaProvider(connection);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.graphics.Rect getTaskBounds(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Rect _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTaskBounds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTaskBounds(taskId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Rect.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setProcessMemoryTrimLevel(java.lang.String process, int uid, int level) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(process);
          _data.writeInt(uid);
          _data.writeInt(level);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProcessMemoryTrimLevel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setProcessMemoryTrimLevel(process, uid, level);
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
      // Start of L transactions

      @Override public java.lang.String getTagForIntentSender(android.content.IIntentSender sender, java.lang.String prefix) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          _data.writeString(prefix);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTagForIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTagForIntentSender(sender, prefix);
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
      @Override public boolean startUserInBackground(int userid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startUserInBackground, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startUserInBackground(userid);
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
      @Override public boolean isInLockTaskMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInLockTaskMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInLockTaskMode();
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
      @Override public void startRecentsActivity(android.content.Intent intent, android.app.IAssistDataReceiver assistDataReceiver, android.view.IRecentsAnimationRunner recentsAnimationRunner) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((assistDataReceiver!=null))?(assistDataReceiver.asBinder()):(null)));
          _data.writeStrongBinder((((recentsAnimationRunner!=null))?(recentsAnimationRunner.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecentsActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startRecentsActivity(intent, assistDataReceiver, recentsAnimationRunner);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelRecentsAnimation(boolean restoreHomeStackPosition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((restoreHomeStackPosition)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelRecentsAnimation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelRecentsAnimation(restoreHomeStackPosition);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int startActivityFromRecents(int taskId, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityFromRecents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityFromRecents(taskId, options);
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
      @Override public void startSystemLockTaskMode(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSystemLockTaskMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSystemLockTaskMode(taskId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isTopOfTask(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTopOfTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTopOfTask(token);
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
      @Override public void bootAnimationComplete() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_bootAnimationComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().bootAnimationComplete();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int checkPermissionWithToken(java.lang.String permission, int pid, int uid, android.os.IBinder callerToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(permission);
          _data.writeInt(pid);
          _data.writeInt(uid);
          _data.writeStrongBinder(callerToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkPermissionWithToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkPermissionWithToken(permission, pid, uid, callerToken);
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
      @Override public void registerTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTaskStackListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTaskStackListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTaskStackListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTaskStackListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCleartextNetwork(int uid, byte[] firstPacket) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeByteArray(firstPacket);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCleartextNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCleartextNetwork(uid, firstPacket);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTaskResizeable(int taskId, int resizeableMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(resizeableMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTaskResizeable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTaskResizeable(taskId, resizeableMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resizeTask(int taskId, android.graphics.Rect bounds, int resizeMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(resizeMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resizeTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resizeTask(taskId, bounds, resizeMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getLockTaskModeState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLockTaskModeState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLockTaskModeState();
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
      @Override public void setDumpHeapDebugLimit(java.lang.String processName, int uid, long maxMemSize, java.lang.String reportPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(processName);
          _data.writeInt(uid);
          _data.writeLong(maxMemSize);
          _data.writeString(reportPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDumpHeapDebugLimit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDumpHeapDebugLimit(processName, uid, maxMemSize, reportPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dumpHeapFinished(java.lang.String path) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(path);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpHeapFinished, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpHeapFinished(path);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateLockTaskPackages(int userId, java.lang.String[] packages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStringArray(packages);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateLockTaskPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateLockTaskPackages(userId, packages);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void noteAlarmStart(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sourceUid);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_noteAlarmStart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().noteAlarmStart(sender, workSource, sourceUid, tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void noteAlarmFinish(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sender!=null))?(sender.asBinder()):(null)));
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sourceUid);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_noteAlarmFinish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().noteAlarmFinish(sender, workSource, sourceUid, tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getPackageProcessState(java.lang.String packageName, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageProcessState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageProcessState(packageName, callingPackage);
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
      @Override public void updateDeviceOwner(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateDeviceOwner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateDeviceOwner(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Start of N transactions
      // Start Binder transaction tracking for all applications.

      @Override public boolean startBinderTracking() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startBinderTracking, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startBinderTracking();
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
      // Stop Binder transaction tracking for all applications and dump trace data to the given file
      // descriptor.

      @Override public boolean stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopBinderTrackingAndDump, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopBinderTrackingAndDump(fd);
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
           * Try to place task to provided position. The final position might be different depending on
           * current user and stacks state. The task will be moved to target stack if it's currently in
           * different stack.
           */
      @Override public void positionTaskInStack(int taskId, int stackId, int position) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(stackId);
          _data.writeInt(position);
          boolean _status = mRemote.transact(Stub.TRANSACTION_positionTaskInStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().positionTaskInStack(taskId, stackId, position);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void suppressResizeConfigChanges(boolean suppress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((suppress)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_suppressResizeConfigChanges, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().suppressResizeConfigChanges(suppress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean moveTopActivityToPinnedStack(int stackId, android.graphics.Rect bounds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveTopActivityToPinnedStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().moveTopActivityToPinnedStack(stackId, bounds);
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
      @Override public boolean isAppStartModeDisabled(int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAppStartModeDisabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAppStartModeDisabled(uid, packageName);
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
      @Override public boolean unlockUser(int userid, byte[] token, byte[] secret, android.os.IProgressListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          _data.writeByteArray(token);
          _data.writeByteArray(secret);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unlockUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unlockUser(userid, token, secret, listener);
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
      @Override public void killPackageDependents(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_killPackageDependents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().killPackageDependents(packageName, userId);
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
           * Resizes the docked stack, and all other stacks as the result of the dock stack bounds change.
           *
           * @param dockedBounds The bounds for the docked stack.
           * @param tempDockedTaskBounds The temporary bounds for the tasks in the docked stack, which
           *                             might be different from the stack bounds to allow more
           *                             flexibility while resizing, or {@code null} if they should be the
           *                             same as the stack bounds.
           * @param tempDockedTaskInsetBounds The temporary bounds for the tasks to calculate the insets.
           *                                  When resizing, we usually "freeze" the layout of a task. To
           *                                  achieve that, we also need to "freeze" the insets, which
           *                                  gets achieved by changing task bounds but not bounds used
           *                                  to calculate the insets in this transient state
           * @param tempOtherTaskBounds The temporary bounds for the tasks in all other stacks, or
           *                            {@code null} if they should be the same as the stack bounds.
           * @param tempOtherTaskInsetBounds Like {@code tempDockedTaskInsetBounds}, but for the other
           *                                 stacks.
           * @throws RemoteException
           */
      @Override public void resizeDockedStack(android.graphics.Rect dockedBounds, android.graphics.Rect tempDockedTaskBounds, android.graphics.Rect tempDockedTaskInsetBounds, android.graphics.Rect tempOtherTaskBounds, android.graphics.Rect tempOtherTaskInsetBounds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((dockedBounds!=null)) {
            _data.writeInt(1);
            dockedBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((tempDockedTaskBounds!=null)) {
            _data.writeInt(1);
            tempDockedTaskBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((tempDockedTaskInsetBounds!=null)) {
            _data.writeInt(1);
            tempDockedTaskInsetBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((tempOtherTaskBounds!=null)) {
            _data.writeInt(1);
            tempOtherTaskBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((tempOtherTaskInsetBounds!=null)) {
            _data.writeInt(1);
            tempOtherTaskInsetBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resizeDockedStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resizeDockedStack(dockedBounds, tempDockedTaskBounds, tempDockedTaskInsetBounds, tempOtherTaskBounds, tempOtherTaskInsetBounds);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeStack(int stackId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeStack(stackId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void makePackageIdle(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_makePackageIdle, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makePackageIdle(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMemoryTrimLevel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMemoryTrimLevel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMemoryTrimLevel();
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
      @Override public boolean isVrModePackageEnabled(android.content.ComponentName packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((packageName!=null)) {
            _data.writeInt(1);
            packageName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVrModePackageEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVrModePackageEnabled(packageName);
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
      @Override public void notifyLockedProfile(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyLockedProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyLockedProfile(userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startConfirmDeviceCredentialIntent(android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startConfirmDeviceCredentialIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startConfirmDeviceCredentialIntent(intent, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendIdleJobTrigger() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendIdleJobTrigger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendIdleJobTrigger();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int sendIntentSender(android.content.IIntentSender target, android.os.IBinder whitelistToken, int code, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((target!=null))?(target.asBinder()):(null)));
          _data.writeStrongBinder(whitelistToken);
          _data.writeInt(code);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder((((finishedReceiver!=null))?(finishedReceiver.asBinder()):(null)));
          _data.writeString(requiredPermission);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendIntentSender(target, whitelistToken, code, intent, resolvedType, finishedReceiver, requiredPermission, options);
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
      @Override public boolean isBackgroundRestricted(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBackgroundRestricted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBackgroundRestricted(packageName);
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
      // Start of N MR1 transactions

      @Override public void setRenderThread(int tid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRenderThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRenderThread(tid);
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
           * Lets activity manager know whether the calling process is currently showing "top-level" UI
           * that is not an activity, i.e. windows on the screen the user is currently interacting with.
           *
           * <p>This flag can only be set for persistent processes.
           *
           * @param hasTopUi Whether the calling process has "top-level" UI.
           */
      @Override public void setHasTopUi(boolean hasTopUi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hasTopUi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHasTopUi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHasTopUi(hasTopUi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Start of O transactions

      @Override public int restartUserInBackground(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restartUserInBackground, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().restartUserInBackground(userId);
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
      /** Cancels the window transitions for the given task. */
      @Override public void cancelTaskWindowTransition(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelTaskWindowTransition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelTaskWindowTransition(taskId);
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
           * @param taskId the id of the task to retrieve the sAutoapshots for
           * @param reducedResolution if set, if the snapshot needs to be loaded from disk, this will load
           *                          a reduced resolution of it, which is much faster
           * @return a graphic buffer representing a screenshot of a task
           */
      @Override public android.app.ActivityManager.TaskSnapshot getTaskSnapshot(int taskId, boolean reducedResolution) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ActivityManager.TaskSnapshot _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(((reducedResolution)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTaskSnapshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTaskSnapshot(taskId, reducedResolution);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ActivityManager.TaskSnapshot.CREATOR.createFromParcel(_reply);
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
      @Override public void scheduleApplicationInfoChanged(java.util.List<java.lang.String> packageNames, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(packageNames);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleApplicationInfoChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleApplicationInfoChanged(packageNames, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPersistentVrThread(int tid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPersistentVrThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPersistentVrThread(tid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void waitForNetworkStateUpdate(long procStateSeq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(procStateSeq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_waitForNetworkStateUpdate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().waitForNetworkStateUpdate(procStateSeq);
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
           * Add a bare uid to the background restrictions whitelist.  Only the system uid may call this.
           */
      @Override public void backgroundWhitelistUid(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_backgroundWhitelistUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().backgroundWhitelistUid(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Start of P transactions
      /**
           *  Similar to {@link #startUserInBackground(int userId), but with a listener to report
           *  user unlock progress.
           */
      @Override public boolean startUserInBackgroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          _data.writeStrongBinder((((unlockProgressListener!=null))?(unlockProgressListener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startUserInBackgroundWithListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startUserInBackgroundWithListener(userid, unlockProgressListener);
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
           * Method for the shell UID to start deletating its permission identity to an
           * active instrumenation. The shell can delegate permissions only to one active
           * instrumentation at a time. An active instrumentation is one running and
           * started from the shell.
           */
      @Override public void startDelegateShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeStringArray(permissions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startDelegateShellPermissionIdentity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startDelegateShellPermissionIdentity(uid, permissions);
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
           * Method for the shell UID to stop deletating its permission identity to an
           * active instrumenation. An active instrumentation is one running and
           * started from the shell.
           */
      @Override public void stopDelegateShellPermissionIdentity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopDelegateShellPermissionIdentity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopDelegateShellPermissionIdentity();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Returns a file descriptor that'll be closed when the system server process dies. */
      @Override public android.os.ParcelFileDescriptor getLifeMonitor() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLifeMonitor, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLifeMonitor();
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
      /**
           * Start user, if it us not already running, and bring it to foreground.
           * unlockProgressListener can be null if monitoring progress is not necessary.
           */
      @Override public boolean startUserInForegroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userid);
          _data.writeStrongBinder((((unlockProgressListener!=null))?(unlockProgressListener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startUserInForegroundWithListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startUserInForegroundWithListener(userid, unlockProgressListener);
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
      public static android.app.IActivityManager sDefaultImpl;
    }
    static final int TRANSACTION_openContentUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerUidObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterUidObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isUidActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getUidProcessState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_handleApplicationCrash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_startActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_unhandledBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_finishActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_registerReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_unregisterReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_broadcastIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_unbroadcastIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_finishReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_attachApplication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getFilteredTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_moveTaskToFront = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getTaskForActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getContentProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_publishContentProviders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_refContentProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getRunningServiceControlPanel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_startService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_stopService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_bindService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_bindIsolatedService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_updateServiceGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_unbindService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_publishService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_setDebugApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setAgentApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_setAlwaysFinish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_startInstrumentation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_addInstrumentationResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_finishInstrumentation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_updateConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_stopServiceToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_setProcessLimit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_getProcessLimit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_checkPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_checkUriPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_grantUriPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_revokeUriPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_setActivityController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_showWaitingForDebugger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_signalPersistentProcesses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_getRecentTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_serviceDoneExecuting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_getIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_cancelIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_getPackageForIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_registerIntentSenderCancelListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_unregisterIntentSenderCancelListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_enterSafeMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_noteWakeupAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_removeContentProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_setRequestedOrientation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_unbindFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_setProcessImportant = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_setServiceForeground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_getForegroundServiceType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_moveActivityTaskToBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_getMemoryInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_getProcessesInErrorState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_clearApplicationUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_forceStopPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_killPids = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_getServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_getRunningAppProcesses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_peekService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_profileControl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_stopAppSwitches = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_resumeAppSwitches = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_bindBackupAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_backupAgentCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_unbindBackupAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_getUidForIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_handleIncomingUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_addPackageDependency = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_killApplication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_closeSystemDialogs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_getProcessMemoryInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_killApplicationProcess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_handleApplicationWtf = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_killBackgroundProcesses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_isUserAMonkey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_getRunningExternalApplications = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    static final int TRANSACTION_finishHeavyWeightApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 90);
    static final int TRANSACTION_handleApplicationStrictModeViolation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 91);
    static final int TRANSACTION_isTopActivityImmersive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 92);
    static final int TRANSACTION_crashApplication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 93);
    static final int TRANSACTION_getProviderMimeType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 94);
    static final int TRANSACTION_dumpHeap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 95);
    static final int TRANSACTION_isUserRunning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 96);
    static final int TRANSACTION_setPackageScreenCompatMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 97);
    static final int TRANSACTION_switchUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 98);
    static final int TRANSACTION_removeTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 99);
    static final int TRANSACTION_registerProcessObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 100);
    static final int TRANSACTION_unregisterProcessObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 101);
    static final int TRANSACTION_isIntentSenderTargetedToPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 102);
    static final int TRANSACTION_updatePersistentConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 103);
    static final int TRANSACTION_getProcessPss = (android.os.IBinder.FIRST_CALL_TRANSACTION + 104);
    static final int TRANSACTION_showBootMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 105);
    static final int TRANSACTION_killAllBackgroundProcesses = (android.os.IBinder.FIRST_CALL_TRANSACTION + 106);
    static final int TRANSACTION_getContentProviderExternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 107);
    static final int TRANSACTION_removeContentProviderExternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 108);
    static final int TRANSACTION_removeContentProviderExternalAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 109);
    static final int TRANSACTION_getMyMemoryState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 110);
    static final int TRANSACTION_killProcessesBelowForeground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 111);
    static final int TRANSACTION_getCurrentUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 112);
    static final int TRANSACTION_getLaunchedFromUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 113);
    static final int TRANSACTION_unstableProviderDied = (android.os.IBinder.FIRST_CALL_TRANSACTION + 114);
    static final int TRANSACTION_isIntentSenderAnActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 115);
    static final int TRANSACTION_isIntentSenderAForegroundService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 116);
    static final int TRANSACTION_isIntentSenderABroadcast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 117);
    static final int TRANSACTION_startActivityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 118);
    static final int TRANSACTION_stopUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 119);
    static final int TRANSACTION_registerUserSwitchObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 120);
    static final int TRANSACTION_unregisterUserSwitchObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 121);
    static final int TRANSACTION_getRunningUserIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 122);
    static final int TRANSACTION_requestSystemServerHeapDump = (android.os.IBinder.FIRST_CALL_TRANSACTION + 123);
    static final int TRANSACTION_requestBugReport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 124);
    static final int TRANSACTION_requestTelephonyBugReport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 125);
    static final int TRANSACTION_requestWifiBugReport = (android.os.IBinder.FIRST_CALL_TRANSACTION + 126);
    static final int TRANSACTION_getIntentForIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 127);
    static final int TRANSACTION_getLaunchedFromPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 128);
    static final int TRANSACTION_killUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 129);
    static final int TRANSACTION_setUserIsMonkey = (android.os.IBinder.FIRST_CALL_TRANSACTION + 130);
    static final int TRANSACTION_hang = (android.os.IBinder.FIRST_CALL_TRANSACTION + 131);
    static final int TRANSACTION_getAllStackInfos = (android.os.IBinder.FIRST_CALL_TRANSACTION + 132);
    static final int TRANSACTION_moveTaskToStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 133);
    static final int TRANSACTION_resizeStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 134);
    static final int TRANSACTION_setFocusedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 135);
    static final int TRANSACTION_getFocusedStackInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 136);
    static final int TRANSACTION_restart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 137);
    static final int TRANSACTION_performIdleMaintenance = (android.os.IBinder.FIRST_CALL_TRANSACTION + 138);
    static final int TRANSACTION_appNotRespondingViaProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 139);
    static final int TRANSACTION_getTaskBounds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 140);
    static final int TRANSACTION_setProcessMemoryTrimLevel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 141);
    static final int TRANSACTION_getTagForIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 142);
    static final int TRANSACTION_startUserInBackground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 143);
    static final int TRANSACTION_isInLockTaskMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 144);
    static final int TRANSACTION_startRecentsActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 145);
    static final int TRANSACTION_cancelRecentsAnimation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 146);
    static final int TRANSACTION_startActivityFromRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 147);
    static final int TRANSACTION_startSystemLockTaskMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 148);
    static final int TRANSACTION_isTopOfTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 149);
    static final int TRANSACTION_bootAnimationComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 150);
    static final int TRANSACTION_checkPermissionWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 151);
    static final int TRANSACTION_registerTaskStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 152);
    static final int TRANSACTION_unregisterTaskStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 153);
    static final int TRANSACTION_notifyCleartextNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 154);
    static final int TRANSACTION_setTaskResizeable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 155);
    static final int TRANSACTION_resizeTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 156);
    static final int TRANSACTION_getLockTaskModeState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 157);
    static final int TRANSACTION_setDumpHeapDebugLimit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 158);
    static final int TRANSACTION_dumpHeapFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 159);
    static final int TRANSACTION_updateLockTaskPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 160);
    static final int TRANSACTION_noteAlarmStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 161);
    static final int TRANSACTION_noteAlarmFinish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 162);
    static final int TRANSACTION_getPackageProcessState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 163);
    static final int TRANSACTION_updateDeviceOwner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 164);
    static final int TRANSACTION_startBinderTracking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 165);
    static final int TRANSACTION_stopBinderTrackingAndDump = (android.os.IBinder.FIRST_CALL_TRANSACTION + 166);
    static final int TRANSACTION_positionTaskInStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 167);
    static final int TRANSACTION_suppressResizeConfigChanges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 168);
    static final int TRANSACTION_moveTopActivityToPinnedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 169);
    static final int TRANSACTION_isAppStartModeDisabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 170);
    static final int TRANSACTION_unlockUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 171);
    static final int TRANSACTION_killPackageDependents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 172);
    static final int TRANSACTION_resizeDockedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 173);
    static final int TRANSACTION_removeStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 174);
    static final int TRANSACTION_makePackageIdle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 175);
    static final int TRANSACTION_getMemoryTrimLevel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 176);
    static final int TRANSACTION_isVrModePackageEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 177);
    static final int TRANSACTION_notifyLockedProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 178);
    static final int TRANSACTION_startConfirmDeviceCredentialIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 179);
    static final int TRANSACTION_sendIdleJobTrigger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 180);
    static final int TRANSACTION_sendIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 181);
    static final int TRANSACTION_isBackgroundRestricted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 182);
    static final int TRANSACTION_setRenderThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 183);
    static final int TRANSACTION_setHasTopUi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 184);
    static final int TRANSACTION_restartUserInBackground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 185);
    static final int TRANSACTION_cancelTaskWindowTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 186);
    static final int TRANSACTION_getTaskSnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 187);
    static final int TRANSACTION_scheduleApplicationInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 188);
    static final int TRANSACTION_setPersistentVrThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 189);
    static final int TRANSACTION_waitForNetworkStateUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 190);
    static final int TRANSACTION_backgroundWhitelistUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 191);
    static final int TRANSACTION_startUserInBackgroundWithListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 192);
    static final int TRANSACTION_startDelegateShellPermissionIdentity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 193);
    static final int TRANSACTION_stopDelegateShellPermissionIdentity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 194);
    static final int TRANSACTION_getLifeMonitor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 195);
    static final int TRANSACTION_startUserInForegroundWithListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 196);
    public static boolean setDefaultImpl(android.app.IActivityManager impl) {
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
    public static android.app.IActivityManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // WARNING: when these transactions are updated, check if they are any callers on the native
  // side. If so, make sure they are using the correct transaction ids and arguments.
  // If a transaction which will also be used on the native side is being inserted, add it to
  // below block of transactions.
  // Since these transactions are also called from native code, these must be kept in sync with
  // the ones in frameworks/native/libs/binder/include/binder/IActivityManager.h
  // =============== Beginning of transactions used on native side as well ======================

  public android.os.ParcelFileDescriptor openContentUri(java.lang.String uriString) throws android.os.RemoteException;
  public void registerUidObserver(android.app.IUidObserver observer, int which, int cutpoint, java.lang.String callingPackage) throws android.os.RemoteException;
  public void unregisterUidObserver(android.app.IUidObserver observer) throws android.os.RemoteException;
  public boolean isUidActive(int uid, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getUidProcessState(int uid, java.lang.String callingPackage) throws android.os.RemoteException;
  // =============== End of transactions used on native side as well ============================
  // Special low-level communication with activity manager.

  public void handleApplicationCrash(android.os.IBinder app, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:105:1:105:25")
  public int startActivity(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:109:1:109:25")
  public void unhandledBack() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:111:1:111:25")
  public boolean finishActivity(android.os.IBinder token, int code, android.content.Intent data, int finishTask) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:113:1:113:25")
  public android.content.Intent registerReceiver(android.app.IApplicationThread caller, java.lang.String callerPackage, android.content.IIntentReceiver receiver, android.content.IntentFilter filter, java.lang.String requiredPermission, int userId, int flags) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:117:1:117:25")
  public void unregisterReceiver(android.content.IIntentReceiver receiver) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:119:1:119:25")
  public int broadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver resultTo, int resultCode, java.lang.String resultData, android.os.Bundle map, java.lang.String[] requiredPermissions, int appOp, android.os.Bundle options, boolean serialized, boolean sticky, int userId) throws android.os.RemoteException;
  public void unbroadcastIntent(android.app.IApplicationThread caller, android.content.Intent intent, int userId) throws android.os.RemoteException;
  public void finishReceiver(android.os.IBinder who, int resultCode, java.lang.String resultData, android.os.Bundle map, boolean abortBroadcast, int flags) throws android.os.RemoteException;
  public void attachApplication(android.app.IApplicationThread app, long startSeq) throws android.os.RemoteException;
  public java.util.List<android.app.ActivityManager.RunningTaskInfo> getTasks(int maxNum) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:129:1:129:25")
  public java.util.List<android.app.ActivityManager.RunningTaskInfo> getFilteredTasks(int maxNum, int ignoreActivityType, int ignoreWindowingMode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:132:1:132:25")
  public void moveTaskToFront(android.app.IApplicationThread caller, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:135:1:135:25")
  public int getTaskForActivity(android.os.IBinder token, boolean onlyRoot) throws android.os.RemoteException;
  public android.app.ContentProviderHolder getContentProvider(android.app.IApplicationThread caller, java.lang.String callingPackage, java.lang.String name, int userId, boolean stable) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:139:1:139:25")
  public void publishContentProviders(android.app.IApplicationThread caller, java.util.List<android.app.ContentProviderHolder> providers) throws android.os.RemoteException;
  public boolean refContentProvider(android.os.IBinder connection, int stableDelta, int unstableDelta) throws android.os.RemoteException;
  public android.app.PendingIntent getRunningServiceControlPanel(android.content.ComponentName service) throws android.os.RemoteException;
  public android.content.ComponentName startService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, boolean requireForeground, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:146:1:146:25")
  public int stopService(android.app.IApplicationThread caller, android.content.Intent service, java.lang.String resolvedType, int userId) throws android.os.RemoteException;
  // Currently keeping old bindService because it is on the greylist

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:150:1:150:25")
  public int bindService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public int bindIsolatedService(android.app.IApplicationThread caller, android.os.IBinder token, android.content.Intent service, java.lang.String resolvedType, android.app.IServiceConnection connection, int flags, java.lang.String instanceName, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public void updateServiceGroup(android.app.IServiceConnection connection, int group, int importance) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:158:1:158:25")
  public boolean unbindService(android.app.IServiceConnection connection) throws android.os.RemoteException;
  public void publishService(android.os.IBinder token, android.content.Intent intent, android.os.IBinder service) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:161:1:161:25")
  public void setDebugApp(java.lang.String packageName, boolean waitForDebugger, boolean persistent) throws android.os.RemoteException;
  public void setAgentApp(java.lang.String packageName, java.lang.String agent) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:164:1:164:25")
  public void setAlwaysFinish(boolean enabled) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:166:1:166:25")
  public boolean startInstrumentation(android.content.ComponentName className, java.lang.String profileFile, int flags, android.os.Bundle arguments, android.app.IInstrumentationWatcher watcher, android.app.IUiAutomationConnection connection, int userId, java.lang.String abiOverride) throws android.os.RemoteException;
  public void addInstrumentationResults(android.app.IApplicationThread target, android.os.Bundle results) throws android.os.RemoteException;
  public void finishInstrumentation(android.app.IApplicationThread target, int resultCode, android.os.Bundle results) throws android.os.RemoteException;
  /**
       * @return A copy of global {@link Configuration}, contains general settings for the entire
       *         system. Corresponds to the configuration of the default display.
       * @throws RemoteException
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:179:1:179:25")
  public android.content.res.Configuration getConfiguration() throws android.os.RemoteException;
  /**
       * Updates global configuration and applies changes to the entire system.
       * @param values Update values for global configuration. If null is passed it will request the
       *               Window Manager to compute new config for the default display.
       * @throws RemoteException
       * @return Returns true if the configuration was updated.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:188:1:188:25")
  public boolean updateConfiguration(android.content.res.Configuration values) throws android.os.RemoteException;
  public boolean stopServiceToken(android.content.ComponentName className, android.os.IBinder token, int startId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:191:1:191:25")
  public void setProcessLimit(int max) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:193:1:193:25")
  public int getProcessLimit() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:195:1:195:25")
  public int checkPermission(java.lang.String permission, int pid, int uid) throws android.os.RemoteException;
  public int checkUriPermission(android.net.Uri uri, int pid, int uid, int mode, int userId, android.os.IBinder callerToken) throws android.os.RemoteException;
  public void grantUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException;
  public void revokeUriPermission(android.app.IApplicationThread caller, java.lang.String targetPkg, android.net.Uri uri, int mode, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:203:1:203:25")
  public void setActivityController(android.app.IActivityController watcher, boolean imAMonkey) throws android.os.RemoteException;
  public void showWaitingForDebugger(android.app.IApplicationThread who, boolean waiting) throws android.os.RemoteException;
  /*
       * This will deliver the specified signal to all the persistent processes. Currently only
       * SIGUSR1 is delivered. All others are ignored.
       */
  public void signalPersistentProcesses(int signal) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:212:1:212:25")
  public android.content.pm.ParceledListSlice getRecentTasks(int maxNum, int flags, int userId) throws android.os.RemoteException;
  public void serviceDoneExecuting(android.os.IBinder token, int type, int startId, int res) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:215:1:215:25")
  public android.content.IIntentSender getIntentSender(int type, java.lang.String packageName, android.os.IBinder token, java.lang.String resultWho, int requestCode, android.content.Intent[] intents, java.lang.String[] resolvedTypes, int flags, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public void cancelIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException;
  public java.lang.String getPackageForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException;
  public void registerIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException;
  public void unregisterIntentSenderCancelListener(android.content.IIntentSender sender, com.android.internal.os.IResultReceiver receiver) throws android.os.RemoteException;
  public void enterSafeMode() throws android.os.RemoteException;
  public void noteWakeupAlarm(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String sourcePkg, java.lang.String tag) throws android.os.RemoteException;
  public void removeContentProvider(android.os.IBinder connection, boolean stable) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:227:1:227:25")
  public void setRequestedOrientation(android.os.IBinder token, int requestedOrientation) throws android.os.RemoteException;
  public void unbindFinished(android.os.IBinder token, android.content.Intent service, boolean doRebind) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:230:1:230:25")
  public void setProcessImportant(android.os.IBinder token, int pid, boolean isForeground, java.lang.String reason) throws android.os.RemoteException;
  public void setServiceForeground(android.content.ComponentName className, android.os.IBinder token, int id, android.app.Notification notification, int flags, int foregroundServiceType) throws android.os.RemoteException;
  public int getForegroundServiceType(android.content.ComponentName className, android.os.IBinder token) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:235:1:235:25")
  public boolean moveActivityTaskToBack(android.os.IBinder token, boolean nonRoot) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:237:1:237:25")
  public void getMemoryInfo(android.app.ActivityManager.MemoryInfo outInfo) throws android.os.RemoteException;
  public java.util.List<android.app.ActivityManager.ProcessErrorStateInfo> getProcessesInErrorState() throws android.os.RemoteException;
  public boolean clearApplicationUserData(java.lang.String packageName, boolean keepState, android.content.pm.IPackageDataObserver observer, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:242:1:242:25")
  public void forceStopPackage(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public boolean killPids(int[] pids, java.lang.String reason, boolean secure) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:245:1:245:25")
  public java.util.List<android.app.ActivityManager.RunningServiceInfo> getServices(int maxNum, int flags) throws android.os.RemoteException;
  // Retrieve running application processes in the system

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:248:1:248:25")
  public java.util.List<android.app.ActivityManager.RunningAppProcessInfo> getRunningAppProcesses() throws android.os.RemoteException;
  public android.os.IBinder peekService(android.content.Intent service, java.lang.String resolvedType, java.lang.String callingPackage) throws android.os.RemoteException;
  // Turn on/off profiling in a particular process.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:252:1:252:25")
  public boolean profileControl(java.lang.String process, int userId, boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:255:1:255:25")
  public boolean shutdown(int timeout) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:257:1:257:25")
  public void stopAppSwitches() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:259:1:259:25")
  public void resumeAppSwitches() throws android.os.RemoteException;
  public boolean bindBackupAgent(java.lang.String packageName, int backupRestoreMode, int targetUserId) throws android.os.RemoteException;
  public void backupAgentCreated(java.lang.String packageName, android.os.IBinder agent, int userId) throws android.os.RemoteException;
  public void unbindBackupAgent(android.content.pm.ApplicationInfo appInfo) throws android.os.RemoteException;
  public int getUidForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException;
  public int handleIncomingUser(int callingPid, int callingUid, int userId, boolean allowAll, boolean requireFull, java.lang.String name, java.lang.String callerPackage) throws android.os.RemoteException;
  public void addPackageDependency(java.lang.String packageName) throws android.os.RemoteException;
  public void killApplication(java.lang.String pkg, int appId, int userId, java.lang.String reason) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:269:1:269:25")
  public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:271:1:271:25")
  public android.os.Debug.MemoryInfo[] getProcessMemoryInfo(int[] pids) throws android.os.RemoteException;
  public void killApplicationProcess(java.lang.String processName, int uid) throws android.os.RemoteException;
  // Special low-level communication with activity manager.

  public boolean handleApplicationWtf(android.os.IBinder app, java.lang.String tag, boolean system, android.app.ApplicationErrorReport.ParcelableCrashInfo crashInfo) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:277:1:277:25")
  public void killBackgroundProcesses(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public boolean isUserAMonkey() throws android.os.RemoteException;
  // Retrieve info of applications installed on external media that are currently
  // running.

  public java.util.List<android.content.pm.ApplicationInfo> getRunningExternalApplications() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:283:1:283:25")
  public void finishHeavyWeightApp() throws android.os.RemoteException;
  // A StrictMode violation to be handled.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:286:1:286:25")
  public void handleApplicationStrictModeViolation(android.os.IBinder app, int penaltyMask, android.os.StrictMode.ViolationInfo crashInfo) throws android.os.RemoteException;
  public boolean isTopActivityImmersive() throws android.os.RemoteException;
  public void crashApplication(int uid, int initialPid, java.lang.String packageName, int userId, java.lang.String message, boolean force) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:292:1:292:25")
  public java.lang.String getProviderMimeType(android.net.Uri uri, int userId) throws android.os.RemoteException;
  // Cause the specified process to dump the specified heap.

  public boolean dumpHeap(java.lang.String process, int userId, boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:298:1:298:25")
  public boolean isUserRunning(int userid, int flags) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:300:1:300:25")
  public void setPackageScreenCompatMode(java.lang.String packageName, int mode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:302:1:302:25")
  public boolean switchUser(int userid) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:304:1:304:25")
  public boolean removeTask(int taskId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:306:1:306:25")
  public void registerProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:308:1:308:25")
  public void unregisterProcessObserver(android.app.IProcessObserver observer) throws android.os.RemoteException;
  public boolean isIntentSenderTargetedToPackage(android.content.IIntentSender sender) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:311:1:311:25")
  public void updatePersistentConfiguration(android.content.res.Configuration values) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:313:1:313:25")
  public long[] getProcessPss(int[] pids) throws android.os.RemoteException;
  public void showBootMessage(java.lang.CharSequence msg, boolean always) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:316:1:316:25")
  public void killAllBackgroundProcesses() throws android.os.RemoteException;
  public android.app.ContentProviderHolder getContentProviderExternal(java.lang.String name, int userId, android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException;
  /** @deprecated - Use {@link #removeContentProviderExternalAsUser} which takes a user ID. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:321:1:321:25")
  public void removeContentProviderExternal(java.lang.String name, android.os.IBinder token) throws android.os.RemoteException;
  public void removeContentProviderExternalAsUser(java.lang.String name, android.os.IBinder token, int userId) throws android.os.RemoteException;
  // Get memory information about the calling process.

  public void getMyMemoryState(android.app.ActivityManager.RunningAppProcessInfo outInfo) throws android.os.RemoteException;
  public boolean killProcessesBelowForeground(java.lang.String reason) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:327:1:327:25")
  public android.content.pm.UserInfo getCurrentUser() throws android.os.RemoteException;
  // This is not public because you need to be very careful in how you
  // manage your activity to make sure it is always the uid you expect.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:331:1:331:25")
  public int getLaunchedFromUid(android.os.IBinder activityToken) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:333:1:333:25")
  public void unstableProviderDied(android.os.IBinder connection) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:335:1:335:25")
  public boolean isIntentSenderAnActivity(android.content.IIntentSender sender) throws android.os.RemoteException;
  public boolean isIntentSenderAForegroundService(android.content.IIntentSender sender) throws android.os.RemoteException;
  public boolean isIntentSenderABroadcast(android.content.IIntentSender sender) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:339:1:339:25")
  public int startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:344:1:344:25")
  public int stopUser(int userid, boolean force, android.app.IStopUserCallback callback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:346:1:346:25")
  public void registerUserSwitchObserver(android.app.IUserSwitchObserver observer, java.lang.String name) throws android.os.RemoteException;
  public void unregisterUserSwitchObserver(android.app.IUserSwitchObserver observer) throws android.os.RemoteException;
  public int[] getRunningUserIds() throws android.os.RemoteException;
  // Request a heap dump for the system server.

  public void requestSystemServerHeapDump() throws android.os.RemoteException;
  // Deprecated - This method is only used by a few internal components and it will soon be
  // replaced by a proper bug report API (which will be restricted to a few, pre-defined apps).
  // No new code should be calling it.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:357:1:357:25")
  public void requestBugReport(int bugreportType) throws android.os.RemoteException;
  /**
       *  Takes a telephony bug report and notifies the user with the title and description
       *  that are passed to this API as parameters
       *
       *  @param shareTitle should be a valid legible string less than 50 chars long
       *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
       *
       *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
       *          paremeters cannot be encoding to an UTF-8 charset.
       */
  public void requestTelephonyBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException;
  /**
       *  Deprecated - This method is only used by Wifi, and it will soon be replaced by a proper
       *  bug report API.
       *
       *  Takes a minimal bugreport of Wifi-related state.
       *
       *  @param shareTitle should be a valid legible string less than 50 chars long
       *  @param shareDescription should be less than 91 bytes when encoded into UTF-8 format
       *
       *  @throws IllegalArgumentException if shareTitle or shareDescription is too big or if the
       *          parameters cannot be encoding to an UTF-8 charset.
       */
  public void requestWifiBugReport(java.lang.String shareTitle, java.lang.String shareDescription) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:386:1:386:25")
  public android.content.Intent getIntentForIntentSender(android.content.IIntentSender sender) throws android.os.RemoteException;
  // This is not public because you need to be very careful in how you
  // manage your activity to make sure it is always the uid you expect.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:390:1:390:25")
  public java.lang.String getLaunchedFromPackage(android.os.IBinder activityToken) throws android.os.RemoteException;
  public void killUid(int appId, int userId, java.lang.String reason) throws android.os.RemoteException;
  public void setUserIsMonkey(boolean monkey) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:394:1:394:25")
  public void hang(android.os.IBinder who, boolean allowRestart) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:397:1:397:25")
  public java.util.List<android.app.ActivityManager.StackInfo> getAllStackInfos() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:399:1:399:25")
  public void moveTaskToStack(int taskId, int stackId, boolean toTop) throws android.os.RemoteException;
  /**
       * Resizes the input stack id to the given bounds.
       *
       * @param stackId Id of the stack to resize.
       * @param bounds Bounds to resize the stack to or {@code null} for fullscreen.
       * @param allowResizeInDockedMode True if the resize should be allowed when the docked stack is
       *                                active.
       * @param preserveWindows True if the windows of activities contained in the stack should be
       *                        preserved.
       * @param animate True if the stack resize should be animated.
       * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
       *                          default animation duration should be used.
       * @throws RemoteException
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:415:1:415:25")
  public void resizeStack(int stackId, android.graphics.Rect bounds, boolean allowResizeInDockedMode, boolean preserveWindows, boolean animate, int animationDuration) throws android.os.RemoteException;
  public void setFocusedStack(int stackId) throws android.os.RemoteException;
  public android.app.ActivityManager.StackInfo getFocusedStackInfo() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:420:1:420:25")
  public void restart() throws android.os.RemoteException;
  public void performIdleMaintenance() throws android.os.RemoteException;
  public void appNotRespondingViaProvider(android.os.IBinder connection) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:424:1:424:25")
  public android.graphics.Rect getTaskBounds(int taskId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:426:1:426:25")
  public boolean setProcessMemoryTrimLevel(java.lang.String process, int uid, int level) throws android.os.RemoteException;
  // Start of L transactions

  public java.lang.String getTagForIntentSender(android.content.IIntentSender sender, java.lang.String prefix) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:432:1:432:25")
  public boolean startUserInBackground(int userid) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:434:1:434:25")
  public boolean isInLockTaskMode() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:436:1:436:25")
  public void startRecentsActivity(android.content.Intent intent, android.app.IAssistDataReceiver assistDataReceiver, android.view.IRecentsAnimationRunner recentsAnimationRunner) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:439:1:439:25")
  public void cancelRecentsAnimation(boolean restoreHomeStackPosition) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:441:1:441:25")
  public int startActivityFromRecents(int taskId, android.os.Bundle options) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:443:1:443:25")
  public void startSystemLockTaskMode(int taskId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:445:1:445:25")
  public boolean isTopOfTask(android.os.IBinder token) throws android.os.RemoteException;
  public void bootAnimationComplete() throws android.os.RemoteException;
  public int checkPermissionWithToken(java.lang.String permission, int pid, int uid, android.os.IBinder callerToken) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:450:1:450:25")
  public void registerTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException;
  public void unregisterTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException;
  public void notifyCleartextNetwork(int uid, byte[] firstPacket) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:454:1:454:25")
  public void setTaskResizeable(int taskId, int resizeableMode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:456:1:456:25")
  public void resizeTask(int taskId, android.graphics.Rect bounds, int resizeMode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:458:1:458:25")
  public int getLockTaskModeState() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:460:1:460:25")
  public void setDumpHeapDebugLimit(java.lang.String processName, int uid, long maxMemSize, java.lang.String reportPackage) throws android.os.RemoteException;
  public void dumpHeapFinished(java.lang.String path) throws android.os.RemoteException;
  public void updateLockTaskPackages(int userId, java.lang.String[] packages) throws android.os.RemoteException;
  public void noteAlarmStart(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException;
  public void noteAlarmFinish(android.content.IIntentSender sender, android.os.WorkSource workSource, int sourceUid, java.lang.String tag) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:467:1:467:25")
  public int getPackageProcessState(java.lang.String packageName, java.lang.String callingPackage) throws android.os.RemoteException;
  public void updateDeviceOwner(java.lang.String packageName) throws android.os.RemoteException;
  // Start of N transactions
  // Start Binder transaction tracking for all applications.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:473:1:473:25")
  public boolean startBinderTracking() throws android.os.RemoteException;
  // Stop Binder transaction tracking for all applications and dump trace data to the given file
  // descriptor.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:477:1:477:25")
  public boolean stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  /**
       * Try to place task to provided position. The final position might be different depending on
       * current user and stacks state. The task will be moved to target stack if it's currently in
       * different stack.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:484:1:484:25")
  public void positionTaskInStack(int taskId, int stackId, int position) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:486:1:486:25")
  public void suppressResizeConfigChanges(boolean suppress) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:488:1:488:25")
  public boolean moveTopActivityToPinnedStack(int stackId, android.graphics.Rect bounds) throws android.os.RemoteException;
  public boolean isAppStartModeDisabled(int uid, java.lang.String packageName) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:491:1:491:25")
  public boolean unlockUser(int userid, byte[] token, byte[] secret, android.os.IProgressListener listener) throws android.os.RemoteException;
  public void killPackageDependents(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /**
       * Resizes the docked stack, and all other stacks as the result of the dock stack bounds change.
       *
       * @param dockedBounds The bounds for the docked stack.
       * @param tempDockedTaskBounds The temporary bounds for the tasks in the docked stack, which
       *                             might be different from the stack bounds to allow more
       *                             flexibility while resizing, or {@code null} if they should be the
       *                             same as the stack bounds.
       * @param tempDockedTaskInsetBounds The temporary bounds for the tasks to calculate the insets.
       *                                  When resizing, we usually "freeze" the layout of a task. To
       *                                  achieve that, we also need to "freeze" the insets, which
       *                                  gets achieved by changing task bounds but not bounds used
       *                                  to calculate the insets in this transient state
       * @param tempOtherTaskBounds The temporary bounds for the tasks in all other stacks, or
       *                            {@code null} if they should be the same as the stack bounds.
       * @param tempOtherTaskInsetBounds Like {@code tempDockedTaskInsetBounds}, but for the other
       *                                 stacks.
       * @throws RemoteException
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:514:1:514:25")
  public void resizeDockedStack(android.graphics.Rect dockedBounds, android.graphics.Rect tempDockedTaskBounds, android.graphics.Rect tempDockedTaskInsetBounds, android.graphics.Rect tempOtherTaskBounds, android.graphics.Rect tempOtherTaskInsetBounds) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:518:1:518:25")
  public void removeStack(int stackId) throws android.os.RemoteException;
  public void makePackageIdle(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public int getMemoryTrimLevel() throws android.os.RemoteException;
  public boolean isVrModePackageEnabled(android.content.ComponentName packageName) throws android.os.RemoteException;
  public void notifyLockedProfile(int userId) throws android.os.RemoteException;
  public void startConfirmDeviceCredentialIntent(android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:525:1:525:25")
  public void sendIdleJobTrigger() throws android.os.RemoteException;
  public int sendIntentSender(android.content.IIntentSender target, android.os.IBinder whitelistToken, int code, android.content.Intent intent, java.lang.String resolvedType, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException;
  public boolean isBackgroundRestricted(java.lang.String packageName) throws android.os.RemoteException;
  // Start of N MR1 transactions

  public void setRenderThread(int tid) throws android.os.RemoteException;
  /**
       * Lets activity manager know whether the calling process is currently showing "top-level" UI
       * that is not an activity, i.e. windows on the screen the user is currently interacting with.
       *
       * <p>This flag can only be set for persistent processes.
       *
       * @param hasTopUi Whether the calling process has "top-level" UI.
       */
  public void setHasTopUi(boolean hasTopUi) throws android.os.RemoteException;
  // Start of O transactions

  public int restartUserInBackground(int userId) throws android.os.RemoteException;
  /** Cancels the window transitions for the given task. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:547:1:547:25")
  public void cancelTaskWindowTransition(int taskId) throws android.os.RemoteException;
  /**
       * @param taskId the id of the task to retrieve the sAutoapshots for
       * @param reducedResolution if set, if the snapshot needs to be loaded from disk, this will load
       *                          a reduced resolution of it, which is much faster
       * @return a graphic buffer representing a screenshot of a task
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IActivityManager.aidl:555:1:555:25")
  public android.app.ActivityManager.TaskSnapshot getTaskSnapshot(int taskId, boolean reducedResolution) throws android.os.RemoteException;
  public void scheduleApplicationInfoChanged(java.util.List<java.lang.String> packageNames, int userId) throws android.os.RemoteException;
  public void setPersistentVrThread(int tid) throws android.os.RemoteException;
  public void waitForNetworkStateUpdate(long procStateSeq) throws android.os.RemoteException;
  /**
       * Add a bare uid to the background restrictions whitelist.  Only the system uid may call this.
       */
  public void backgroundWhitelistUid(int uid) throws android.os.RemoteException;
  // Start of P transactions
  /**
       *  Similar to {@link #startUserInBackground(int userId), but with a listener to report
       *  user unlock progress.
       */
  public boolean startUserInBackgroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException;
  /**
       * Method for the shell UID to start deletating its permission identity to an
       * active instrumenation. The shell can delegate permissions only to one active
       * instrumentation at a time. An active instrumentation is one running and
       * started from the shell.
       */
  public void startDelegateShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException;
  /**
       * Method for the shell UID to stop deletating its permission identity to an
       * active instrumenation. An active instrumentation is one running and
       * started from the shell.
       */
  public void stopDelegateShellPermissionIdentity() throws android.os.RemoteException;
  /** Returns a file descriptor that'll be closed when the system server process dies. */
  public android.os.ParcelFileDescriptor getLifeMonitor() throws android.os.RemoteException;
  /**
       * Start user, if it us not already running, and bring it to foreground.
       * unlockProgressListener can be null if monitoring progress is not necessary.
       */
  public boolean startUserInForegroundWithListener(int userid, android.os.IProgressListener unlockProgressListener) throws android.os.RemoteException;
}
