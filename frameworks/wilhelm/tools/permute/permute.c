/*
 * Copyright (C) 2010 The Android Open Source Project
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

/** Permute is a host tool to randomly permute an audio file.
 *  It takes as input an ordinary .wav file and produces as output a
 *  permuted .wav file and .map which can be given the seek torture test
 *  located in seekTorture.c.  A build prerequisite is libsndfile;
 *  see installation instructions at http://www.mega-nerd.com/libsndfile/
 *  The format of the .map file is a sequence of lines, each of which is:
 *     seek_position_in_ms  duration_in_ms
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>


/** Global variables */

// command line options

// mean length of each segment of the permutation, in seconds
static double meanSegmentLengthSeconds = 5.0;
// minimum length of each segment of the permutation, in seconds
static double minSegmentLengthSeconds = 1.0;


/** Describes each contiguous segment generated */

typedef struct {
    unsigned mFrameStart;
    unsigned mFrameLength;
    unsigned mPermutedStart;
} Segment;


/** Global state during the split phase */

typedef struct {
    // derived from command line options combined with file properties
    unsigned mMinSegmentLengthFrames;
    //unsigned mMeanSegmentLengthFrames;
    unsigned mSegmentMax;   // maximum number of segments allowed
    unsigned mSegmentCount; // number of segments generated so far
    Segment *mSegmentArray; // storage for the segments [max]
} State;


/** Called by qsort as the comparison handler */

static int qsortCompare(const void *x, const void *y)
{
    const Segment *x_ = (Segment *) x;
    const Segment *y_ = (Segment *) y;
    return x_->mFrameStart - y_->mFrameStart;
}


/** Split the specified range of frames, using the allowed budget of segments.
 *  Returns the actual number of segments consumed.
 */

static unsigned split(State *s, unsigned frameStart, unsigned frameLength, unsigned segmentBudget)
{
    if (frameLength <= 0)
        return 0;
    assert(segmentBudget > 0);
    if ((frameLength <= s->mMinSegmentLengthFrames*2) || (segmentBudget <= 1)) {
        assert(s->mSegmentCount < s->mSegmentMax);
        Segment *seg = &s->mSegmentArray[s->mSegmentCount++];
        seg->mFrameStart = frameStart;
        seg->mFrameLength = frameLength;
        seg->mPermutedStart = ~0;
        return 1;
    }
    // slop is how much wiggle room we have to play with
    unsigned slop = frameLength - s->mMinSegmentLengthFrames*2;
    assert(slop > 0);
    // choose a random cut point within the slop region
    unsigned r = rand() & 0x7FFFFFFF;
    unsigned cut = r % slop;
    unsigned leftStart = frameStart;
    unsigned leftLength = s->mMinSegmentLengthFrames + cut;
    unsigned rightStart = frameStart + leftLength;
    unsigned rightLength = s->mMinSegmentLengthFrames + (slop - cut);
    assert(leftLength + rightLength == frameLength);
    // process the two sides in random order
    assert(segmentBudget >= 2);
    unsigned used;
    if (leftLength <= rightLength) {
        used = split(s, leftStart, leftLength, segmentBudget / 2);
        used += split(s, rightStart, rightLength, segmentBudget - used);
    } else {
        used = split(s, rightStart, rightLength, segmentBudget / 2);
        used += split(s, leftStart, leftLength, segmentBudget - used);
    }
    assert(used >= 2);
    assert(used <= segmentBudget);
    return used;
}


/** Permute the specified input file */

void permute(char *path_in)
{

    // Open the file using libsndfile
    SNDFILE *sf_in;
    SF_INFO sfinfo_in;
    sfinfo_in.format = 0;
    sf_in = sf_open(path_in, SFM_READ, &sfinfo_in);
    if (NULL == sf_in) {
        perror(path_in);
        return;
    }

    // Check if it is a supported file format: must be WAV
    unsigned type = sfinfo_in.format & SF_FORMAT_TYPEMASK;
    switch (type) {
    case SF_FORMAT_WAV:
        break;
    default:
        fprintf(stderr, "%s: unsupported type 0x%X\n", path_in, type);
        goto out;
    }

#if 0
    // Must be 16-bit signed or 8-bit unsigned PCM
    unsigned subtype = sfinfo_in.format & SF_FORMAT_SUBMASK;
    unsigned sampleSizeIn = 0;
    switch (subtype) {
    case SF_FORMAT_PCM_16:
        sampleSizeIn = 2;
        break;
    case SF_FORMAT_PCM_U8:
        sampleSizeIn = 1;
        break;
    default:
        fprintf(stderr, "%s: unsupported subtype 0x%X\n", path_in, subtype);
        goto out;
    }
#endif
    // always read shorts
    unsigned sampleSizeRead = 2;

    // Must be little-endian
    unsigned endianness = sfinfo_in.format & SF_FORMAT_ENDMASK;
    switch (endianness) {
    case SF_ENDIAN_FILE:
    case SF_ENDIAN_LITTLE:
        break;
    default:
        fprintf(stderr, "%s: unsupported endianness 0x%X\n", path_in, endianness);
        goto out;
    }

    // Must be a known sample rate
    switch (sfinfo_in.samplerate) {
    case 8000:
    case 11025:
    case 16000:
    case 22050:
    case 32000:
    case 44100:
    case 48000:
        break;
    default:
        fprintf(stderr, "%s: unsupported sample rate %d\n", path_in, sfinfo_in.samplerate);
        goto out;
    }

    // Must be either stereo or mono
    unsigned frameSizeRead = 0;
    switch (sfinfo_in.channels) {
    case 1:
    case 2:
        frameSizeRead = sampleSizeRead * sfinfo_in.channels;
        break;
    default:
        fprintf(stderr, "%s: unsupported channels %d\n", path_in, sfinfo_in.channels);
        goto out;
    }

    // Duration must be known
    switch (sfinfo_in.frames) {
    case (sf_count_t) 0:
    case (sf_count_t) ~0:
        fprintf(stderr, "%s: unsupported frames %d\n", path_in, (int) sfinfo_in.frames);
        goto out;
    default:
        break;
    }

    // Allocate space to hold the audio data, based on duration
    double durationSeconds = (double) sfinfo_in.frames / (double) sfinfo_in.samplerate;
    State s;
    s.mMinSegmentLengthFrames = minSegmentLengthSeconds * sfinfo_in.samplerate;
    if (s.mMinSegmentLengthFrames <= 0)
        s.mMinSegmentLengthFrames = 1;
    s.mSegmentMax = durationSeconds / meanSegmentLengthSeconds;
    if (s.mSegmentMax <= 0)
        s.mSegmentMax = 1;
    s.mSegmentCount = 0;
    s.mSegmentArray = (Segment *) malloc(sizeof(Segment) * s.mSegmentMax);
    assert(s.mSegmentArray != NULL);
    unsigned used;
    used = split(&s, 0, sfinfo_in.frames, s.mSegmentMax);
    assert(used <= s.mSegmentMax);
    assert(used == s.mSegmentCount);

    // now permute the segments randomly using a bad algorithm
    unsigned i;
    for (i = 0; i < used; ++i) {
        unsigned r = rand() & 0x7FFFFFFF;
        unsigned j = r % used;
        if (j != i) {
            Segment temp = s.mSegmentArray[i];
            s.mSegmentArray[i] = s.mSegmentArray[j];
            s.mSegmentArray[j] = temp;
        }
    }

    // read the entire file into memory
    void *ptr = malloc(sfinfo_in.frames * frameSizeRead);
    assert(NULL != ptr);
    sf_count_t count;
    count = sf_readf_short(sf_in, ptr, sfinfo_in.frames);
    if (count != sfinfo_in.frames) {
        fprintf(stderr, "%s: expected to read %d frames but actually read %d frames\n", path_in,
            (int) sfinfo_in.frames, (int) count);
        goto out;
    }

    // Create a permuted output file
    char *path_out = malloc(strlen(path_in) + 8);
    assert(path_out != NULL);
    strcpy(path_out, path_in);
    strcat(path_out, ".wav");
    SNDFILE *sf_out;
    SF_INFO sfinfo_out;
    memset(&sfinfo_out, 0, sizeof(SF_INFO));
    sfinfo_out.samplerate = sfinfo_in.samplerate;
    sfinfo_out.channels = sfinfo_in.channels;
    sfinfo_out.format = sfinfo_in.format;
    sf_out = sf_open(path_out, SFM_WRITE, &sfinfo_out);
    if (sf_out == NULL) {
        perror(path_out);
        goto out;
    }
    unsigned permutedStart = 0;
    for (i = 0; i < used; ++i) {
        s.mSegmentArray[i].mPermutedStart = permutedStart;
        count = sf_writef_short(sf_out, &((short *) ptr)[sfinfo_in.channels * s.mSegmentArray[i]
            .mFrameStart], s.mSegmentArray[i].mFrameLength);
        if (count != s.mSegmentArray[i].mFrameLength) {
            fprintf(stderr, "%s: expected to write %d frames but actually wrote %d frames\n",
                path_out, (int) s.mSegmentArray[i].mFrameLength, (int) count);
            break;
        }
        permutedStart += s.mSegmentArray[i].mFrameLength;
    }
    assert(permutedStart == sfinfo_in.frames);
    sf_close(sf_out);

    // now create a seek map to let us play this back in a reasonable order
    qsort((void *) s.mSegmentArray, used, sizeof(Segment), qsortCompare);
    char *path_map = malloc(strlen(path_in) + 8);
    assert(path_map != NULL);
    strcpy(path_map, path_in);
    strcat(path_map, ".map");
    FILE *fp_map = fopen(path_map, "w");
    if (fp_map == NULL) {
        perror(path_map);
    } else {
        for (i = 0; i < used; ++i)
            fprintf(fp_map, "%u %u\n", (unsigned) ((s.mSegmentArray[i].mPermutedStart * 1000.0) /
                sfinfo_in.samplerate), (unsigned) ((s.mSegmentArray[i].mFrameLength * 1000.0) /
                sfinfo_in.samplerate));
        fclose(fp_map);
    }

out:
    // Close the input file
    sf_close(sf_in);
}


// main program

int main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; ++i) {
        char *arg = argv[i];

        // process command line options
        if (!strncmp(arg, "-m", 2)) {
            double mval = atof(&arg[2]);
            if (mval >= 0.1 && mval <= 1000.0)
                minSegmentLengthSeconds = mval;
            else
                fprintf(stderr, "%s: invalid value %s\n", argv[0], arg);
            continue;
        }
        if (!strncmp(arg, "-s", 2)) {
            double sval = atof(&arg[2]);
            if (sval >= 0.1 && sval <= 1000.0)
                meanSegmentLengthSeconds = sval;
            else
                fprintf(stderr, "%s: invalid value %s\n", argv[0], arg);
            continue;
        }
        if (!strncmp(arg, "-r", 2)) {
            srand(atoi(&arg[2]));
            continue;
        }
        if (meanSegmentLengthSeconds < minSegmentLengthSeconds)
            meanSegmentLengthSeconds = minSegmentLengthSeconds * 2.0;

        // Permute the file
        permute(arg);

    }
    return EXIT_SUCCESS;
}
