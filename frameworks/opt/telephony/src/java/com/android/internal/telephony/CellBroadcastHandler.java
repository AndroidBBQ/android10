/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.internal.telephony;

import static android.content.PermissionChecker.PERMISSION_GRANTED;
import static android.provider.Settings.Secure.CMAS_ADDITIONAL_BROADCAST_PKG;

import android.Manifest;
import android.annotation.NonNull;
import android.annotation.Nullable;
import android.app.Activity;
import android.app.AppOpsManager;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.PermissionChecker;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Telephony;
import android.provider.Telephony.CellBroadcasts;
import android.telephony.SmsCbMessage;
import android.telephony.SubscriptionManager;
import android.text.format.DateUtils;
import android.util.LocalLog;
import android.util.Log;

import com.android.internal.telephony.CbGeoUtils.Geometry;
import com.android.internal.telephony.CbGeoUtils.LatLng;
import com.android.internal.telephony.metrics.TelephonyMetrics;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Dispatch new Cell Broadcasts to receivers. Acquires a private wakelock until the broadcast
 * completes and our result receiver is called.
 */
public class CellBroadcastHandler extends WakeLockStateMachine {
    private static final String EXTRA_MESSAGE = "message";

    private final LocalLog mLocalLog = new LocalLog(100);

    protected static final Uri CELL_BROADCAST_URI = Uri.parse("content://cellbroadcasts_fwk");

    /** Uses to request the location update. */
    public final LocationRequester mLocationRequester;

    private CellBroadcastHandler(Context context, Phone phone) {
        this("CellBroadcastHandler", context, phone);
    }

    protected CellBroadcastHandler(String debugTag, Context context, Phone phone) {
        super(debugTag, context, phone);
        mLocationRequester = new LocationRequester(
                context,
                (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE),
                getHandler().getLooper());
    }

    /**
     * Create a new CellBroadcastHandler.
     * @param context the context to use for dispatching Intents
     * @return the new handler
     */
    public static CellBroadcastHandler makeCellBroadcastHandler(Context context, Phone phone) {
        CellBroadcastHandler handler = new CellBroadcastHandler(context, phone);
        handler.start();
        return handler;
    }

    /**
     * Handle Cell Broadcast messages from {@code CdmaInboundSmsHandler}.
     * 3GPP-format Cell Broadcast messages sent from radio are handled in the subclass.
     *
     * @param message the message to process
     * @return true if need to wait for geo-fencing or an ordered broadcast was sent.
     */
    @Override
    protected boolean handleSmsMessage(Message message) {
        if (message.obj instanceof SmsCbMessage) {
            handleBroadcastSms((SmsCbMessage) message.obj);
            return true;
        } else {
            loge("handleMessage got object of type: " + message.obj.getClass().getName());
            return false;
        }
    }

    /**
     * Dispatch a Cell Broadcast message to listeners.
     * @param message the Cell Broadcast to broadcast
     */
    protected void handleBroadcastSms(SmsCbMessage message) {
        // Log Cellbroadcast msg received event
        TelephonyMetrics metrics = TelephonyMetrics.getInstance();
        metrics.writeNewCBSms(mPhone.getPhoneId(), message.getMessageFormat(),
                message.getMessagePriority(), message.isCmasMessage(), message.isEtwsMessage(),
                message.getServiceCategory(), message.getSerialNumber(),
                System.currentTimeMillis());

        // TODO: Database inserting can be time consuming, therefore this should be changed to
        // asynchronous.
        ContentValues cv = message.getContentValues();
        Uri uri = mContext.getContentResolver().insert(CELL_BROADCAST_URI, cv);

        if (message.needGeoFencingCheck()) {
            if (DBG) {
                log("Request location update for geo-fencing. serialNumber = "
                        + message.getSerialNumber());
            }

            requestLocationUpdate(location -> {
                if (location == null) {
                    // Broadcast the message directly if the location is not available.
                    broadcastMessage(message, uri);
                } else {
                    performGeoFencing(message, uri, message.getGeometries(), location);
                }
            }, message.getMaximumWaitingTime());
        } else {
            if (DBG) {
                log("Broadcast the message directly because no geo-fencing required, "
                        + "serialNumber = " + message.getSerialNumber()
                        + " needGeoFencing = " + message.needGeoFencingCheck());
            }
            broadcastMessage(message, uri);
        }
    }

    /**
     * Perform a geo-fencing check for {@code message}. Broadcast the {@code message} if the
     * {@code location} is inside the {@code broadcastArea}.
     * @param message the message need to geo-fencing check
     * @param uri the message's uri
     * @param broadcastArea the broadcast area of the message
     * @param location current location
     */
    protected void performGeoFencing(SmsCbMessage message, Uri uri, List<Geometry> broadcastArea,
            LatLng location) {

        if (DBG) {
            logd("Perform geo-fencing check for message identifier = "
                    + message.getServiceCategory()
                    + " serialNumber = " + message.getSerialNumber());
        }

        for (Geometry geo : broadcastArea) {
            if (geo.contains(location)) {
                broadcastMessage(message, uri);
                return;
            }
        }

        if (DBG) {
            logd("Device location is outside the broadcast area "
                    + CbGeoUtils.encodeGeometriesToString(broadcastArea));
        }

        sendMessage(EVENT_BROADCAST_NOT_REQUIRED);
    }

    /**
     * Request a single location update.
     * @param callback a callback will be called when the location is available.
     * @param maximumWaitTimeSec the maximum wait time of this request. If location is not updated
     * within the maximum wait time, {@code callback#onLocationUpadte(null)} will be called.
     */
    protected void requestLocationUpdate(LocationUpdateCallback callback, int maximumWaitTimeSec) {
        mLocationRequester.requestLocationUpdate(callback, maximumWaitTimeSec);
    }

    /**
     * Broadcast a list of cell broadcast messages.
     * @param cbMessages a list of cell broadcast message.
     * @param cbMessageUris the corresponding {@link Uri} of the cell broadcast messages.
     */
    protected void broadcastMessage(List<SmsCbMessage> cbMessages, List<Uri> cbMessageUris) {
        for (int i = 0; i < cbMessages.size(); i++) {
            broadcastMessage(cbMessages.get(i), cbMessageUris.get(i));
        }
    }

    /**
     * Broadcast the {@code message} to the applications.
     * @param message a message need to broadcast
     * @param messageUri message's uri
     */
    protected void broadcastMessage(@NonNull SmsCbMessage message, @Nullable Uri messageUri) {
        String receiverPermission;
        int appOp;
        String msg;
        Intent intent;
        if (message.isEmergencyMessage()) {
            msg = "Dispatching emergency SMS CB, SmsCbMessage is: " + message;
            log(msg);
            mLocalLog.log(msg);
            intent = new Intent(Telephony.Sms.Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
            //Emergency alerts need to be delivered with high priority
            intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
            receiverPermission = Manifest.permission.RECEIVE_EMERGENCY_BROADCAST;
            appOp = AppOpsManager.OP_RECEIVE_EMERGECY_SMS;

            intent.putExtra(EXTRA_MESSAGE, message);
            SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhone.getPhoneId());

            if (Build.IS_DEBUGGABLE) {
                // Send additional broadcast intent to the specified package. This is only for sl4a
                // automation tests.
                final String additionalPackage = Settings.Secure.getString(
                        mContext.getContentResolver(), CMAS_ADDITIONAL_BROADCAST_PKG);
                if (additionalPackage != null) {
                    Intent additionalIntent = new Intent(intent);
                    additionalIntent.setPackage(additionalPackage);
                    mContext.sendOrderedBroadcastAsUser(additionalIntent, UserHandle.ALL,
                            receiverPermission, appOp, null, getHandler(), Activity.RESULT_OK,
                            null, null);
                }
            }

            String[] pkgs = mContext.getResources().getStringArray(
                    com.android.internal.R.array.config_defaultCellBroadcastReceiverPkgs);
            mReceiverCount.addAndGet(pkgs.length);
            for (String pkg : pkgs) {
                // Explicitly send the intent to all the configured cell broadcast receivers.
                intent.setPackage(pkg);
                mContext.sendOrderedBroadcastAsUser(intent, UserHandle.ALL, receiverPermission,
                        appOp, mReceiver, getHandler(), Activity.RESULT_OK, null, null);
            }
        } else {
            msg = "Dispatching SMS CB, SmsCbMessage is: " + message;
            log(msg);
            mLocalLog.log(msg);
            intent = new Intent(Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION);
            // Send implicit intent since there are various 3rd party carrier apps listen to
            // this intent.
            intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
            receiverPermission = Manifest.permission.RECEIVE_SMS;
            appOp = AppOpsManager.OP_RECEIVE_SMS;

            intent.putExtra(EXTRA_MESSAGE, message);
            SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhone.getPhoneId());

            mReceiverCount.incrementAndGet();
            mContext.sendOrderedBroadcastAsUser(intent, UserHandle.ALL, receiverPermission, appOp,
                    mReceiver, getHandler(), Activity.RESULT_OK, null, null);
        }

        if (messageUri != null) {
            ContentValues cv = new ContentValues();
            cv.put(CellBroadcasts.MESSAGE_BROADCASTED, 1);
            mContext.getContentResolver().update(CELL_BROADCAST_URI, cv,
                    CellBroadcasts._ID + "=?", new String[] {messageUri.getLastPathSegment()});
        }
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("CellBroadcastHandler:");
        mLocalLog.dump(fd, pw, args);
        pw.flush();
    }

    /** The callback interface of a location request. */
    public interface LocationUpdateCallback {
        /**
         * Call when the location update is available.
         * @param location a location in (latitude, longitude) format, or {@code null} if the
         * location service is not available.
         */
        void onLocationUpdate(@Nullable LatLng location);
    }

    private static final class LocationRequester {
        private static final String TAG = CellBroadcastHandler.class.getSimpleName();

        /**
         * Use as the default maximum wait time if the cell broadcast doesn't specify the value.
         * Most of the location request should be responded within 30 seconds.
         */
        private static final int DEFAULT_MAXIMUM_WAIT_TIME_SEC = 30;

        /**
         * Trigger this event when the {@link LocationManager} is not responded within the given
         * time.
         */
        private static final int EVENT_LOCATION_REQUEST_TIMEOUT = 1;

        /** Request a single location update. */
        private static final int EVENT_REQUEST_LOCATION_UPDATE = 2;

        /**
         * Request location update from network or gps location provider. Network provider will be
         * used if available, otherwise use the gps provider.
         */
        private static final List<String> LOCATION_PROVIDERS = Arrays.asList(
                LocationManager.NETWORK_PROVIDER, LocationManager.GPS_PROVIDER);

        private final LocationManager mLocationManager;
        private final Looper mLooper;
        private final List<LocationUpdateCallback> mCallbacks;
        private final Context mContext;
        private Handler mLocationHandler;

        LocationRequester(Context context, LocationManager locationManager, Looper looper) {
            mLocationManager = locationManager;
            mLooper = looper;
            mCallbacks = new ArrayList<>();
            mContext = context;
            mLocationHandler = new LocationHandler(looper);
        }

        /**
         * Request a single location update. If the location is not available, a callback with
         * {@code null} location will be called immediately.
         *
         * @param callback a callback to the the response when the location is available
         * @param maximumWaitTimeSec the maximum wait time of this request. If location is not
         * updated within the maximum wait time, {@code callback#onLocationUpadte(null)} will be
         * called.
         */
        void requestLocationUpdate(@NonNull LocationUpdateCallback callback,
                int maximumWaitTimeSec) {
            mLocationHandler.obtainMessage(EVENT_REQUEST_LOCATION_UPDATE, maximumWaitTimeSec,
                    0 /* arg2 */, callback).sendToTarget();
        }

        private void onLocationUpdate(@Nullable LatLng location) {
            for (LocationUpdateCallback callback : mCallbacks) {
                callback.onLocationUpdate(location);
            }
            mCallbacks.clear();

            for (LocationListener listener : mLocationListenerList) {
                mLocationManager.removeUpdates(listener);
            }
            mLocationListenerList.clear();
        }

        private void requestLocationUpdateInternal(@NonNull LocationUpdateCallback callback,
                int maximumWaitTimeSec) {
            if (DBG) Log.d(TAG, "requestLocationUpdate");
            if (!isLocationServiceAvailable()) {
                if (DBG) {
                    Log.d(TAG, "Can't request location update because of no location permission");
                }
                callback.onLocationUpdate(null);
                return;
            }

            if (!mLocationHandler.hasMessages(EVENT_LOCATION_REQUEST_TIMEOUT)) {
                if (maximumWaitTimeSec == SmsCbMessage.MAXIMUM_WAIT_TIME_NOT_SET) {
                    maximumWaitTimeSec = DEFAULT_MAXIMUM_WAIT_TIME_SEC;
                }
                mLocationHandler.sendMessageDelayed(
                        mLocationHandler.obtainMessage(EVENT_LOCATION_REQUEST_TIMEOUT),
                        maximumWaitTimeSec * DateUtils.SECOND_IN_MILLIS);
            }

            mCallbacks.add(callback);

            for (String provider : LOCATION_PROVIDERS) {
                if (mLocationManager.isProviderEnabled(provider)) {
                    LocationListener listener = new LocationListener() {
                        @Override
                        public void onLocationChanged(Location location) {
                            mLocationListenerList.remove(this);
                            mLocationHandler.removeMessages(EVENT_LOCATION_REQUEST_TIMEOUT);
                            onLocationUpdate(new LatLng(location.getLatitude(),
                                    location.getLongitude()));
                        }

                        @Override
                        public void onStatusChanged(String provider, int status, Bundle extras) {}

                        @Override
                        public void onProviderEnabled(String provider) {}

                        @Override
                        public void onProviderDisabled(String provider) {}
                    };
                    mLocationListenerList.add(listener);
                    Log.d(TAG, "Request location single update from " + provider);
                    mLocationManager.requestSingleUpdate(provider, listener, mLooper);
                }
            }
        }

        private boolean isLocationServiceAvailable() {
            if (!hasPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                    && !hasPermission(Manifest.permission.ACCESS_COARSE_LOCATION)) return false;
            for (String provider : LOCATION_PROVIDERS) {
                if (mLocationManager.isProviderEnabled(provider)) return true;
            }
            return false;
        }

        private boolean hasPermission(String permission) {
            return PermissionChecker.checkCallingOrSelfPermissionForDataDelivery(mContext,
                    permission) == PERMISSION_GRANTED;
        }

        private final List<LocationListener> mLocationListenerList = new ArrayList<>();

        private final class LocationHandler extends Handler {
            LocationHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case EVENT_LOCATION_REQUEST_TIMEOUT:
                        if (DBG) Log.d(TAG, "location request timeout");
                        onLocationUpdate(null);
                        break;
                    case EVENT_REQUEST_LOCATION_UPDATE:
                        requestLocationUpdateInternal((LocationUpdateCallback) msg.obj, msg.arg1);
                        break;
                    default:
                        Log.e(TAG, "Unsupported message type " + msg.what);
                }
            }
        }
    }
}
