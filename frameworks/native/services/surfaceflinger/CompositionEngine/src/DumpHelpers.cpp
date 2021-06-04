/*
 * Copyright 2019 The Android Open Source Project
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

#include <cinttypes>

#include <android-base/stringprintf.h>
#include <compositionengine/impl/DumpHelpers.h>

namespace android::compositionengine::impl {

using android::base::StringAppendF;

void dumpVal(std::string& out, const char* name, bool value) {
    StringAppendF(&out, "%s=%s ", name, value ? "true" : "false");
}

void dumpVal(std::string& out, const char* name, const void* value) {
    StringAppendF(&out, "%s=%p ", name, value);
}

void dumpVal(std::string& out, const char* name, int value) {
    StringAppendF(&out, "%s=%d ", name, value);
}

void dumpVal(std::string& out, const char* name, float value) {
    StringAppendF(&out, "%s=%f ", name, value);
}

void dumpVal(std::string& out, const char* name, uint32_t value) {
    StringAppendF(&out, "%s=%u ", name, value);
}

void dumpHex(std::string& out, const char* name, uint64_t value) {
    StringAppendF(&out, "%s=0x08%" PRIx64 " ", name, value);
}

void dumpVal(std::string& out, const char* name, const char* value) {
    StringAppendF(&out, "%s=%s ", name, value);
}

void dumpVal(std::string& out, const char* name, const std::string& value) {
    dumpVal(out, name, value.c_str());
}

void dumpVal(std::string& out, const char* name, const char* valueName, int value) {
    StringAppendF(&out, "%s=%s (%d) ", name, valueName, value);
}

void dumpVal(std::string& out, const char* name, const std::string& valueName, int value) {
    dumpVal(out, name, valueName.c_str(), value);
}

void dumpVal(std::string& out, const char* name, const FloatRect& rect) {
    StringAppendF(&out, "%s=[%f %f %f %f] ", name, rect.left, rect.top, rect.right, rect.bottom);
}

void dumpVal(std::string& out, const char* name, const Rect& rect) {
    StringAppendF(&out, "%s=[%d %d %d %d] ", name, rect.left, rect.top, rect.right, rect.bottom);
}

void dumpVal(std::string& out, const char* name, const Region& region) {
    region.dump(out, name, 0);
}

void dumpVal(std::string& out, const char* name, const ui::Transform& transform) {
    transform.dump(out, name);
}

void dumpVal(std::string& out, const char* name, const ui::Size& size) {
    StringAppendF(&out, "%s=[%d %d] ", name, size.width, size.height);
}

void dumpVal(std::string& out, const char* name, const mat4& tr) {
    StringAppendF(&out,
                  "%s=["
                  /* clang-format off */
                  "[%0.3f,%0.3f,%0.3f,%0.3f]"
                  "[%0.3f,%0.3f,%0.3f,%0.3f]"
                  "[%0.3f,%0.3f,%0.3f,%0.3f]"
                  "[%0.3f,%0.3f,%0.3f,%0.3f]]",
                  name,
                  tr[0][0], tr[1][0], tr[2][0], tr[3][0],
                  tr[0][1], tr[1][1], tr[2][1], tr[3][1],
                  tr[0][2], tr[1][2], tr[2][2], tr[3][2],
                  tr[0][3], tr[1][3], tr[2][3], tr[3][3]
                  ); /* clang-format on */
}

} // namespace android::compositionengine::impl
