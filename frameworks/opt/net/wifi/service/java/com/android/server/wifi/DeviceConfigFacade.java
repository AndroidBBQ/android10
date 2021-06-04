/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.android.server.wifi;

import android.provider.DeviceConfig;
import android.util.ArraySet;

import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;

/**
 * This class allows getting all configurable flags from DeviceConfig.
 */
public class DeviceConfigFacade {
    private static final int DEFAULT_ABNORMAL_CONNECTION_DURATION_MS =
            (int) TimeUnit.SECONDS.toMillis(30);
    private static final String NAMESPACE = "wifi";
    // Default duration for evaluating Wifi condition to trigger a data stall
    // measured in milliseconds
    public static final int DEFAULT_DATA_STALL_DURATION_MS = 1500;
    // Default threshold of Tx throughput below which to trigger a data stall measured in Mbps
    public static final int DEFAULT_DATA_STALL_TX_TPUT_THR_MBPS = 2;
    // Default threshold of Rx throughput below which to trigger a data stall measured in Mbps
    public static final int DEFAULT_DATA_STALL_RX_TPUT_THR_MBPS = 2;
    // Default threshold of Tx packet error rate above which to trigger a data stall in percentage
    public static final int DEFAULT_DATA_STALL_TX_PER_THR = 90;
    // Default threshold of CCA level above which to trigger a data stall in percentage
    public static final int DEFAULT_DATA_STALL_CCA_LEVEL_THR = 100;

    /**
     * Gets the feature flag for reporting abnormally long connections.
     */
    public boolean isAbnormalConnectionBugreportEnabled() {
        return DeviceConfig.getBoolean(NAMESPACE, "abnormal_connection_bugreport_enabled", false);
    }

    /**
     * Gets the threshold for classifying abnormally long connections.
     */
    public int getAbnormalConnectionDurationMs() {
        return DeviceConfig.getInt(NAMESPACE, "abnormal_connection_duration_ms",
                DEFAULT_ABNORMAL_CONNECTION_DURATION_MS);
    }

    /**
     * Adds a listener that will be run on the specified executor.
     * @param executor
     * @param onPropertiesChangedListener
     */
    public void addOnPropertiesChangedListener(Executor executor,
            DeviceConfig.OnPropertiesChangedListener onPropertiesChangedListener) {
        DeviceConfig.addOnPropertiesChangedListener(NAMESPACE, executor,
                onPropertiesChangedListener);
    }

    /**
     * Gets the duration of evaluating Wifi condition to trigger a data stall.
     */
    public int getDataStallDurationMs() {
        return DeviceConfig.getInt(NAMESPACE, "data_stall_duration_ms",
                DEFAULT_DATA_STALL_DURATION_MS);
    }

    /**
     * Gets the threshold of Tx throughput below which to trigger a data stall.
     */
    public int getDataStallTxTputThrMbps() {
        return DeviceConfig.getInt(NAMESPACE, "data_stall_tx_tput_thr_mbps",
                DEFAULT_DATA_STALL_TX_TPUT_THR_MBPS);
    }

    /**
     * Gets the threshold of Rx throughput below which to trigger a data stall.
     */
    public int getDataStallRxTputThrMbps() {
        return DeviceConfig.getInt(NAMESPACE, "data_stall_rx_tput_thr_mbps",
                DEFAULT_DATA_STALL_RX_TPUT_THR_MBPS);
    }

    /**
     * Gets the threshold of Tx packet error rate above which to trigger a data stall.
     */
    public int getDataStallTxPerThr() {
        return DeviceConfig.getInt(NAMESPACE, "data_stall_tx_per_thr",
                DEFAULT_DATA_STALL_TX_PER_THR);
    }

    /**
     * Gets the threshold of CCA level above which to trigger a data stall.
     */
    public int getDataStallCcaLevelThr() {
        return DeviceConfig.getInt(NAMESPACE, "data_stall_cca_level_thr",
                DEFAULT_DATA_STALL_CCA_LEVEL_THR);
    }

    /**
     * Gets the Set of SSIDs in the flaky SSID hotlist.
     */
    public Set<String> getRandomizationFlakySsidHotlist() {
        String ssidHotlist = DeviceConfig.getString(NAMESPACE,
                "randomization_flaky_ssid_hotlist", "");
        Set<String> result = new ArraySet<String>();
        String[] ssidHotlistArray = ssidHotlist.split(",");
        for (int i = 0; i < ssidHotlistArray.length; i++) {
            String cur = ssidHotlistArray[i];
            if (cur.length() == 0) {
                continue;
            }
            // Make sure the SSIDs are quoted. Server side should not quote ssids.
            result.add("\"" + cur + "\"");
        }
        return result;
    }
}
