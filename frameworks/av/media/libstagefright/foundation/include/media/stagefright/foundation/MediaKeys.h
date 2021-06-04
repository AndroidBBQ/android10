/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef MEDIA_KEYS_H_

#define MEDIA_KEYS_H_

namespace android {

// When signalling a discontinuity to IStreamListener you can optionally
// specify the type(s) of discontinuity, i.e. if the audio format has changed,
// the video format has changed, time has jumped or any combination thereof.
// To do so, include a non-zero int32_t value under the key
// "kIStreamListenerKeyDiscontinuityMask" when issuing the
// IStreamListener::DISCONTINUITY command.
// If there is a change in audio/video format, The new logical stream
// must start with proper codec initialization
// information for playback to continue, i.e. SPS and PPS in the case
// of AVC video etc.
// If this key is not present, only a time discontinuity is assumed.
// The value should be a bitmask of values from
// ATSParser::DiscontinuityType.
extern const char *const kIStreamListenerKeyDiscontinuityMask;

// When signalling a discontinuity to ATSParser you can optionally
// specify an int64_t PTS timestamp in "extra".
// If present, rendering of data following the discontinuity
// will be suppressed until media time reaches this timestamp.
extern const char *const kATSParserKeyResumeAtPTS;

// When signalling a discontinuity to ATSParser you can optionally
// specify an int64_t PTS timestamp in "extra".
// It indicates the media time (in us) to be associated
// with the next PTS occuring in the stream. The value is of type int64_t.
extern const char *const kATSParserKeyMediaTimeUs;

// When signalling a discontinuity to ATSParser you can optionally
// specify an int64_t PTS timestamp in "extra".
// It indicates the media time (in us) of a recent
// sample from the same content, and is used as a hint for the parser to
// handle PTS wraparound. This is required when a new parser is created
// to continue parsing content from the same timeline.
extern const char *const kATSParserKeyRecentMediaTimeUs;

}  // namespace android

#endif  // MEDIA_KEYS_H_
