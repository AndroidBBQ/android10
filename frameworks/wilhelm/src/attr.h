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

// Attributes

// bit indices, used for fast lookup into handler table

#define ATTR_INDEX_GAIN        0 // volume: volume, stereo position, mute
                                 // mute solo: channel mute, channel solo
                                 // effect send: set direct level
#define ATTR_INDEX_TRANSPORT   1 // play: callback events mask,
                                 //       marker position, position update period
                                 //       (currently looping is done differently, not by attributes)
                                 // recorder: duration limit, events mask,
                                 //           marker position, position update period
#define ATTR_INDEX_POSITION    2 // requested position (a.k.a. seek position)
#define ATTR_INDEX_BQ_ENQUEUE  3 // (buffer queue non-empty and in playing state) became true
#define ATTR_INDEX_ABQ_ENQUEUE 4 // Android buffer queue became non-empty and in playing state
#define ATTR_INDEX_PLAY_STATE  5 // play: play state
#define ATTR_INDEX_UNUSED6     6 // reserved for future use
#define ATTR_INDEX_UNUSED7     7 // reserved for future use
#define ATTR_INDEX_MAX         8 // total number of bits used so far

// bit masks, used with unlock_exclusive_attributes

#define ATTR_NONE        0
#define ATTR_GAIN        (1 << ATTR_INDEX_GAIN)
#define ATTR_TRANSPORT   (1 << ATTR_INDEX_TRANSPORT)
#define ATTR_POSITION    (1 << ATTR_INDEX_POSITION)
#define ATTR_BQ_ENQUEUE  (1 << ATTR_INDEX_BQ_ENQUEUE)
#define ATTR_ABQ_ENQUEUE (1 << ATTR_INDEX_ABQ_ENQUEUE)
#define ATTR_PLAY_STATE  (1 << ATTR_INDEX_PLAY_STATE)
#define ATTR_UNUSED6     (1 << ATTR_INDEX_UNUSED6)
#define ATTR_UNUSED7     (1 << ATTR_INDEX_UNUSED7)
