/*
 * Copyright 2017 The Android Open Source Project
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

import android.app.timedetector.TimeDetector;
import android.os.SystemClock;
import android.util.TimestampedValue;

/**
 * An interface to various time / time zone detection behaviors that should be centralized into a
 * new service.
 */
public interface TimeServiceHelper {

    /**
     * Callback interface for automatic detection enable/disable changes.
     */
    interface Listener {
        /**
         * Automatic time zone detection has been enabled or disabled.
         */
        void onTimeZoneDetectionChange(boolean enabled);
    }

    /**
     * Sets a listener that will be called when the automatic time / time zone detection setting
     * changes.
     */
    void setListener(Listener listener);

    /**
     * Returns the same value as {@link System#currentTimeMillis()}.
     */
    long currentTimeMillis();

    /**
     * Returns the same value as {@link SystemClock#elapsedRealtime()}.
     */
    long elapsedRealtime();

    /**
     * Returns true if the device has an explicit time zone set.
     */
    boolean isTimeZoneSettingInitialized();

    /**
     * Returns true if automatic time zone detection is enabled in settings.
     */
    boolean isTimeZoneDetectionEnabled();

    /**
     * Set the device time zone and send out a sticky broadcast so the system can
     * determine if the timezone was set by the carrier.
     *
     * @param zoneId timezone set by carrier
     */
    void setDeviceTimeZone(String zoneId);

    /**
     * Suggest the time to the {@link TimeDetector}.
     *
     * @param signalTimeMillis the signal time as received from the network
     */
    void suggestDeviceTime(TimestampedValue<Long> signalTimeMillis);
}
