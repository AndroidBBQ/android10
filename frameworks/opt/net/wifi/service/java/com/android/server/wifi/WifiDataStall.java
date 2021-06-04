/*
 * Copyright 2018 The Android Open Source Project
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

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;

import com.android.server.wifi.nano.WifiMetricsProto.WifiIsUnusableEvent;

/**
 * Looks for Wifi data stalls
 */
public class WifiDataStall {

    // Default minimum number of txBadDelta to trigger data stall
    public static final int MIN_TX_BAD_DEFAULT = 1;
    // Default minimum number of txSuccessDelta to trigger data stall
    // when rxSuccessDelta is 0
    public static final int MIN_TX_SUCCESS_WITHOUT_RX_DEFAULT = 50;
    // Maximum time gap between two WifiLinkLayerStats to trigger a data stall
    public static final long MAX_MS_DELTA_FOR_DATA_STALL = 60 * 1000; // 1 minute
    // Maximum time that a data stall start time stays valid.
    public static final long VALIDITY_PERIOD_OF_DATA_STALL_START_MS = 30 * 1000; // 0.5 minutes
    // Default Tx packet error rate when there is no Tx attempt
    public static final int DEFAULT_TX_PACKET_ERROR_RATE = 20;
    // Default CCA level when CCA stats are not available
    public static final int DEFAULT_CCA_LEVEL = 0;

    private final Context mContext;
    private final DeviceConfigFacade mDeviceConfigFacade;
    private final FrameworkFacade mFacade;
    private final WifiMetrics mWifiMetrics;

    private Handler mHandler;
    private int mMinTxBad;
    private int mMinTxSuccessWithoutRx;
    private int mDataStallDurationMs;
    private int mDataStallTxTputThrMbps;
    private int mDataStallRxTputThrMbps;
    private int mDataStallTxPerThr;
    private int mDataStallCcaLevelThr;
    private int mLastFrequency = -1;
    private String mLastBssid;
    private long mLastTotalRadioOnFreqTimeMs = -1;
    private long mLastTotalCcaBusyFreqTimeMs = -1;
    private long mDataStallStartTimeMs = -1;
    private Clock mClock;
    private boolean mDataStallTx = false;
    private boolean mDataStallRx = false;

    public WifiDataStall(Context context, FrameworkFacade facade, WifiMetrics wifiMetrics,
            DeviceConfigFacade deviceConfigFacade, Looper clientModeImplLooper, Clock clock) {
        mContext = context;
        mDeviceConfigFacade = deviceConfigFacade;
        mFacade = facade;
        mHandler = new Handler(clientModeImplLooper);
        mWifiMetrics = wifiMetrics;
        mClock = clock;
        loadSettings();

        mDeviceConfigFacade.addOnPropertiesChangedListener(
                command -> mHandler.post(command),
                properties -> {
                    updateUsabilityDataCollectionFlags();
                });
    }

    /**
     * Load setting values related to wifi data stall.
     */
    public void loadSettings() {
        mMinTxBad = mFacade.getIntegerSetting(
                mContext, Settings.Global.WIFI_DATA_STALL_MIN_TX_BAD, MIN_TX_BAD_DEFAULT);
        mMinTxSuccessWithoutRx = mFacade.getIntegerSetting(
                mContext, Settings.Global.WIFI_DATA_STALL_MIN_TX_SUCCESS_WITHOUT_RX,
                MIN_TX_SUCCESS_WITHOUT_RX_DEFAULT);
        mWifiMetrics.setWifiDataStallMinTxBad(mMinTxBad);
        mWifiMetrics.setWifiDataStallMinRxWithoutTx(mMinTxSuccessWithoutRx);
        updateUsabilityDataCollectionFlags();
    }

    /**
     * Checks for data stall by looking at tx/rx packet counts
     * @param oldStats second most recent WifiLinkLayerStats
     * @param newStats most recent WifiLinkLayerStats
     * @param wifiInfo WifiInfo for current connection
     * @return trigger type of WifiIsUnusableEvent
     */
    public int checkForDataStall(WifiLinkLayerStats oldStats, WifiLinkLayerStats newStats,
            WifiInfo wifiInfo) {
        if (oldStats == null || newStats == null) {
            mWifiMetrics.resetWifiIsUnusableLinkLayerStats();
            return WifiIsUnusableEvent.TYPE_UNKNOWN;
        }

        long txSuccessDelta = (newStats.txmpdu_be + newStats.txmpdu_bk
                + newStats.txmpdu_vi + newStats.txmpdu_vo)
                - (oldStats.txmpdu_be + oldStats.txmpdu_bk
                + oldStats.txmpdu_vi + oldStats.txmpdu_vo);
        long txRetriesDelta = (newStats.retries_be + newStats.retries_bk
                + newStats.retries_vi + newStats.retries_vo)
                - (oldStats.retries_be + oldStats.retries_bk
                + oldStats.retries_vi + oldStats.retries_vo);
        long txBadDelta = (newStats.lostmpdu_be + newStats.lostmpdu_bk
                + newStats.lostmpdu_vi + newStats.lostmpdu_vo)
                - (oldStats.lostmpdu_be + oldStats.lostmpdu_bk
                + oldStats.lostmpdu_vi + oldStats.lostmpdu_vo);
        long rxSuccessDelta = (newStats.rxmpdu_be + newStats.rxmpdu_bk
                + newStats.rxmpdu_vi + newStats.rxmpdu_vo)
                - (oldStats.rxmpdu_be + oldStats.rxmpdu_bk
                + oldStats.rxmpdu_vi + oldStats.rxmpdu_vo);
        long timeMsDelta = newStats.timeStampInMs - oldStats.timeStampInMs;

        if (timeMsDelta < 0
                || txSuccessDelta < 0
                || txRetriesDelta < 0
                || txBadDelta < 0
                || rxSuccessDelta < 0) {
            // There was a reset in WifiLinkLayerStats
            mWifiMetrics.resetWifiIsUnusableLinkLayerStats();
            return WifiIsUnusableEvent.TYPE_UNKNOWN;
        }

        mWifiMetrics.updateWifiIsUnusableLinkLayerStats(txSuccessDelta, txRetriesDelta,
                txBadDelta, rxSuccessDelta, timeMsDelta);

        if (timeMsDelta < MAX_MS_DELTA_FOR_DATA_STALL) {
            int txLinkSpeed = wifiInfo.getLinkSpeed();
            int rxLinkSpeed = wifiInfo.getRxLinkSpeedMbps();
            boolean isSameBssidAndFreq = mLastBssid == null || mLastFrequency == -1
                    || (mLastBssid.equals(wifiInfo.getBSSID())
                    && mLastFrequency == wifiInfo.getFrequency());
            mLastFrequency = wifiInfo.getFrequency();
            mLastBssid = wifiInfo.getBSSID();

            int ccaLevel = updateCcaLevel(newStats, wifiInfo, isSameBssidAndFreq);
            int txPer = updateTxPer(txSuccessDelta, txRetriesDelta, isSameBssidAndFreq);

            boolean isTxTputLow = false;
            boolean isRxTputLow = false;
            if (txLinkSpeed > 0) {
                int txTput = txLinkSpeed * (100 - txPer) * (100 - ccaLevel);
                isTxTputLow = txTput < mDataStallTxTputThrMbps * 100 * 100;
            }
            if (rxLinkSpeed > 0) {
                int rxTput = rxLinkSpeed * (100 - ccaLevel);
                isRxTputLow = rxTput < mDataStallRxTputThrMbps * 100;
            }

            boolean dataStallTx = isTxTputLow || ccaLevel >= mDataStallCcaLevelThr
                    || txPer >= mDataStallTxPerThr;
            boolean dataStallRx = isRxTputLow || ccaLevel >= mDataStallCcaLevelThr;

            // Data stall event is triggered if there are consecutive Tx and/or Rx data stalls
            // Reset mDataStallStartTimeMs to -1 if currently there is no Tx or Rx data stall
            if (dataStallTx || dataStallRx) {
                mDataStallTx = mDataStallTx || dataStallTx;
                mDataStallRx = mDataStallRx || dataStallRx;
                if (mDataStallStartTimeMs == -1) {
                    mDataStallStartTimeMs = mClock.getElapsedSinceBootMillis();
                    if (mDataStallDurationMs == 0) {
                        mDataStallStartTimeMs = -1;
                        int result = calculateUsabilityEventType(mDataStallTx, mDataStallRx);
                        mDataStallRx = false;
                        mDataStallTx = false;
                        return result;
                    }
                } else {
                    long elapsedTime =  mClock.getElapsedSinceBootMillis() - mDataStallStartTimeMs;
                    if (elapsedTime >= mDataStallDurationMs) {
                        mDataStallStartTimeMs = -1;
                        if (elapsedTime <= VALIDITY_PERIOD_OF_DATA_STALL_START_MS) {
                            int result = calculateUsabilityEventType(mDataStallTx, mDataStallRx);
                            mDataStallRx = false;
                            mDataStallTx = false;
                            return result;
                        } else {
                            mDataStallTx = false;
                            mDataStallRx = false;
                        }
                    } else {
                        // No need to do anything.
                    }
                }
            } else {
                mDataStallStartTimeMs = -1;
                mDataStallTx = false;
                mDataStallRx = false;
            }
        }

        return WifiIsUnusableEvent.TYPE_UNKNOWN;
    }

    private int updateCcaLevel(WifiLinkLayerStats newStats, WifiInfo wifiInfo,
            boolean isSameBssidAndFreq) {
        WifiLinkLayerStats.ChannelStats statsMap = newStats.channelStatsMap.get(mLastFrequency);
        if (statsMap == null || !isSameBssidAndFreq) {
            return DEFAULT_CCA_LEVEL;
        }
        int radioOnTimeDelta = (int) (statsMap.radioOnTimeMs - mLastTotalRadioOnFreqTimeMs);
        int ccaBusyTimeDelta = (int) (statsMap.ccaBusyTimeMs - mLastTotalCcaBusyFreqTimeMs);
        mLastTotalRadioOnFreqTimeMs = statsMap.radioOnTimeMs;
        mLastTotalCcaBusyFreqTimeMs = statsMap.ccaBusyTimeMs;

        boolean isCcaValid = (radioOnTimeDelta > 0) && (ccaBusyTimeDelta >= 0)
                && (ccaBusyTimeDelta <= radioOnTimeDelta);
        // Update CCA level only if CCA stats are valid.
        if (!isCcaValid) {
            return DEFAULT_CCA_LEVEL;
        }
        return (int) (ccaBusyTimeDelta * 100 / radioOnTimeDelta);
    }

    private int updateTxPer(long txSuccessDelta, long txRetriesDelta, boolean isSameBssidAndFreq) {
        if (!isSameBssidAndFreq) {
            return DEFAULT_TX_PACKET_ERROR_RATE;
        }
        long txAttempts = txSuccessDelta + txRetriesDelta;
        if (txAttempts <= 0) {
            return DEFAULT_TX_PACKET_ERROR_RATE;
        }
        return (int) (txRetriesDelta * 100 / txAttempts);
    }

    private int calculateUsabilityEventType(boolean dataStallTx, boolean dataStallRx) {
        int result = WifiIsUnusableEvent.TYPE_UNKNOWN;
        if (dataStallTx && dataStallRx) {
            result = WifiIsUnusableEvent.TYPE_DATA_STALL_BOTH;
        } else if (dataStallTx) {
            result = WifiIsUnusableEvent.TYPE_DATA_STALL_BAD_TX;
        } else if (dataStallRx) {
            result = WifiIsUnusableEvent.TYPE_DATA_STALL_TX_WITHOUT_RX;
        }
        mWifiMetrics.logWifiIsUnusableEvent(result);
        return result;
    }

    private void updateUsabilityDataCollectionFlags() {
        mDataStallDurationMs = mDeviceConfigFacade.getDataStallDurationMs();
        mDataStallTxTputThrMbps = mDeviceConfigFacade.getDataStallTxTputThrMbps();
        mDataStallRxTputThrMbps = mDeviceConfigFacade.getDataStallRxTputThrMbps();
        mDataStallTxPerThr = mDeviceConfigFacade.getDataStallTxPerThr();
        mDataStallCcaLevelThr = mDeviceConfigFacade.getDataStallCcaLevelThr();

        mWifiMetrics.setDataStallDurationMs(mDataStallDurationMs);
        mWifiMetrics.setDataStallTxTputThrMbps(mDataStallTxTputThrMbps);
        mWifiMetrics.setDataStallRxTputThrMbps(mDataStallRxTputThrMbps);
        mWifiMetrics.setDataStallTxPerThr(mDataStallTxPerThr);
        mWifiMetrics.setDataStallCcaLevelThr(mDataStallCcaLevelThr);
    }
}
