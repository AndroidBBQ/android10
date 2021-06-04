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

#ifndef ITEM_TABLE_H_
#define ITEM_TABLE_H_

#include <set>

#include <media/NdkMediaFormat.h>

#include <media/stagefright/foundation/ADebug.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

namespace android {

class DataSourceHelper;
class MetaData;

namespace heif {

struct AssociationEntry;
struct ImageItem;
struct ExifItem;
struct ItemLoc;
struct ItemInfo;
struct ItemProperty;
struct ItemReference;

/*
 * ItemTable keeps track of all image items (including coded images, grids and
 * tiles) inside a HEIF still image (ISO/IEC FDIS 23008-12.2:2017(E)).
 */

class ItemTable : public RefBase {
public:
    explicit ItemTable(DataSourceHelper *source);

    status_t parse(uint32_t type, off64_t offset, size_t size);

    bool isValid() { return mImageItemsValid; }
    uint32_t countImages() const;
    AMediaFormat *getImageMeta(const uint32_t imageIndex);
    status_t findImageItem(const uint32_t imageIndex, uint32_t *itemIndex);
    status_t findThumbnailItem(const uint32_t imageIndex, uint32_t *itemIndex);
    status_t getImageOffsetAndSize(
            uint32_t *itemIndex, off64_t *offset, size_t *size);
    status_t getExifOffsetAndSize(off64_t *offset, size_t *size);

protected:
    ~ItemTable();

private:
    DataSourceHelper *mDataSource;

    KeyedVector<uint32_t, ItemLoc> mItemLocs;
    Vector<ItemInfo> mItemInfos;
    Vector<AssociationEntry> mAssociations;
    Vector<sp<ItemProperty> > mItemProperties;
    Vector<sp<ItemReference> > mItemReferences;

    uint32_t mPrimaryItemId;
    off64_t mIdatOffset;
    size_t mIdatSize;

    std::set<uint32_t> mRequiredBoxes;
    std::set<uint32_t> mBoxesSeen;

    bool mImageItemsValid;
    uint32_t mCurrentItemIndex;
    KeyedVector<uint32_t, ImageItem> mItemIdToItemMap;
    KeyedVector<uint32_t, ExifItem> mItemIdToExifMap;
    Vector<uint32_t> mDisplayables;

    status_t parseIlocBox(off64_t offset, size_t size);
    status_t parseIinfBox(off64_t offset, size_t size);
    status_t parsePitmBox(off64_t offset, size_t size);
    status_t parseIprpBox(off64_t offset, size_t size);
    status_t parseIdatBox(off64_t offset, size_t size);
    status_t parseIrefBox(off64_t offset, size_t size);

    void attachProperty(const AssociationEntry &association);
    status_t buildImageItemsIfPossible(uint32_t type);

    DISALLOW_EVIL_CONSTRUCTORS(ItemTable);
};

} // namespace heif
} // namespace android

#endif  // ITEM_TABLE_H_
