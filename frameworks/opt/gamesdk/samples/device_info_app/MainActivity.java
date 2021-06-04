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

package com.google.deviceinfotest;

import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.os.Build;

import com.google.androidgamesdk.DeviceInfoProto;
import com.google.androidgamesdk.DeviceInfoJni;

public class MainActivity extends Activity {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    Button sample_button = (Button) findViewById(R.id.sample_button);
    sample_button.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        TextView tv = (TextView) findViewById(R.id.sample_text);
        String msg = "Fingerprint(JAVA):\n" + Build.FINGERPRINT;
        try{
          DeviceInfoProto.InfoWithErrors proto;
          byte[] nativeBytes = DeviceInfoJni.getProtoSerialized();
          proto = DeviceInfoProto.InfoWithErrors.parseFrom(nativeBytes);

          DeviceInfoProto.Info info = proto.getInfo();
          msg += "\nFingerprint(ro.build.fingerprint):\n" + info.getRoBuildFingerprint();
          msg += "\nro_chipname:\n" + info.getRoChipname();
          msg += "\nro_board_platform:\n" + info.getRoBoardPlatform();
          msg += "\nro_product_board:\n" + info.getRoProductBoard();
          msg += "\nro_mediatek_platform:\n" + info.getRoMediatekPlatform();
          msg += "\nro_arch:\n" + info.getRoArch();
          msg += "\nro_build_version_sdk:\n" + info.getRoBuildVersionSdk();
        }catch(Exception e){
          android.util.Log.e("device_info", "could not create proto.", e);
        }
        tv.setText(msg);
      }
    });
  }
}
