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
 * limitations under the License
 */


package com.google.tuningfork.validation;

import java.util.regex.Pattern;

/** Settings and proto paths inside apk */
final class ApkConfig {

  public static final String ASSETS_DIRECTORY = "assets/tuningfork/";

  public static final String DEV_TUNINGFORK_PROTO = ASSETS_DIRECTORY + "dev_tuningfork.proto";
  public static final String DEV_TUNINGFORK_PROTO_DESCRIPTOR =
      ASSETS_DIRECTORY + "dev_tuningfork.descriptor";
  public static final String TUNINGFORK_SETTINGS = ASSETS_DIRECTORY + "tuningfork_settings.bin";
  public static final Pattern DEV_FIDELITY_PATTERN =
      Pattern.compile(ASSETS_DIRECTORY + "dev_tuningfork_fidelityparams_.{1,15}.bin");
}
