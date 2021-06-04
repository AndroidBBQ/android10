/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * System private API for communicating with the application.  This is given to
 * the activity manager by an application  when it starts up, for the activity
 * manager to tell the application about things it needs to do.
 *
 * {@hide}
 */
public interface IApplicationThread extends android.os.IInterface
{
  /** Default implementation for IApplicationThread. */
  public static class Default implements android.app.IApplicationThread
  {
    @Override public void scheduleReceiver(android.content.Intent intent, android.content.pm.ActivityInfo info, android.content.res.CompatibilityInfo compatInfo, int resultCode, java.lang.String data, android.os.Bundle extras, boolean sync, int sendingUser, int processState) throws android.os.RemoteException
    {
    }
    @Override public void scheduleCreateService(android.os.IBinder token, android.content.pm.ServiceInfo info, android.content.res.CompatibilityInfo compatInfo, int processState) throws android.os.RemoteException
    {
    }
    @Override public void scheduleStopService(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void bindApplication(java.lang.String packageName, android.content.pm.ApplicationInfo info, java.util.List<android.content.pm.ProviderInfo> providers, android.content.ComponentName testName, android.app.ProfilerInfo profilerInfo, android.os.Bundle testArguments, android.app.IInstrumentationWatcher testWatcher, android.app.IUiAutomationConnection uiAutomationConnection, int debugMode, boolean enableBinderTracking, boolean trackAllocation, boolean restrictedBackupMode, boolean persistent, android.content.res.Configuration config, android.content.res.CompatibilityInfo compatInfo, java.util.Map services, android.os.Bundle coreSettings, java.lang.String buildSerial, android.content.AutofillOptions autofillOptions, android.content.ContentCaptureOptions contentCaptureOptions) throws android.os.RemoteException
    {
    }
    @Override public void runIsolatedEntryPoint(java.lang.String entryPoint, java.lang.String[] entryPointArgs) throws android.os.RemoteException
    {
    }
    @Override public void scheduleExit() throws android.os.RemoteException
    {
    }
    @Override public void scheduleServiceArgs(android.os.IBinder token, android.content.pm.ParceledListSlice args) throws android.os.RemoteException
    {
    }
    @Override public void updateTimeZone() throws android.os.RemoteException
    {
    }
    @Override public void processInBackground() throws android.os.RemoteException
    {
    }
    @Override public void scheduleBindService(android.os.IBinder token, android.content.Intent intent, boolean rebind, int processState) throws android.os.RemoteException
    {
    }
    @Override public void scheduleUnbindService(android.os.IBinder token, android.content.Intent intent) throws android.os.RemoteException
    {
    }
    @Override public void dumpService(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void scheduleRegisteredReceiver(android.content.IIntentReceiver receiver, android.content.Intent intent, int resultCode, java.lang.String data, android.os.Bundle extras, boolean ordered, boolean sticky, int sendingUser, int processState) throws android.os.RemoteException
    {
    }
    @Override public void scheduleLowMemory() throws android.os.RemoteException
    {
    }
    @Override public void scheduleSleeping(android.os.IBinder token, boolean sleeping) throws android.os.RemoteException
    {
    }
    @Override public void profilerControl(boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException
    {
    }
    @Override public void setSchedulingGroup(int group) throws android.os.RemoteException
    {
    }
    @Override public void scheduleCreateBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int backupMode, int userId) throws android.os.RemoteException
    {
    }
    @Override public void scheduleDestroyBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int userId) throws android.os.RemoteException
    {
    }
    @Override public void scheduleOnNewActivityOptions(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void scheduleSuicide() throws android.os.RemoteException
    {
    }
    @Override public void dispatchPackageBroadcast(int cmd, java.lang.String[] packages) throws android.os.RemoteException
    {
    }
    @Override public void scheduleCrash(java.lang.String msg) throws android.os.RemoteException
    {
    }
    @Override public void dumpHeap(boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException
    {
    }
    @Override public void dumpActivity(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String prefix, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void clearDnsCache() throws android.os.RemoteException
    {
    }
    @Override public void updateHttpProxy() throws android.os.RemoteException
    {
    }
    @Override public void setCoreSettings(android.os.Bundle coreSettings) throws android.os.RemoteException
    {
    }
    @Override public void updatePackageCompatibilityInfo(java.lang.String pkg, android.content.res.CompatibilityInfo info) throws android.os.RemoteException
    {
    }
    @Override public void scheduleTrimMemory(int level) throws android.os.RemoteException
    {
    }
    @Override public void dumpMemInfo(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean checkin, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void dumpMemInfoProto(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void dumpGfxInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void dumpProvider(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void dumpDbInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException
    {
    }
    @Override public void unstableProviderDied(android.os.IBinder provider) throws android.os.RemoteException
    {
    }
    @Override public void requestAssistContextExtras(android.os.IBinder activityToken, android.os.IBinder requestToken, int requestType, int sessionId, int flags) throws android.os.RemoteException
    {
    }
    @Override public void scheduleTranslucentConversionComplete(android.os.IBinder token, boolean timeout) throws android.os.RemoteException
    {
    }
    @Override public void setProcessState(int state) throws android.os.RemoteException
    {
    }
    @Override public void scheduleInstallProvider(android.content.pm.ProviderInfo provider) throws android.os.RemoteException
    {
    }
    @Override public void updateTimePrefs(int timeFormatPreference) throws android.os.RemoteException
    {
    }
    @Override public void scheduleEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void notifyCleartextNetwork(byte[] firstPacket) throws android.os.RemoteException
    {
    }
    @Override public void startBinderTracking() throws android.os.RemoteException
    {
    }
    @Override public void stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
    }
    @Override public void scheduleLocalVoiceInteractionStarted(android.os.IBinder token, com.android.internal.app.IVoiceInteractor voiceInteractor) throws android.os.RemoteException
    {
    }
    @Override public void handleTrustStorageUpdate() throws android.os.RemoteException
    {
    }
    @Override public void attachAgent(java.lang.String path) throws android.os.RemoteException
    {
    }
    @Override public void scheduleApplicationInfoChanged(android.content.pm.ApplicationInfo ai) throws android.os.RemoteException
    {
    }
    @Override public void setNetworkBlockSeq(long procStateSeq) throws android.os.RemoteException
    {
    }
    @Override public void scheduleTransaction(android.app.servertransaction.ClientTransaction transaction) throws android.os.RemoteException
    {
    }
    @Override public void requestDirectActions(android.os.IBinder activityToken, com.android.internal.app.IVoiceInteractor intractor, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void performDirectAction(android.os.IBinder activityToken, java.lang.String actionId, android.os.Bundle arguments, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IApplicationThread
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IApplicationThread";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IApplicationThread interface,
     * generating a proxy if needed.
     */
    public static android.app.IApplicationThread asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IApplicationThread))) {
        return ((android.app.IApplicationThread)iin);
      }
      return new android.app.IApplicationThread.Stub.Proxy(obj);
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
        case TRANSACTION_scheduleReceiver:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.pm.ActivityInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ActivityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.content.res.CompatibilityInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          int _arg7;
          _arg7 = data.readInt();
          int _arg8;
          _arg8 = data.readInt();
          this.scheduleReceiver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_scheduleCreateService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.pm.ServiceInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ServiceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.content.res.CompatibilityInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.scheduleCreateService(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_scheduleStopService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.scheduleStopService(_arg0);
          return true;
        }
        case TRANSACTION_bindApplication:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.pm.ApplicationInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.util.List<android.content.pm.ProviderInfo> _arg2;
          _arg2 = data.createTypedArrayList(android.content.pm.ProviderInfo.CREATOR);
          android.content.ComponentName _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.app.ProfilerInfo _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.app.ProfilerInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          android.app.IInstrumentationWatcher _arg6;
          _arg6 = android.app.IInstrumentationWatcher.Stub.asInterface(data.readStrongBinder());
          android.app.IUiAutomationConnection _arg7;
          _arg7 = android.app.IUiAutomationConnection.Stub.asInterface(data.readStrongBinder());
          int _arg8;
          _arg8 = data.readInt();
          boolean _arg9;
          _arg9 = (0!=data.readInt());
          boolean _arg10;
          _arg10 = (0!=data.readInt());
          boolean _arg11;
          _arg11 = (0!=data.readInt());
          boolean _arg12;
          _arg12 = (0!=data.readInt());
          android.content.res.Configuration _arg13;
          if ((0!=data.readInt())) {
            _arg13 = android.content.res.Configuration.CREATOR.createFromParcel(data);
          }
          else {
            _arg13 = null;
          }
          android.content.res.CompatibilityInfo _arg14;
          if ((0!=data.readInt())) {
            _arg14 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg14 = null;
          }
          java.util.Map _arg15;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg15 = data.readHashMap(cl);
          android.os.Bundle _arg16;
          if ((0!=data.readInt())) {
            _arg16 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg16 = null;
          }
          java.lang.String _arg17;
          _arg17 = data.readString();
          android.content.AutofillOptions _arg18;
          if ((0!=data.readInt())) {
            _arg18 = android.content.AutofillOptions.CREATOR.createFromParcel(data);
          }
          else {
            _arg18 = null;
          }
          android.content.ContentCaptureOptions _arg19;
          if ((0!=data.readInt())) {
            _arg19 = android.content.ContentCaptureOptions.CREATOR.createFromParcel(data);
          }
          else {
            _arg19 = null;
          }
          this.bindApplication(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17, _arg18, _arg19);
          return true;
        }
        case TRANSACTION_runIsolatedEntryPoint:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.runIsolatedEntryPoint(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_scheduleExit:
        {
          data.enforceInterface(descriptor);
          this.scheduleExit();
          return true;
        }
        case TRANSACTION_scheduleServiceArgs:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.scheduleServiceArgs(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_updateTimeZone:
        {
          data.enforceInterface(descriptor);
          this.updateTimeZone();
          return true;
        }
        case TRANSACTION_processInBackground:
        {
          data.enforceInterface(descriptor);
          this.processInBackground();
          return true;
        }
        case TRANSACTION_scheduleBindService:
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
          int _arg3;
          _arg3 = data.readInt();
          this.scheduleBindService(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_scheduleUnbindService:
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
          this.scheduleUnbindService(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dumpService:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          this.dumpService(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_scheduleRegisteredReceiver:
        {
          data.enforceInterface(descriptor);
          android.content.IIntentReceiver _arg0;
          _arg0 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          int _arg7;
          _arg7 = data.readInt();
          int _arg8;
          _arg8 = data.readInt();
          this.scheduleRegisteredReceiver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_scheduleLowMemory:
        {
          data.enforceInterface(descriptor);
          this.scheduleLowMemory();
          return true;
        }
        case TRANSACTION_scheduleSleeping:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.scheduleSleeping(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_profilerControl:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.app.ProfilerInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.ProfilerInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.profilerControl(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setSchedulingGroup:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setSchedulingGroup(_arg0);
          return true;
        }
        case TRANSACTION_scheduleCreateBackupAgent:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ApplicationInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.res.CompatibilityInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.scheduleCreateBackupAgent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_scheduleDestroyBackupAgent:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ApplicationInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.res.CompatibilityInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.scheduleDestroyBackupAgent(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_scheduleOnNewActivityOptions:
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
          this.scheduleOnNewActivityOptions(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_scheduleSuicide:
        {
          data.enforceInterface(descriptor);
          this.scheduleSuicide();
          return true;
        }
        case TRANSACTION_dispatchPackageBroadcast:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.dispatchPackageBroadcast(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_scheduleCrash:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.scheduleCrash(_arg0);
          return true;
        }
        case TRANSACTION_dumpHeap:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.ParcelFileDescriptor _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.os.RemoteCallback _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.dumpHeap(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_dumpActivity:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          this.dumpActivity(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_clearDnsCache:
        {
          data.enforceInterface(descriptor);
          this.clearDnsCache();
          return true;
        }
        case TRANSACTION_updateHttpProxy:
        {
          data.enforceInterface(descriptor);
          this.updateHttpProxy();
          return true;
        }
        case TRANSACTION_setCoreSettings:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setCoreSettings(_arg0);
          return true;
        }
        case TRANSACTION_updatePackageCompatibilityInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.content.res.CompatibilityInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.res.CompatibilityInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updatePackageCompatibilityInfo(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_scheduleTrimMemory:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.scheduleTrimMemory(_arg0);
          return true;
        }
        case TRANSACTION_dumpMemInfo:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Debug.MemoryInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Debug.MemoryInfo.CREATOR.createFromParcel(data);
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
          java.lang.String[] _arg7;
          _arg7 = data.createStringArray();
          this.dumpMemInfo(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_dumpMemInfoProto:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Debug.MemoryInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Debug.MemoryInfo.CREATOR.createFromParcel(data);
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
          java.lang.String[] _arg6;
          _arg6 = data.createStringArray();
          this.dumpMemInfoProto(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_dumpGfxInfo:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.dumpGfxInfo(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dumpProvider:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          this.dumpProvider(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_dumpDbInfo:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.dumpDbInfo(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unstableProviderDied:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.unstableProviderDied(_arg0);
          return true;
        }
        case TRANSACTION_requestAssistContextExtras:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.requestAssistContextExtras(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_scheduleTranslucentConversionComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.scheduleTranslucentConversionComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setProcessState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setProcessState(_arg0);
          return true;
        }
        case TRANSACTION_scheduleInstallProvider:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ProviderInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ProviderInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.scheduleInstallProvider(_arg0);
          return true;
        }
        case TRANSACTION_updateTimePrefs:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.updateTimePrefs(_arg0);
          return true;
        }
        case TRANSACTION_scheduleEnterAnimationComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.scheduleEnterAnimationComplete(_arg0);
          return true;
        }
        case TRANSACTION_notifyCleartextNetwork:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.notifyCleartextNetwork(_arg0);
          return true;
        }
        case TRANSACTION_startBinderTracking:
        {
          data.enforceInterface(descriptor);
          this.startBinderTracking();
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
          this.stopBinderTrackingAndDump(_arg0);
          return true;
        }
        case TRANSACTION_scheduleLocalVoiceInteractionStarted:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          com.android.internal.app.IVoiceInteractor _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractor.Stub.asInterface(data.readStrongBinder());
          this.scheduleLocalVoiceInteractionStarted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_handleTrustStorageUpdate:
        {
          data.enforceInterface(descriptor);
          this.handleTrustStorageUpdate();
          return true;
        }
        case TRANSACTION_attachAgent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.attachAgent(_arg0);
          return true;
        }
        case TRANSACTION_scheduleApplicationInfoChanged:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ApplicationInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ApplicationInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.scheduleApplicationInfoChanged(_arg0);
          return true;
        }
        case TRANSACTION_setNetworkBlockSeq:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setNetworkBlockSeq(_arg0);
          return true;
        }
        case TRANSACTION_scheduleTransaction:
        {
          data.enforceInterface(descriptor);
          android.app.servertransaction.ClientTransaction _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.servertransaction.ClientTransaction.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.scheduleTransaction(_arg0);
          return true;
        }
        case TRANSACTION_requestDirectActions:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          com.android.internal.app.IVoiceInteractor _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractor.Stub.asInterface(data.readStrongBinder());
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.requestDirectActions(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_performDirectAction:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.performDirectAction(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IApplicationThread
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
      @Override public void scheduleReceiver(android.content.Intent intent, android.content.pm.ActivityInfo info, android.content.res.CompatibilityInfo compatInfo, int resultCode, java.lang.String data, android.os.Bundle extras, boolean sync, int sendingUser, int processState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((compatInfo!=null)) {
            _data.writeInt(1);
            compatInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(resultCode);
          _data.writeString(data);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((sync)?(1):(0)));
          _data.writeInt(sendingUser);
          _data.writeInt(processState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleReceiver, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleReceiver(intent, info, compatInfo, resultCode, data, extras, sync, sendingUser, processState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleCreateService(android.os.IBinder token, android.content.pm.ServiceInfo info, android.content.res.CompatibilityInfo compatInfo, int processState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((compatInfo!=null)) {
            _data.writeInt(1);
            compatInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(processState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleCreateService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleCreateService(token, info, compatInfo, processState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleStopService(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleStopService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleStopService(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void bindApplication(java.lang.String packageName, android.content.pm.ApplicationInfo info, java.util.List<android.content.pm.ProviderInfo> providers, android.content.ComponentName testName, android.app.ProfilerInfo profilerInfo, android.os.Bundle testArguments, android.app.IInstrumentationWatcher testWatcher, android.app.IUiAutomationConnection uiAutomationConnection, int debugMode, boolean enableBinderTracking, boolean trackAllocation, boolean restrictedBackupMode, boolean persistent, android.content.res.Configuration config, android.content.res.CompatibilityInfo compatInfo, java.util.Map services, android.os.Bundle coreSettings, java.lang.String buildSerial, android.content.AutofillOptions autofillOptions, android.content.ContentCaptureOptions contentCaptureOptions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedList(providers);
          if ((testName!=null)) {
            _data.writeInt(1);
            testName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((profilerInfo!=null)) {
            _data.writeInt(1);
            profilerInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((testArguments!=null)) {
            _data.writeInt(1);
            testArguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((testWatcher!=null))?(testWatcher.asBinder()):(null)));
          _data.writeStrongBinder((((uiAutomationConnection!=null))?(uiAutomationConnection.asBinder()):(null)));
          _data.writeInt(debugMode);
          _data.writeInt(((enableBinderTracking)?(1):(0)));
          _data.writeInt(((trackAllocation)?(1):(0)));
          _data.writeInt(((restrictedBackupMode)?(1):(0)));
          _data.writeInt(((persistent)?(1):(0)));
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((compatInfo!=null)) {
            _data.writeInt(1);
            compatInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeMap(services);
          if ((coreSettings!=null)) {
            _data.writeInt(1);
            coreSettings.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(buildSerial);
          if ((autofillOptions!=null)) {
            _data.writeInt(1);
            autofillOptions.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((contentCaptureOptions!=null)) {
            _data.writeInt(1);
            contentCaptureOptions.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindApplication, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().bindApplication(packageName, info, providers, testName, profilerInfo, testArguments, testWatcher, uiAutomationConnection, debugMode, enableBinderTracking, trackAllocation, restrictedBackupMode, persistent, config, compatInfo, services, coreSettings, buildSerial, autofillOptions, contentCaptureOptions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void runIsolatedEntryPoint(java.lang.String entryPoint, java.lang.String[] entryPointArgs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(entryPoint);
          _data.writeStringArray(entryPointArgs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_runIsolatedEntryPoint, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().runIsolatedEntryPoint(entryPoint, entryPointArgs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleExit() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleExit, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleExit();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleServiceArgs(android.os.IBinder token, android.content.pm.ParceledListSlice args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleServiceArgs, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleServiceArgs(token, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateTimeZone() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateTimeZone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateTimeZone();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void processInBackground() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_processInBackground, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().processInBackground();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleBindService(android.os.IBinder token, android.content.Intent intent, boolean rebind, int processState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
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
          _data.writeInt(((rebind)?(1):(0)));
          _data.writeInt(processState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleBindService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleBindService(token, intent, rebind, processState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleUnbindService(android.os.IBinder token, android.content.Intent intent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleUnbindService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleUnbindService(token, intent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpService(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(servicetoken);
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpService(fd, servicetoken, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleRegisteredReceiver(android.content.IIntentReceiver receiver, android.content.Intent intent, int resultCode, java.lang.String data, android.os.Bundle extras, boolean ordered, boolean sticky, int sendingUser, int processState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(resultCode);
          _data.writeString(data);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((ordered)?(1):(0)));
          _data.writeInt(((sticky)?(1):(0)));
          _data.writeInt(sendingUser);
          _data.writeInt(processState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleRegisteredReceiver, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleRegisteredReceiver(receiver, intent, resultCode, data, extras, ordered, sticky, sendingUser, processState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleLowMemory() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleLowMemory, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleLowMemory();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleSleeping(android.os.IBinder token, boolean sleeping) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((sleeping)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleSleeping, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleSleeping(token, sleeping);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void profilerControl(boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((start)?(1):(0)));
          if ((profilerInfo!=null)) {
            _data.writeInt(1);
            profilerInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(profileType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_profilerControl, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().profilerControl(start, profilerInfo, profileType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setSchedulingGroup(int group) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(group);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSchedulingGroup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSchedulingGroup(group);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleCreateBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int backupMode, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((app!=null)) {
            _data.writeInt(1);
            app.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((compatInfo!=null)) {
            _data.writeInt(1);
            compatInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(backupMode);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleCreateBackupAgent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleCreateBackupAgent(app, compatInfo, backupMode, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleDestroyBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((app!=null)) {
            _data.writeInt(1);
            app.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((compatInfo!=null)) {
            _data.writeInt(1);
            compatInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleDestroyBackupAgent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleDestroyBackupAgent(app, compatInfo, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleOnNewActivityOptions(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleOnNewActivityOptions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleOnNewActivityOptions(token, options);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleSuicide() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleSuicide, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleSuicide();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchPackageBroadcast(int cmd, java.lang.String[] packages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cmd);
          _data.writeStringArray(packages);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchPackageBroadcast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchPackageBroadcast(cmd, packages);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleCrash(java.lang.String msg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(msg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleCrash, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleCrash(msg);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpHeap(boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpHeap, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpHeap(managed, mallocInfo, runGc, path, fd, finishCallback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpActivity(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String prefix, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(servicetoken);
          _data.writeString(prefix);
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpActivity, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpActivity(fd, servicetoken, prefix, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clearDnsCache() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearDnsCache, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearDnsCache();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateHttpProxy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateHttpProxy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateHttpProxy();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setCoreSettings(android.os.Bundle coreSettings) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((coreSettings!=null)) {
            _data.writeInt(1);
            coreSettings.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCoreSettings, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCoreSettings(coreSettings);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updatePackageCompatibilityInfo(java.lang.String pkg, android.content.res.CompatibilityInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePackageCompatibilityInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updatePackageCompatibilityInfo(pkg, info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleTrimMemory(int level) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(level);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleTrimMemory, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleTrimMemory(level);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpMemInfo(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean checkin, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((mem!=null)) {
            _data.writeInt(1);
            mem.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((checkin)?(1):(0)));
          _data.writeInt(((dumpInfo)?(1):(0)));
          _data.writeInt(((dumpDalvik)?(1):(0)));
          _data.writeInt(((dumpSummaryOnly)?(1):(0)));
          _data.writeInt(((dumpUnreachable)?(1):(0)));
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpMemInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpMemInfo(fd, mem, checkin, dumpInfo, dumpDalvik, dumpSummaryOnly, dumpUnreachable, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpMemInfoProto(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((mem!=null)) {
            _data.writeInt(1);
            mem.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((dumpInfo)?(1):(0)));
          _data.writeInt(((dumpDalvik)?(1):(0)));
          _data.writeInt(((dumpSummaryOnly)?(1):(0)));
          _data.writeInt(((dumpUnreachable)?(1):(0)));
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpMemInfoProto, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpMemInfoProto(fd, mem, dumpInfo, dumpDalvik, dumpSummaryOnly, dumpUnreachable, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpGfxInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpGfxInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpGfxInfo(fd, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpProvider(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(servicetoken);
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpProvider(fd, servicetoken, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dumpDbInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStringArray(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpDbInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dumpDbInfo(fd, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unstableProviderDied(android.os.IBinder provider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(provider);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unstableProviderDied, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unstableProviderDied(provider);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestAssistContextExtras(android.os.IBinder activityToken, android.os.IBinder requestToken, int requestType, int sessionId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          _data.writeStrongBinder(requestToken);
          _data.writeInt(requestType);
          _data.writeInt(sessionId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAssistContextExtras, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestAssistContextExtras(activityToken, requestToken, requestType, sessionId, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleTranslucentConversionComplete(android.os.IBinder token, boolean timeout) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((timeout)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleTranslucentConversionComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleTranslucentConversionComplete(token, timeout);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setProcessState(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProcessState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProcessState(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleInstallProvider(android.content.pm.ProviderInfo provider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((provider!=null)) {
            _data.writeInt(1);
            provider.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleInstallProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleInstallProvider(provider);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateTimePrefs(int timeFormatPreference) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeFormatPreference);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateTimePrefs, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateTimePrefs(timeFormatPreference);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleEnterAnimationComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleEnterAnimationComplete(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyCleartextNetwork(byte[] firstPacket) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(firstPacket);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCleartextNetwork, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCleartextNetwork(firstPacket);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startBinderTracking() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startBinderTracking, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startBinderTracking();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopBinderTrackingAndDump, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopBinderTrackingAndDump(fd);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleLocalVoiceInteractionStarted(android.os.IBinder token, com.android.internal.app.IVoiceInteractor voiceInteractor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((voiceInteractor!=null))?(voiceInteractor.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleLocalVoiceInteractionStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleLocalVoiceInteractionStarted(token, voiceInteractor);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handleTrustStorageUpdate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleTrustStorageUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleTrustStorageUpdate();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void attachAgent(java.lang.String path) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(path);
          boolean _status = mRemote.transact(Stub.TRANSACTION_attachAgent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attachAgent(path);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleApplicationInfoChanged(android.content.pm.ApplicationInfo ai) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ai!=null)) {
            _data.writeInt(1);
            ai.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleApplicationInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleApplicationInfoChanged(ai);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setNetworkBlockSeq(long procStateSeq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(procStateSeq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNetworkBlockSeq, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNetworkBlockSeq(procStateSeq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void scheduleTransaction(android.app.servertransaction.ClientTransaction transaction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((transaction!=null)) {
            _data.writeInt(1);
            transaction.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_scheduleTransaction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scheduleTransaction(transaction);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestDirectActions(android.os.IBinder activityToken, com.android.internal.app.IVoiceInteractor intractor, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          _data.writeStrongBinder((((intractor!=null))?(intractor.asBinder()):(null)));
          if ((cancellationCallback!=null)) {
            _data.writeInt(1);
            cancellationCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestDirectActions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestDirectActions(activityToken, intractor, cancellationCallback, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void performDirectAction(android.os.IBinder activityToken, java.lang.String actionId, android.os.Bundle arguments, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          _data.writeString(actionId);
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((cancellationCallback!=null)) {
            _data.writeInt(1);
            cancellationCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((resultCallback!=null)) {
            _data.writeInt(1);
            resultCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_performDirectAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performDirectAction(activityToken, actionId, arguments, cancellationCallback, resultCallback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IApplicationThread sDefaultImpl;
    }
    static final int TRANSACTION_scheduleReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_scheduleCreateService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_scheduleStopService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_bindApplication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_runIsolatedEntryPoint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_scheduleExit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_scheduleServiceArgs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_updateTimeZone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_processInBackground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_scheduleBindService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_scheduleUnbindService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_dumpService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_scheduleRegisteredReceiver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_scheduleLowMemory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_scheduleSleeping = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_profilerControl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setSchedulingGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_scheduleCreateBackupAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_scheduleDestroyBackupAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_scheduleOnNewActivityOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_scheduleSuicide = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_dispatchPackageBroadcast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_scheduleCrash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_dumpHeap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_dumpActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_clearDnsCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_updateHttpProxy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_setCoreSettings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_updatePackageCompatibilityInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_scheduleTrimMemory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_dumpMemInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_dumpMemInfoProto = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_dumpGfxInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_dumpProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_dumpDbInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_unstableProviderDied = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_requestAssistContextExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_scheduleTranslucentConversionComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_setProcessState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_scheduleInstallProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_updateTimePrefs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_scheduleEnterAnimationComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_notifyCleartextNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_startBinderTracking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_stopBinderTrackingAndDump = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_scheduleLocalVoiceInteractionStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_handleTrustStorageUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_attachAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_scheduleApplicationInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_setNetworkBlockSeq = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_scheduleTransaction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_requestDirectActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_performDirectAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    public static boolean setDefaultImpl(android.app.IApplicationThread impl) {
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
    public static android.app.IApplicationThread getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void scheduleReceiver(android.content.Intent intent, android.content.pm.ActivityInfo info, android.content.res.CompatibilityInfo compatInfo, int resultCode, java.lang.String data, android.os.Bundle extras, boolean sync, int sendingUser, int processState) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IApplicationThread.aidl:63:1:63:25")
  public void scheduleCreateService(android.os.IBinder token, android.content.pm.ServiceInfo info, android.content.res.CompatibilityInfo compatInfo, int processState) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IApplicationThread.aidl:66:1:66:25")
  public void scheduleStopService(android.os.IBinder token) throws android.os.RemoteException;
  public void bindApplication(java.lang.String packageName, android.content.pm.ApplicationInfo info, java.util.List<android.content.pm.ProviderInfo> providers, android.content.ComponentName testName, android.app.ProfilerInfo profilerInfo, android.os.Bundle testArguments, android.app.IInstrumentationWatcher testWatcher, android.app.IUiAutomationConnection uiAutomationConnection, int debugMode, boolean enableBinderTracking, boolean trackAllocation, boolean restrictedBackupMode, boolean persistent, android.content.res.Configuration config, android.content.res.CompatibilityInfo compatInfo, java.util.Map services, android.os.Bundle coreSettings, java.lang.String buildSerial, android.content.AutofillOptions autofillOptions, android.content.ContentCaptureOptions contentCaptureOptions) throws android.os.RemoteException;
  public void runIsolatedEntryPoint(java.lang.String entryPoint, java.lang.String[] entryPointArgs) throws android.os.RemoteException;
  public void scheduleExit() throws android.os.RemoteException;
  public void scheduleServiceArgs(android.os.IBinder token, android.content.pm.ParceledListSlice args) throws android.os.RemoteException;
  public void updateTimeZone() throws android.os.RemoteException;
  public void processInBackground() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IApplicationThread.aidl:82:1:82:25")
  public void scheduleBindService(android.os.IBinder token, android.content.Intent intent, boolean rebind, int processState) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IApplicationThread.aidl:85:1:85:25")
  public void scheduleUnbindService(android.os.IBinder token, android.content.Intent intent) throws android.os.RemoteException;
  public void dumpService(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException;
  public void scheduleRegisteredReceiver(android.content.IIntentReceiver receiver, android.content.Intent intent, int resultCode, java.lang.String data, android.os.Bundle extras, boolean ordered, boolean sticky, int sendingUser, int processState) throws android.os.RemoteException;
  public void scheduleLowMemory() throws android.os.RemoteException;
  public void scheduleSleeping(android.os.IBinder token, boolean sleeping) throws android.os.RemoteException;
  public void profilerControl(boolean start, android.app.ProfilerInfo profilerInfo, int profileType) throws android.os.RemoteException;
  public void setSchedulingGroup(int group) throws android.os.RemoteException;
  public void scheduleCreateBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int backupMode, int userId) throws android.os.RemoteException;
  public void scheduleDestroyBackupAgent(android.content.pm.ApplicationInfo app, android.content.res.CompatibilityInfo compatInfo, int userId) throws android.os.RemoteException;
  public void scheduleOnNewActivityOptions(android.os.IBinder token, android.os.Bundle options) throws android.os.RemoteException;
  public void scheduleSuicide() throws android.os.RemoteException;
  public void dispatchPackageBroadcast(int cmd, java.lang.String[] packages) throws android.os.RemoteException;
  public void scheduleCrash(java.lang.String msg) throws android.os.RemoteException;
  public void dumpHeap(boolean managed, boolean mallocInfo, boolean runGc, java.lang.String path, android.os.ParcelFileDescriptor fd, android.os.RemoteCallback finishCallback) throws android.os.RemoteException;
  public void dumpActivity(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String prefix, java.lang.String[] args) throws android.os.RemoteException;
  public void clearDnsCache() throws android.os.RemoteException;
  public void updateHttpProxy() throws android.os.RemoteException;
  public void setCoreSettings(android.os.Bundle coreSettings) throws android.os.RemoteException;
  public void updatePackageCompatibilityInfo(java.lang.String pkg, android.content.res.CompatibilityInfo info) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IApplicationThread.aidl:113:1:113:25")
  public void scheduleTrimMemory(int level) throws android.os.RemoteException;
  public void dumpMemInfo(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean checkin, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException;
  public void dumpMemInfoProto(android.os.ParcelFileDescriptor fd, android.os.Debug.MemoryInfo mem, boolean dumpInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable, java.lang.String[] args) throws android.os.RemoteException;
  public void dumpGfxInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException;
  public void dumpProvider(android.os.ParcelFileDescriptor fd, android.os.IBinder servicetoken, java.lang.String[] args) throws android.os.RemoteException;
  public void dumpDbInfo(android.os.ParcelFileDescriptor fd, java.lang.String[] args) throws android.os.RemoteException;
  public void unstableProviderDied(android.os.IBinder provider) throws android.os.RemoteException;
  public void requestAssistContextExtras(android.os.IBinder activityToken, android.os.IBinder requestToken, int requestType, int sessionId, int flags) throws android.os.RemoteException;
  public void scheduleTranslucentConversionComplete(android.os.IBinder token, boolean timeout) throws android.os.RemoteException;
  public void setProcessState(int state) throws android.os.RemoteException;
  public void scheduleInstallProvider(android.content.pm.ProviderInfo provider) throws android.os.RemoteException;
  public void updateTimePrefs(int timeFormatPreference) throws android.os.RemoteException;
  public void scheduleEnterAnimationComplete(android.os.IBinder token) throws android.os.RemoteException;
  public void notifyCleartextNetwork(byte[] firstPacket) throws android.os.RemoteException;
  public void startBinderTracking() throws android.os.RemoteException;
  public void stopBinderTrackingAndDump(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  public void scheduleLocalVoiceInteractionStarted(android.os.IBinder token, com.android.internal.app.IVoiceInteractor voiceInteractor) throws android.os.RemoteException;
  public void handleTrustStorageUpdate() throws android.os.RemoteException;
  public void attachAgent(java.lang.String path) throws android.os.RemoteException;
  public void scheduleApplicationInfoChanged(android.content.pm.ApplicationInfo ai) throws android.os.RemoteException;
  public void setNetworkBlockSeq(long procStateSeq) throws android.os.RemoteException;
  public void scheduleTransaction(android.app.servertransaction.ClientTransaction transaction) throws android.os.RemoteException;
  public void requestDirectActions(android.os.IBinder activityToken, com.android.internal.app.IVoiceInteractor intractor, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void performDirectAction(android.os.IBinder activityToken, java.lang.String actionId, android.os.Bundle arguments, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException;
}
