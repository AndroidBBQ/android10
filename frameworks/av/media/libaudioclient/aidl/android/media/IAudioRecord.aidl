/*
 * Copyright 2016 The Android Open Source Project
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

package android.media;

import android.media.MicrophoneInfo;

/* Native code must specify namespace media (media::IAudioRecord) when referring to this class */
interface IAudioRecord {

  /* After it's created the track is not active. Call start() to
   * make it active.
   */
  void start(int /*AudioSystem::sync_event_t*/ event,
             int /*audio_session_t*/ triggerSession);

  /* Stop a track. If set, the callback will cease being called and
   * obtainBuffer will return an error. Buffers that are already released
   * will be processed, unless flush() is called.
   */
  void stop();

  /* Get a list of current active microphones.
   */
  void getActiveMicrophones(out MicrophoneInfo[] activeMicrophones);

  /* Set the microphone direction (for processing).
   */
  void setPreferredMicrophoneDirection(int /*audio_microphone_direction_t*/ direction);

  /* Set the microphone zoom (for processing).
   */
  void setPreferredMicrophoneFieldDimension(float zoom);
}
