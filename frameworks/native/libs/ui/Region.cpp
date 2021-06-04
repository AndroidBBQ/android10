/*
 * Copyright (C) 2007 The Android Open Source Project
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

#define LOG_TAG "Region"

#include <inttypes.h>
#include <limits.h>

#include <android-base/stringprintf.h>

#include <utils/Log.h>

#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Point.h>

#include <private/ui/RegionHelper.h>

// ----------------------------------------------------------------------------

// ### VALIDATE_REGIONS ###
// To enable VALIDATE_REGIONS traces, use the "libui-validate-regions-defaults"
// in Android.bp. Do not #define VALIDATE_REGIONS here as it requires extra libs.

#define VALIDATE_WITH_CORECG    (false)
// ----------------------------------------------------------------------------

#if defined(VALIDATE_REGIONS)
#include <utils/CallStack.h>
#endif

#if VALIDATE_WITH_CORECG
#include <core/SkRegion.h>
#endif

namespace android {
// ----------------------------------------------------------------------------

using base::StringAppendF;

enum {
    op_nand = region_operator<Rect>::op_nand,
    op_and  = region_operator<Rect>::op_and,
    op_or   = region_operator<Rect>::op_or,
    op_xor  = region_operator<Rect>::op_xor
};

enum {
    direction_LTR,
    direction_RTL
};

const Region Region::INVALID_REGION(Rect::INVALID_RECT);

// ----------------------------------------------------------------------------

Region::Region() {
    mStorage.add(Rect(0,0));
}

Region::Region(const Region& rhs)
    : mStorage(rhs.mStorage)
{
#if defined(VALIDATE_REGIONS)
    validate(rhs, "rhs copy-ctor");
#endif
}

Region::Region(const Rect& rhs) {
    mStorage.add(rhs);
}

Region::~Region()
{
}

/**
 * Copy rects from the src vector into the dst vector, resolving vertical T-Junctions along the way
 *
 * First pass through, divideSpanRTL will be set because the 'previous span' (indexing into the dst
 * vector) will be reversed. Each rectangle in the original list, starting from the bottom, will be
 * compared with the span directly below, and subdivided as needed to resolve T-junctions.
 *
 * The resulting temporary vector will be a completely reversed copy of the original, without any
 * bottom-up T-junctions.
 *
 * Second pass through, divideSpanRTL will be false since the previous span will index into the
 * final, correctly ordered region buffer. Each rectangle will be compared with the span directly
 * above it, and subdivided to resolve any remaining T-junctions.
 */
static void reverseRectsResolvingJunctions(const Rect* begin, const Rect* end,
        Vector<Rect>& dst, int spanDirection) {
    dst.clear();

    const Rect* current = end - 1;
    int lastTop = current->top;

    // add first span immediately
    do {
        dst.add(*current);
        current--;
    } while (current->top == lastTop && current >= begin);

    int beginLastSpan = -1;
    int endLastSpan = -1;
    int top = -1;
    int bottom = -1;

    // for all other spans, split if a t-junction exists in the span directly above
    while (current >= begin) {
        if (current->top != (current + 1)->top) {
            // new span
            if ((spanDirection == direction_RTL && current->bottom != (current + 1)->top) ||
                    (spanDirection == direction_LTR && current->top != (current + 1)->bottom)) {
                // previous span not directly adjacent, don't check for T junctions
                beginLastSpan = INT_MAX;
            } else {
                beginLastSpan = endLastSpan + 1;
            }
            endLastSpan = static_cast<int>(dst.size()) - 1;

            top = current->top;
            bottom = current->bottom;
        }
        int left = current->left;
        int right = current->right;

        for (int prevIndex = beginLastSpan; prevIndex <= endLastSpan; prevIndex++) {
            // prevIndex can't be -1 here because if endLastSpan is set to a
            // value greater than -1 (allowing the loop to execute),
            // beginLastSpan (and therefore prevIndex) will also be increased
            const Rect prev = dst[static_cast<size_t>(prevIndex)];
            if (spanDirection == direction_RTL) {
                // iterating over previous span RTL, quit if it's too far left
                if (prev.right <= left) break;

                if (prev.right > left && prev.right < right) {
                    dst.add(Rect(prev.right, top, right, bottom));
                    right = prev.right;
                }

                if (prev.left > left && prev.left < right) {
                    dst.add(Rect(prev.left, top, right, bottom));
                    right = prev.left;
                }

                // if an entry in the previous span is too far right, nothing further left in the
                // current span will need it
                if (prev.left >= right) {
                    beginLastSpan = prevIndex;
                }
            } else {
                // iterating over previous span LTR, quit if it's too far right
                if (prev.left >= right) break;

                if (prev.left > left && prev.left < right) {
                    dst.add(Rect(left, top, prev.left, bottom));
                    left = prev.left;
                }

                if (prev.right > left && prev.right < right) {
                    dst.add(Rect(left, top, prev.right, bottom));
                    left = prev.right;
                }
                // if an entry in the previous span is too far left, nothing further right in the
                // current span will need it
                if (prev.right <= left) {
                    beginLastSpan = prevIndex;
                }
            }
        }

        if (left < right) {
            dst.add(Rect(left, top, right, bottom));
        }

        current--;
    }
}

/**
 * Creates a new region with the same data as the argument, but divides rectangles as necessary to
 * remove T-Junctions
 *
 * Note: the output will not necessarily be a very efficient representation of the region, since it
 * may be that a triangle-based approach would generate significantly simpler geometry
 */
Region Region::createTJunctionFreeRegion(const Region& r) {
    if (r.isEmpty()) return r;
    if (r.isRect()) return r;

    Vector<Rect> reversed;
    reverseRectsResolvingJunctions(r.begin(), r.end(), reversed, direction_RTL);

    Region outputRegion;
    reverseRectsResolvingJunctions(reversed.begin(), reversed.end(),
            outputRegion.mStorage, direction_LTR);
    outputRegion.mStorage.add(r.getBounds()); // to make region valid, mStorage must end with bounds

#if defined(VALIDATE_REGIONS)
    validate(outputRegion, "T-Junction free region");
#endif

    return outputRegion;
}

Region& Region::operator = (const Region& rhs)
{
#if defined(VALIDATE_REGIONS)
    validate(*this, "this->operator=");
    validate(rhs, "rhs.operator=");
#endif
    mStorage = rhs.mStorage;
    return *this;
}

Region& Region::makeBoundsSelf()
{
    if (mStorage.size() >= 2) {
        const Rect bounds(getBounds());
        mStorage.clear();
        mStorage.add(bounds);
    }
    return *this;
}

bool Region::contains(const Point& point) const {
    return contains(point.x, point.y);
}

bool Region::contains(int x, int y) const {
    const_iterator cur = begin();
    const_iterator const tail = end();
    while (cur != tail) {
        if (y >= cur->top && y < cur->bottom && x >= cur->left && x < cur->right) {
            return true;
        }
        cur++;
    }
    return false;
}

void Region::clear()
{
    mStorage.clear();
    mStorage.add(Rect(0,0));
}

void Region::set(const Rect& r)
{
    mStorage.clear();
    mStorage.add(r);
}

void Region::set(int32_t w, int32_t h)
{
    mStorage.clear();
    mStorage.add(Rect(w, h));
}

void Region::set(uint32_t w, uint32_t h)
{
    mStorage.clear();
    mStorage.add(Rect(w, h));
}

bool Region::isTriviallyEqual(const Region& region) const {
    return begin() == region.begin();
}

// ----------------------------------------------------------------------------

void Region::addRectUnchecked(int l, int t, int r, int b)
{
    Rect rect(l,t,r,b);
    size_t where = mStorage.size() - 1;
    mStorage.insertAt(rect, where, 1);
}

// ----------------------------------------------------------------------------

Region& Region::orSelf(const Rect& r) {
    return operationSelf(r, op_or);
}
Region& Region::xorSelf(const Rect& r) {
    return operationSelf(r, op_xor);
}
Region& Region::andSelf(const Rect& r) {
    return operationSelf(r, op_and);
}
Region& Region::subtractSelf(const Rect& r) {
    return operationSelf(r, op_nand);
}
Region& Region::operationSelf(const Rect& r, uint32_t op) {
    Region lhs(*this);
    boolean_operation(op, *this, lhs, r);
    return *this;
}

// ----------------------------------------------------------------------------

Region& Region::orSelf(const Region& rhs) {
    return operationSelf(rhs, op_or);
}
Region& Region::xorSelf(const Region& rhs) {
    return operationSelf(rhs, op_xor);
}
Region& Region::andSelf(const Region& rhs) {
    return operationSelf(rhs, op_and);
}
Region& Region::subtractSelf(const Region& rhs) {
    return operationSelf(rhs, op_nand);
}
Region& Region::operationSelf(const Region& rhs, uint32_t op) {
    Region lhs(*this);
    boolean_operation(op, *this, lhs, rhs);
    return *this;
}

Region& Region::translateSelf(int x, int y) {
    if (x|y) translate(*this, x, y);
    return *this;
}

Region& Region::scaleSelf(float sx, float sy) {
    size_t count = mStorage.size();
    Rect* rects = mStorage.editArray();
    while (count) {
        rects->left = static_cast<int32_t>(rects->left * sx + 0.5f);
        rects->right = static_cast<int32_t>(rects->right * sx + 0.5f);
        rects->top = static_cast<int32_t>(rects->top * sy + 0.5f);
        rects->bottom = static_cast<int32_t>(rects->bottom * sy + 0.5f);
        rects++;
        count--;
    }
    return *this;
}

// ----------------------------------------------------------------------------

const Region Region::merge(const Rect& rhs) const {
    return operation(rhs, op_or);
}
const Region Region::mergeExclusive(const Rect& rhs) const {
    return operation(rhs, op_xor);
}
const Region Region::intersect(const Rect& rhs) const {
    return operation(rhs, op_and);
}
const Region Region::subtract(const Rect& rhs) const {
    return operation(rhs, op_nand);
}
const Region Region::operation(const Rect& rhs, uint32_t op) const {
    Region result;
    boolean_operation(op, result, *this, rhs);
    return result;
}

// ----------------------------------------------------------------------------

const Region Region::merge(const Region& rhs) const {
    return operation(rhs, op_or);
}
const Region Region::mergeExclusive(const Region& rhs) const {
    return operation(rhs, op_xor);
}
const Region Region::intersect(const Region& rhs) const {
    return operation(rhs, op_and);
}
const Region Region::subtract(const Region& rhs) const {
    return operation(rhs, op_nand);
}
const Region Region::operation(const Region& rhs, uint32_t op) const {
    Region result;
    boolean_operation(op, result, *this, rhs);
    return result;
}

const Region Region::translate(int x, int y) const {
    Region result;
    translate(result, *this, x, y);
    return result;
}

// ----------------------------------------------------------------------------

Region& Region::orSelf(const Region& rhs, int dx, int dy) {
    return operationSelf(rhs, dx, dy, op_or);
}
Region& Region::xorSelf(const Region& rhs, int dx, int dy) {
    return operationSelf(rhs, dx, dy, op_xor);
}
Region& Region::andSelf(const Region& rhs, int dx, int dy) {
    return operationSelf(rhs, dx, dy, op_and);
}
Region& Region::subtractSelf(const Region& rhs, int dx, int dy) {
    return operationSelf(rhs, dx, dy, op_nand);
}
Region& Region::operationSelf(const Region& rhs, int dx, int dy, uint32_t op) {
    Region lhs(*this);
    boolean_operation(op, *this, lhs, rhs, dx, dy);
    return *this;
}

// ----------------------------------------------------------------------------

const Region Region::merge(const Region& rhs, int dx, int dy) const {
    return operation(rhs, dx, dy, op_or);
}
const Region Region::mergeExclusive(const Region& rhs, int dx, int dy) const {
    return operation(rhs, dx, dy, op_xor);
}
const Region Region::intersect(const Region& rhs, int dx, int dy) const {
    return operation(rhs, dx, dy, op_and);
}
const Region Region::subtract(const Region& rhs, int dx, int dy) const {
    return operation(rhs, dx, dy, op_nand);
}
const Region Region::operation(const Region& rhs, int dx, int dy, uint32_t op) const {
    Region result;
    boolean_operation(op, result, *this, rhs, dx, dy);
    return result;
}

// ----------------------------------------------------------------------------

// This is our region rasterizer, which merges rects and spans together
// to obtain an optimal region.
class Region::rasterizer : public region_operator<Rect>::region_rasterizer
{
    Rect bounds;
    Vector<Rect>& storage;
    Rect* head;
    Rect* tail;
    Vector<Rect> span;
    Rect* cur;
public:
    explicit rasterizer(Region& reg)
        : bounds(INT_MAX, 0, INT_MIN, 0), storage(reg.mStorage), head(), tail(), cur() {
        storage.clear();
    }

    virtual ~rasterizer();

    virtual void operator()(const Rect& rect);

private:
    template<typename T>
    static inline T min(T rhs, T lhs) { return rhs < lhs ? rhs : lhs; }
    template<typename T>
    static inline T max(T rhs, T lhs) { return rhs > lhs ? rhs : lhs; }

    void flushSpan();
};

Region::rasterizer::~rasterizer()
{
    if (span.size()) {
        flushSpan();
    }
    if (storage.size()) {
        bounds.top = storage.itemAt(0).top;
        bounds.bottom = storage.top().bottom;
        if (storage.size() == 1) {
            storage.clear();
        }
    } else {
        bounds.left  = 0;
        bounds.right = 0;
    }
    storage.add(bounds);
}

void Region::rasterizer::operator()(const Rect& rect)
{
    //ALOGD(">>> %3d, %3d, %3d, %3d",
    //        rect.left, rect.top, rect.right, rect.bottom);
    if (span.size()) {
        if (cur->top != rect.top) {
            flushSpan();
        } else if (cur->right == rect.left) {
            cur->right = rect.right;
            return;
        }
    }
    span.add(rect);
    cur = span.editArray() + (span.size() - 1);
}

void Region::rasterizer::flushSpan()
{
    bool merge = false;
    if (tail-head == ssize_t(span.size())) {
        Rect const* p = span.editArray();
        Rect const* q = head;
        if (p->top == q->bottom) {
            merge = true;
            while (q != tail) {
                if ((p->left != q->left) || (p->right != q->right)) {
                    merge = false;
                    break;
                }
                p++;
                q++;
            }
        }
    }
    if (merge) {
        const int bottom = span[0].bottom;
        Rect* r = head;
        while (r != tail) {
            r->bottom = bottom;
            r++;
        }
    } else {
        bounds.left = min(span.itemAt(0).left, bounds.left);
        bounds.right = max(span.top().right, bounds.right);
        storage.appendVector(span);
        tail = storage.editArray() + storage.size();
        head = tail - span.size();
    }
    span.clear();
}

bool Region::validate(const Region& reg, const char* name, bool silent)
{
    if (reg.mStorage.isEmpty()) {
        ALOGE_IF(!silent, "%s: mStorage is empty, which is never valid", name);
        // return immediately as the code below assumes mStorage is non-empty
        return false;
    }

    bool result = true;
    const_iterator cur = reg.begin();
    const_iterator const tail = reg.end();
    const_iterator prev = cur;
    Rect b(*prev);
    while (cur != tail) {
        if (cur->isValid() == false) {
            // We allow this particular flavor of invalid Rect, since it is used
            // as a signal value in various parts of the system
            if (*cur != Rect::INVALID_RECT) {
                ALOGE_IF(!silent, "%s: region contains an invalid Rect", name);
                result = false;
            }
        }
        if (cur->right > region_operator<Rect>::max_value) {
            ALOGE_IF(!silent, "%s: rect->right > max_value", name);
            result = false;
        }
        if (cur->bottom > region_operator<Rect>::max_value) {
            ALOGE_IF(!silent, "%s: rect->right > max_value", name);
            result = false;
        }
        if (prev != cur) {
            b.left   = b.left   < cur->left   ? b.left   : cur->left;
            b.top    = b.top    < cur->top    ? b.top    : cur->top;
            b.right  = b.right  > cur->right  ? b.right  : cur->right;
            b.bottom = b.bottom > cur->bottom ? b.bottom : cur->bottom;
            if ((*prev < *cur) == false) {
                ALOGE_IF(!silent, "%s: region's Rects not sorted", name);
                result = false;
            }
            if (cur->top == prev->top) {
                if (cur->bottom != prev->bottom) {
                    ALOGE_IF(!silent, "%s: invalid span %p", name, cur);
                    result = false;
                } else if (cur->left < prev->right) {
                    ALOGE_IF(!silent,
                            "%s: spans overlap horizontally prev=%p, cur=%p",
                            name, prev, cur);
                    result = false;
                }
            } else if (cur->top < prev->bottom) {
                ALOGE_IF(!silent,
                        "%s: spans overlap vertically prev=%p, cur=%p",
                        name, prev, cur);
                result = false;
            }
            prev = cur;
        }
        cur++;
    }
    if (b != reg.getBounds()) {
        result = false;
        ALOGE_IF(!silent,
                "%s: invalid bounds [%d,%d,%d,%d] vs. [%d,%d,%d,%d]", name,
                b.left, b.top, b.right, b.bottom,
                reg.getBounds().left, reg.getBounds().top, 
                reg.getBounds().right, reg.getBounds().bottom);
    }
    if (reg.mStorage.size() == 2) {
        result = false;
        ALOGE_IF(!silent, "%s: mStorage size is 2, which is never valid", name);
    }
#if defined(VALIDATE_REGIONS)
    if (result == false && !silent) {
        reg.dump(name);
        CallStack stack(LOG_TAG);
    }
#endif
    return result;
}

void Region::boolean_operation(uint32_t op, Region& dst,
        const Region& lhs,
        const Region& rhs, int dx, int dy)
{
#if defined(VALIDATE_REGIONS)
    validate(lhs, "boolean_operation (before): lhs");
    validate(rhs, "boolean_operation (before): rhs");
    validate(dst, "boolean_operation (before): dst");
#endif

    size_t lhs_count;
    Rect const * const lhs_rects = lhs.getArray(&lhs_count);

    size_t rhs_count;
    Rect const * const rhs_rects = rhs.getArray(&rhs_count);

    region_operator<Rect>::region lhs_region(lhs_rects, lhs_count);
    region_operator<Rect>::region rhs_region(rhs_rects, rhs_count, dx, dy);
    region_operator<Rect> operation(op, lhs_region, rhs_region);
    { // scope for rasterizer (dtor has side effects)
        rasterizer r(dst);
        operation(r);
    }

#if defined(VALIDATE_REGIONS)
    validate(lhs, "boolean_operation: lhs");
    validate(rhs, "boolean_operation: rhs");
    validate(dst, "boolean_operation: dst");
#endif

#if VALIDATE_WITH_CORECG
    SkRegion sk_lhs;
    SkRegion sk_rhs;
    SkRegion sk_dst;
    
    for (size_t i=0 ; i<lhs_count ; i++)
        sk_lhs.op(
                lhs_rects[i].left   + dx,
                lhs_rects[i].top    + dy,
                lhs_rects[i].right  + dx,
                lhs_rects[i].bottom + dy,
                SkRegion::kUnion_Op);
    
    for (size_t i=0 ; i<rhs_count ; i++)
        sk_rhs.op(
                rhs_rects[i].left   + dx,
                rhs_rects[i].top    + dy,
                rhs_rects[i].right  + dx,
                rhs_rects[i].bottom + dy,
                SkRegion::kUnion_Op);
 
    const char* name = "---";
    SkRegion::Op sk_op;
    switch (op) {
        case op_or: sk_op = SkRegion::kUnion_Op; name="OR"; break;
        case op_xor: sk_op = SkRegion::kUnion_XOR; name="XOR"; break;
        case op_and: sk_op = SkRegion::kIntersect_Op; name="AND"; break;
        case op_nand: sk_op = SkRegion::kDifference_Op; name="NAND"; break;
    }
    sk_dst.op(sk_lhs, sk_rhs, sk_op);

    if (sk_dst.isEmpty() && dst.isEmpty())
        return;
    
    bool same = true;
    Region::const_iterator head = dst.begin();
    Region::const_iterator const tail = dst.end();
    SkRegion::Iterator it(sk_dst);
    while (!it.done()) {
        if (head != tail) {
            if (
                    head->left != it.rect().fLeft ||     
                    head->top != it.rect().fTop ||     
                    head->right != it.rect().fRight ||     
                    head->bottom != it.rect().fBottom
            ) {
                same = false;
                break;
            }
        } else {
            same = false;
            break;
        }
        head++;
        it.next();
    }
    
    if (head != tail) {
        same = false;
    }
    
    if(!same) {
        ALOGD("---\nregion boolean %s failed", name);
        lhs.dump("lhs");
        rhs.dump("rhs");
        dst.dump("dst");
        ALOGD("should be");
        SkRegion::Iterator it(sk_dst);
        while (!it.done()) {
            ALOGD("    [%3d, %3d, %3d, %3d]",
                it.rect().fLeft,
                it.rect().fTop,
                it.rect().fRight,
                it.rect().fBottom);
            it.next();
        }
    }
#endif
}

void Region::boolean_operation(uint32_t op, Region& dst,
        const Region& lhs,
        const Rect& rhs, int dx, int dy)
{
    // We allow this particular flavor of invalid Rect, since it is used as a
    // signal value in various parts of the system
    if (!rhs.isValid() && rhs != Rect::INVALID_RECT) {
        ALOGE("Region::boolean_operation(op=%d) invalid Rect={%d,%d,%d,%d}",
                op, rhs.left, rhs.top, rhs.right, rhs.bottom);
        return;
    }

#if VALIDATE_WITH_CORECG || defined(VALIDATE_REGIONS)
    boolean_operation(op, dst, lhs, Region(rhs), dx, dy);
#else
    size_t lhs_count;
    Rect const * const lhs_rects = lhs.getArray(&lhs_count);

    region_operator<Rect>::region lhs_region(lhs_rects, lhs_count);
    region_operator<Rect>::region rhs_region(&rhs, 1, dx, dy);
    region_operator<Rect> operation(op, lhs_region, rhs_region);
    { // scope for rasterizer (dtor has side effects)
        rasterizer r(dst);
        operation(r);
    }

#endif
}

void Region::boolean_operation(uint32_t op, Region& dst,
        const Region& lhs, const Region& rhs)
{
    boolean_operation(op, dst, lhs, rhs, 0, 0);
}

void Region::boolean_operation(uint32_t op, Region& dst,
        const Region& lhs, const Rect& rhs)
{
    boolean_operation(op, dst, lhs, rhs, 0, 0);
}

void Region::translate(Region& reg, int dx, int dy)
{
    if ((dx || dy) && !reg.isEmpty()) {
#if defined(VALIDATE_REGIONS)
        validate(reg, "translate (before)");
#endif
        size_t count = reg.mStorage.size();
        Rect* rects = reg.mStorage.editArray();
        while (count) {
            rects->offsetBy(dx, dy);
            rects++;
            count--;
        }
#if defined(VALIDATE_REGIONS)
        validate(reg, "translate (after)");
#endif
    }
}

void Region::translate(Region& dst, const Region& reg, int dx, int dy)
{
    dst = reg;
    translate(dst, dx, dy);
}

// ----------------------------------------------------------------------------

size_t Region::getFlattenedSize() const {
    return sizeof(uint32_t) + mStorage.size() * sizeof(Rect);
}

status_t Region::flatten(void* buffer, size_t size) const {
#if defined(VALIDATE_REGIONS)
    validate(*this, "Region::flatten");
#endif
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }
    // Cast to uint32_t since the size of a size_t can vary between 32- and
    // 64-bit processes
    FlattenableUtils::write(buffer, size, static_cast<uint32_t>(mStorage.size()));
    for (auto rect : mStorage) {
        status_t result = rect.flatten(buffer, size);
        if (result != NO_ERROR) {
            return result;
        }
        FlattenableUtils::advance(buffer, size, sizeof(rect));
    }
    return NO_ERROR;
}

status_t Region::unflatten(void const* buffer, size_t size) {
    if (size < sizeof(uint32_t)) {
        return NO_MEMORY;
    }

    uint32_t numRects = 0;
    FlattenableUtils::read(buffer, size, numRects);
    if (size < numRects * sizeof(Rect)) {
        return NO_MEMORY;
    }

    if (numRects > (UINT32_MAX / sizeof(Rect))) {
        android_errorWriteWithInfoLog(0x534e4554, "29983260", -1, nullptr, 0);
        return NO_MEMORY;
    }

    Region result;
    result.mStorage.clear();
    for (size_t r = 0; r < numRects; ++r) {
        Rect rect(Rect::EMPTY_RECT);
        status_t status = rect.unflatten(buffer, size);
        if (status != NO_ERROR) {
            return status;
        }
        FlattenableUtils::advance(buffer, size, sizeof(rect));
        result.mStorage.push_back(rect);
    }

#if defined(VALIDATE_REGIONS)
    validate(result, "Region::unflatten");
#endif

    if (!result.validate(result, "Region::unflatten", true)) {
        ALOGE("Region::unflatten() failed, invalid region");
        return BAD_VALUE;
    }
    mStorage = result.mStorage;
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

Region::const_iterator Region::begin() const {
    return mStorage.array();
}

Region::const_iterator Region::end() const {
    // Workaround for b/77643177
    // mStorage should never be empty, but somehow it is and it's causing
    // an abort in ubsan
    if (mStorage.isEmpty()) return mStorage.array();

    size_t numRects = isRect() ? 1 : mStorage.size() - 1;
    return mStorage.array() + numRects;
}

Rect const* Region::getArray(size_t* count) const {
    if (count) *count = static_cast<size_t>(end() - begin());
    return begin();
}

// ----------------------------------------------------------------------------

void Region::dump(std::string& out, const char* what, uint32_t /* flags */) const {
    const_iterator head = begin();
    const_iterator const tail = end();

    StringAppendF(&out, "  Region %s (this=%p, count=%" PRIdPTR ")\n", what, this, tail - head);
    while (head != tail) {
        StringAppendF(&out, "    [%3d, %3d, %3d, %3d]\n", head->left, head->top, head->right,
                      head->bottom);
        ++head;
    }
}

void Region::dump(const char* what, uint32_t /* flags */) const
{
    const_iterator head = begin();
    const_iterator const tail = end();
    ALOGD("  Region %s (this=%p, count=%" PRIdPTR ")\n", what, this, tail-head);
    while (head != tail) {
        ALOGD("    [%3d, %3d, %3d, %3d]\n",
                head->left, head->top, head->right, head->bottom);
        head++;
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
