/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.car;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.app.ActivityManager;
import android.app.IActivityManager;
import android.app.admin.DevicePolicyManager;
import android.car.userlib.CarUserManagerHelper;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.UserInfo;
import android.hidl.manager.V1_0.IServiceManager;
import android.os.Binder;
import android.os.IBinder;
import android.os.Parcel;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.Trace;
import android.os.UserHandle;
import android.os.UserManager;
import android.sysprop.CarProperties;
import android.util.Slog;
import android.util.TimingsTraceLog;
import android.util.SparseBooleanArray;

import com.android.internal.annotations.GuardedBy;
import com.android.internal.annotations.VisibleForTesting;
import com.android.server.SystemService;
import com.android.server.Watchdog;
import com.android.server.am.ActivityManagerService;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * System service side companion service for CarService.
 * Starts car service and provide necessary API for CarService. Only for car product.
 */
public class CarServiceHelperService extends SystemService {
    // Place holder for user name of the first user created.
    private static final String TAG = "CarServiceHelper";
    private static final boolean DBG = true;
    private static final String CAR_SERVICE_INTERFACE = "android.car.ICar";
    // These numbers should match with binder call order of
    // packages/services/Car/car-lib/src/android/car/ICar.aidl
    private static final int ICAR_CALL_SET_CAR_SERVICE_HELPER = 0;
    private static final int ICAR_CALL_SET_USER_UNLOCK_STATUS = 1;
    private static final int ICAR_CALL_SET_SWITCH_USER = 2;

    private static final String PROP_RESTART_RUNTIME = "ro.car.recovery.restart_runtime.enabled";

    private static final List<String> CAR_HAL_INTERFACES_OF_INTEREST = Arrays.asList(
            "android.hardware.automotive.vehicle@2.0::IVehicle",
            "android.hardware.automotive.audiocontrol@1.0::IAudioControl"
    );

    @GuardedBy("mLock")
    private int mLastSwitchedUser = UserHandle.USER_NULL;

    private final ICarServiceHelperImpl mHelper = new ICarServiceHelperImpl();
    private final Context mContext;
    private final Object mLock = new Object();
    @GuardedBy("mLock")
    private IBinder mCarService;
    @GuardedBy("mLock")
    private boolean mSystemBootCompleted;

    @GuardedBy("mLock")
    private final HashMap<Integer, Boolean> mUserUnlockedStatus = new HashMap<>();
    private final CarUserManagerHelper mCarUserManagerHelper;
    private final UserManager mUserManager;
    private final ServiceConnection mCarServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            if (DBG) {
                Slog.d(TAG, "onServiceConnected:" + iBinder);
            }
            handleCarServiceConnection(iBinder);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            handleCarServiceCrash();
        }
    };

    public CarServiceHelperService(Context context) {
        this(context, new CarUserManagerHelper(context));
    }

    @VisibleForTesting
    CarServiceHelperService(Context context, CarUserManagerHelper carUserManagerHelper) {
        super(context);
        mContext = context;
        mCarUserManagerHelper = carUserManagerHelper;
        mUserManager = UserManager.get(context);
    }

    @Override
    public void onBootPhase(int phase) {
        if (DBG) {
            Slog.d(TAG, "onBootPhase:" + phase);
        }
        if (phase == SystemService.PHASE_THIRD_PARTY_APPS_CAN_START) {
            checkForCarServiceConnection();
            // TODO(b/126199560) Consider doing this earlier in onStart().
            // Other than onStart, PHASE_THIRD_PARTY_APPS_CAN_START is the earliest timing.
            setupAndStartUsers();
            checkForCarServiceConnection();
        } else if (phase == SystemService.PHASE_BOOT_COMPLETED) {
            TimingsTraceLog t = new TimingsTraceLog(TAG, Trace.TRACE_TAG_SYSTEM_SERVER);
            t.traceBegin("onBootPhase.completed");
            managePreCreatedUsers();
            boolean shouldNotify = false;
            synchronized (mLock) {
                mSystemBootCompleted = true;
                if (mCarService != null) {
                    shouldNotify = true;
                }
            }
            if (shouldNotify) {
                notifyAllUnlockedUsers();
            }
            t.traceEnd();
        }
    }

    @Override
    public void onStart() {
        Intent intent = new Intent();
        intent.setPackage("com.android.car");
        intent.setAction(CAR_SERVICE_INTERFACE);
        if (!getContext().bindServiceAsUser(intent, mCarServiceConnection, Context.BIND_AUTO_CREATE,
                UserHandle.SYSTEM)) {
            Slog.wtf(TAG, "cannot start car service");
        }
        System.loadLibrary("car-framework-service-jni");
    }


    @Override
    public void onUnlockUser(int userHandle) {
        handleUserLockStatusChange(userHandle, true);
        if (DBG) {
            Slog.d(TAG, "User" + userHandle + " unlocked");
        }
    }

    @Override
    public void onStopUser(int userHandle) {
        handleUserLockStatusChange(userHandle, false);
    }

    @Override
    public void onCleanupUser(int userHandle) {
        handleUserLockStatusChange(userHandle, false);
    }

    @Override
    public void onSwitchUser(int userHandle) {
        synchronized (mLock) {
            mLastSwitchedUser = userHandle;
            if (mCarService == null) {
                return;  // The event will be delivered upon CarService connection.
            }
        }
        sendSwitchUserBindercall(userHandle);
    }

    // Sometimes car service onConnected call is delayed a lot. car service binder can be
    // found from ServiceManager directly. So do some polling during boot-up to connect to
    // car service ASAP.
    private void checkForCarServiceConnection() {
        synchronized (mLock) {
            if (mCarService != null) {
                return;
            }
        }
        IBinder iBinder = ServiceManager.checkService("car_service");
        if (iBinder != null) {
            if (DBG) {
                Slog.d(TAG, "Car service found through ServiceManager:" + iBinder);
            }
            handleCarServiceConnection(iBinder);
        }
    }

    private void handleCarServiceConnection(IBinder iBinder) {
        int lastSwitchedUser;
        boolean systemBootCompleted;
        synchronized (mLock) {
            if (mCarService == iBinder) {
                return; // already connected.
            }
            if (mCarService != null) {
                Slog.i(TAG, "car service binder changed, was:" + mCarService
                        + " new:" + iBinder);
            }
            mCarService = iBinder;
            lastSwitchedUser = mLastSwitchedUser;
            systemBootCompleted = mSystemBootCompleted;
        }
        Slog.i(TAG, "**CarService connected**");
        sendSetCarServiceHelperBinderCall();
        if (systemBootCompleted) {
            notifyAllUnlockedUsers();
        }
        if (lastSwitchedUser != UserHandle.USER_NULL) {
            sendSwitchUserBindercall(lastSwitchedUser);
        }
    }

    private void handleUserLockStatusChange(int userHandle, boolean unlocked) {
        boolean shouldNotify = false;
        synchronized (mLock) {
            Boolean oldStatus = mUserUnlockedStatus.get(userHandle);
            if (oldStatus == null || oldStatus != unlocked) {
                mUserUnlockedStatus.put(userHandle, unlocked);
                if (mCarService != null && mSystemBootCompleted) {
                    shouldNotify = true;
                }
            }
        }
        if (shouldNotify) {
            sendSetUserLockStatusBinderCall(userHandle, unlocked);
        }
    }

    private void setupAndStartUsers() {
        DevicePolicyManager devicePolicyManager =
                mContext.getSystemService(DevicePolicyManager.class);
        if (devicePolicyManager != null && devicePolicyManager.getUserProvisioningState()
                != DevicePolicyManager.STATE_USER_UNMANAGED) {
            Slog.i(TAG, "DevicePolicyManager active, skip user unlock/switch");
            return;
        }
        // Offloading the whole unlock into separate thread did not help due to single locks
        // used in AMS / PMS ended up stopping the world with lots of lock contention.
        // To run these in background, there should be some improvements there.
        int targetUserId = UserHandle.USER_SYSTEM;
        if (mCarUserManagerHelper.getAllUsers().size() == 0) {
            Slog.i(TAG, "Create new admin user and switch");
            // On very first boot, create an admin user and switch to that user.
            UserInfo admin = mCarUserManagerHelper.createNewAdminUser();
            if (admin == null) {
                Slog.e(TAG, "cannot create admin user");
                return;
            }
            targetUserId = admin.id;
        } else {
            targetUserId = mCarUserManagerHelper.getInitialUser();
            Slog.i(TAG, "Switching to user " + targetUserId + " on boot");
        }

        IActivityManager am = ActivityManager.getService();
        if (am == null) {
            Slog.wtf(TAG, "cannot get ActivityManagerService");
            return;
        }

        // If system user is the only user to unlock, handle it when system completes the boot.
        if (targetUserId == UserHandle.USER_SYSTEM) {
            return;
        }

        TimingsTraceLog t = new TimingsTraceLog(TAG, Trace.TRACE_TAG_SYSTEM_SERVER);
        unlockSystemUser(t, am);

        t.traceBegin("ForegroundUserStart" + targetUserId);
        try {
            if (!am.startUserInForegroundWithListener(targetUserId, null)) {
                Slog.e(TAG, "cannot start foreground user:" + targetUserId);
            } else {
                mCarUserManagerHelper.setLastActiveUser(targetUserId);
            }
        } catch (RemoteException e) {
            // should not happen for local call.
            Slog.wtf("RemoteException from AMS", e);
        }
        t.traceEnd();
    }

    private void unlockSystemUser(@NonNull TimingsTraceLog t, @NonNull IActivityManager am) {
        t.traceBegin("UnlockSystemUser");
        try {
            // This is for force changing state into RUNNING_LOCKED. Otherwise unlock does not
            // update the state and user 0 unlock happens twice.
            boolean started = am.startUserInBackground(UserHandle.USER_SYSTEM);
            if (!started) {
                Slog.w(TAG, "could not restart system user in foreground; trying unlock instead");
                t.traceBegin("forceUnlockSystemUser");
                boolean unlocked = am.unlockUser(UserHandle.USER_SYSTEM,
                        /* token= */ null, /* secret= */ null, /* listner= */ null);
                t.traceEnd();
                if (!unlocked) {
                    Slog.w(TAG, "could not unlock system user neither");
                    return;
                }
            }
        } catch (RemoteException e) {
            // should not happen for local call.
            Slog.wtf("RemoteException from AMS", e);
        } finally {
            t.traceEnd();
        }
    }

    private void managePreCreatedUsers() {

        // First gets how many pre-createad users are defined by the OEM
        int numberRequestedGuests = CarProperties.number_pre_created_guests().orElse(0);
        int numberRequestedUsers = CarProperties.number_pre_created_users().orElse(0);
        Slog.i(TAG, "managePreCreatedUsers(): OEM asked for " + numberRequestedGuests
                + " guests and " + numberRequestedUsers + " users");
        if (numberRequestedGuests < 0 || numberRequestedUsers < 0) {
            Slog.w(TAG, "preCreateUsers(): invalid values provided by OEM; "
                    + "number_pre_created_guests=" + numberRequestedGuests
                    + ", number_pre_created_users=" + numberRequestedUsers);
            return;
        }

        if (numberRequestedGuests == 0 && numberRequestedUsers == 0) {
            Slog.i(TAG, "managePreCreatedUsers(): not defined by OEM");
            return;
        }

        // Then checks how many exist already
        List<UserInfo> allUsers = mUserManager.getUsers(/* excludePartial= */ true,
                /* excludeDying= */ true, /* excludePreCreated= */ false);

        int allUsersSize = allUsers.size();
        if (DBG) Slog.d(TAG, "preCreateUsers: total users size is "  + allUsersSize);

        int numberExistingGuests = 0;
        int numberExistingUsers = 0;

        // List of pre-created users that were not properly initialized. Typically happens when
        // the system crashed / rebooted before they were fully started.
        SparseBooleanArray invalidUsers = new SparseBooleanArray();

        for (int i = 0; i < allUsersSize; i++) {
            UserInfo user = allUsers.get(i);
            if (!user.preCreated) continue;
            if (!user.isInitialized()) {
                Slog.w(TAG, "Found invalid pre-created user that needs to be removed: "
                        + user.toFullString());
                invalidUsers.append(user.id, /* notUsed=*/ true);
                continue;
            }
            if (user.isGuest()) {
                numberExistingGuests++;
            } else {
                numberExistingUsers++;
            }
        }
        if (DBG) {
            Slog.i(TAG, "managePreCreatedUsers(): system already has " + numberExistingGuests
                    + " pre-created guests," + numberExistingUsers + " pre-created users, and these"
                    + " invalid users: " + invalidUsers );
        }

        int numberGuests = numberRequestedGuests - numberExistingGuests;
        int numberUsers = numberRequestedUsers - numberExistingUsers;
        int numberInvalidUsers = invalidUsers.size();

        if (numberGuests <= 0 && numberUsers <= 0 && numberInvalidUsers == 0) {
            Slog.i(TAG, "managePreCreatedUsers(): all pre-created and no invalid ones");
            return;
        }

        // Finally, manage them....

        // In theory, we could submit multiple user pre-creations in parallel, but we're
        // submitting just 1 task, for 2 reasons:
        //   1.To minimize it's effect on other system server initialization tasks.
        //   2.The pre-created users will be unlocked in parallel anyways.
        new Thread( () -> {
            TimingsTraceLog t = new TimingsTraceLog(TAG, Trace.TRACE_TAG_SYSTEM_SERVER);

            t.traceBegin("preCreateUsers");
            if (numberUsers > 0) {
                preCreateUsers(t, numberUsers, /* isGuest= */ false);
            }
            if (numberGuests > 0) {
                preCreateUsers(t, numberGuests, /* isGuest= */ true);
            }
            t.traceEnd();

            if (numberInvalidUsers > 0) {
                t.traceBegin("removeInvalidPreCreatedUsers");
                for (int i = 0; i < numberInvalidUsers; i++) {
                    int userId = invalidUsers.keyAt(i);
                    Slog.i(TAG, "removing invalid pre-created user " + userId);
                    mUserManager.removeUser(userId);
                }
                t.traceEnd();
            }
        }, "CarServiceHelperManagePreCreatedUsers").start();
    }

    private void preCreateUsers(@NonNull TimingsTraceLog t, int size, boolean isGuest) {
        String msg = isGuest ? "preCreateGuests-" + size : "preCreateUsers-" + size;
        t.traceBegin(msg);
        for (int i = 1; i <= size; i++) {
            UserInfo preCreated = preCreateUsers(t, isGuest);
            if (preCreated == null) {
                Slog.w(TAG, "Could not pre-create " + (isGuest ? " guest " : "")
                        + " user #" + i);
                continue;
            }
        }
        t.traceEnd();
    }

    // TODO(b/111451156): add unit test?
    @Nullable
    public UserInfo preCreateUsers(@NonNull TimingsTraceLog t, boolean isGuest) {
        int flags = 0;
        String traceMsg =  "pre-create";
        if (isGuest) {
            flags |= UserInfo.FLAG_GUEST;
            traceMsg += "-guest";
        } else {
            traceMsg += "-user";
        }
        t.traceBegin(traceMsg);
        // NOTE: we want to get rid of UserManagerHelper, so let's call UserManager directly
        UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        UserInfo user = um.preCreateUser(flags);
        try {
            if (user == null) {
                // Couldn't create user, most likely because there are too many.
                Slog.w(TAG, "couldn't " + traceMsg);
                return null;
            }
        } finally {
            t.traceEnd();
        }
        return user;
    }

    private void notifyAllUnlockedUsers() {
        // only care about unlocked users
        LinkedList<Integer> users = new LinkedList<>();
        synchronized (mLock) {
            for (Map.Entry<Integer, Boolean> entry : mUserUnlockedStatus.entrySet()) {
                if (entry.getValue()) {
                    users.add(entry.getKey());
                }
            }
        }
        if (DBG) {
            Slog.d(TAG, "notifyAllUnlockedUsers:" + users);
        }
        for (Integer i : users) {
            sendSetUserLockStatusBinderCall(i, true);
        }
    }

    private void sendSetCarServiceHelperBinderCall() {
        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(CAR_SERVICE_INTERFACE);
        data.writeStrongBinder(mHelper.asBinder());
        // void setCarServiceHelper(in IBinder helper)
        sendBinderCallToCarService(data, ICAR_CALL_SET_CAR_SERVICE_HELPER);
    }

    private void sendSetUserLockStatusBinderCall(int userHandle, boolean unlocked) {
        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(CAR_SERVICE_INTERFACE);
        data.writeInt(userHandle);
        data.writeInt(unlocked ? 1 : 0);
        // void setUserLockStatus(in int userHandle, in int unlocked)
        sendBinderCallToCarService(data, ICAR_CALL_SET_USER_UNLOCK_STATUS);
    }

    private void sendSwitchUserBindercall(int userHandle) {
        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(CAR_SERVICE_INTERFACE);
        data.writeInt(userHandle);
        // void onSwitchUser(in int userHandle)
        sendBinderCallToCarService(data, ICAR_CALL_SET_SWITCH_USER);
    }

    private void sendBinderCallToCarService(Parcel data, int callNumber) {
        // Cannot depend on ICar which is defined in CarService, so handle binder call directly
        // instead.
        IBinder carService;
        synchronized (mLock) {
            carService = mCarService;
        }
        try {
            carService.transact(IBinder.FIRST_CALL_TRANSACTION + callNumber,
                    data, null, Binder.FLAG_ONEWAY);
        } catch (RemoteException e) {
            Slog.w(TAG, "RemoteException from car service", e);
            handleCarServiceCrash();
        } finally {
            data.recycle();
        }
    }

    // Adapted from frameworks/base/services/core/java/com/android/server/Watchdog.java
    // TODO(b/131861630) use implementation common with Watchdog.java
    //
    private static ArrayList<Integer> getInterestingHalPids() {
        try {
            IServiceManager serviceManager = IServiceManager.getService();
            ArrayList<IServiceManager.InstanceDebugInfo> dump =
                    serviceManager.debugDump();
            HashSet<Integer> pids = new HashSet<>();
            for (IServiceManager.InstanceDebugInfo info : dump) {
                if (info.pid == IServiceManager.PidConstant.NO_PID) {
                    continue;
                }

                if (Watchdog.HAL_INTERFACES_OF_INTEREST.contains(info.interfaceName) ||
                        CAR_HAL_INTERFACES_OF_INTEREST.contains(info.interfaceName)) {
                    pids.add(info.pid);
                }
            }

            return new ArrayList<Integer>(pids);
        } catch (RemoteException e) {
            return new ArrayList<Integer>();
        }
    }

    // Adapted from frameworks/base/services/core/java/com/android/server/Watchdog.java
    // TODO(b/131861630) use implementation common with Watchdog.java
    //
    private static ArrayList<Integer> getInterestingNativePids() {
        ArrayList<Integer> pids = getInterestingHalPids();

        int[] nativePids = Process.getPidsForCommands(Watchdog.NATIVE_STACKS_OF_INTEREST);
        if (nativePids != null) {
            pids.ensureCapacity(pids.size() + nativePids.length);
            for (int i : nativePids) {
                pids.add(i);
            }
        }

        return pids;
    }

    // Borrowed from Watchdog.java.  Create an ANR file from the call stacks.
    //
    private static void dumpServiceStacks() {
        ArrayList<Integer> pids = new ArrayList<>();
        pids.add(Process.myPid());

        ActivityManagerService.dumpStackTraces(
                pids, null, null, getInterestingNativePids());
    }

    private void handleCarServiceCrash() {
        // Recovery behavior.  Kill the system server and reset
        // everything if enabled by the property.
        boolean restartOnServiceCrash = SystemProperties.getBoolean(PROP_RESTART_RUNTIME, false);

        dumpServiceStacks();
        if (restartOnServiceCrash) {
            Slog.w(TAG, "*** CARHELPER KILLING SYSTEM PROCESS: " + "CarService crash");
            Slog.w(TAG, "*** GOODBYE!");
            Process.killProcess(Process.myPid());
            System.exit(10);
        } else {
            Slog.w(TAG, "*** CARHELPER ignoring: " + "CarService crash");
        }
    }

    private static native int nativeForceSuspend(int timeoutMs);

    private class ICarServiceHelperImpl extends ICarServiceHelper.Stub {
        /**
         * Force device to suspend
         */
        @Override // Binder call
        public int forceSuspend(int timeoutMs) {
            int retVal;
            mContext.enforceCallingOrSelfPermission(android.Manifest.permission.DEVICE_POWER, null);
            final long ident = Binder.clearCallingIdentity();
            try {
                retVal = nativeForceSuspend(timeoutMs);
            } finally {
                Binder.restoreCallingIdentity(ident);
            }
            return retVal;
        }
    }
}
