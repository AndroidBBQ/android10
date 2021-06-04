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

#include <hardware/audio.h>
#include <hardware/audio_effect.h>
#include "EffectsFactory.h"

namespace android {
enum {
    SUB_FX_HOST,       // Index of HOST in the descriptor and handle arrays
                       // of the Proxy context
    SUB_FX_OFFLOAD,    // Index of OFFLOAD in the descriptor and handle arrays
                       // of the Proxy context
    SUB_FX_COUNT       // The number of sub effects for a Proxy(1 HW, 1 SW)
};
#if __cplusplus
extern "C" {
#endif

int EffectProxyCreate(const effect_uuid_t *uuid,
                          int32_t             sessionId,
                          int32_t             ioId,
                          effect_handle_t  *pHandle);
int EffectProxyRelease(effect_handle_t handle);
int EffectProxyGetDescriptor(const effect_uuid_t *uuid,
                                 effect_descriptor_t *pDescriptor);
/* Effect Control Interface Implementation: Process */
int Effect_process(effect_handle_t     self,
                            audio_buffer_t         *inBuffer,
                            audio_buffer_t         *outBuffer);

/* Effect Control Interface Implementation: Command */
int Effect_command(effect_handle_t  self,
                            uint32_t            cmdCode,
                            uint32_t            cmdSize,
                            void                *pCmdData,
                            uint32_t            *replySize,
                            void                *pReplyData);
int Effect_getDescriptor(effect_handle_t   self,
                       effect_descriptor_t *pDescriptor);

const struct effect_interface_s gEffectInterface = {
  Effect_process,
  Effect_command,
  Effect_getDescriptor,
  NULL,
};

#define PROXY_REPLY_SIZE_MAX     (64 * 1024) // must be power of two
#define PROXY_REPLY_SIZE_DEFAULT 32          // must be power of two

struct EffectContext {
  const struct effect_interface_s  *common_itfe; // Holds the itfe of the Proxy
  sub_effect_entry_t** sube;                     // Points to the sub effects
  effect_descriptor_t*  desc;                    // Points to the sub effect descriptors
  audio_effect_library_t**  aeli;                // Points to the sub effect aeli
  effect_handle_t       eHandle[SUB_FX_COUNT];   // The effect handles of the sub effects
  int                   index;       // The index that is currently active - HOST or OFFLOAD
  int32_t               sessionId;   // The sessiond in which the effect is created.
                                     // Stored in context to pass on to sub effect creation
  int32_t               ioId;        // The ioId in which the effect is created.
                                     // Stored in context to pass on to sub effect creation
  effect_uuid_t         uuid;        // UUID of the Proxy
  char*                 replyData;   // temporary buffer for non active sub effect command reply
  uint32_t              replySize;   // current size of temporary reply buffer
};

#if __cplusplus
}  // extern "C"
#endif
} //namespace android
