/*
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

package com.google.tuningfork;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Choreographer;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import com.google.android.gms.common.GoogleApiAvailability;

public class TFTestActivity extends AppCompatActivity implements Choreographer.FrameCallback, SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }
    public native void initTuningFork();
    public static native void resize(Surface surface, int width, int height);
    public static native void clearSurface();
    public static native void onChoreographer(long t);
    public static native void start();
    public static native void stop();

    private SurfaceView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // gLView = new TFTestGLSurfaceView(this);
        view = new SurfaceView(this);
        setContentView(view);
        view.getHolder().addCallback(this);

        CheckGMS();
        initTuningFork();
    }

    @Override
    protected void onStart() {
        super.onStart();
        start();
        Choreographer.getInstance().postFrameCallback(this);
    }
    @Override
    protected void onStop() {
        super.onStop();
        stop();
    }

    @Override
    public void doFrame(long t) {

        onChoreographer(t);
        Choreographer.getInstance().postFrameCallback(this);
        android.content.res.AssetManager a = getAssets();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // Do nothing here, waiting for surfaceChanged instead
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Surface surface = holder.getSurface();
        resize(surface, width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        clearSurface();
    }

    private void CheckGMS() {

        GoogleApiAvailability availability = GoogleApiAvailability.getInstance();
        int status = availability.isGooglePlayServicesAvailable(this.getApplicationContext());
        int version = GoogleApiAvailability.GOOGLE_PLAY_SERVICES_VERSION_CODE;

        Log.i("Tuningfork Clearcut", "CheckGMS status: " + status);
        Log.i("TUningfork Clearcut", "CheckGMS version: " + version);
    }

}
