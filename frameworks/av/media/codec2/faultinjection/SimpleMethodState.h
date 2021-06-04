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

#ifndef SIMPLE_METHOD_STATE_H_
#define SIMPLE_METHOD_STATE_H_

#include <C2.h>

namespace android {

/**
 * State for a simple method which returns c2_status_t and takes no parameters.
 */
class SimpleMethodState {
public:
    enum Mode {
        // Execute the normal operation
        EXECUTE,
        // Don't do anything
        NO_OP,
        // Hang; never return
        HANG,
    };

    /**
     * Injecter class that modifies the internal states of this class.
     */
    class Injecter {
    public:
        explicit Injecter(SimpleMethodState *thiz);

        /**
         * Hang the operation.
         */
        void hang();

        /**
         * Fail the operation with given params.
         *
         * \param err       error code to replace the actual return value
         * \param execute   whether the wrapper should execute the operation
         */
        void fail(c2_status_t err, bool execute = false);

    private:
        SimpleMethodState *const mThiz;
    };

    SimpleMethodState();

    /**
     * Get execution mode.
     */
    Mode getMode() const;

    /**
     * Override result from running the operation if configured so.
     */
    bool overrideResult(c2_status_t *result) const;

private:
    Mode mMode;
    bool mOverride;
    c2_status_t mResultOverride;
};

}  // namespace android

#endif // SIMPLE_METHOD_STATE_H_
