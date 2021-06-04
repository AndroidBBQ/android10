/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef __handlers_h
#define __handlers_h

#define IObject struct Object_interface
IObject;

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned handler_MediaPlayer_gain(IObject *thiz);
#ifdef ANDROID
extern unsigned handler_MediaPlayer_transport(IObject *thiz);
extern unsigned handler_MediaPlayer_position(IObject *thiz);
extern unsigned handler_MediaPlayer_abq_enqueue(IObject *thiz);
extern unsigned handler_MediaPlayer_play_state(IObject *thiz);
extern unsigned handler_AudioPlayer_gain(IObject *thiz);
extern unsigned handler_AudioPlayer_transport(IObject *thiz);
extern unsigned handler_AudioPlayer_position(IObject *thiz);
extern unsigned handler_AudioPlayer_bq_enqueue(IObject *thiz);
extern unsigned handler_AudioPlayer_abq_enqueue(IObject *thiz);
extern unsigned handler_AudioPlayer_play_state(IObject *thiz);
extern unsigned handler_AudioRecorder_transport(IObject *thiz);
extern unsigned handler_MidiPlayer_gain(IObject *thiz);
extern unsigned handler_MidiPlayer_position(IObject *thiz);
extern unsigned handler_OutputMix_gain(IObject *thiz);
#else
#define handler_MediaPlayer_gain        NULL
#define handler_MediaPlayer_transport   NULL
#define handler_MediaPlayer_position    NULL
#define handler_MediaPlayer_abq_enqueue NULL
#define handler_MediaPlayer_play_state  NULL
#define handler_AudioPlayer_transport   NULL
#define handler_AudioPlayer_position    NULL
#define handler_AudioPlayer_bq_enqueue  NULL
#define handler_AudioPlayer_abq_enqueue NULL
#define handler_AudioPlayer_play_state  NULL
#define handler_AudioRecorder_transport NULL
#define handler_MidiPlayer_gain         NULL
#define handler_MidiPlayer_position     NULL
#define handler_OutputMix_gain          NULL
#endif

#ifdef __cplusplus
}
#endif


/* Table entry for an attribute update handler.
 *
 * If the Object ID table index matches the object's ID, and the attribute table index
 * match any of the updated attributes, then the specified handler
 * is called.  The handler is called with the object's mutex locked,
 * and should return with mutex locked.  The handler returns a bitmask of the attributes which
 * were actually handled.  This is normally the same as the attribute index,
 * but could be a subset (including 0) if the handler discovers that the
 * update is too complex to be handled synchronously and so must
 * be deferred to the sync thread.  Note that this direct lookup table organization
 * gives O(1) search.
 */

typedef unsigned (*AttributeHandler)(IObject *thiz);

extern const AttributeHandler handlerTable[][ATTR_INDEX_MAX];

#undef IObject

#endif // !defined(__handlers_h)
