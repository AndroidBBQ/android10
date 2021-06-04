/*
 * Copyright (C) 2018 The Android Open Source Project
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

#pragma once

#include <system/audio.h>

namespace android {
/**
 * @brief AudioProductStrategies hard coded array of strategies to fill new engine API contract.
 */
const engineConfig::ProductStrategies gOrderedStrategies = {
    {"STRATEGY_PHONE",
     {
         {"phone", AUDIO_STREAM_VOICE_CALL, "AUDIO_STREAM_VOICE_CALL",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_VOICE_COMMUNICATION, AUDIO_SOURCE_DEFAULT, 0,
            ""}},
         },
         {"sco", AUDIO_STREAM_BLUETOOTH_SCO, "AUDIO_STREAM_BLUETOOTH_SCO",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT, AUDIO_FLAG_SCO,
            ""}},
         }
     },
    },
    {"STRATEGY_SONIFICATION",
     {
         {"ring", AUDIO_STREAM_RING, "AUDIO_STREAM_RING",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION_TELEPHONY_RINGTONE,
            AUDIO_SOURCE_DEFAULT, 0, ""}}
         },
         {"alarm", AUDIO_STREAM_ALARM, "AUDIO_STREAM_ALARM",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_ALARM, AUDIO_SOURCE_DEFAULT, 0, ""}},
         }
     },
    },
    {"STRATEGY_ENFORCED_AUDIBLE",
     {
         {"", AUDIO_STREAM_ENFORCED_AUDIBLE, "AUDIO_STREAM_ENFORCED_AUDIBLE",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT,
            AUDIO_FLAG_AUDIBILITY_ENFORCED, ""}}
         }
     },
    },
    {"STRATEGY_ACCESSIBILITY",
     {
         {"", AUDIO_STREAM_ACCESSIBILITY, "AUDIO_STREAM_ACCESSIBILITY",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_ASSISTANCE_ACCESSIBILITY,
            AUDIO_SOURCE_DEFAULT, 0, ""}}
         }
     },
    },
    {"STRATEGY_SONIFICATION_RESPECTFUL",
     {
         {"", AUDIO_STREAM_NOTIFICATION, "AUDIO_STREAM_NOTIFICATION",
          {
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION, AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION_COMMUNICATION_REQUEST,
               AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION_COMMUNICATION_INSTANT,
               AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION_COMMUNICATION_DELAYED,
               AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_NOTIFICATION_EVENT,
               AUDIO_SOURCE_DEFAULT, 0, ""}
          }
         }
     },
    },
    {"STRATEGY_MEDIA",
     {
         {"music", AUDIO_STREAM_MUSIC, "AUDIO_STREAM_MUSIC",
          {
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_MEDIA, AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_GAME, AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_ASSISTANT, AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE,
               AUDIO_SOURCE_DEFAULT, 0, ""},
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT, 0, ""}
          },
         },
         {"system", AUDIO_STREAM_SYSTEM, "AUDIO_STREAM_SYSTEM",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_ASSISTANCE_SONIFICATION,
            AUDIO_SOURCE_DEFAULT, 0, ""}}
         }
     },
    },
    {"STRATEGY_DTMF",
     {
         {"", AUDIO_STREAM_DTMF, "AUDIO_STREAM_DTMF",
          {
              {AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING,
               AUDIO_SOURCE_DEFAULT, 0, ""}
          }
         }
     },
    },
    {"STRATEGY_TRANSMITTED_THROUGH_SPEAKER",
     {
         {"", AUDIO_STREAM_TTS, "AUDIO_STREAM_TTS",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT,
            AUDIO_FLAG_BEACON, ""}}
         }
     },
    },
    {"STRATEGY_REROUTING",
     {
         {"", AUDIO_STREAM_REROUTING, "AUDIO_STREAM_REROUTING",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT, 0, ""}}
         }
     },
    },
    {"STRATEGY_PATCH",
     {
         {"", AUDIO_STREAM_PATCH, "AUDIO_STREAM_PATCH",
          {{AUDIO_CONTENT_TYPE_UNKNOWN, AUDIO_USAGE_UNKNOWN, AUDIO_SOURCE_DEFAULT, 0, ""}}
         }
     },
    }
};

const engineConfig::Config gDefaultEngineConfig = {
    1.0,
    gOrderedStrategies,
    {},
    {},
    {}
};
} // namespace android
