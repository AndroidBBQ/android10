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

package com.prefabulated.bouncyball;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Trace;
import android.preference.ListPreference;
import android.preference.PreferenceManager;
import android.text.Layout;
import android.util.Log;
import android.view.Choreographer;
import android.view.Display;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.GridLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayDeque;
import java.util.Locale;
import java.util.Queue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;

public class OrbitActivity extends AppCompatActivity implements Choreographer.FrameCallback, SurfaceHolder.Callback {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final long ONE_MS_IN_NS = 1000000;
    private static final long ONE_S_IN_NS = 1000 * ONE_MS_IN_NS;

    private static final String LOG_TAG = "OrbitActivity.java";

    private void configureGridCell(AppCompatTextView cell) {
        // A bunch of optimizations to reduce the cost of setText
        cell.setEllipsize(null);
        cell.setMaxLines(1);
        cell.setSingleLine(true);
        if (android.os.Build.VERSION.SDK_INT >= 23) {
            cell.setBreakStrategy(Layout.BREAK_STRATEGY_SIMPLE);
            cell.setHyphenationFrequency(Layout.HYPHENATION_FREQUENCY_NONE);
        }

        cell.setTextAppearance(getApplicationContext(), R.style.InfoTextSmall);
        cell.setText("0");
    }

    private void buildChoreographerInfoGrid() {
        GridLayout infoGrid = findViewById(R.id.choreographer_info_grid);

        // Add the header row
        GridLayout.Spec headerRowSpec = GridLayout.spec(0);
        for (int column = 0; column < mChoreographerInfoGrid[0].length; ++column) {
            AppCompatTextView cell = new AppCompatTextView(getApplicationContext());
            GridLayout.Spec colSpec = GridLayout.spec(column, 1.0f);
            cell.setLayoutParams(new GridLayout.LayoutParams(headerRowSpec, colSpec));
            configureGridCell(cell);

            if (column == 0) {
                cell.setText("");
            } else {
                cell.setText(String.format(Locale.US, "%d", column - 1));
            }
            infoGrid.addView(cell);
            mChoreographerInfoGrid[0][column] = cell;
        }

        // Add the data rows
        for (int row = 1; row < mChoreographerInfoGrid.length; ++row) {
            GridLayout.Spec rowSpec = GridLayout.spec(row);

            for (int column = 0; column < mChoreographerInfoGrid[row].length; ++column) {
                AppCompatTextView cell = new AppCompatTextView(getApplicationContext());
                GridLayout.Spec colSpec = GridLayout.spec(column, 1.0f);
                cell.setLayoutParams(new GridLayout.LayoutParams(rowSpec, colSpec));
                cell.setTextAppearance(getApplicationContext(), R.style.InfoTextSmall);
                configureGridCell(cell);

                if (column == 0) {
                    switch (row) {
                        case 1:
                            cell.setText(R.string.grid_1s_arr);
                            break;
                        case 2:
                            cell.setText(R.string.grid_1s_ts);
                            break;
                        case 3:
                            cell.setText(R.string.grid_1m_arr);
                            break;
                        case 4:
                            cell.setText(R.string.grid_1m_ts);
                            break;
                    }
                }
                infoGrid.addView(cell);
                mChoreographerInfoGrid[row][column] = cell;
            }
        }

        for (TextView[] row : mChoreographerInfoGrid) {
            for (TextView column : row) {
                column.setWidth(infoGrid.getWidth() / infoGrid.getColumnCount());
            }
        }
    }

    private void buildSwappyStatsGrid() {
        GridLayout infoGrid = findViewById(R.id.swappy_stats_grid);

        // Add the header row
        GridLayout.Spec headerRowSpec = GridLayout.spec(0);
        for (int column = 0; column < mSwappyGrid[0].length; ++column) {
            AppCompatTextView cell = new AppCompatTextView(getApplicationContext());
            GridLayout.Spec colSpec = GridLayout.spec(column, 1.0f);
            cell.setLayoutParams(new GridLayout.LayoutParams(headerRowSpec, colSpec));
            configureGridCell(cell);

            if (column == 0) {
                cell.setText("");
            } else {
                cell.setText(String.format(Locale.US, "%d", column - 1));
            }
            infoGrid.addView(cell);
            mSwappyGrid[0][column] = cell;
        }

        // Add the data rows
        for (int row = 1; row < mSwappyGrid.length; ++row) {
            GridLayout.Spec rowSpec = GridLayout.spec(row);

            for (int column = 0; column < mSwappyGrid[row].length; ++column) {
                AppCompatTextView cell = new AppCompatTextView(getApplicationContext());
                GridLayout.Spec colSpec = GridLayout.spec(column, 1.0f);
                cell.setLayoutParams(new GridLayout.LayoutParams(rowSpec, colSpec));
                cell.setTextAppearance(getApplicationContext(), R.style.InfoTextSmall);
                configureGridCell(cell);

                if (column == 0) {
                    switch (row) {
                        case 1:
                            cell.setText(R.string.idle_frames);
                            break;
                        case 2:
                            cell.setText(R.string.late_frames);
                            break;
                        case 3:
                            cell.setText(R.string.offset_frames);
                            break;
                        case 4:
                            cell.setText(R.string.latency_frames);
                            break;
                    }
                } else {
                    cell.setText("0%");
                }
                infoGrid.addView(cell);
                mSwappyGrid[row][column] = cell;
            }
        }

        for (TextView[] row : mSwappyGrid) {
            for (TextView column : row) {
                column.setWidth(infoGrid.getWidth() / infoGrid.getColumnCount());
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_orbit);

        // Get display metrics

        WindowManager wm = getWindowManager();
        Display display = wm.getDefaultDisplay();
        float refreshRateHz = display.getRefreshRate();
        Log.i(LOG_TAG, String.format("Refresh rate: %.1f Hz", refreshRateHz));
        long refreshPeriodNanos = (long) (ONE_S_IN_NS / refreshRateHz);
        long appVsyncOffsetNanos = display.getAppVsyncOffsetNanos();
        long sfVsyncOffsetNanos = refreshPeriodNanos - (display.getPresentationDeadlineNanos() - ONE_MS_IN_NS);

        // Initialize UI

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(this);

        mInfoOverlay = findViewById(R.id.info_overlay);
        mInfoOverlay.setBackgroundColor(0x80000000);

        buildChoreographerInfoGrid();
        buildSwappyStatsGrid();

        TextView appOffsetView = findViewById(R.id.app_offset);
        appOffsetView.setText(String.format(Locale.US, "App Offset: %.1f ms", appVsyncOffsetNanos / (float) ONE_MS_IN_NS));
        TextView sfOffsetView = findViewById(R.id.sf_offset);
        sfOffsetView.setText(String.format(Locale.US, "SF Offset: %.1f ms", sfVsyncOffsetNanos / (float) ONE_MS_IN_NS));

        // Initialize the native renderer

        nInit();

        nSetPreference("refresh_period", String.valueOf(refreshPeriodNanos));
    }

    private void infoOverlayToggle() {
        if (mInfoOverlay == null) {
            return;
        }

        mInfoOverlayEnabled = !mInfoOverlayEnabled;
        if (mInfoOverlayEnabled) {
            mInfoOverlay.setVisibility(View.VISIBLE);
            mInfoOverlayButton.setIcon(R.drawable.ic_info_solid_white_24dp);
        } else {
            mInfoOverlay.setVisibility(View.INVISIBLE);
            mInfoOverlayButton.setIcon(R.drawable.ic_info_outline_white_24dp);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_orbit, menu);

        mInfoOverlayButton = menu.findItem(R.id.info_overlay_button);
        if (mInfoOverlayButton != null) {
            mInfoOverlayButton.setOnMenuItemClickListener((MenuItem item) -> {
                infoOverlayToggle();
                return true;
            });
        }

        MenuItem settingsButton = menu.findItem(R.id.settings_item);
        if (settingsButton != null) {
            settingsButton.setOnMenuItemClickListener((MenuItem) -> {
                Intent intent = new Intent();
                intent.setClass(getApplicationContext(), SettingsActivity.class);
                startActivity(intent);
                return true;
            });
        }

        return true;
    }

    @Override
    protected void onStart() {
        super.onStart();

        SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        for (String key : sharedPreferences.getAll().keySet()) {
            if (key.equals("use_affinity")) {
                nSetPreference(key, sharedPreferences.getBoolean(key, true) ? "true" : "false");
                continue;
            } else if (key.equals("hot_pocket")) {
                nSetPreference(key, sharedPreferences.getBoolean(key, false) ? "true" : "false");
                continue;
            } else if (key.equals("use_auto_swap_interval")) {
                nSetAutoSwapInterval(sharedPreferences.getBoolean(key, true));
                continue;
            } else if (key.equals("workload")) {
                nSetWorkload(sharedPreferences.getInt(key, 0));
                continue;
            }
            nSetPreference(key, sharedPreferences.getString(key, null));
        }

        mIsRunning = true;
        nStart();
        Choreographer.getInstance().postFrameCallback(this);
    }

    @Override
    protected void onStop() {
        super.onStop();

        mIsRunning = false;
        nStop();
        clearChoreographerInfo();
    }

    private int deltaToBin(long delta) {
        long framePeriodNanos = 16666666;
        return (int) ((delta + framePeriodNanos / 2) / framePeriodNanos);
    }

    private void clearChoreographerInfo() {
        for (int i = 0; i < CHOREOGRAPHER_INFO_BIN_COUNT; ++i) {
            mArrivalBinsLastSecond[i] = 0;
            mArrivalBinsLastMinute[i] = 0;
            mTimestampBinsLastSecond[i] = 0;
            mTimestampBinsLastMinute[i] = 0;
        }
        mArrivalBinQueue.clear();
        mTimestampBinQueue.clear();
    }

    private void updateChoregrapherInfoBin(int row, int bin, int value) {
        if (value == mLastChoreographerInfoValues[row - 1][bin]) {
            return;
        }

        mLastChoreographerInfoValues[row - 1][bin] = value;
        mChoreographerInfoGrid[row][bin + 1].setText(String.valueOf(value));
    }

    private void updateSwappyStatsBin(int row, int bin, int value) {
        if (value == mLastSwappyStatsValues[row - 1][bin]) {
            return;
        }

        mLastSwappyStatsValues[row - 1][bin] = value;
        mSwappyGrid[row][bin + 1].setText(String.valueOf(value) + "%");
    }

    private void dumpBins() {
        Trace.beginSection("dumpBins");

        Trace.beginSection("addToQueues");
        mArrivalBinQueue.add(mArrivalBinsLastSecond.clone());
        mTimestampBinQueue.add(mTimestampBinsLastSecond.clone());
        Trace.endSection();

        Trace.beginSection("updateMinuteBins");
        for (int bin = 0; bin < CHOREOGRAPHER_INFO_BIN_COUNT; ++bin) {
            mArrivalBinsLastMinute[bin] += mArrivalBinsLastSecond[bin];
            mTimestampBinsLastMinute[bin] += mTimestampBinsLastSecond[bin];
        }
        Trace.endSection();

        Trace.beginSection("pruneQueue");
        if (mArrivalBinQueue.size() > 60) {
            int[] oldestArrivalBin = mArrivalBinQueue.remove();
            int[] oldestTimestampBin = mTimestampBinQueue.remove();
            for (int bin = 0; bin < CHOREOGRAPHER_INFO_BIN_COUNT; ++bin) {
                mArrivalBinsLastMinute[bin] -= oldestArrivalBin[bin];
                mTimestampBinsLastMinute[bin] -= oldestTimestampBin[bin];
            }
        }
        Trace.endSection();

        Trace.beginSection("updateInfoGrid");
        for (int bin = 0; bin < CHOREOGRAPHER_INFO_BIN_COUNT; ++bin) {
            updateChoregrapherInfoBin(1, bin, mArrivalBinsLastSecond[bin]);
            updateChoregrapherInfoBin(2, bin, mTimestampBinsLastSecond[bin]);
            updateChoregrapherInfoBin(3, bin, mArrivalBinsLastMinute[bin]);
            updateChoregrapherInfoBin(4, bin, mTimestampBinsLastMinute[bin]);
        }
        Trace.endSection();

        Trace.beginSection("updateSwappyStatsGrid");
        for (int stat = 0; stat < 4; ++stat) {
            for (int bin = 0; bin < SWAPPY_STATS_BIN_COUNT; ++bin) {
                updateSwappyStatsBin(stat +1, bin, nGetSwappyStats(stat, bin));
            }
        }
        TextView appOffsetView = findViewById(R.id.swappy_stats);
        appOffsetView.setText(String.format(Locale.US, "SwappyStats: %d Total Frames", nGetSwappyStats(-1, 0)));
        Trace.endSection();

        Trace.beginSection("clearSecondBins");
        for (int bin = 0; bin < CHOREOGRAPHER_INFO_BIN_COUNT; ++bin) {
            mArrivalBinsLastSecond[bin] = 0;
            mTimestampBinsLastSecond[bin] = 0;
        }
        Trace.endSection();

        Trace.endSection();
    }

    @Override
    public void doFrame(long frameTimeNanos) {
        Trace.beginSection("doFrame");

        TextView fpsView = findViewById(R.id.fps);
        fpsView.setText(String.format(Locale.US, "FPS: %.1f", nGetAverageFps()));

        long now = System.nanoTime();

        if (mIsRunning) {
            Trace.beginSection("Requesting callback");
            Choreographer.getInstance().postFrameCallback(this);
            Trace.endSection();
        }

        long arrivalDelta = now - mLastArrivalTime;
        long timestampDelta = now - mLastFrameTimestamp;
        mLastArrivalTime = now;
        mLastFrameTimestamp = frameTimeNanos;

        mArrivalBinsLastSecond[Math.min(deltaToBin(arrivalDelta), mArrivalBinsLastSecond.length - 1)] += 1;
        mTimestampBinsLastSecond[Math.min(deltaToBin(timestampDelta), mTimestampBinsLastSecond.length - 1)] += 1;

        if (now - mLastDumpTime > 1000000000) {
            dumpBins();
            // Trim off excess precision so we don't drift forward over time
            mLastDumpTime = now - (now % 1000000000);
        }

        Trace.endSection();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // Do nothing here, waiting for surfaceChanged instead
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Surface surface = holder.getSurface();
        nSetSurface(surface, width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        nClearSurface();
    }

    public native void nInit();
    public native void nSetSurface(Surface surface, int width, int height);
    public native void nClearSurface();
    public native void nStart();
    public native void nStop();
    public native void nSetPreference(String key, String value);
    public native void nSetWorkload(int load);
    public native void nSetAutoSwapInterval(boolean enabled);
    public native float nGetAverageFps();
    public native int nGetSwappyStats(int stat, int bin);

    private MenuItem mInfoOverlayButton;

    private LinearLayout mInfoOverlay;
    private final TextView[][] mChoreographerInfoGrid = new TextView[5][7];
    private final int[][] mLastChoreographerInfoValues = new int[4][6];

    private final TextView[][] mSwappyGrid = new TextView[5][7];
    private final int[][] mLastSwappyStatsValues = new int[4][6];

    private boolean mIsRunning;
    private boolean mInfoOverlayEnabled = false;

    private final ExecutorService mChoreographerExecutor = Executors.newSingleThreadExecutor();
    private final Handler mUIThreadHandler = new Handler(Looper.getMainLooper());

    private static final int CHOREOGRAPHER_INFO_BIN_COUNT = 6;
    private static final int SWAPPY_STATS_BIN_COUNT = 6;
    private long mLastDumpTime;
    private long mLastArrivalTime;
    private long mLastFrameTimestamp;
    private final int[] mArrivalBinsLastSecond = new int[CHOREOGRAPHER_INFO_BIN_COUNT];
    private final int[] mArrivalBinsLastMinute = new int[CHOREOGRAPHER_INFO_BIN_COUNT];
    private final Queue<int[]> mArrivalBinQueue = new ArrayDeque<>(60);
    private final int[] mTimestampBinsLastSecond = new int[CHOREOGRAPHER_INFO_BIN_COUNT];
    private final int[] mTimestampBinsLastMinute = new int[CHOREOGRAPHER_INFO_BIN_COUNT];
    private final Queue<int[]> mTimestampBinQueue = new ArrayDeque<>(60);
}
