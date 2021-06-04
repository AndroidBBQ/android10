/*
 * Copyright (C) 2015 The Android Open Source Project
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

/**
 * A wrapper around ICU's line break iterator, that gives customized line
 * break opportunities, as well as identifying words for the purpose of
 * hyphenation.
 */

#ifndef MINIKIN_WORD_BREAKER_H
#define MINIKIN_WORD_BREAKER_H

#include <list>
#include <mutex>

#include <unicode/ubrk.h>

#include "minikin/IcuUtils.h"
#include "minikin/Macros.h"
#include "minikin/Range.h"

#include "Locale.h"

namespace minikin {

// A class interface for providing pooling implementation of ICU's line breaker.
// The implementation can be customized for testing purposes.
class ICULineBreakerPool {
public:
    struct Slot {
        Slot() : localeId(0), breaker(nullptr) {}
        Slot(uint64_t localeId, IcuUbrkUniquePtr&& breaker)
                : localeId(localeId), breaker(std::move(breaker)) {}

        Slot(Slot&& other) = default;
        Slot& operator=(Slot&& other) = default;

        // Forbid copy and assignment.
        Slot(const Slot&) = delete;
        Slot& operator=(const Slot&) = delete;

        uint64_t localeId;
        IcuUbrkUniquePtr breaker;
    };
    virtual ~ICULineBreakerPool() {}
    virtual Slot acquire(const Locale& locale) = 0;
    virtual void release(Slot&& slot) = 0;
};

// An singleton implementation of the ICU line breaker pool.
// Since creating ICU line breaker instance takes some time. Pool it for later use.
class ICULineBreakerPoolImpl : public ICULineBreakerPool {
public:
    Slot acquire(const Locale& locale) override;
    void release(Slot&& slot) override;

    static ICULineBreakerPoolImpl& getInstance() {
        static ICULineBreakerPoolImpl pool;
        return pool;
    }

protected:
    // protected for testing purposes.
    static constexpr size_t MAX_POOL_SIZE = 4;
    ICULineBreakerPoolImpl(){};  // singleton.
    size_t getPoolSize() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mPool.size();
    }

private:
    std::list<Slot> mPool GUARDED_BY(mMutex);
    mutable std::mutex mMutex;
};

class WordBreaker {
public:
    virtual ~WordBreaker() { finish(); }

    WordBreaker();

    void setText(const uint16_t* data, size_t size);

    // Advance iterator to next word break with current locale. Return offset, or -1 if EOT
    ssize_t next();

    // Advance iterator to the break just after "from" with using the new provided locale.
    // Return offset, or -1 if EOT
    ssize_t followingWithLocale(const Locale& locale, size_t from);

    // Current offset of iterator, equal to 0 at BOT or last return from next()
    ssize_t current() const;

    // After calling next(), wordStart() and wordEnd() are offsets defining the previous
    // word. If wordEnd <= wordStart, it's not a word for the purpose of hyphenation.
    ssize_t wordStart() const;

    ssize_t wordEnd() const;

    // Returns the range from wordStart() to wordEnd().
    // If wordEnd() <= wordStart(), returns empty range.
    inline Range wordRange() const {
        const uint32_t start = wordStart();
        const uint32_t end = wordEnd();
        return start < end ? Range(start, end) : Range(end, end);
    }

    int breakBadness() const;

    void finish();

protected:
    // protected virtual for testing purpose.
    // Caller must release the pool.
    WordBreaker(ICULineBreakerPool* pool);

private:
    int32_t iteratorNext();
    void detectEmailOrUrl();
    ssize_t findNextBreakInEmailOrUrl();

    // Doesn't take ownership. Must not be nullptr. Must be set in constructor.
    ICULineBreakerPool* mPool;

    ICULineBreakerPool::Slot mIcuBreaker;

    UText mUText = UTEXT_INITIALIZER;
    const uint16_t* mText = nullptr;
    size_t mTextSize;
    ssize_t mLast;
    ssize_t mCurrent;

    // state for the email address / url detector
    ssize_t mScanOffset;
    bool mInEmailOrUrl;
};

}  // namespace minikin

#endif  // MINIKIN_WORD_BREAKER_H
