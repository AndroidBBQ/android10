/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/** @hide */
public interface IPowerManager extends android.os.IInterface
{
  /** Default implementation for IPowerManager. */
  public static class Default implements android.os.IPowerManager
  {
    // WARNING: When methods are inserted or deleted, the transaction IDs in
    // frameworks/native/include/powermanager/IPowerManager.h must be updated to match the order in this file.
    //
    // When a method's argument list is changed, BnPowerManager's corresponding serialization code (if any) in
    // frameworks/native/services/powermanager/IPowerManager.cpp must be updated.

    @Override public void acquireWakeLock(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException
    {
    }
    @Override public void acquireWakeLockWithUid(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, int uidtoblame) throws android.os.RemoteException
    {
    }
    @Override public void releaseWakeLock(android.os.IBinder lock, int flags) throws android.os.RemoteException
    {
    }
    @Override public void updateWakeLockUids(android.os.IBinder lock, int[] uids) throws android.os.RemoteException
    {
    }
    @Override public void powerHint(int hintId, int data) throws android.os.RemoteException
    {
    }
    @Override public void updateWakeLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException
    {
    }
    @Override public boolean isWakeLockLevelSupported(int level) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void userActivity(long time, int event, int flags) throws android.os.RemoteException
    {
    }
    @Override public void wakeUp(long time, int reason, java.lang.String details, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void goToSleep(long time, int reason, int flags) throws android.os.RemoteException
    {
    }
    @Override public void nap(long time) throws android.os.RemoteException
    {
    }
    @Override public boolean isInteractive() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isPowerSaveMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.os.PowerSaveState getPowerSaveState(int serviceType) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setPowerSaveModeEnabled(boolean mode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setDynamicPowerSaveHint(boolean powerSaveHint, int disableThreshold) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAdaptivePowerSavePolicy(android.os.BatterySaverPolicyConfig config) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAdaptivePowerSaveEnabled(boolean enabled) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getPowerSaveModeTrigger() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isDeviceIdleMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isLightDeviceIdleMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void reboot(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException
    {
    }
    @Override public void rebootSafeMode(boolean confirm, boolean wait) throws android.os.RemoteException
    {
    }
    @Override public void shutdown(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException
    {
    }
    @Override public void crash(java.lang.String message) throws android.os.RemoteException
    {
    }
    @Override public int getLastShutdownReason() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getLastSleepReason() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setStayOnSetting(int val) throws android.os.RemoteException
    {
    }
    @Override public void boostScreenBrightness(long time) throws android.os.RemoteException
    {
    }
    // --- deprecated ---

    @Override public boolean isScreenBrightnessBoosted() throws android.os.RemoteException
    {
      return false;
    }
    // sets the attention light (used by phone app only)

    @Override public void setAttentionLight(boolean on, int color) throws android.os.RemoteException
    {
    }
    // controls whether PowerManager should doze after the screen turns off or not

    @Override public void setDozeAfterScreenOff(boolean on) throws android.os.RemoteException
    {
    }
    // Forces the system to suspend even if there are held wakelocks.

    @Override public boolean forceSuspend() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IPowerManager
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IPowerManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IPowerManager interface,
     * generating a proxy if needed.
     */
    public static android.os.IPowerManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IPowerManager))) {
        return ((android.os.IPowerManager)iin);
      }
      return new android.os.IPowerManager.Stub.Proxy(obj);
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
        case TRANSACTION_acquireWakeLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.WorkSource _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.acquireWakeLock(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acquireWakeLockWithUid:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          this.acquireWakeLockWithUid(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseWakeLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          this.releaseWakeLock(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateWakeLockUids:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int[] _arg1;
          _arg1 = data.createIntArray();
          this.updateWakeLockUids(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_powerHint:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.powerHint(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_updateWakeLockWorkSource:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.WorkSource _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.updateWakeLockWorkSource(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isWakeLockLevelSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isWakeLockLevelSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_userActivity:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.userActivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_wakeUp:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.wakeUp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_goToSleep:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.goToSleep(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_nap:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.nap(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isInteractive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInteractive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isPowerSaveMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isPowerSaveMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPowerSaveState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.PowerSaveState _result = this.getPowerSaveState(_arg0);
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
        case TRANSACTION_setPowerSaveModeEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setPowerSaveModeEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDynamicPowerSaveHint:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setDynamicPowerSaveHint(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAdaptivePowerSavePolicy:
        {
          data.enforceInterface(descriptor);
          android.os.BatterySaverPolicyConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.BatterySaverPolicyConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.setAdaptivePowerSavePolicy(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAdaptivePowerSaveEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setAdaptivePowerSaveEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPowerSaveModeTrigger:
        {
          data.enforceInterface(descriptor);
          int _result = this.getPowerSaveModeTrigger();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isDeviceIdleMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDeviceIdleMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isLightDeviceIdleMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isLightDeviceIdleMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_reboot:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.reboot(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_rebootSafeMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.rebootSafeMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.shutdown(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_crash:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.crash(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLastShutdownReason:
        {
          data.enforceInterface(descriptor);
          int _result = this.getLastShutdownReason();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getLastSleepReason:
        {
          data.enforceInterface(descriptor);
          int _result = this.getLastSleepReason();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setStayOnSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setStayOnSetting(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_boostScreenBrightness:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.boostScreenBrightness(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isScreenBrightnessBoosted:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isScreenBrightnessBoosted();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAttentionLight:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setAttentionLight(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDozeAfterScreenOff:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setDozeAfterScreenOff(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forceSuspend:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.forceSuspend();
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
    private static class Proxy implements android.os.IPowerManager
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
      // WARNING: When methods are inserted or deleted, the transaction IDs in
      // frameworks/native/include/powermanager/IPowerManager.h must be updated to match the order in this file.
      //
      // When a method's argument list is changed, BnPowerManager's corresponding serialization code (if any) in
      // frameworks/native/services/powermanager/IPowerManager.cpp must be updated.

      @Override public void acquireWakeLock(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          _data.writeInt(flags);
          _data.writeString(tag);
          _data.writeString(packageName);
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(historyTag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireWakeLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acquireWakeLock(lock, flags, tag, packageName, ws, historyTag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void acquireWakeLockWithUid(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, int uidtoblame) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          _data.writeInt(flags);
          _data.writeString(tag);
          _data.writeString(packageName);
          _data.writeInt(uidtoblame);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireWakeLockWithUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acquireWakeLockWithUid(lock, flags, tag, packageName, uidtoblame);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releaseWakeLock(android.os.IBinder lock, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseWakeLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseWakeLock(lock, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateWakeLockUids(android.os.IBinder lock, int[] uids) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          _data.writeIntArray(uids);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateWakeLockUids, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateWakeLockUids(lock, uids);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void powerHint(int hintId, int data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(hintId);
          _data.writeInt(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_powerHint, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().powerHint(hintId, data);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateWakeLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(lock);
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(historyTag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateWakeLockWorkSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateWakeLockWorkSource(lock, ws, historyTag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isWakeLockLevelSupported(int level) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(level);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isWakeLockLevelSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isWakeLockLevelSupported(level);
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
      @Override public void userActivity(long time, int event, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(time);
          _data.writeInt(event);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_userActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().userActivity(time, event, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void wakeUp(long time, int reason, java.lang.String details, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(time);
          _data.writeInt(reason);
          _data.writeString(details);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_wakeUp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().wakeUp(time, reason, details, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void goToSleep(long time, int reason, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(time);
          _data.writeInt(reason);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_goToSleep, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().goToSleep(time, reason, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void nap(long time) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(time);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().nap(time);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isInteractive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInteractive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInteractive();
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
      @Override public boolean isPowerSaveMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isPowerSaveMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isPowerSaveMode();
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
      @Override public android.os.PowerSaveState getPowerSaveState(int serviceType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.PowerSaveState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(serviceType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPowerSaveState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPowerSaveState(serviceType);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.PowerSaveState.CREATOR.createFromParcel(_reply);
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
      @Override public boolean setPowerSaveModeEnabled(boolean mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((mode)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPowerSaveModeEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setPowerSaveModeEnabled(mode);
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
      @Override public boolean setDynamicPowerSaveHint(boolean powerSaveHint, int disableThreshold) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((powerSaveHint)?(1):(0)));
          _data.writeInt(disableThreshold);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDynamicPowerSaveHint, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDynamicPowerSaveHint(powerSaveHint, disableThreshold);
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
      @Override public boolean setAdaptivePowerSavePolicy(android.os.BatterySaverPolicyConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAdaptivePowerSavePolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAdaptivePowerSavePolicy(config);
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
      @Override public boolean setAdaptivePowerSaveEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAdaptivePowerSaveEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAdaptivePowerSaveEnabled(enabled);
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
      @Override public int getPowerSaveModeTrigger() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPowerSaveModeTrigger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPowerSaveModeTrigger();
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
      @Override public boolean isDeviceIdleMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDeviceIdleMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDeviceIdleMode();
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
      @Override public boolean isLightDeviceIdleMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isLightDeviceIdleMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isLightDeviceIdleMode();
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
      @Override public void reboot(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((confirm)?(1):(0)));
          _data.writeString(reason);
          _data.writeInt(((wait)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reboot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reboot(confirm, reason, wait);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void rebootSafeMode(boolean confirm, boolean wait) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((confirm)?(1):(0)));
          _data.writeInt(((wait)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_rebootSafeMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rebootSafeMode(confirm, wait);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void shutdown(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((confirm)?(1):(0)));
          _data.writeString(reason);
          _data.writeInt(((wait)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown(confirm, reason, wait);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void crash(java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_crash, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().crash(message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getLastShutdownReason() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastShutdownReason, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastShutdownReason();
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
      @Override public int getLastSleepReason() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastSleepReason, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastSleepReason();
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
      @Override public void setStayOnSetting(int val) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(val);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStayOnSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStayOnSetting(val);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void boostScreenBrightness(long time) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(time);
          boolean _status = mRemote.transact(Stub.TRANSACTION_boostScreenBrightness, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().boostScreenBrightness(time);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // --- deprecated ---

      @Override public boolean isScreenBrightnessBoosted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isScreenBrightnessBoosted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isScreenBrightnessBoosted();
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
      // sets the attention light (used by phone app only)

      @Override public void setAttentionLight(boolean on, int color) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          _data.writeInt(color);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAttentionLight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAttentionLight(on, color);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // controls whether PowerManager should doze after the screen turns off or not

      @Override public void setDozeAfterScreenOff(boolean on) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((on)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDozeAfterScreenOff, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDozeAfterScreenOff(on);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Forces the system to suspend even if there are held wakelocks.

      @Override public boolean forceSuspend() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceSuspend, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().forceSuspend();
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
      public static android.os.IPowerManager sDefaultImpl;
    }
    static final int TRANSACTION_acquireWakeLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_acquireWakeLockWithUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_releaseWakeLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_updateWakeLockUids = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_powerHint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_updateWakeLockWorkSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isWakeLockLevelSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_userActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_wakeUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_goToSleep = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_nap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_isInteractive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_isPowerSaveMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getPowerSaveState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setPowerSaveModeEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setDynamicPowerSaveHint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setAdaptivePowerSavePolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setAdaptivePowerSaveEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getPowerSaveModeTrigger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_isDeviceIdleMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_isLightDeviceIdleMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_reboot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_rebootSafeMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_crash = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getLastShutdownReason = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getLastSleepReason = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_setStayOnSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_boostScreenBrightness = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_isScreenBrightnessBoosted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_setAttentionLight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setDozeAfterScreenOff = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_forceSuspend = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    public static boolean setDefaultImpl(android.os.IPowerManager impl) {
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
    public static android.os.IPowerManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // WARNING: When methods are inserted or deleted, the transaction IDs in
  // frameworks/native/include/powermanager/IPowerManager.h must be updated to match the order in this file.
  //
  // When a method's argument list is changed, BnPowerManager's corresponding serialization code (if any) in
  // frameworks/native/services/powermanager/IPowerManager.cpp must be updated.

  public void acquireWakeLock(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException;
  public void acquireWakeLockWithUid(android.os.IBinder lock, int flags, java.lang.String tag, java.lang.String packageName, int uidtoblame) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/IPowerManager.aidl:37:1:37:25")
  public void releaseWakeLock(android.os.IBinder lock, int flags) throws android.os.RemoteException;
  public void updateWakeLockUids(android.os.IBinder lock, int[] uids) throws android.os.RemoteException;
  public void powerHint(int hintId, int data) throws android.os.RemoteException;
  public void updateWakeLockWorkSource(android.os.IBinder lock, android.os.WorkSource ws, java.lang.String historyTag) throws android.os.RemoteException;
  public boolean isWakeLockLevelSupported(int level) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/IPowerManager.aidl:45:1:45:25")
  public void userActivity(long time, int event, int flags) throws android.os.RemoteException;
  public void wakeUp(long time, int reason, java.lang.String details, java.lang.String opPackageName) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/IPowerManager.aidl:48:1:48:25")
  public void goToSleep(long time, int reason, int flags) throws android.os.RemoteException;
  public void nap(long time) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/IPowerManager.aidl:51:1:51:25")
  public boolean isInteractive() throws android.os.RemoteException;
  public boolean isPowerSaveMode() throws android.os.RemoteException;
  public android.os.PowerSaveState getPowerSaveState(int serviceType) throws android.os.RemoteException;
  public boolean setPowerSaveModeEnabled(boolean mode) throws android.os.RemoteException;
  public boolean setDynamicPowerSaveHint(boolean powerSaveHint, int disableThreshold) throws android.os.RemoteException;
  public boolean setAdaptivePowerSavePolicy(android.os.BatterySaverPolicyConfig config) throws android.os.RemoteException;
  public boolean setAdaptivePowerSaveEnabled(boolean enabled) throws android.os.RemoteException;
  public int getPowerSaveModeTrigger() throws android.os.RemoteException;
  public boolean isDeviceIdleMode() throws android.os.RemoteException;
  public boolean isLightDeviceIdleMode() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/os/IPowerManager.aidl:63:1:63:25")
  public void reboot(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException;
  public void rebootSafeMode(boolean confirm, boolean wait) throws android.os.RemoteException;
  public void shutdown(boolean confirm, java.lang.String reason, boolean wait) throws android.os.RemoteException;
  public void crash(java.lang.String message) throws android.os.RemoteException;
  public int getLastShutdownReason() throws android.os.RemoteException;
  public int getLastSleepReason() throws android.os.RemoteException;
  public void setStayOnSetting(int val) throws android.os.RemoteException;
  public void boostScreenBrightness(long time) throws android.os.RemoteException;
  // --- deprecated ---

  public boolean isScreenBrightnessBoosted() throws android.os.RemoteException;
  // sets the attention light (used by phone app only)

  public void setAttentionLight(boolean on, int color) throws android.os.RemoteException;
  // controls whether PowerManager should doze after the screen turns off or not

  public void setDozeAfterScreenOff(boolean on) throws android.os.RemoteException;
  // Forces the system to suspend even if there are held wakelocks.

  public boolean forceSuspend() throws android.os.RemoteException;
}
