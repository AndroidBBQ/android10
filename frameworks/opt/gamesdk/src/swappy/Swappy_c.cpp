/*
 * Copyright 2018 The Android Open Source Project
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

#include "swappy/swappy.h"

#include "Swappy.h"

#include "Settings.h"

#include <chrono>

using namespace swappy;

extern "C" {

void Swappy_init(JNIEnv *env, jobject jactivity) {
    Swappy::init(env, jactivity);
}

void Swappy_destroy() {
    Swappy::destroyInstance();
}

void Swappy_onChoreographer(int64_t frameTimeNanos) {
    Swappy::onChoreographer(frameTimeNanos);
}

bool Swappy_swap(EGLDisplay display, EGLSurface surface) {
    return Swappy::swap(display, surface);
}

void Swappy_setRefreshPeriod(uint64_t period_ns) {
    Settings::getInstance()->setRefreshPeriod(std::chrono::nanoseconds(period_ns));
}

void Swappy_setUseAffinity(bool tf) {
    Settings::getInstance()->setUseAffinity(tf);
}

void Swappy_setSwapIntervalNS(uint64_t swap_ns) {
    Settings::getInstance()->setSwapIntervalNS(swap_ns);
}

uint64_t Swappy_getRefreshPeriodNanos() {
    return Settings::getInstance()->getRefreshPeriod().count();
}

bool Swappy_getUseAffinity() {
    return Settings::getInstance()->getUseAffinity();
}

uint64_t Swappy_getSwapIntervalNS() {
    return Swappy::getSwapIntervalNS();
}

void Swappy_injectTracer(const SwappyTracer *t) {
    Swappy::addTracer(t);
}

void Swappy_setAutoSwapInterval(bool enabled) {
    Swappy::setAutoSwapInterval(enabled);
}

void Swappy_setAutoPipelineMode(bool enabled) {
    Swappy::setAutoPipelineMode(enabled);
}

void Swappy_enableStats(bool enabled) {
    Swappy::enableStats(enabled);
}

void Swappy_recordFrameStart(EGLDisplay display, EGLSurface surface) {
    Swappy::recordFrameStart(display, surface);
}

void Swappy_getStats(Swappy_Stats *stats) {
    Swappy::getStats(stats);
}

bool Swappy_isEnabled() {
    return Swappy::isEnabled();
}

} // extern "C" {
