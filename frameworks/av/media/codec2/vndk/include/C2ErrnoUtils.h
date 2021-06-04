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

#ifndef STAGEFRIGHT_CODEC2_ERRNO_UTILS_H_
#define STAGEFRIGHT_CODEC2_ERRNO_UTILS_H_

#include <errno.h>
#include <C2.h>

// standard ERRNO mappings
template<int N> constexpr c2_status_t _c2_errno2status_impl();
template<> constexpr c2_status_t _c2_errno2status_impl<0>()       { return C2_OK; }
template<> constexpr c2_status_t _c2_errno2status_impl<EINVAL>()  { return C2_BAD_VALUE; }
template<> constexpr c2_status_t _c2_errno2status_impl<EACCES>()  { return C2_REFUSED; }
template<> constexpr c2_status_t _c2_errno2status_impl<EPERM>()   { return C2_REFUSED; }
template<> constexpr c2_status_t _c2_errno2status_impl<ENOMEM>()  { return C2_NO_MEMORY; }

// map standard errno-s to the equivalent c2_status_t
template<int... N> struct _c2_map_errno_impl;
template<int E, int ... N> struct _c2_map_errno_impl<E, N...> {
    static c2_status_t map(int result) {
        if (result == E) {
            return _c2_errno2status_impl <E>();
        } else {
            return _c2_map_errno_impl<N...>::map(result);
        }
    }
};
template<> struct _c2_map_errno_impl<> {
    static c2_status_t map(int result) {
        return result == 0 ? C2_OK : C2_CORRUPTED;
    }
};

template<int... N>
c2_status_t c2_map_errno(int result) {
    return _c2_map_errno_impl<N...>::map(result);
}

#endif // STAGEFRIGHT_CODEC2_ERRNO_UTILS_H_

