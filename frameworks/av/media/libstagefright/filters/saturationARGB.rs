/*
 * Copyright (C) 2014 The Android Open Source Project
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

#pragma version(1)
#pragma rs java_package_name(com.android.rs.cppbasic)
#pragma rs_fp_relaxed

const static float3 gMonoMult = {0.299f, 0.587f, 0.114f};

// global variables (parameters accessible to application code)
float gSaturation = 1.0f;

void root(const uchar4 *v_in, uchar4 *v_out) {
    v_out->x = v_in->x; // don't modify A

    // get RGB, scale 0-255 uchar to 0-1.0 float
    float3 rgb = {v_in->y * 0.003921569f, v_in->z * 0.003921569f,
            v_in->w * 0.003921569f};

    // apply saturation filter
    float3 result = dot(rgb, gMonoMult);
    result = mix(result, rgb, gSaturation);

    v_out->y = (uchar)clamp((result.r * 255.f + 0.5f), 0.f, 255.f);
    v_out->z = (uchar)clamp((result.g * 255.f + 0.5f), 0.f, 255.f);
    v_out->w = (uchar)clamp((result.b * 255.f + 0.5f), 0.f, 255.f);
}
