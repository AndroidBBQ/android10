/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * System private API for talking with the activity task manager that handles how activities are
 * managed on screen.
 *
 * {@hide}
 */
public interface IActivityTaskManager extends android.os.IInterface
{
  /** Default implementation for IActivityTaskManager. */
  public static class Default implements android.app.IActivityTaskManager
  {
    @Override public int startActivity(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startActivities(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent[] intents, java.lang.String[] resolvedTypes, android.os.IBinder resultTo, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean startNextMatchingActivity(android.os.IBinder callingActivity, android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int startActivityIntentSender(android.app.IApplicationThread caller, android.content.IIntentSender target, android.os.IBinder whitelistToken, android.content.Intent fillInIntent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flagsMask, int flagsValues, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.app.WaitResult startActivityAndWait(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int startActivityWithConfig(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int startFlags, android.content.res.Configuration newConfig, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startVoiceActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startAssistantActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.os.Bundle options, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void startRecentsActivity(android.content.Intent intent, android.app.IAssistDataReceiver assistDataReceiver, android.view.IRecentsAnimationRunner recentsAnimationRunner) throws android.os.RemoteException
    {
    }
    @Override public int startActivityFromRecents(int taskId, android.os.Bundle options) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startActivityAsCaller(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, android.os.IBinder permissionToken, boolean ignoreTargetSecurity, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isActivityStartAllowedOnDisplay(int displayId, android.content.Intent intent, java.lang.String resolvedType, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void unhandledBack() throws android.os.RemoteException
    {
    }
    @Override public boolean finishActivity(android.os.IBinder token, int code, android.content.Intent data, int finishTask) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean finishActivityAffinity(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void activityIdle(android.os.IBinder token, android.content.res.Configuration config, boolean stopProfiling) throws android.os.RemoteException
    {
    }
    @Override public void activityResumed(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void activityTopResumedStateLost() throws android.os.RemoteException
    {
    }
    @Override public void activityPaused(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void activityStopped(android.os.IBinder token, android.os.Bundle state, android.os.PersistableBundle persistentState, java.lang.CharSequence description) throws android.os.RemoteException
    {
    }
    @Override public void activityDestroyed(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void activityRelaunched(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void activitySlept(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public int getFrontActivityScreenCompatMode() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setFrontActivityScreenCompatMode(int mode) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getCallingPackage(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.ComponentName getCallingActivity(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFocusedTask(int taskId) throws android.os.RemoteException
    {
    }
    @Override public boolean removeTask(int taskId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeAllVisibleRecentTasks() throws android.os.RemoteException
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
    @Override public boolean shouldUpRecreateTask(android.os.IBinder token, java.lang.String destAffinity) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean navigateUpTo(android.os.IBinder token, android.content.Intent target, int resultCode, android.content.Intent resultData) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void moveTaskToFront(android.app.IApplicationThread app, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public int getTaskForActivity(android.os.IBinder token, boolean onlyRoot) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void finishSubActivity(android.os.IBinder token, java.lang.String resultWho, int requestCode) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getRecentTasks(int maxNum, int flags, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean willActivityBeVisible(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setRequestedOrientation(android.os.IBinder token, int requestedOrientation) throws android.os.RemoteException
    {
    }
    @Override public int getRequestedOrientation(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean convertFromTranslucent(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean convertToTranslucent(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyActivityDrawn(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void reportActivityFullyDrawn(android.os.IBinder token, boolean restoredFromBundle) throws android.os.RemoteException
    {
    }
    @Override public int getActivityDisplayId(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isImmersive(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setImmersive(android.os.IBinder token, boolean immersive) throws android.os.RemoteException
    {
    }
    @Override public boolean isTopActivityImmersive() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean moveActivityTaskToBack(android.os.IBinder token, boolean nonRoot) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.app.ActivityManager.TaskDescription getTaskDescription(int taskId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void overridePendingTransition(android.os.IBinder token, java.lang.String packageName, int enterAnim, int exitAnim) throws android.os.RemoteException
    {
    }
    @Override public int getLaunchedFromUid(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getLaunchedFromPackage(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void reportAssistContextExtras(android.os.IBinder token, android.os.Bundle extras, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, android.net.Uri referrer) throws android.os.RemoteException
    {
    }
    @Override public void setFocusedStack(int stackId) throws android.os.RemoteException
    {
    }
    @Override public android.app.ActivityManager.StackInfo getFocusedStackInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.graphics.Rect getTaskBounds(int taskId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void cancelRecentsAnimation(boolean restoreHomeStackPosition) throws android.os.RemoteException
    {
    }
    @Override public void startLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void stopLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void updateLockTaskPackages(int userId, java.lang.String[] packages) throws android.os.RemoteException
    {
    }
    @Override public boolean isInLockTaskMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getLockTaskModeState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setTaskDescription(android.os.IBinder token, android.app.ActivityManager.TaskDescription values) throws android.os.RemoteException
    {
    }
    @Override public android.os.Bundle getActivityOptions(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.os.IBinder> getAppTasks(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void startSystemLockTaskMode(int taskId) throws android.os.RemoteException
    {
    }
    @Override public void stopSystemLockTaskMode() throws android.os.RemoteException
    {
    }
    @Override public void finishVoiceTask(android.service.voice.IVoiceInteractionSession session) throws android.os.RemoteException
    {
    }
    @Override public boolean isTopOfTask(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyLaunchTaskBehindComplete(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void notifyEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public int addAppTask(android.os.IBinder activityToken, android.content.Intent intent, android.app.ActivityManager.TaskDescription description, android.graphics.Bitmap thumbnail) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.graphics.Point getAppTaskThumbnailSize() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean releaseActivityInstance(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Only callable from the system. This token grants a temporary permission to call
         * #startActivityAsCallerWithToken. The token will time out after
         * START_AS_CALLER_TOKEN_TIMEOUT if it is not used.
         *
         * @param delegatorToken The Binder token referencing the system Activity that wants to delegate
         *        the #startActivityAsCaller to another app. The "caller" will be the caller of this
         *        activity's token, not the delegate's caller (which is probably the delegator itself).
         *
         * @return Returns a token that can be given to a "delegate" app that may call
         *         #startActivityAsCaller
         */
    @Override public android.os.IBinder requestStartActivityPermissionToken(android.os.IBinder delegatorToken) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void releaseSomeActivities(android.app.IApplicationThread app) throws android.os.RemoteException
    {
    }
    @Override public android.graphics.Bitmap getTaskDescriptionIcon(java.lang.String filename, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void startInPlaceAnimationOnFrontMostApplication(android.os.Bundle opts) throws android.os.RemoteException
    {
    }
    @Override public void registerTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException
    {
    }
    @Override public void setTaskResizeable(int taskId, int resizeableMode) throws android.os.RemoteException
    {
    }
    @Override public void toggleFreeformWindowingMode(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void resizeTask(int taskId, android.graphics.Rect bounds, int resizeMode) throws android.os.RemoteException
    {
    }
    @Override public void moveStackToDisplay(int stackId, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void removeStack(int stackId) throws android.os.RemoteException
    {
    }
    /**
         * Sets the windowing mode for a specific task. Only works on tasks of type
         * {@link WindowConfiguration#ACTIVITY_TYPE_STANDARD}
         * @param taskId The id of the task to set the windowing mode for.
         * @param windowingMode The windowing mode to set for the task.
         * @param toTop If the task should be moved to the top once the windowing mode changes.
         */
    @Override public void setTaskWindowingMode(int taskId, int windowingMode, boolean toTop) throws android.os.RemoteException
    {
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
    @Override public boolean setTaskWindowingModeSplitScreenPrimary(int taskId, int createMode, boolean toTop, boolean animate, android.graphics.Rect initialBounds, boolean showRecents) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Use the offset to adjust the stack boundary with animation.
         *
         * @param stackId Id of the stack to adjust.
         * @param compareBounds Offset is only applied if the current pinned stack bounds is equal to
         *                      the compareBounds.
         * @param xOffset The horizontal offset.
         * @param yOffset The vertical offset.
         * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
         *                          default animation duration should be used.
         * @throws RemoteException
         */
    @Override public void offsetPinnedStackBounds(int stackId, android.graphics.Rect compareBounds, int xOffset, int yOffset, int animationDuration) throws android.os.RemoteException
    {
    }
    /**
         * Removes stacks in the input windowing modes from the system if they are of activity type
         * ACTIVITY_TYPE_STANDARD or ACTIVITY_TYPE_UNDEFINED
         */
    @Override public void removeStacksInWindowingModes(int[] windowingModes) throws android.os.RemoteException
    {
    }
    /** Removes stack of the activity types from the system. */
    @Override public void removeStacksWithActivityTypes(int[] activityTypes) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.app.ActivityManager.StackInfo> getAllStackInfos() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.ActivityManager.StackInfo getStackInfo(int windowingMode, int activityType) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Informs ActivityTaskManagerService that the keyguard is showing.
         *
         * @param showingKeyguard True if the keyguard is showing, false otherwise.
         * @param showingAod True if AOD is showing, false otherwise.
         */
    @Override public void setLockScreenShown(boolean showingKeyguard, boolean showingAod) throws android.os.RemoteException
    {
    }
    @Override public android.os.Bundle getAssistContextExtras(int requestType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean launchAssistIntent(android.content.Intent intent, int requestType, java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean requestAssistContextExtras(int requestType, android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, boolean focused, boolean newSessionId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean requestAutofillData(android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, int flags) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isAssistDataAllowedOnCurrentActivity() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean showAssistFromActivity(android.os.IBinder token, android.os.Bundle args) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isRootVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void showLockTaskEscapeMessage(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    /**
         * Notify the system that the keyguard is going away.
         *
         * @param flags See
         *              {@link android.view.WindowManagerPolicyConstants#KEYGUARD_GOING_AWAY_FLAG_TO_SHADE}
         *              etc.
         */
    @Override public void keyguardGoingAway(int flags) throws android.os.RemoteException
    {
    }
    @Override public android.content.ComponentName getActivityClassForToken(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getPackageForToken(android.os.IBinder token) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Try to place task to provided position. The final position might be different depending on
         * current user and stacks state. The task will be moved to target stack if it's currently in
         * different stack.
         */
    @Override public void positionTaskInStack(int taskId, int stackId, int position) throws android.os.RemoteException
    {
    }
    @Override public void reportSizeConfigurations(android.os.IBinder token, int[] horizontalSizeConfiguration, int[] verticalSizeConfigurations, int[] smallestWidthConfigurations) throws android.os.RemoteException
    {
    }
    /**
         * Dismisses split-screen multi-window mode.
         * {@param toTop} If true the current primary split-screen stack will be placed or left on top.
         */
    @Override public void dismissSplitScreenMode(boolean toTop) throws android.os.RemoteException
    {
    }
    /**
         * Dismisses PiP
         * @param animate True if the dismissal should be animated.
         * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
         *                          default animation duration should be used.
         */
    @Override public void dismissPip(boolean animate, int animationDuration) throws android.os.RemoteException
    {
    }
    @Override public void suppressResizeConfigChanges(boolean suppress) throws android.os.RemoteException
    {
    }
    @Override public void moveTasksToFullscreenStack(int fromStackId, boolean onTop) throws android.os.RemoteException
    {
    }
    @Override public boolean moveTopActivityToPinnedStack(int stackId, android.graphics.Rect bounds) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isInMultiWindowMode(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isInPictureInPictureMode(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean enterPictureInPictureMode(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setPictureInPictureParams(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException
    {
    }
    @Override public int getMaxNumPictureInPictureActions(android.os.IBinder token) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public android.os.IBinder getUriPermissionOwnerForActivity(android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return null;
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
    /**
         * Sets whether we are currently in an interactive split screen resize operation where we
         * are changing the docked stack size.
         */
    @Override public void setSplitScreenResizing(boolean resizing) throws android.os.RemoteException
    {
    }
    @Override public int setVrMode(android.os.IBinder token, boolean enabled, android.content.ComponentName packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void startLocalVoiceInteraction(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void stopLocalVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public boolean supportsLocalVoiceInteraction() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyPinnedStackAnimationStarted() throws android.os.RemoteException
    {
    }
    @Override public void notifyPinnedStackAnimationEnded() throws android.os.RemoteException
    {
    }
    // Get device configuration

    @Override public android.content.pm.ConfigurationInfo getDeviceConfigurationInfo() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Resizes the pinned stack.
         *
         * @param pinnedBounds The bounds for the pinned stack.
         * @param tempPinnedTaskBounds The temporary bounds for the tasks in the pinned stack, which
         *                             might be different from the stack bounds to allow more
         *                             flexibility while resizing, or {@code null} if they should be the
         *                             same as the stack bounds.
         */
    @Override public void resizePinnedStack(android.graphics.Rect pinnedBounds, android.graphics.Rect tempPinnedTaskBounds) throws android.os.RemoteException
    {
    }
    /**
         * Updates override configuration applied to specific display.
         * @param values Update values for display configuration. If null is passed it will request the
         *               Window Manager to compute new config for the specified display.
         * @param displayId Id of the display to apply the config to.
         * @throws RemoteException
         * @return Returns true if the configuration was updated.
         */
    @Override public boolean updateDisplayOverrideConfiguration(android.content.res.Configuration values, int displayId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void dismissKeyguard(android.os.IBinder token, com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
    {
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
    /**
         * See {@link android.app.Activity#setDisablePreviewScreenshots}
         */
    @Override public void setDisablePreviewScreenshots(android.os.IBinder token, boolean disable) throws android.os.RemoteException
    {
    }
    /**
         * Return the user id of last resumed activity.
         */
    @Override public int getLastResumedActivityUserId() throws android.os.RemoteException
    {
      return 0;
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
    @Override public void updateLockTaskFeatures(int userId, int flags) throws android.os.RemoteException
    {
    }
    @Override public void setShowWhenLocked(android.os.IBinder token, boolean showWhenLocked) throws android.os.RemoteException
    {
    }
    @Override public void setInheritShowWhenLocked(android.os.IBinder token, boolean setInheritShownWhenLocked) throws android.os.RemoteException
    {
    }
    @Override public void setTurnScreenOn(android.os.IBinder token, boolean turnScreenOn) throws android.os.RemoteException
    {
    }
    /**
         * Registers remote animations for a specific activity.
         */
    @Override public void registerRemoteAnimations(android.os.IBinder token, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException
    {
    }
    /**
         * Registers a remote animation to be run for all activity starts from a certain package during
         * a short predefined amount of time.
         */
    @Override public void registerRemoteAnimationForNextActivityStart(java.lang.String packageName, android.view.RemoteAnimationAdapter adapter) throws android.os.RemoteException
    {
    }
    /**
         * Registers remote animations for a display.
         */
    @Override public void registerRemoteAnimationsForDisplay(int displayId, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException
    {
    }
    /** @see android.app.ActivityManager#alwaysShowUnsupportedCompileSdkWarning */
    @Override public void alwaysShowUnsupportedCompileSdkWarning(android.content.ComponentName activity) throws android.os.RemoteException
    {
    }
    @Override public void setVrThread(int tid) throws android.os.RemoteException
    {
    }
    @Override public void setPersistentVrThread(int tid) throws android.os.RemoteException
    {
    }
    @Override public void stopAppSwitches() throws android.os.RemoteException
    {
    }
    @Override public void resumeAppSwitches() throws android.os.RemoteException
    {
    }
    @Override public void setActivityController(android.app.IActivityController watcher, boolean imAMonkey) throws android.os.RemoteException
    {
    }
    @Override public void setVoiceKeepAwake(android.service.voice.IVoiceInteractionSession session, boolean keepAwake) throws android.os.RemoteException
    {
    }
    @Override public int getPackageScreenCompatMode(java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setPackageScreenCompatMode(java.lang.String packageName, int mode) throws android.os.RemoteException
    {
    }
    @Override public boolean getPackageAskScreenCompat(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setPackageAskScreenCompat(java.lang.String packageName, boolean ask) throws android.os.RemoteException
    {
    }
    /**
         * Clears launch params for given packages.
         */
    @Override public void clearLaunchParamsForPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException
    {
    }
    /**
         * Makes the display with the given id a single task instance display. I.e the display can only
         * contain one task.
         */
    @Override public void setDisplayToSingleTaskInstance(int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Restarts the activity by killing its process if it is visible. If the activity is not
         * visible, the activity will not be restarted immediately and just keep the activity record in
         * the stack. It also resets the current override configuration so the activity will use the
         * configuration according to the latest state.
         *
         * @param activityToken The token of the target activity to restart.
         */
    @Override public void restartActivityProcessIfVisible(android.os.IBinder activityToken) throws android.os.RemoteException
    {
    }
    /**
         * Reports that an Activity received a back key press when there were no additional activities
         * on the back stack. If the Activity should be finished, the callback will be invoked. A
         * callback is used instead of finishing the activity directly from the server such that the
         * client may perform actions prior to finishing.
         */
    @Override public void onBackPressedOnTaskRoot(android.os.IBinder activityToken, android.app.IRequestFinishCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IActivityTaskManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IActivityTaskManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IActivityTaskManager interface,
     * generating a proxy if needed.
     */
    public static android.app.IActivityTaskManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IActivityTaskManager))) {
        return ((android.app.IActivityTaskManager)iin);
      }
      return new android.app.IActivityTaskManager.Stub.Proxy(obj);
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
        case TRANSACTION_startActivities:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent[] _arg2;
          _arg2 = data.createTypedArray(android.content.Intent.CREATOR);
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          android.os.IBinder _arg4;
          _arg4 = data.readStrongBinder();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          int _result = this.startActivities(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(_result);
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
        case TRANSACTION_startNextMatchingActivity:
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
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.startNextMatchingActivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startActivityIntentSender:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          android.content.IIntentSender _arg1;
          _arg1 = android.content.IIntentSender.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          android.content.Intent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.IBinder _arg5;
          _arg5 = data.readStrongBinder();
          java.lang.String _arg6;
          _arg6 = data.readString();
          int _arg7;
          _arg7 = data.readInt();
          int _arg8;
          _arg8 = data.readInt();
          int _arg9;
          _arg9 = data.readInt();
          android.os.Bundle _arg10;
          if ((0!=data.readInt())) {
            _arg10 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg10 = null;
          }
          int _result = this.startActivityIntentSender(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startActivityAndWait:
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
          android.app.WaitResult _result = this.startActivityAndWait(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
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
        case TRANSACTION_startActivityWithConfig:
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
          android.content.res.Configuration _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.content.res.Configuration.CREATOR.createFromParcel(data);
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
          int _result = this.startActivityWithConfig(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startVoiceActivity:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
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
          android.service.voice.IVoiceInteractionSession _arg5;
          _arg5 = android.service.voice.IVoiceInteractionSession.Stub.asInterface(data.readStrongBinder());
          com.android.internal.app.IVoiceInteractor _arg6;
          _arg6 = com.android.internal.app.IVoiceInteractor.Stub.asInterface(data.readStrongBinder());
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
          int _result = this.startVoiceActivity(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startAssistantActivity:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
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
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          int _result = this.startAssistantActivity(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(_result);
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
        case TRANSACTION_startActivityAsCaller:
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
          android.os.IBinder _arg10;
          _arg10 = data.readStrongBinder();
          boolean _arg11;
          _arg11 = (0!=data.readInt());
          int _arg12;
          _arg12 = data.readInt();
          int _result = this.startActivityAsCaller(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11, _arg12);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isActivityStartAllowedOnDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
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
          boolean _result = this.isActivityStartAllowedOnDisplay(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
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
        case TRANSACTION_finishActivityAffinity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.finishActivityAffinity(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_activityIdle:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.res.Configuration _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.res.Configuration.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.activityIdle(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_activityResumed:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.activityResumed(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activityTopResumedStateLost:
        {
          data.enforceInterface(descriptor);
          this.activityTopResumedStateLost();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activityPaused:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.activityPaused(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activityStopped:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.PersistableBundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.PersistableBundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.CharSequence _arg3;
          if (0!=data.readInt()) {
            _arg3 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.activityStopped(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activityDestroyed:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.activityDestroyed(_arg0);
          return true;
        }
        case TRANSACTION_activityRelaunched:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.activityRelaunched(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activitySlept:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.activitySlept(_arg0);
          return true;
        }
        case TRANSACTION_getFrontActivityScreenCompatMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getFrontActivityScreenCompatMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFrontActivityScreenCompatMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setFrontActivityScreenCompatMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCallingPackage:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _result = this.getCallingPackage(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCallingActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.ComponentName _result = this.getCallingActivity(_arg0);
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
        case TRANSACTION_setFocusedTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setFocusedTask(_arg0);
          reply.writeNoException();
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
        case TRANSACTION_removeAllVisibleRecentTasks:
        {
          data.enforceInterface(descriptor);
          this.removeAllVisibleRecentTasks();
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
        case TRANSACTION_shouldUpRecreateTask:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.shouldUpRecreateTask(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_navigateUpTo:
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
          int _arg2;
          _arg2 = data.readInt();
          android.content.Intent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _result = this.navigateUpTo(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
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
        case TRANSACTION_finishSubActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.finishSubActivity(_arg0, _arg1, _arg2);
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
        case TRANSACTION_willActivityBeVisible:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.willActivityBeVisible(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
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
        case TRANSACTION_getRequestedOrientation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.getRequestedOrientation(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_convertFromTranslucent:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.convertFromTranslucent(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_convertToTranslucent:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.convertToTranslucent(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyActivityDrawn:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.notifyActivityDrawn(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportActivityFullyDrawn:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.reportActivityFullyDrawn(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActivityDisplayId:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.getActivityDisplayId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isImmersive:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.isImmersive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setImmersive:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setImmersive(_arg0, _arg1);
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
        case TRANSACTION_getTaskDescription:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.ActivityManager.TaskDescription _result = this.getTaskDescription(_arg0);
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
        case TRANSACTION_overridePendingTransition:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.overridePendingTransition(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
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
        case TRANSACTION_reportAssistContextExtras:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.assist.AssistStructure _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.assist.AssistStructure.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.app.assist.AssistContent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.assist.AssistContent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.net.Uri _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.reportAssistContextExtras(_arg0, _arg1, _arg2, _arg3, _arg4);
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
        case TRANSACTION_cancelRecentsAnimation:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.cancelRecentsAnimation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startLockTaskModeByToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.startLockTaskModeByToken(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopLockTaskModeByToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.stopLockTaskModeByToken(_arg0);
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
        case TRANSACTION_isInLockTaskMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInLockTaskMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
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
        case TRANSACTION_setTaskDescription:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.app.ActivityManager.TaskDescription _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.ActivityManager.TaskDescription.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setTaskDescription(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActivityOptions:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _result = this.getActivityOptions(_arg0);
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
        case TRANSACTION_getAppTasks:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.os.IBinder> _result = this.getAppTasks(_arg0);
          reply.writeNoException();
          reply.writeBinderList(_result);
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
        case TRANSACTION_stopSystemLockTaskMode:
        {
          data.enforceInterface(descriptor);
          this.stopSystemLockTaskMode();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finishVoiceTask:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionSession _arg0;
          _arg0 = android.service.voice.IVoiceInteractionSession.Stub.asInterface(data.readStrongBinder());
          this.finishVoiceTask(_arg0);
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
        case TRANSACTION_notifyLaunchTaskBehindComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.notifyLaunchTaskBehindComplete(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyEnterAnimationComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.notifyEnterAnimationComplete(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addAppTask:
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
          android.app.ActivityManager.TaskDescription _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.ActivityManager.TaskDescription.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Bitmap _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Bitmap.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _result = this.addAppTask(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAppTaskThumbnailSize:
        {
          data.enforceInterface(descriptor);
          android.graphics.Point _result = this.getAppTaskThumbnailSize();
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
        case TRANSACTION_releaseActivityInstance:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.releaseActivityInstance(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestStartActivityPermissionToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _result = this.requestStartActivityPermissionToken(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_releaseSomeActivities:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          this.releaseSomeActivities(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTaskDescriptionIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Bitmap _result = this.getTaskDescriptionIcon(_arg0, _arg1);
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
        case TRANSACTION_startInPlaceAnimationOnFrontMostApplication:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startInPlaceAnimationOnFrontMostApplication(_arg0);
          reply.writeNoException();
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
        case TRANSACTION_toggleFreeformWindowingMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.toggleFreeformWindowingMode(_arg0);
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
        case TRANSACTION_moveStackToDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.moveStackToDisplay(_arg0, _arg1);
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
        case TRANSACTION_setTaskWindowingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setTaskWindowingMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
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
        case TRANSACTION_setTaskWindowingModeSplitScreenPrimary:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.graphics.Rect _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _result = this.setTaskWindowingModeSplitScreenPrimary(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_offsetPinnedStackBounds:
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
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.offsetPinnedStackBounds(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeStacksInWindowingModes:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          this.removeStacksInWindowingModes(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeStacksWithActivityTypes:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          this.removeStacksWithActivityTypes(_arg0);
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
        case TRANSACTION_getStackInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.app.ActivityManager.StackInfo _result = this.getStackInfo(_arg0, _arg1);
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
        case TRANSACTION_setLockScreenShown:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setLockScreenShown(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAssistContextExtras:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Bundle _result = this.getAssistContextExtras(_arg0);
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
        case TRANSACTION_launchAssistIntent:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _result = this.launchAssistIntent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestAssistContextExtras:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.IAssistDataReceiver _arg1;
          _arg1 = android.app.IAssistDataReceiver.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _result = this.requestAssistContextExtras(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestAutofillData:
        {
          data.enforceInterface(descriptor);
          android.app.IAssistDataReceiver _arg0;
          _arg0 = android.app.IAssistDataReceiver.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          int _arg3;
          _arg3 = data.readInt();
          boolean _result = this.requestAutofillData(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isAssistDataAllowedOnCurrentActivity:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isAssistDataAllowedOnCurrentActivity();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_showAssistFromActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.showAssistFromActivity(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRootVoiceInteraction:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.isRootVoiceInteraction(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_showLockTaskEscapeMessage:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.showLockTaskEscapeMessage(_arg0);
          return true;
        }
        case TRANSACTION_keyguardGoingAway:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.keyguardGoingAway(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActivityClassForToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.ComponentName _result = this.getActivityClassForToken(_arg0);
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
        case TRANSACTION_getPackageForToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _result = this.getPackageForToken(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
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
        case TRANSACTION_reportSizeConfigurations:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int[] _arg1;
          _arg1 = data.createIntArray();
          int[] _arg2;
          _arg2 = data.createIntArray();
          int[] _arg3;
          _arg3 = data.createIntArray();
          this.reportSizeConfigurations(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dismissSplitScreenMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.dismissSplitScreenMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dismissPip:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.dismissPip(_arg0, _arg1);
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
        case TRANSACTION_moveTasksToFullscreenStack:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.moveTasksToFullscreenStack(_arg0, _arg1);
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
        case TRANSACTION_isInMultiWindowMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.isInMultiWindowMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isInPictureInPictureMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.isInPictureInPictureMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enterPictureInPictureMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.app.PictureInPictureParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PictureInPictureParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.enterPictureInPictureMode(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPictureInPictureParams:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.app.PictureInPictureParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PictureInPictureParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setPictureInPictureParams(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMaxNumPictureInPictureActions:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.getMaxNumPictureInPictureActions(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getUriPermissionOwnerForActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _result = this.getUriPermissionOwnerForActivity(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
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
        case TRANSACTION_setSplitScreenResizing:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSplitScreenResizing(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVrMode:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _result = this.setVrMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startLocalVoiceInteraction:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.startLocalVoiceInteraction(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopLocalVoiceInteraction:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.stopLocalVoiceInteraction(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_supportsLocalVoiceInteraction:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.supportsLocalVoiceInteraction();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyPinnedStackAnimationStarted:
        {
          data.enforceInterface(descriptor);
          this.notifyPinnedStackAnimationStarted();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyPinnedStackAnimationEnded:
        {
          data.enforceInterface(descriptor);
          this.notifyPinnedStackAnimationEnded();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDeviceConfigurationInfo:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ConfigurationInfo _result = this.getDeviceConfigurationInfo();
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
        case TRANSACTION_resizePinnedStack:
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
          this.resizePinnedStack(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateDisplayOverrideConfiguration:
        {
          data.enforceInterface(descriptor);
          android.content.res.Configuration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.res.Configuration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.updateDisplayOverrideConfiguration(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_dismissKeyguard:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          com.android.internal.policy.IKeyguardDismissCallback _arg1;
          _arg1 = com.android.internal.policy.IKeyguardDismissCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.CharSequence _arg2;
          if (0!=data.readInt()) {
            _arg2 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.dismissKeyguard(_arg0, _arg1, _arg2);
          reply.writeNoException();
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
        case TRANSACTION_setDisablePreviewScreenshots:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setDisablePreviewScreenshots(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLastResumedActivityUserId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getLastResumedActivityUserId();
          reply.writeNoException();
          reply.writeInt(_result);
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
        case TRANSACTION_updateLockTaskFeatures:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.updateLockTaskFeatures(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setShowWhenLocked:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setShowWhenLocked(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInheritShowWhenLocked:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setInheritShowWhenLocked(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTurnScreenOn:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setTurnScreenOn(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerRemoteAnimations:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.view.RemoteAnimationDefinition _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.RemoteAnimationDefinition.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.registerRemoteAnimations(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerRemoteAnimationForNextActivityStart:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.view.RemoteAnimationAdapter _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.RemoteAnimationAdapter.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.registerRemoteAnimationForNextActivityStart(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerRemoteAnimationsForDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.RemoteAnimationDefinition _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.RemoteAnimationDefinition.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.registerRemoteAnimationsForDisplay(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_alwaysShowUnsupportedCompileSdkWarning:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.alwaysShowUnsupportedCompileSdkWarning(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVrThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setVrThread(_arg0);
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
        case TRANSACTION_setVoiceKeepAwake:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionSession _arg0;
          _arg0 = android.service.voice.IVoiceInteractionSession.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setVoiceKeepAwake(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackageScreenCompatMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getPackageScreenCompatMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
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
        case TRANSACTION_getPackageAskScreenCompat:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.getPackageAskScreenCompat(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPackageAskScreenCompat:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setPackageAskScreenCompat(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearLaunchParamsForPackages:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          this.clearLaunchParamsForPackages(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDisplayToSingleTaskInstance:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDisplayToSingleTaskInstance(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restartActivityProcessIfVisible:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.restartActivityProcessIfVisible(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onBackPressedOnTaskRoot:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.app.IRequestFinishCallback _arg1;
          _arg1 = android.app.IRequestFinishCallback.Stub.asInterface(data.readStrongBinder());
          this.onBackPressedOnTaskRoot(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IActivityTaskManager
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
      @Override public int startActivities(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent[] intents, java.lang.String[] resolvedTypes, android.os.IBinder resultTo, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          _data.writeTypedArray(intents, 0);
          _data.writeStringArray(resolvedTypes);
          _data.writeStrongBinder(resultTo);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivities(caller, callingPackage, intents, resolvedTypes, resultTo, options, userId);
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
      @Override public boolean startNextMatchingActivity(android.os.IBinder callingActivity, android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(callingActivity);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_startNextMatchingActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startNextMatchingActivity(callingActivity, intent, options);
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
      @Override public int startActivityIntentSender(android.app.IApplicationThread caller, android.content.IIntentSender target, android.os.IBinder whitelistToken, android.content.Intent fillInIntent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flagsMask, int flagsValues, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeStrongBinder((((target!=null))?(target.asBinder()):(null)));
          _data.writeStrongBinder(whitelistToken);
          if ((fillInIntent!=null)) {
            _data.writeInt(1);
            fillInIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder(resultTo);
          _data.writeString(resultWho);
          _data.writeInt(requestCode);
          _data.writeInt(flagsMask);
          _data.writeInt(flagsValues);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityIntentSender, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityIntentSender(caller, target, whitelistToken, fillInIntent, resolvedType, resultTo, resultWho, requestCode, flagsMask, flagsValues, options);
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
      @Override public android.app.WaitResult startActivityAndWait(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.WaitResult _result;
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityAndWait, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityAndWait(caller, callingPackage, intent, resolvedType, resultTo, resultWho, requestCode, flags, profilerInfo, options, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.WaitResult.CREATOR.createFromParcel(_reply);
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
      @Override public int startActivityWithConfig(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int startFlags, android.content.res.Configuration newConfig, android.os.Bundle options, int userId) throws android.os.RemoteException
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
          _data.writeInt(startFlags);
          if ((newConfig!=null)) {
            _data.writeInt(1);
            newConfig.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityWithConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityWithConfig(caller, callingPackage, intent, resolvedType, resultTo, resultWho, requestCode, startFlags, newConfig, options, userId);
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
      @Override public int startVoiceActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(callingPid);
          _data.writeInt(callingUid);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeStrongBinder((((interactor!=null))?(interactor.asBinder()):(null)));
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_startVoiceActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startVoiceActivity(callingPackage, callingPid, callingUid, intent, resolvedType, session, interactor, flags, profilerInfo, options, userId);
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
      @Override public int startAssistantActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.os.Bundle options, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(callingPid);
          _data.writeInt(callingUid);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAssistantActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startAssistantActivity(callingPackage, callingPid, callingUid, intent, resolvedType, options, userId);
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
      @Override public int startActivityAsCaller(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, android.os.IBinder permissionToken, boolean ignoreTargetSecurity, int userId) throws android.os.RemoteException
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
          _data.writeStrongBinder(permissionToken);
          _data.writeInt(((ignoreTargetSecurity)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityAsCaller, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startActivityAsCaller(caller, callingPackage, intent, resolvedType, resultTo, resultWho, requestCode, flags, profilerInfo, options, permissionToken, ignoreTargetSecurity, userId);
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
      @Override public boolean isActivityStartAllowedOnDisplay(int displayId, android.content.Intent intent, java.lang.String resolvedType, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActivityStartAllowedOnDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActivityStartAllowedOnDisplay(displayId, intent, resolvedType, userId);
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
      @Override public boolean finishActivityAffinity(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishActivityAffinity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().finishActivityAffinity(token);
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
      @Override public void activityIdle(android.os.IBinder token, android.content.res.Configuration config, boolean stopProfiling) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((stopProfiling)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityIdle, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityIdle(token, config, stopProfiling);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void activityResumed(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityResumed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityResumed(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activityTopResumedStateLost() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityTopResumedStateLost, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityTopResumedStateLost();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activityPaused(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityPaused, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityPaused(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activityStopped(android.os.IBinder token, android.os.Bundle state, android.os.PersistableBundle persistentState, java.lang.CharSequence description) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((persistentState!=null)) {
            _data.writeInt(1);
            persistentState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if (description!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(description, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityStopped, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityStopped(token, state, persistentState, description);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activityDestroyed(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityDestroyed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityDestroyed(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void activityRelaunched(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activityRelaunched, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activityRelaunched(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activitySlept(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activitySlept, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activitySlept(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public int getFrontActivityScreenCompatMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFrontActivityScreenCompatMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFrontActivityScreenCompatMode();
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
      @Override public void setFrontActivityScreenCompatMode(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFrontActivityScreenCompatMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFrontActivityScreenCompatMode(mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getCallingPackage(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallingPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallingPackage(token);
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
      @Override public android.content.ComponentName getCallingActivity(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallingActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallingActivity(token);
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
      @Override public void setFocusedTask(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFocusedTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFocusedTask(taskId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      @Override public void removeAllVisibleRecentTasks() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAllVisibleRecentTasks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAllVisibleRecentTasks();
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
      @Override public boolean shouldUpRecreateTask(android.os.IBinder token, java.lang.String destAffinity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(destAffinity);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldUpRecreateTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldUpRecreateTask(token, destAffinity);
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
      @Override public boolean navigateUpTo(android.os.IBinder token, android.content.Intent target, int resultCode, android.content.Intent resultData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((target!=null)) {
            _data.writeInt(1);
            target.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(resultCode);
          if ((resultData!=null)) {
            _data.writeInt(1);
            resultData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_navigateUpTo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().navigateUpTo(token, target, resultCode, resultData);
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
      @Override public void moveTaskToFront(android.app.IApplicationThread app, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((app!=null))?(app.asBinder()):(null)));
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
            getDefaultImpl().moveTaskToFront(app, callingPackage, task, flags, options);
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
      @Override public void finishSubActivity(android.os.IBinder token, java.lang.String resultWho, int requestCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(resultWho);
          _data.writeInt(requestCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSubActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSubActivity(token, resultWho, requestCode);
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
      @Override public boolean willActivityBeVisible(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_willActivityBeVisible, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().willActivityBeVisible(token);
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
      @Override public int getRequestedOrientation(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRequestedOrientation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRequestedOrientation(token);
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
      @Override public boolean convertFromTranslucent(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_convertFromTranslucent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().convertFromTranslucent(token);
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
      @Override public boolean convertToTranslucent(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_convertToTranslucent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().convertToTranslucent(token, options);
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
      @Override public void notifyActivityDrawn(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyActivityDrawn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyActivityDrawn(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportActivityFullyDrawn(android.os.IBinder token, boolean restoredFromBundle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((restoredFromBundle)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportActivityFullyDrawn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportActivityFullyDrawn(token, restoredFromBundle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getActivityDisplayId(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivityDisplayId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivityDisplayId(activityToken);
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
      @Override public boolean isImmersive(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isImmersive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isImmersive(token);
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
      @Override public void setImmersive(android.os.IBinder token, boolean immersive) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((immersive)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImmersive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImmersive(token, immersive);
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
      @Override public android.app.ActivityManager.TaskDescription getTaskDescription(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ActivityManager.TaskDescription _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTaskDescription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTaskDescription(taskId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ActivityManager.TaskDescription.CREATOR.createFromParcel(_reply);
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
      @Override public void overridePendingTransition(android.os.IBinder token, java.lang.String packageName, int enterAnim, int exitAnim) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(packageName);
          _data.writeInt(enterAnim);
          _data.writeInt(exitAnim);
          boolean _status = mRemote.transact(Stub.TRANSACTION_overridePendingTransition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().overridePendingTransition(token, packageName, enterAnim, exitAnim);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
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
      @Override public void reportAssistContextExtras(android.os.IBinder token, android.os.Bundle extras, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, android.net.Uri referrer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((structure!=null)) {
            _data.writeInt(1);
            structure.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((content!=null)) {
            _data.writeInt(1);
            content.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((referrer!=null)) {
            _data.writeInt(1);
            referrer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportAssistContextExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportAssistContextExtras(token, extras, structure, content, referrer);
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
      @Override public void startLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startLockTaskModeByToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startLockTaskModeByToken(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopLockTaskModeByToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopLockTaskModeByToken(token);
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
      @Override public void setTaskDescription(android.os.IBinder token, android.app.ActivityManager.TaskDescription values) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((values!=null)) {
            _data.writeInt(1);
            values.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTaskDescription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTaskDescription(token, values);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Bundle getActivityOptions(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivityOptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivityOptions(token);
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
      @Override public java.util.List<android.os.IBinder> getAppTasks(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.IBinder> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppTasks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppTasks(callingPackage);
          }
          _reply.readException();
          _result = _reply.createBinderArrayList();
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
      @Override public void stopSystemLockTaskMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopSystemLockTaskMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopSystemLockTaskMode();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finishVoiceTask(android.service.voice.IVoiceInteractionSession session) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishVoiceTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishVoiceTask(session);
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
      @Override public void notifyLaunchTaskBehindComplete(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyLaunchTaskBehindComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyLaunchTaskBehindComplete(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyEnterAnimationComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyEnterAnimationComplete(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int addAppTask(android.os.IBinder activityToken, android.content.Intent intent, android.app.ActivityManager.TaskDescription description, android.graphics.Bitmap thumbnail) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((description!=null)) {
            _data.writeInt(1);
            description.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((thumbnail!=null)) {
            _data.writeInt(1);
            thumbnail.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAppTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addAppTask(activityToken, intent, description, thumbnail);
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
      @Override public android.graphics.Point getAppTaskThumbnailSize() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Point _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppTaskThumbnailSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppTaskThumbnailSize();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Point.CREATOR.createFromParcel(_reply);
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
      @Override public boolean releaseActivityInstance(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseActivityInstance, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().releaseActivityInstance(token);
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
           * Only callable from the system. This token grants a temporary permission to call
           * #startActivityAsCallerWithToken. The token will time out after
           * START_AS_CALLER_TOKEN_TIMEOUT if it is not used.
           *
           * @param delegatorToken The Binder token referencing the system Activity that wants to delegate
           *        the #startActivityAsCaller to another app. The "caller" will be the caller of this
           *        activity's token, not the delegate's caller (which is probably the delegator itself).
           *
           * @return Returns a token that can be given to a "delegate" app that may call
           *         #startActivityAsCaller
           */
      @Override public android.os.IBinder requestStartActivityPermissionToken(android.os.IBinder delegatorToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(delegatorToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestStartActivityPermissionToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestStartActivityPermissionToken(delegatorToken);
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
      @Override public void releaseSomeActivities(android.app.IApplicationThread app) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((app!=null))?(app.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseSomeActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseSomeActivities(app);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.graphics.Bitmap getTaskDescriptionIcon(java.lang.String filename, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Bitmap _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(filename);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTaskDescriptionIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTaskDescriptionIcon(filename, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
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
      @Override public void startInPlaceAnimationOnFrontMostApplication(android.os.Bundle opts) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((opts!=null)) {
            _data.writeInt(1);
            opts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInPlaceAnimationOnFrontMostApplication, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startInPlaceAnimationOnFrontMostApplication(opts);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      @Override public void toggleFreeformWindowingMode(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleFreeformWindowingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleFreeformWindowingMode(token);
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
      @Override public void moveStackToDisplay(int stackId, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveStackToDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().moveStackToDisplay(stackId, displayId);
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
      /**
           * Sets the windowing mode for a specific task. Only works on tasks of type
           * {@link WindowConfiguration#ACTIVITY_TYPE_STANDARD}
           * @param taskId The id of the task to set the windowing mode for.
           * @param windowingMode The windowing mode to set for the task.
           * @param toTop If the task should be moved to the top once the windowing mode changes.
           */
      @Override public void setTaskWindowingMode(int taskId, int windowingMode, boolean toTop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(windowingMode);
          _data.writeInt(((toTop)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTaskWindowingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTaskWindowingMode(taskId, windowingMode, toTop);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      @Override public boolean setTaskWindowingModeSplitScreenPrimary(int taskId, int createMode, boolean toTop, boolean animate, android.graphics.Rect initialBounds, boolean showRecents) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeInt(createMode);
          _data.writeInt(((toTop)?(1):(0)));
          _data.writeInt(((animate)?(1):(0)));
          if ((initialBounds!=null)) {
            _data.writeInt(1);
            initialBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((showRecents)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTaskWindowingModeSplitScreenPrimary, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setTaskWindowingModeSplitScreenPrimary(taskId, createMode, toTop, animate, initialBounds, showRecents);
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
           * Use the offset to adjust the stack boundary with animation.
           *
           * @param stackId Id of the stack to adjust.
           * @param compareBounds Offset is only applied if the current pinned stack bounds is equal to
           *                      the compareBounds.
           * @param xOffset The horizontal offset.
           * @param yOffset The vertical offset.
           * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
           *                          default animation duration should be used.
           * @throws RemoteException
           */
      @Override public void offsetPinnedStackBounds(int stackId, android.graphics.Rect compareBounds, int xOffset, int yOffset, int animationDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(stackId);
          if ((compareBounds!=null)) {
            _data.writeInt(1);
            compareBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(xOffset);
          _data.writeInt(yOffset);
          _data.writeInt(animationDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_offsetPinnedStackBounds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().offsetPinnedStackBounds(stackId, compareBounds, xOffset, yOffset, animationDuration);
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
           * Removes stacks in the input windowing modes from the system if they are of activity type
           * ACTIVITY_TYPE_STANDARD or ACTIVITY_TYPE_UNDEFINED
           */
      @Override public void removeStacksInWindowingModes(int[] windowingModes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(windowingModes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeStacksInWindowingModes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeStacksInWindowingModes(windowingModes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Removes stack of the activity types from the system. */
      @Override public void removeStacksWithActivityTypes(int[] activityTypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(activityTypes);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeStacksWithActivityTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeStacksWithActivityTypes(activityTypes);
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
      @Override public android.app.ActivityManager.StackInfo getStackInfo(int windowingMode, int activityType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ActivityManager.StackInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(windowingMode);
          _data.writeInt(activityType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStackInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStackInfo(windowingMode, activityType);
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
      /**
           * Informs ActivityTaskManagerService that the keyguard is showing.
           *
           * @param showingKeyguard True if the keyguard is showing, false otherwise.
           * @param showingAod True if AOD is showing, false otherwise.
           */
      @Override public void setLockScreenShown(boolean showingKeyguard, boolean showingAod) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showingKeyguard)?(1):(0)));
          _data.writeInt(((showingAod)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLockScreenShown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLockScreenShown(showingKeyguard, showingAod);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Bundle getAssistContextExtras(int requestType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(requestType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAssistContextExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAssistContextExtras(requestType);
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
      @Override public boolean launchAssistIntent(android.content.Intent intent, int requestType, java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(requestType);
          _data.writeString(hint);
          _data.writeInt(userHandle);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchAssistIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().launchAssistIntent(intent, requestType, hint, userHandle, args);
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
      @Override public boolean requestAssistContextExtras(int requestType, android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, boolean focused, boolean newSessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(requestType);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          if ((receiverExtras!=null)) {
            _data.writeInt(1);
            receiverExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(activityToken);
          _data.writeInt(((focused)?(1):(0)));
          _data.writeInt(((newSessionId)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAssistContextExtras, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestAssistContextExtras(requestType, receiver, receiverExtras, activityToken, focused, newSessionId);
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
      @Override public boolean requestAutofillData(android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          if ((receiverExtras!=null)) {
            _data.writeInt(1);
            receiverExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(activityToken);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAutofillData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestAutofillData(receiver, receiverExtras, activityToken, flags);
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
      @Override public boolean isAssistDataAllowedOnCurrentActivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAssistDataAllowedOnCurrentActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAssistDataAllowedOnCurrentActivity();
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
      @Override public boolean showAssistFromActivity(android.os.IBinder token, android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_showAssistFromActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().showAssistFromActivity(token, args);
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
      @Override public boolean isRootVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRootVoiceInteraction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRootVoiceInteraction(token);
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
      @Override public void showLockTaskEscapeMessage(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showLockTaskEscapeMessage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showLockTaskEscapeMessage(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notify the system that the keyguard is going away.
           *
           * @param flags See
           *              {@link android.view.WindowManagerPolicyConstants#KEYGUARD_GOING_AWAY_FLAG_TO_SHADE}
           *              etc.
           */
      @Override public void keyguardGoingAway(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_keyguardGoingAway, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().keyguardGoingAway(flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.ComponentName getActivityClassForToken(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivityClassForToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivityClassForToken(token);
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
      @Override public java.lang.String getPackageForToken(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageForToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageForToken(token);
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
      @Override public void reportSizeConfigurations(android.os.IBinder token, int[] horizontalSizeConfiguration, int[] verticalSizeConfigurations, int[] smallestWidthConfigurations) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeIntArray(horizontalSizeConfiguration);
          _data.writeIntArray(verticalSizeConfigurations);
          _data.writeIntArray(smallestWidthConfigurations);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportSizeConfigurations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportSizeConfigurations(token, horizontalSizeConfiguration, verticalSizeConfigurations, smallestWidthConfigurations);
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
           * Dismisses split-screen multi-window mode.
           * {@param toTop} If true the current primary split-screen stack will be placed or left on top.
           */
      @Override public void dismissSplitScreenMode(boolean toTop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((toTop)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissSplitScreenMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissSplitScreenMode(toTop);
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
           * Dismisses PiP
           * @param animate True if the dismissal should be animated.
           * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
           *                          default animation duration should be used.
           */
      @Override public void dismissPip(boolean animate, int animationDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((animate)?(1):(0)));
          _data.writeInt(animationDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissPip, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissPip(animate, animationDuration);
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
      @Override public void moveTasksToFullscreenStack(int fromStackId, boolean onTop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(fromStackId);
          _data.writeInt(((onTop)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_moveTasksToFullscreenStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().moveTasksToFullscreenStack(fromStackId, onTop);
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
      @Override public boolean isInMultiWindowMode(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInMultiWindowMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInMultiWindowMode(token);
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
      @Override public boolean isInPictureInPictureMode(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInPictureInPictureMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInPictureInPictureMode(token);
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
      @Override public boolean enterPictureInPictureMode(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_enterPictureInPictureMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enterPictureInPictureMode(token, params);
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
      @Override public void setPictureInPictureParams(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPictureInPictureParams, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPictureInPictureParams(token, params);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMaxNumPictureInPictureActions(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMaxNumPictureInPictureActions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMaxNumPictureInPictureActions(token);
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
      @Override public android.os.IBinder getUriPermissionOwnerForActivity(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUriPermissionOwnerForActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUriPermissionOwnerForActivity(activityToken);
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
      /**
           * Sets whether we are currently in an interactive split screen resize operation where we
           * are changing the docked stack size.
           */
      @Override public void setSplitScreenResizing(boolean resizing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((resizing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSplitScreenResizing, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSplitScreenResizing(resizing);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int setVrMode(android.os.IBinder token, boolean enabled, android.content.ComponentName packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((enabled)?(1):(0)));
          if ((packageName!=null)) {
            _data.writeInt(1);
            packageName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVrMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setVrMode(token, enabled, packageName);
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
      @Override public void startLocalVoiceInteraction(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startLocalVoiceInteraction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startLocalVoiceInteraction(token, options);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopLocalVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopLocalVoiceInteraction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopLocalVoiceInteraction(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean supportsLocalVoiceInteraction() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supportsLocalVoiceInteraction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supportsLocalVoiceInteraction();
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
      @Override public void notifyPinnedStackAnimationStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPinnedStackAnimationStarted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPinnedStackAnimationStarted();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyPinnedStackAnimationEnded() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPinnedStackAnimationEnded, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPinnedStackAnimationEnded();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Get device configuration

      @Override public android.content.pm.ConfigurationInfo getDeviceConfigurationInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ConfigurationInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceConfigurationInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceConfigurationInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ConfigurationInfo.CREATOR.createFromParcel(_reply);
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
           * Resizes the pinned stack.
           *
           * @param pinnedBounds The bounds for the pinned stack.
           * @param tempPinnedTaskBounds The temporary bounds for the tasks in the pinned stack, which
           *                             might be different from the stack bounds to allow more
           *                             flexibility while resizing, or {@code null} if they should be the
           *                             same as the stack bounds.
           */
      @Override public void resizePinnedStack(android.graphics.Rect pinnedBounds, android.graphics.Rect tempPinnedTaskBounds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pinnedBounds!=null)) {
            _data.writeInt(1);
            pinnedBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((tempPinnedTaskBounds!=null)) {
            _data.writeInt(1);
            tempPinnedTaskBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resizePinnedStack, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resizePinnedStack(pinnedBounds, tempPinnedTaskBounds);
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
           * Updates override configuration applied to specific display.
           * @param values Update values for display configuration. If null is passed it will request the
           *               Window Manager to compute new config for the specified display.
           * @param displayId Id of the display to apply the config to.
           * @throws RemoteException
           * @return Returns true if the configuration was updated.
           */
      @Override public boolean updateDisplayOverrideConfiguration(android.content.res.Configuration values, int displayId) throws android.os.RemoteException
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
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateDisplayOverrideConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateDisplayOverrideConfiguration(values, displayId);
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
      @Override public void dismissKeyguard(android.os.IBinder token, com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissKeyguard(token, callback, message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
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
      /**
           * See {@link android.app.Activity#setDisablePreviewScreenshots}
           */
      @Override public void setDisablePreviewScreenshots(android.os.IBinder token, boolean disable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((disable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisablePreviewScreenshots, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisablePreviewScreenshots(token, disable);
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
           * Return the user id of last resumed activity.
           */
      @Override public int getLastResumedActivityUserId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastResumedActivityUserId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastResumedActivityUserId();
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
      @Override public void updateLockTaskFeatures(int userId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateLockTaskFeatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateLockTaskFeatures(userId, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setShowWhenLocked(android.os.IBinder token, boolean showWhenLocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((showWhenLocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setShowWhenLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setShowWhenLocked(token, showWhenLocked);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setInheritShowWhenLocked(android.os.IBinder token, boolean setInheritShownWhenLocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((setInheritShownWhenLocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInheritShowWhenLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInheritShowWhenLocked(token, setInheritShownWhenLocked);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTurnScreenOn(android.os.IBinder token, boolean turnScreenOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((turnScreenOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTurnScreenOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTurnScreenOn(token, turnScreenOn);
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
           * Registers remote animations for a specific activity.
           */
      @Override public void registerRemoteAnimations(android.os.IBinder token, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((definition!=null)) {
            _data.writeInt(1);
            definition.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerRemoteAnimations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerRemoteAnimations(token, definition);
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
           * Registers a remote animation to be run for all activity starts from a certain package during
           * a short predefined amount of time.
           */
      @Override public void registerRemoteAnimationForNextActivityStart(java.lang.String packageName, android.view.RemoteAnimationAdapter adapter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((adapter!=null)) {
            _data.writeInt(1);
            adapter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerRemoteAnimationForNextActivityStart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerRemoteAnimationForNextActivityStart(packageName, adapter);
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
           * Registers remote animations for a display.
           */
      @Override public void registerRemoteAnimationsForDisplay(int displayId, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          if ((definition!=null)) {
            _data.writeInt(1);
            definition.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerRemoteAnimationsForDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerRemoteAnimationsForDisplay(displayId, definition);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** @see android.app.ActivityManager#alwaysShowUnsupportedCompileSdkWarning */
      @Override public void alwaysShowUnsupportedCompileSdkWarning(android.content.ComponentName activity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((activity!=null)) {
            _data.writeInt(1);
            activity.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_alwaysShowUnsupportedCompileSdkWarning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().alwaysShowUnsupportedCompileSdkWarning(activity);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setVrThread(int tid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVrThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVrThread(tid);
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
      @Override public void setVoiceKeepAwake(android.service.voice.IVoiceInteractionSession session, boolean keepAwake) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeInt(((keepAwake)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoiceKeepAwake, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoiceKeepAwake(session, keepAwake);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getPackageScreenCompatMode(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageScreenCompatMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageScreenCompatMode(packageName);
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
      @Override public boolean getPackageAskScreenCompat(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackageAskScreenCompat, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackageAskScreenCompat(packageName);
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
      @Override public void setPackageAskScreenCompat(java.lang.String packageName, boolean ask) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((ask)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPackageAskScreenCompat, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPackageAskScreenCompat(packageName, ask);
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
           * Clears launch params for given packages.
           */
      @Override public void clearLaunchParamsForPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearLaunchParamsForPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearLaunchParamsForPackages(packageNames);
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
           * Makes the display with the given id a single task instance display. I.e the display can only
           * contain one task.
           */
      @Override public void setDisplayToSingleTaskInstance(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplayToSingleTaskInstance, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisplayToSingleTaskInstance(displayId);
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
           * Restarts the activity by killing its process if it is visible. If the activity is not
           * visible, the activity will not be restarted immediately and just keep the activity record in
           * the stack. It also resets the current override configuration so the activity will use the
           * configuration according to the latest state.
           *
           * @param activityToken The token of the target activity to restart.
           */
      @Override public void restartActivityProcessIfVisible(android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restartActivityProcessIfVisible, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restartActivityProcessIfVisible(activityToken);
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
           * Reports that an Activity received a back key press when there were no additional activities
           * on the back stack. If the Activity should be finished, the callback will be invoked. A
           * callback is used instead of finishing the activity directly from the server such that the
           * client may perform actions prior to finishing.
           */
      @Override public void onBackPressedOnTaskRoot(android.os.IBinder activityToken, android.app.IRequestFinishCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBackPressedOnTaskRoot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBackPressedOnTaskRoot(activityToken, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.IActivityTaskManager sDefaultImpl;
    }
    static final int TRANSACTION_startActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_startActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startActivityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_startNextMatchingActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startActivityIntentSender = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_startActivityAndWait = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_startActivityWithConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_startVoiceActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_startAssistantActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_startRecentsActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_startActivityFromRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_startActivityAsCaller = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_isActivityStartAllowedOnDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_unhandledBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_finishActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_finishActivityAffinity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_activityIdle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_activityResumed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_activityTopResumedStateLost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_activityPaused = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_activityStopped = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_activityDestroyed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_activityRelaunched = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_activitySlept = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getFrontActivityScreenCompatMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setFrontActivityScreenCompatMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getCallingPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getCallingActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_setFocusedTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_removeTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_removeAllVisibleRecentTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getFilteredTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_shouldUpRecreateTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_navigateUpTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_moveTaskToFront = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getTaskForActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_finishSubActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_getRecentTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_willActivityBeVisible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setRequestedOrientation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getRequestedOrientation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_convertFromTranslucent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_convertToTranslucent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_notifyActivityDrawn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_reportActivityFullyDrawn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_getActivityDisplayId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_isImmersive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_setImmersive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_isTopActivityImmersive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_moveActivityTaskToBack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_getTaskDescription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_overridePendingTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_getLaunchedFromUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_getLaunchedFromPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_reportAssistContextExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_setFocusedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_getFocusedStackInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_getTaskBounds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_cancelRecentsAnimation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_startLockTaskModeByToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_stopLockTaskModeByToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_updateLockTaskPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_isInLockTaskMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_getLockTaskModeState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_setTaskDescription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_getActivityOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_getAppTasks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_startSystemLockTaskMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_stopSystemLockTaskMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_finishVoiceTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_isTopOfTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_notifyLaunchTaskBehindComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_notifyEnterAnimationComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_addAppTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_getAppTaskThumbnailSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_releaseActivityInstance = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_requestStartActivityPermissionToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_releaseSomeActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_getTaskDescriptionIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_startInPlaceAnimationOnFrontMostApplication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_registerTaskStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_unregisterTaskStackListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_setTaskResizeable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_toggleFreeformWindowingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_resizeTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_moveStackToDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_removeStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_setTaskWindowingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_moveTaskToStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    static final int TRANSACTION_resizeStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 90);
    static final int TRANSACTION_setTaskWindowingModeSplitScreenPrimary = (android.os.IBinder.FIRST_CALL_TRANSACTION + 91);
    static final int TRANSACTION_offsetPinnedStackBounds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 92);
    static final int TRANSACTION_removeStacksInWindowingModes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 93);
    static final int TRANSACTION_removeStacksWithActivityTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 94);
    static final int TRANSACTION_getAllStackInfos = (android.os.IBinder.FIRST_CALL_TRANSACTION + 95);
    static final int TRANSACTION_getStackInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 96);
    static final int TRANSACTION_setLockScreenShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 97);
    static final int TRANSACTION_getAssistContextExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 98);
    static final int TRANSACTION_launchAssistIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 99);
    static final int TRANSACTION_requestAssistContextExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 100);
    static final int TRANSACTION_requestAutofillData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 101);
    static final int TRANSACTION_isAssistDataAllowedOnCurrentActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 102);
    static final int TRANSACTION_showAssistFromActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 103);
    static final int TRANSACTION_isRootVoiceInteraction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 104);
    static final int TRANSACTION_showLockTaskEscapeMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 105);
    static final int TRANSACTION_keyguardGoingAway = (android.os.IBinder.FIRST_CALL_TRANSACTION + 106);
    static final int TRANSACTION_getActivityClassForToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 107);
    static final int TRANSACTION_getPackageForToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 108);
    static final int TRANSACTION_positionTaskInStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 109);
    static final int TRANSACTION_reportSizeConfigurations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 110);
    static final int TRANSACTION_dismissSplitScreenMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 111);
    static final int TRANSACTION_dismissPip = (android.os.IBinder.FIRST_CALL_TRANSACTION + 112);
    static final int TRANSACTION_suppressResizeConfigChanges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 113);
    static final int TRANSACTION_moveTasksToFullscreenStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 114);
    static final int TRANSACTION_moveTopActivityToPinnedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 115);
    static final int TRANSACTION_isInMultiWindowMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 116);
    static final int TRANSACTION_isInPictureInPictureMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 117);
    static final int TRANSACTION_enterPictureInPictureMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 118);
    static final int TRANSACTION_setPictureInPictureParams = (android.os.IBinder.FIRST_CALL_TRANSACTION + 119);
    static final int TRANSACTION_getMaxNumPictureInPictureActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 120);
    static final int TRANSACTION_getUriPermissionOwnerForActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 121);
    static final int TRANSACTION_resizeDockedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 122);
    static final int TRANSACTION_setSplitScreenResizing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 123);
    static final int TRANSACTION_setVrMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 124);
    static final int TRANSACTION_startLocalVoiceInteraction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 125);
    static final int TRANSACTION_stopLocalVoiceInteraction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 126);
    static final int TRANSACTION_supportsLocalVoiceInteraction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 127);
    static final int TRANSACTION_notifyPinnedStackAnimationStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 128);
    static final int TRANSACTION_notifyPinnedStackAnimationEnded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 129);
    static final int TRANSACTION_getDeviceConfigurationInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 130);
    static final int TRANSACTION_resizePinnedStack = (android.os.IBinder.FIRST_CALL_TRANSACTION + 131);
    static final int TRANSACTION_updateDisplayOverrideConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 132);
    static final int TRANSACTION_dismissKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 133);
    static final int TRANSACTION_cancelTaskWindowTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 134);
    static final int TRANSACTION_getTaskSnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 135);
    static final int TRANSACTION_setDisablePreviewScreenshots = (android.os.IBinder.FIRST_CALL_TRANSACTION + 136);
    static final int TRANSACTION_getLastResumedActivityUserId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 137);
    static final int TRANSACTION_updateConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 138);
    static final int TRANSACTION_updateLockTaskFeatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 139);
    static final int TRANSACTION_setShowWhenLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 140);
    static final int TRANSACTION_setInheritShowWhenLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 141);
    static final int TRANSACTION_setTurnScreenOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 142);
    static final int TRANSACTION_registerRemoteAnimations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 143);
    static final int TRANSACTION_registerRemoteAnimationForNextActivityStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 144);
    static final int TRANSACTION_registerRemoteAnimationsForDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 145);
    static final int TRANSACTION_alwaysShowUnsupportedCompileSdkWarning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 146);
    static final int TRANSACTION_setVrThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 147);
    static final int TRANSACTION_setPersistentVrThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 148);
    static final int TRANSACTION_stopAppSwitches = (android.os.IBinder.FIRST_CALL_TRANSACTION + 149);
    static final int TRANSACTION_resumeAppSwitches = (android.os.IBinder.FIRST_CALL_TRANSACTION + 150);
    static final int TRANSACTION_setActivityController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 151);
    static final int TRANSACTION_setVoiceKeepAwake = (android.os.IBinder.FIRST_CALL_TRANSACTION + 152);
    static final int TRANSACTION_getPackageScreenCompatMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 153);
    static final int TRANSACTION_setPackageScreenCompatMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 154);
    static final int TRANSACTION_getPackageAskScreenCompat = (android.os.IBinder.FIRST_CALL_TRANSACTION + 155);
    static final int TRANSACTION_setPackageAskScreenCompat = (android.os.IBinder.FIRST_CALL_TRANSACTION + 156);
    static final int TRANSACTION_clearLaunchParamsForPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 157);
    static final int TRANSACTION_setDisplayToSingleTaskInstance = (android.os.IBinder.FIRST_CALL_TRANSACTION + 158);
    static final int TRANSACTION_restartActivityProcessIfVisible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 159);
    static final int TRANSACTION_onBackPressedOnTaskRoot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 160);
    public static boolean setDefaultImpl(android.app.IActivityTaskManager impl) {
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
    public static android.app.IActivityTaskManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int startActivity(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options) throws android.os.RemoteException;
  public int startActivities(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent[] intents, java.lang.String[] resolvedTypes, android.os.IBinder resultTo, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public int startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public boolean startNextMatchingActivity(android.os.IBinder callingActivity, android.content.Intent intent, android.os.Bundle options) throws android.os.RemoteException;
  public int startActivityIntentSender(android.app.IApplicationThread caller, android.content.IIntentSender target, android.os.IBinder whitelistToken, android.content.Intent fillInIntent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flagsMask, int flagsValues, android.os.Bundle options) throws android.os.RemoteException;
  public android.app.WaitResult startActivityAndWait(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public int startActivityWithConfig(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int startFlags, android.content.res.Configuration newConfig, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public int startVoiceActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public int startAssistantActivity(java.lang.String callingPackage, int callingPid, int callingUid, android.content.Intent intent, java.lang.String resolvedType, android.os.Bundle options, int userId) throws android.os.RemoteException;
  public void startRecentsActivity(android.content.Intent intent, android.app.IAssistDataReceiver assistDataReceiver, android.view.IRecentsAnimationRunner recentsAnimationRunner) throws android.os.RemoteException;
  public int startActivityFromRecents(int taskId, android.os.Bundle options) throws android.os.RemoteException;
  public int startActivityAsCaller(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder resultTo, java.lang.String resultWho, int requestCode, int flags, android.app.ProfilerInfo profilerInfo, android.os.Bundle options, android.os.IBinder permissionToken, boolean ignoreTargetSecurity, int userId) throws android.os.RemoteException;
  public boolean isActivityStartAllowedOnDisplay(int displayId, android.content.Intent intent, java.lang.String resolvedType, int userId) throws android.os.RemoteException;
  public void unhandledBack() throws android.os.RemoteException;
  public boolean finishActivity(android.os.IBinder token, int code, android.content.Intent data, int finishTask) throws android.os.RemoteException;
  public boolean finishActivityAffinity(android.os.IBinder token) throws android.os.RemoteException;
  public void activityIdle(android.os.IBinder token, android.content.res.Configuration config, boolean stopProfiling) throws android.os.RemoteException;
  public void activityResumed(android.os.IBinder token) throws android.os.RemoteException;
  public void activityTopResumedStateLost() throws android.os.RemoteException;
  public void activityPaused(android.os.IBinder token) throws android.os.RemoteException;
  public void activityStopped(android.os.IBinder token, android.os.Bundle state, android.os.PersistableBundle persistentState, java.lang.CharSequence description) throws android.os.RemoteException;
  public void activityDestroyed(android.os.IBinder token) throws android.os.RemoteException;
  public void activityRelaunched(android.os.IBinder token) throws android.os.RemoteException;
  public void activitySlept(android.os.IBinder token) throws android.os.RemoteException;
  public int getFrontActivityScreenCompatMode() throws android.os.RemoteException;
  public void setFrontActivityScreenCompatMode(int mode) throws android.os.RemoteException;
  public java.lang.String getCallingPackage(android.os.IBinder token) throws android.os.RemoteException;
  public android.content.ComponentName getCallingActivity(android.os.IBinder token) throws android.os.RemoteException;
  public void setFocusedTask(int taskId) throws android.os.RemoteException;
  public boolean removeTask(int taskId) throws android.os.RemoteException;
  public void removeAllVisibleRecentTasks() throws android.os.RemoteException;
  public java.util.List<android.app.ActivityManager.RunningTaskInfo> getTasks(int maxNum) throws android.os.RemoteException;
  public java.util.List<android.app.ActivityManager.RunningTaskInfo> getFilteredTasks(int maxNum, int ignoreActivityType, int ignoreWindowingMode) throws android.os.RemoteException;
  public boolean shouldUpRecreateTask(android.os.IBinder token, java.lang.String destAffinity) throws android.os.RemoteException;
  public boolean navigateUpTo(android.os.IBinder token, android.content.Intent target, int resultCode, android.content.Intent resultData) throws android.os.RemoteException;
  public void moveTaskToFront(android.app.IApplicationThread app, java.lang.String callingPackage, int task, int flags, android.os.Bundle options) throws android.os.RemoteException;
  public int getTaskForActivity(android.os.IBinder token, boolean onlyRoot) throws android.os.RemoteException;
  public void finishSubActivity(android.os.IBinder token, java.lang.String resultWho, int requestCode) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getRecentTasks(int maxNum, int flags, int userId) throws android.os.RemoteException;
  public boolean willActivityBeVisible(android.os.IBinder token) throws android.os.RemoteException;
  public void setRequestedOrientation(android.os.IBinder token, int requestedOrientation) throws android.os.RemoteException;
  public int getRequestedOrientation(android.os.IBinder token) throws android.os.RemoteException;
  public boolean convertFromTranslucent(android.os.IBinder token) throws android.os.RemoteException;
  public boolean convertToTranslucent(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException;
  public void notifyActivityDrawn(android.os.IBinder token) throws android.os.RemoteException;
  public void reportActivityFullyDrawn(android.os.IBinder token, boolean restoredFromBundle) throws android.os.RemoteException;
  public int getActivityDisplayId(android.os.IBinder activityToken) throws android.os.RemoteException;
  public boolean isImmersive(android.os.IBinder token) throws android.os.RemoteException;
  public void setImmersive(android.os.IBinder token, boolean immersive) throws android.os.RemoteException;
  public boolean isTopActivityImmersive() throws android.os.RemoteException;
  public boolean moveActivityTaskToBack(android.os.IBinder token, boolean nonRoot) throws android.os.RemoteException;
  public android.app.ActivityManager.TaskDescription getTaskDescription(int taskId) throws android.os.RemoteException;
  public void overridePendingTransition(android.os.IBinder token, java.lang.String packageName, int enterAnim, int exitAnim) throws android.os.RemoteException;
  public int getLaunchedFromUid(android.os.IBinder activityToken) throws android.os.RemoteException;
  public java.lang.String getLaunchedFromPackage(android.os.IBinder activityToken) throws android.os.RemoteException;
  public void reportAssistContextExtras(android.os.IBinder token, android.os.Bundle extras, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, android.net.Uri referrer) throws android.os.RemoteException;
  public void setFocusedStack(int stackId) throws android.os.RemoteException;
  public android.app.ActivityManager.StackInfo getFocusedStackInfo() throws android.os.RemoteException;
  public android.graphics.Rect getTaskBounds(int taskId) throws android.os.RemoteException;
  public void cancelRecentsAnimation(boolean restoreHomeStackPosition) throws android.os.RemoteException;
  public void startLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException;
  public void stopLockTaskModeByToken(android.os.IBinder token) throws android.os.RemoteException;
  public void updateLockTaskPackages(int userId, java.lang.String[] packages) throws android.os.RemoteException;
  public boolean isInLockTaskMode() throws android.os.RemoteException;
  public int getLockTaskModeState() throws android.os.RemoteException;
  public void setTaskDescription(android.os.IBinder token, android.app.ActivityManager.TaskDescription values) throws android.os.RemoteException;
  public android.os.Bundle getActivityOptions(android.os.IBinder token) throws android.os.RemoteException;
  public java.util.List<android.os.IBinder> getAppTasks(java.lang.String callingPackage) throws android.os.RemoteException;
  public void startSystemLockTaskMode(int taskId) throws android.os.RemoteException;
  public void stopSystemLockTaskMode() throws android.os.RemoteException;
  public void finishVoiceTask(android.service.voice.IVoiceInteractionSession session) throws android.os.RemoteException;
  public boolean isTopOfTask(android.os.IBinder token) throws android.os.RemoteException;
  public void notifyLaunchTaskBehindComplete(android.os.IBinder token) throws android.os.RemoteException;
  public void notifyEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException;
  public int addAppTask(android.os.IBinder activityToken, android.content.Intent intent, android.app.ActivityManager.TaskDescription description, android.graphics.Bitmap thumbnail) throws android.os.RemoteException;
  public android.graphics.Point getAppTaskThumbnailSize() throws android.os.RemoteException;
  public boolean releaseActivityInstance(android.os.IBinder token) throws android.os.RemoteException;
  /**
       * Only callable from the system. This token grants a temporary permission to call
       * #startActivityAsCallerWithToken. The token will time out after
       * START_AS_CALLER_TOKEN_TIMEOUT if it is not used.
       *
       * @param delegatorToken The Binder token referencing the system Activity that wants to delegate
       *        the #startActivityAsCaller to another app. The "caller" will be the caller of this
       *        activity's token, not the delegate's caller (which is probably the delegator itself).
       *
       * @return Returns a token that can be given to a "delegate" app that may call
       *         #startActivityAsCaller
       */
  public android.os.IBinder requestStartActivityPermissionToken(android.os.IBinder delegatorToken) throws android.os.RemoteException;
  public void releaseSomeActivities(android.app.IApplicationThread app) throws android.os.RemoteException;
  public android.graphics.Bitmap getTaskDescriptionIcon(java.lang.String filename, int userId) throws android.os.RemoteException;
  public void startInPlaceAnimationOnFrontMostApplication(android.os.Bundle opts) throws android.os.RemoteException;
  public void registerTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException;
  public void unregisterTaskStackListener(android.app.ITaskStackListener listener) throws android.os.RemoteException;
  public void setTaskResizeable(int taskId, int resizeableMode) throws android.os.RemoteException;
  public void toggleFreeformWindowingMode(android.os.IBinder token) throws android.os.RemoteException;
  public void resizeTask(int taskId, android.graphics.Rect bounds, int resizeMode) throws android.os.RemoteException;
  public void moveStackToDisplay(int stackId, int displayId) throws android.os.RemoteException;
  public void removeStack(int stackId) throws android.os.RemoteException;
  /**
       * Sets the windowing mode for a specific task. Only works on tasks of type
       * {@link WindowConfiguration#ACTIVITY_TYPE_STANDARD}
       * @param taskId The id of the task to set the windowing mode for.
       * @param windowingMode The windowing mode to set for the task.
       * @param toTop If the task should be moved to the top once the windowing mode changes.
       */
  public void setTaskWindowingMode(int taskId, int windowingMode, boolean toTop) throws android.os.RemoteException;
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
  public void resizeStack(int stackId, android.graphics.Rect bounds, boolean allowResizeInDockedMode, boolean preserveWindows, boolean animate, int animationDuration) throws android.os.RemoteException;
  public boolean setTaskWindowingModeSplitScreenPrimary(int taskId, int createMode, boolean toTop, boolean animate, android.graphics.Rect initialBounds, boolean showRecents) throws android.os.RemoteException;
  /**
       * Use the offset to adjust the stack boundary with animation.
       *
       * @param stackId Id of the stack to adjust.
       * @param compareBounds Offset is only applied if the current pinned stack bounds is equal to
       *                      the compareBounds.
       * @param xOffset The horizontal offset.
       * @param yOffset The vertical offset.
       * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
       *                          default animation duration should be used.
       * @throws RemoteException
       */
  public void offsetPinnedStackBounds(int stackId, android.graphics.Rect compareBounds, int xOffset, int yOffset, int animationDuration) throws android.os.RemoteException;
  /**
       * Removes stacks in the input windowing modes from the system if they are of activity type
       * ACTIVITY_TYPE_STANDARD or ACTIVITY_TYPE_UNDEFINED
       */
  public void removeStacksInWindowingModes(int[] windowingModes) throws android.os.RemoteException;
  /** Removes stack of the activity types from the system. */
  public void removeStacksWithActivityTypes(int[] activityTypes) throws android.os.RemoteException;
  public java.util.List<android.app.ActivityManager.StackInfo> getAllStackInfos() throws android.os.RemoteException;
  public android.app.ActivityManager.StackInfo getStackInfo(int windowingMode, int activityType) throws android.os.RemoteException;
  /**
       * Informs ActivityTaskManagerService that the keyguard is showing.
       *
       * @param showingKeyguard True if the keyguard is showing, false otherwise.
       * @param showingAod True if AOD is showing, false otherwise.
       */
  public void setLockScreenShown(boolean showingKeyguard, boolean showingAod) throws android.os.RemoteException;
  public android.os.Bundle getAssistContextExtras(int requestType) throws android.os.RemoteException;
  public boolean launchAssistIntent(android.content.Intent intent, int requestType, java.lang.String hint, int userHandle, android.os.Bundle args) throws android.os.RemoteException;
  public boolean requestAssistContextExtras(int requestType, android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, boolean focused, boolean newSessionId) throws android.os.RemoteException;
  public boolean requestAutofillData(android.app.IAssistDataReceiver receiver, android.os.Bundle receiverExtras, android.os.IBinder activityToken, int flags) throws android.os.RemoteException;
  public boolean isAssistDataAllowedOnCurrentActivity() throws android.os.RemoteException;
  public boolean showAssistFromActivity(android.os.IBinder token, android.os.Bundle args) throws android.os.RemoteException;
  public boolean isRootVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException;
  public void showLockTaskEscapeMessage(android.os.IBinder token) throws android.os.RemoteException;
  /**
       * Notify the system that the keyguard is going away.
       *
       * @param flags See
       *              {@link android.view.WindowManagerPolicyConstants#KEYGUARD_GOING_AWAY_FLAG_TO_SHADE}
       *              etc.
       */
  public void keyguardGoingAway(int flags) throws android.os.RemoteException;
  public android.content.ComponentName getActivityClassForToken(android.os.IBinder token) throws android.os.RemoteException;
  public java.lang.String getPackageForToken(android.os.IBinder token) throws android.os.RemoteException;
  /**
       * Try to place task to provided position. The final position might be different depending on
       * current user and stacks state. The task will be moved to target stack if it's currently in
       * different stack.
       */
  public void positionTaskInStack(int taskId, int stackId, int position) throws android.os.RemoteException;
  public void reportSizeConfigurations(android.os.IBinder token, int[] horizontalSizeConfiguration, int[] verticalSizeConfigurations, int[] smallestWidthConfigurations) throws android.os.RemoteException;
  /**
       * Dismisses split-screen multi-window mode.
       * {@param toTop} If true the current primary split-screen stack will be placed or left on top.
       */
  public void dismissSplitScreenMode(boolean toTop) throws android.os.RemoteException;
  /**
       * Dismisses PiP
       * @param animate True if the dismissal should be animated.
       * @param animationDuration The duration of the resize animation in milliseconds or -1 if the
       *                          default animation duration should be used.
       */
  public void dismissPip(boolean animate, int animationDuration) throws android.os.RemoteException;
  public void suppressResizeConfigChanges(boolean suppress) throws android.os.RemoteException;
  public void moveTasksToFullscreenStack(int fromStackId, boolean onTop) throws android.os.RemoteException;
  public boolean moveTopActivityToPinnedStack(int stackId, android.graphics.Rect bounds) throws android.os.RemoteException;
  public boolean isInMultiWindowMode(android.os.IBinder token) throws android.os.RemoteException;
  public boolean isInPictureInPictureMode(android.os.IBinder token) throws android.os.RemoteException;
  public boolean enterPictureInPictureMode(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException;
  public void setPictureInPictureParams(android.os.IBinder token, android.app.PictureInPictureParams params) throws android.os.RemoteException;
  public int getMaxNumPictureInPictureActions(android.os.IBinder token) throws android.os.RemoteException;
  public android.os.IBinder getUriPermissionOwnerForActivity(android.os.IBinder activityToken) throws android.os.RemoteException;
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
  public void resizeDockedStack(android.graphics.Rect dockedBounds, android.graphics.Rect tempDockedTaskBounds, android.graphics.Rect tempDockedTaskInsetBounds, android.graphics.Rect tempOtherTaskBounds, android.graphics.Rect tempOtherTaskInsetBounds) throws android.os.RemoteException;
  /**
       * Sets whether we are currently in an interactive split screen resize operation where we
       * are changing the docked stack size.
       */
  public void setSplitScreenResizing(boolean resizing) throws android.os.RemoteException;
  public int setVrMode(android.os.IBinder token, boolean enabled, android.content.ComponentName packageName) throws android.os.RemoteException;
  public void startLocalVoiceInteraction(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException;
  public void stopLocalVoiceInteraction(android.os.IBinder token) throws android.os.RemoteException;
  public boolean supportsLocalVoiceInteraction() throws android.os.RemoteException;
  public void notifyPinnedStackAnimationStarted() throws android.os.RemoteException;
  public void notifyPinnedStackAnimationEnded() throws android.os.RemoteException;
  // Get device configuration

  public android.content.pm.ConfigurationInfo getDeviceConfigurationInfo() throws android.os.RemoteException;
  /**
       * Resizes the pinned stack.
       *
       * @param pinnedBounds The bounds for the pinned stack.
       * @param tempPinnedTaskBounds The temporary bounds for the tasks in the pinned stack, which
       *                             might be different from the stack bounds to allow more
       *                             flexibility while resizing, or {@code null} if they should be the
       *                             same as the stack bounds.
       */
  public void resizePinnedStack(android.graphics.Rect pinnedBounds, android.graphics.Rect tempPinnedTaskBounds) throws android.os.RemoteException;
  /**
       * Updates override configuration applied to specific display.
       * @param values Update values for display configuration. If null is passed it will request the
       *               Window Manager to compute new config for the specified display.
       * @param displayId Id of the display to apply the config to.
       * @throws RemoteException
       * @return Returns true if the configuration was updated.
       */
  public boolean updateDisplayOverrideConfiguration(android.content.res.Configuration values, int displayId) throws android.os.RemoteException;
  public void dismissKeyguard(android.os.IBinder token, com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException;
  /** Cancels the window transitions for the given task. */
  public void cancelTaskWindowTransition(int taskId) throws android.os.RemoteException;
  /**
       * @param taskId the id of the task to retrieve the sAutoapshots for
       * @param reducedResolution if set, if the snapshot needs to be loaded from disk, this will load
       *                          a reduced resolution of it, which is much faster
       * @return a graphic buffer representing a screenshot of a task
       */
  public android.app.ActivityManager.TaskSnapshot getTaskSnapshot(int taskId, boolean reducedResolution) throws android.os.RemoteException;
  /**
       * See {@link android.app.Activity#setDisablePreviewScreenshots}
       */
  public void setDisablePreviewScreenshots(android.os.IBinder token, boolean disable) throws android.os.RemoteException;
  /**
       * Return the user id of last resumed activity.
       */
  public int getLastResumedActivityUserId() throws android.os.RemoteException;
  /**
       * Updates global configuration and applies changes to the entire system.
       * @param values Update values for global configuration. If null is passed it will request the
       *               Window Manager to compute new config for the default display.
       * @throws RemoteException
       * @return Returns true if the configuration was updated.
       */
  public boolean updateConfiguration(android.content.res.Configuration values) throws android.os.RemoteException;
  public void updateLockTaskFeatures(int userId, int flags) throws android.os.RemoteException;
  public void setShowWhenLocked(android.os.IBinder token, boolean showWhenLocked) throws android.os.RemoteException;
  public void setInheritShowWhenLocked(android.os.IBinder token, boolean setInheritShownWhenLocked) throws android.os.RemoteException;
  public void setTurnScreenOn(android.os.IBinder token, boolean turnScreenOn) throws android.os.RemoteException;
  /**
       * Registers remote animations for a specific activity.
       */
  public void registerRemoteAnimations(android.os.IBinder token, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException;
  /**
       * Registers a remote animation to be run for all activity starts from a certain package during
       * a short predefined amount of time.
       */
  public void registerRemoteAnimationForNextActivityStart(java.lang.String packageName, android.view.RemoteAnimationAdapter adapter) throws android.os.RemoteException;
  /**
       * Registers remote animations for a display.
       */
  public void registerRemoteAnimationsForDisplay(int displayId, android.view.RemoteAnimationDefinition definition) throws android.os.RemoteException;
  /** @see android.app.ActivityManager#alwaysShowUnsupportedCompileSdkWarning */
  public void alwaysShowUnsupportedCompileSdkWarning(android.content.ComponentName activity) throws android.os.RemoteException;
  public void setVrThread(int tid) throws android.os.RemoteException;
  public void setPersistentVrThread(int tid) throws android.os.RemoteException;
  public void stopAppSwitches() throws android.os.RemoteException;
  public void resumeAppSwitches() throws android.os.RemoteException;
  public void setActivityController(android.app.IActivityController watcher, boolean imAMonkey) throws android.os.RemoteException;
  public void setVoiceKeepAwake(android.service.voice.IVoiceInteractionSession session, boolean keepAwake) throws android.os.RemoteException;
  public int getPackageScreenCompatMode(java.lang.String packageName) throws android.os.RemoteException;
  public void setPackageScreenCompatMode(java.lang.String packageName, int mode) throws android.os.RemoteException;
  public boolean getPackageAskScreenCompat(java.lang.String packageName) throws android.os.RemoteException;
  public void setPackageAskScreenCompat(java.lang.String packageName, boolean ask) throws android.os.RemoteException;
  /**
       * Clears launch params for given packages.
       */
  public void clearLaunchParamsForPackages(java.util.List<java.lang.String> packageNames) throws android.os.RemoteException;
  /**
       * Makes the display with the given id a single task instance display. I.e the display can only
       * contain one task.
       */
  public void setDisplayToSingleTaskInstance(int displayId) throws android.os.RemoteException;
  /**
       * Restarts the activity by killing its process if it is visible. If the activity is not
       * visible, the activity will not be restarted immediately and just keep the activity record in
       * the stack. It also resets the current override configuration so the activity will use the
       * configuration according to the latest state.
       *
       * @param activityToken The token of the target activity to restart.
       */
  public void restartActivityProcessIfVisible(android.os.IBinder activityToken) throws android.os.RemoteException;
  /**
       * Reports that an Activity received a back key press when there were no additional activities
       * on the back stack. If the Activity should be finished, the callback will be invoked. A
       * callback is used instead of finishing the activity directly from the server such that the
       * client may perform actions prior to finishing.
       */
  public void onBackPressedOnTaskRoot(android.os.IBinder activityToken, android.app.IRequestFinishCallback callback) throws android.os.RemoteException;
}
