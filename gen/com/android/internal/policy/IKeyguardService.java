/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.policy;
public interface IKeyguardService extends android.os.IInterface
{
  /** Default implementation for IKeyguardService. */
  public static class Default implements com.android.internal.policy.IKeyguardService
  {
    /**
         * Sets the Keyguard as occluded when a window dismisses the Keyguard with flag
         * FLAG_SHOW_ON_LOCK_SCREEN.
         *
         * @param isOccluded Whether the Keyguard is occluded by another window.
         * @param animate Whether to play an animation for the state change.
         */
    @Override public void setOccluded(boolean isOccluded, boolean animate) throws android.os.RemoteException
    {
    }
    @Override public void addStateMonitorCallback(com.android.internal.policy.IKeyguardStateCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void verifyUnlock(com.android.internal.policy.IKeyguardExitCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void dismiss(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    @Override public void onDreamingStarted() throws android.os.RemoteException
    {
    }
    @Override public void onDreamingStopped() throws android.os.RemoteException
    {
    }
    /**
         * Called when the device has started going to sleep.
         *
         * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
         * or {@link #OFF_BECAUSE_OF_TIMEOUT}.
         */
    @Override public void onStartedGoingToSleep(int reason) throws android.os.RemoteException
    {
    }
    /**
         * Called when the device has finished going to sleep.
         *
         * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
         *            or {@link #OFF_BECAUSE_OF_TIMEOUT}.
         * @param cameraGestureTriggered whether the camera gesture was triggered between
         *                               {@link #onStartedGoingToSleep} and this method; if it's been
         *                               triggered, we shouldn't lock the device.
         */
    @Override public void onFinishedGoingToSleep(int reason, boolean cameraGestureTriggered) throws android.os.RemoteException
    {
    }
    /**
         * Called when the device has started waking up.
         */
    @Override public void onStartedWakingUp() throws android.os.RemoteException
    {
    }
    /**
         * Called when the device has finished waking up.
         */
    @Override public void onFinishedWakingUp() throws android.os.RemoteException
    {
    }
    /**
         * Called when the device screen is turning on.
         */
    @Override public void onScreenTurningOn(com.android.internal.policy.IKeyguardDrawnCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Called when the screen has actually turned on.
         */
    @Override public void onScreenTurnedOn() throws android.os.RemoteException
    {
    }
    /**
         * Called when the screen starts turning off.
         */
    @Override public void onScreenTurningOff() throws android.os.RemoteException
    {
    }
    /**
         * Called when the screen has turned off.
         */
    @Override public void onScreenTurnedOff() throws android.os.RemoteException
    {
    }
    @Override public void setKeyguardEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void onSystemReady() throws android.os.RemoteException
    {
    }
    @Override public void doKeyguardTimeout(android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override public void setSwitchingUser(boolean switching) throws android.os.RemoteException
    {
    }
    @Override public void setCurrentUser(int userId) throws android.os.RemoteException
    {
    }
    @Override public void onBootCompleted() throws android.os.RemoteException
    {
    }
    /**
         * Notifies that the activity behind has now been drawn and it's safe to remove the wallpaper
         * and keyguard flag.
         *
         * @param startTime the start time of the animation in uptime milliseconds
         * @param fadeoutDuration the duration of the exit animation, in milliseconds
         */
    @Override public void startKeyguardExitAnimation(long startTime, long fadeoutDuration) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the Keyguard that the power key was pressed while locked and launched Home rather
         * than putting the device to sleep or waking up.
         */
    @Override public void onShortPowerPressedGoHome() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.policy.IKeyguardService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.policy.IKeyguardService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.policy.IKeyguardService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.policy.IKeyguardService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.policy.IKeyguardService))) {
        return ((com.android.internal.policy.IKeyguardService)iin);
      }
      return new com.android.internal.policy.IKeyguardService.Stub.Proxy(obj);
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
        case TRANSACTION_setOccluded:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setOccluded(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addStateMonitorCallback:
        {
          data.enforceInterface(descriptor);
          com.android.internal.policy.IKeyguardStateCallback _arg0;
          _arg0 = com.android.internal.policy.IKeyguardStateCallback.Stub.asInterface(data.readStrongBinder());
          this.addStateMonitorCallback(_arg0);
          return true;
        }
        case TRANSACTION_verifyUnlock:
        {
          data.enforceInterface(descriptor);
          com.android.internal.policy.IKeyguardExitCallback _arg0;
          _arg0 = com.android.internal.policy.IKeyguardExitCallback.Stub.asInterface(data.readStrongBinder());
          this.verifyUnlock(_arg0);
          return true;
        }
        case TRANSACTION_dismiss:
        {
          data.enforceInterface(descriptor);
          com.android.internal.policy.IKeyguardDismissCallback _arg0;
          _arg0 = com.android.internal.policy.IKeyguardDismissCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.CharSequence _arg1;
          if (0!=data.readInt()) {
            _arg1 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.dismiss(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDreamingStarted:
        {
          data.enforceInterface(descriptor);
          this.onDreamingStarted();
          return true;
        }
        case TRANSACTION_onDreamingStopped:
        {
          data.enforceInterface(descriptor);
          this.onDreamingStopped();
          return true;
        }
        case TRANSACTION_onStartedGoingToSleep:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onStartedGoingToSleep(_arg0);
          return true;
        }
        case TRANSACTION_onFinishedGoingToSleep:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onFinishedGoingToSleep(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onStartedWakingUp:
        {
          data.enforceInterface(descriptor);
          this.onStartedWakingUp();
          return true;
        }
        case TRANSACTION_onFinishedWakingUp:
        {
          data.enforceInterface(descriptor);
          this.onFinishedWakingUp();
          return true;
        }
        case TRANSACTION_onScreenTurningOn:
        {
          data.enforceInterface(descriptor);
          com.android.internal.policy.IKeyguardDrawnCallback _arg0;
          _arg0 = com.android.internal.policy.IKeyguardDrawnCallback.Stub.asInterface(data.readStrongBinder());
          this.onScreenTurningOn(_arg0);
          return true;
        }
        case TRANSACTION_onScreenTurnedOn:
        {
          data.enforceInterface(descriptor);
          this.onScreenTurnedOn();
          return true;
        }
        case TRANSACTION_onScreenTurningOff:
        {
          data.enforceInterface(descriptor);
          this.onScreenTurningOff();
          return true;
        }
        case TRANSACTION_onScreenTurnedOff:
        {
          data.enforceInterface(descriptor);
          this.onScreenTurnedOff();
          return true;
        }
        case TRANSACTION_setKeyguardEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setKeyguardEnabled(_arg0);
          return true;
        }
        case TRANSACTION_onSystemReady:
        {
          data.enforceInterface(descriptor);
          this.onSystemReady();
          return true;
        }
        case TRANSACTION_doKeyguardTimeout:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.doKeyguardTimeout(_arg0);
          return true;
        }
        case TRANSACTION_setSwitchingUser:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSwitchingUser(_arg0);
          return true;
        }
        case TRANSACTION_setCurrentUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setCurrentUser(_arg0);
          return true;
        }
        case TRANSACTION_onBootCompleted:
        {
          data.enforceInterface(descriptor);
          this.onBootCompleted();
          return true;
        }
        case TRANSACTION_startKeyguardExitAnimation:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          this.startKeyguardExitAnimation(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onShortPowerPressedGoHome:
        {
          data.enforceInterface(descriptor);
          this.onShortPowerPressedGoHome();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.policy.IKeyguardService
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
           * Sets the Keyguard as occluded when a window dismisses the Keyguard with flag
           * FLAG_SHOW_ON_LOCK_SCREEN.
           *
           * @param isOccluded Whether the Keyguard is occluded by another window.
           * @param animate Whether to play an animation for the state change.
           */
      @Override public void setOccluded(boolean isOccluded, boolean animate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isOccluded)?(1):(0)));
          _data.writeInt(((animate)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOccluded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOccluded(isOccluded, animate);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addStateMonitorCallback(com.android.internal.policy.IKeyguardStateCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addStateMonitorCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addStateMonitorCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void verifyUnlock(com.android.internal.policy.IKeyguardExitCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_verifyUnlock, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().verifyUnlock(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dismiss(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismiss, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismiss(callback, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDreamingStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDreamingStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDreamingStarted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDreamingStopped() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDreamingStopped, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDreamingStopped();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the device has started going to sleep.
           *
           * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
           * or {@link #OFF_BECAUSE_OF_TIMEOUT}.
           */
      @Override public void onStartedGoingToSleep(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartedGoingToSleep, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartedGoingToSleep(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the device has finished going to sleep.
           *
           * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
           *            or {@link #OFF_BECAUSE_OF_TIMEOUT}.
           * @param cameraGestureTriggered whether the camera gesture was triggered between
           *                               {@link #onStartedGoingToSleep} and this method; if it's been
           *                               triggered, we shouldn't lock the device.
           */
      @Override public void onFinishedGoingToSleep(int reason, boolean cameraGestureTriggered) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          _data.writeInt(((cameraGestureTriggered)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFinishedGoingToSleep, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFinishedGoingToSleep(reason, cameraGestureTriggered);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the device has started waking up.
           */
      @Override public void onStartedWakingUp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartedWakingUp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartedWakingUp();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the device has finished waking up.
           */
      @Override public void onFinishedWakingUp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFinishedWakingUp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFinishedWakingUp();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the device screen is turning on.
           */
      @Override public void onScreenTurningOn(com.android.internal.policy.IKeyguardDrawnCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onScreenTurningOn, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onScreenTurningOn(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the screen has actually turned on.
           */
      @Override public void onScreenTurnedOn() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onScreenTurnedOn, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onScreenTurnedOn();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the screen starts turning off.
           */
      @Override public void onScreenTurningOff() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onScreenTurningOff, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onScreenTurningOff();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the screen has turned off.
           */
      @Override public void onScreenTurnedOff() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onScreenTurnedOff, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onScreenTurnedOff();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setKeyguardEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setKeyguardEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setKeyguardEnabled(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSystemReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSystemReady, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSystemReady();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void doKeyguardTimeout(android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_doKeyguardTimeout, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doKeyguardTimeout(options);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setSwitchingUser(boolean switching) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((switching)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSwitchingUser, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSwitchingUser(switching);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setCurrentUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCurrentUser, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCurrentUser(userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onBootCompleted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBootCompleted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBootCompleted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies that the activity behind has now been drawn and it's safe to remove the wallpaper
           * and keyguard flag.
           *
           * @param startTime the start time of the animation in uptime milliseconds
           * @param fadeoutDuration the duration of the exit animation, in milliseconds
           */
      @Override public void startKeyguardExitAnimation(long startTime, long fadeoutDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(startTime);
          _data.writeLong(fadeoutDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startKeyguardExitAnimation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startKeyguardExitAnimation(startTime, fadeoutDuration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the Keyguard that the power key was pressed while locked and launched Home rather
           * than putting the device to sleep or waking up.
           */
      @Override public void onShortPowerPressedGoHome() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShortPowerPressedGoHome, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShortPowerPressedGoHome();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.policy.IKeyguardService sDefaultImpl;
    }
    static final int TRANSACTION_setOccluded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addStateMonitorCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_verifyUnlock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_dismiss = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onDreamingStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onDreamingStopped = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onStartedGoingToSleep = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onFinishedGoingToSleep = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onStartedWakingUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onFinishedWakingUp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onScreenTurningOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onScreenTurnedOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onScreenTurningOff = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onScreenTurnedOff = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setKeyguardEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onSystemReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_doKeyguardTimeout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setSwitchingUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setCurrentUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_onBootCompleted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_startKeyguardExitAnimation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_onShortPowerPressedGoHome = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    public static boolean setDefaultImpl(com.android.internal.policy.IKeyguardService impl) {
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
    public static com.android.internal.policy.IKeyguardService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Sets the Keyguard as occluded when a window dismisses the Keyguard with flag
       * FLAG_SHOW_ON_LOCK_SCREEN.
       *
       * @param isOccluded Whether the Keyguard is occluded by another window.
       * @param animate Whether to play an animation for the state change.
       */
  public void setOccluded(boolean isOccluded, boolean animate) throws android.os.RemoteException;
  public void addStateMonitorCallback(com.android.internal.policy.IKeyguardStateCallback callback) throws android.os.RemoteException;
  public void verifyUnlock(com.android.internal.policy.IKeyguardExitCallback callback) throws android.os.RemoteException;
  public void dismiss(com.android.internal.policy.IKeyguardDismissCallback callback, java.lang.CharSequence message) throws android.os.RemoteException;
  public void onDreamingStarted() throws android.os.RemoteException;
  public void onDreamingStopped() throws android.os.RemoteException;
  /**
       * Called when the device has started going to sleep.
       *
       * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
       * or {@link #OFF_BECAUSE_OF_TIMEOUT}.
       */
  public void onStartedGoingToSleep(int reason) throws android.os.RemoteException;
  /**
       * Called when the device has finished going to sleep.
       *
       * @param why {@link #OFF_BECAUSE_OF_USER}, {@link #OFF_BECAUSE_OF_ADMIN},
       *            or {@link #OFF_BECAUSE_OF_TIMEOUT}.
       * @param cameraGestureTriggered whether the camera gesture was triggered between
       *                               {@link #onStartedGoingToSleep} and this method; if it's been
       *                               triggered, we shouldn't lock the device.
       */
  public void onFinishedGoingToSleep(int reason, boolean cameraGestureTriggered) throws android.os.RemoteException;
  /**
       * Called when the device has started waking up.
       */
  public void onStartedWakingUp() throws android.os.RemoteException;
  /**
       * Called when the device has finished waking up.
       */
  public void onFinishedWakingUp() throws android.os.RemoteException;
  /**
       * Called when the device screen is turning on.
       */
  public void onScreenTurningOn(com.android.internal.policy.IKeyguardDrawnCallback callback) throws android.os.RemoteException;
  /**
       * Called when the screen has actually turned on.
       */
  public void onScreenTurnedOn() throws android.os.RemoteException;
  /**
       * Called when the screen starts turning off.
       */
  public void onScreenTurningOff() throws android.os.RemoteException;
  /**
       * Called when the screen has turned off.
       */
  public void onScreenTurnedOff() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/policy/IKeyguardService.aidl:91:1:91:25")
  public void setKeyguardEnabled(boolean enabled) throws android.os.RemoteException;
  public void onSystemReady() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/policy/IKeyguardService.aidl:94:1:94:25")
  public void doKeyguardTimeout(android.os.Bundle options) throws android.os.RemoteException;
  public void setSwitchingUser(boolean switching) throws android.os.RemoteException;
  public void setCurrentUser(int userId) throws android.os.RemoteException;
  public void onBootCompleted() throws android.os.RemoteException;
  /**
       * Notifies that the activity behind has now been drawn and it's safe to remove the wallpaper
       * and keyguard flag.
       *
       * @param startTime the start time of the animation in uptime milliseconds
       * @param fadeoutDuration the duration of the exit animation, in milliseconds
       */
  public void startKeyguardExitAnimation(long startTime, long fadeoutDuration) throws android.os.RemoteException;
  /**
       * Notifies the Keyguard that the power key was pressed while locked and launched Home rather
       * than putting the device to sleep or waking up.
       */
  public void onShortPowerPressedGoHome() throws android.os.RemoteException;
}
