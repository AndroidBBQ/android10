/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef C2WORK_H_

#define C2WORK_H_

#include <C2Buffer.h>
#include <C2Param.h>

#include <memory>
#include <list>
#include <vector>

#include <stdint.h>
#include <stdbool.h>

/// \defgroup work Work and data processing
/// @{

/**
 * Information describing the reason a parameter settings may fail, or
 * may be overriden.
 */
struct C2SettingResult {
    enum Failure : uint32_t {
        /* parameter failures below */
        BAD_TYPE,   ///< parameter is not supported
        BAD_PORT,   ///< parameter is not supported on the specific port
        BAD_INDEX,  ///< parameter is not supported on the specific stream
        READ_ONLY,  ///< parameter is read-only and cannot be set
        MISMATCH,   ///< parameter mismatches input data

        /* field failures below */
        BAD_VALUE,  ///< strict parameter does not accept value for the field at all
        CONFLICT,   ///< strict parameter field value is in conflict with an/other setting(s)

        /// parameter field is out of range due to other settings (this failure mode
        /// can only be used for strict calculated parameters)
        UNSUPPORTED,

        /// field does not access the requested parameter value at all. It has been corrected to
        /// the closest supported value. This failure mode is provided to give guidance as to what
        /// are the currently supported values for this field (which may be a subset of the at-all-
        /// potential values)
        INFO_BAD_VALUE,

        /// requested parameter value is in conflict with an/other setting(s)
        /// and has been corrected to the closest supported value. This failure
        /// mode is given to provide guidance as to what are the currently supported values as well
        /// as to optionally provide suggestion to the client as to how to enable the requested
        /// parameter value.
        INFO_CONFLICT,
    };

    Failure failure;    ///< failure code

    /// Failing (or corrected) field or parameterand optionally, currently supported values for the
    /// field. Values must only be set for field failures other than BAD_VALUE, and only if they are
    /// different from the globally supported values (e.g. due to restrictions by another param or
    /// input data).
    C2ParamFieldValues field;

    /// Conflicting parameters or fields with optional suggestions with (optional) suggested values
    /// for any conflicting fields to avoid the conflict. Must only be set for CONFLICT, UNSUPPORTED
    /// and INFO_CONFLICT failure codes.
    std::vector<C2ParamFieldValues> conflicts;
};

// ================================================================================================
//  WORK
// ================================================================================================

/** Unique ID for a processing node. */
typedef uint32_t c2_node_id_t;

enum {
    kParamIndexWorkOrdinal,
};

/**
 * Information for ordering work items on a component port.
 */
struct C2WorkOrdinalStruct {
//public:
    c2_cntr64_t timestamp;     /** frame timestamp in microseconds */
    c2_cntr64_t frameIndex;    /** submission ordinal on the initial component */
    c2_cntr64_t customOrdinal; /** can be given by the component, e.g. decode order */

    DEFINE_AND_DESCRIBE_C2STRUCT(WorkOrdinal)
    C2FIELD(timestamp, "timestamp")
    C2FIELD(frameIndex, "frame-index")
    C2FIELD(customOrdinal, "custom-ordinal")
};

/**
 * This structure represents a Codec 2.0 frame with its metadata.
 *
 * A frame basically consists of an ordered sets of buffers, configuration changes and info buffers
 * along with some non-configuration metadata.
 */
struct C2FrameData {
//public:
    enum flags_t : uint32_t {
        /**
         * For input frames: no output frame shall be generated when processing this frame, but
         * metadata shall still be processed.
         * For output frames: this frame shall be discarded and but metadata is still valid.
         */
        FLAG_DROP_FRAME    = (1 << 0),
        /**
         * This frame is the last frame of the current stream. Further frames are part of a new
         * stream.
         */
        FLAG_END_OF_STREAM = (1 << 1),
        /**
         * This frame shall be discarded with its metadata.
         * This flag is only set by components - e.g. as a response to the flush command.
         */
        FLAG_DISCARD_FRAME = (1 << 2),
        /**
         * This frame is not the last frame produced for the input.
         *
         * This flag is normally set by the component - e.g. when an input frame results in multiple
         * output frames, this flag is set on all but the last output frame.
         *
         * Also, when components are chained, this flag should be propagated down the
         * work chain. That is, if set on an earlier frame of a work-chain, it should be propagated
         * to all later frames in that chain. Additionally, components down the chain could set
         * this flag even if not set earlier, e.g. if multiple output frame is generated at that
         * component for the input frame.
         */
        FLAG_INCOMPLETE = (1 << 3),
        /**
         * This frame contains only codec-specific configuration data, and no actual access unit.
         *
         * \deprecated pass codec configuration with using the \todo codec-specific configuration
         * info together with the access unit.
         */
        FLAG_CODEC_CONFIG  = (1u << 31),
    };

    /**
     * Frame flags */
    flags_t  flags;
    C2WorkOrdinalStruct ordinal;
    std::vector<std::shared_ptr<C2Buffer>> buffers;
    //< for initial work item, these may also come from the parser - if provided
    //< for output buffers, these are the responses to requestedInfos
    std::vector<std::unique_ptr<C2Param>>      configUpdate;
    std::vector<std::shared_ptr<C2InfoBuffer>> infoBuffers;
};

struct C2Worklet {
//public:
    // IN
    c2_node_id_t component;

    /** Configuration changes to be applied before processing this worklet. */
    std::vector<std::unique_ptr<C2Tuning>> tunings;
    std::vector<std::unique_ptr<C2SettingResult>> failures;

    // OUT
    C2FrameData output;
};

/**
 * Information about partial work-chains not part of the current work items.
 *
 * To be defined later.
 */
struct C2WorkChainInfo;

/**
 * This structure holds information about all a single work item.
 *
 * This structure shall be passed by the client to the component for the first worklet. As such,
 * worklets must not be empty. The ownership of this object is passed.
 */
struct C2Work {
//public:
    /// additional work chain info not part of this work
    std::shared_ptr<C2WorkChainInfo> chainInfo;

    /// The input data to be processed as part of this work/work-chain. This is provided by the
    /// client with ownership. When the work is returned (via onWorkDone), the input buffer-pack's
    /// buffer vector shall contain nullptrs.
    C2FrameData input;

    /// The chain of components, tunings (including output buffer pool IDs) and info requests that the
    /// data must pass through. If this has more than a single element, the tunnels between successive
    /// components of the worklet chain must have been (successfully) pre-registered at the time that
    /// the work is submitted. Allocating the output buffers in the worklets is the responsibility of
    /// each component. Upon work submission, each output buffer-pack shall be an appropriately sized
    /// vector containing nullptrs. When the work is completed/returned to the client, output buffers
    /// pointers from all but the final worklet shall be nullptrs.
    std::list<std::unique_ptr<C2Worklet>> worklets;

    /// Number of worklets successfully processed in this chain. This shall be initialized to 0 by the
    /// client when the work is submitted. It shall contain the number of worklets that were
    /// successfully processed when the work is returned to the client. If this is less then the number
    /// of worklets, result must not be success. It must be in the range of [0, worklets.size()].
    uint32_t workletsProcessed;

    /// The final outcome of the work (corresponding to the current workletsProcessed). If 0 when
    /// work is returned, it is assumed that all worklets have been processed.
    c2_status_t result;
};

/**
 * Information about a future work to be submitted to the component. The information is used to
 * reserve the work for work ordering purposes.
 */
struct C2WorkOutline {
//public:
    C2WorkOrdinalStruct ordinal;
    std::vector<c2_node_id_t> chain;
};

/// @}

#endif  // C2WORK_H_
