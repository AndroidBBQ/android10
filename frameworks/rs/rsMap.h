#ifndef ANDROID_RENDERSCRIPT_MAP_H
#define ANDROID_RENDERSCRIPT_MAP_H

#include <stddef.h>

namespace android {
namespace renderscript {

template <class T1, class T2>
class Pair {
public:
    Pair() {}
    Pair(T1 f1, T2 f2) : first(f1), second(f2) {}

    T1 first;
    T2 second;
};

template <class T1, class T2>
Pair<T1, T2> make_pair(T1 first, T2 second) {
    return Pair<T1, T2>(first, second);
}

#define MAP_LOG_NUM_BUCKET 8
#define MAP_NUM_BUCKET (1 << MAP_LOG_NUM_BUCKET)
#define MAP_NUM_BUCKET_MASK (MAP_NUM_BUCKET - 1)

template <class KeyType, class ValueType>
class Map {
private:
    typedef Pair<KeyType, ValueType> MapEntry;

    struct LinkNode {
        MapEntry entry;
        LinkNode* next;
    };

public:
    Map() : endIterator(MAP_NUM_BUCKET, nullptr, this) {
        for (size_t i = 0; i < MAP_NUM_BUCKET; i++) { bucket[i] = nullptr; }
    }

    ~Map() {
        for (size_t i = 0; i < MAP_NUM_BUCKET; i++) {
            LinkNode* p = bucket[i];
            LinkNode* next;
            while (p != nullptr) {
                next = p->next;
                delete p;
                p = next;
            }
        }
    }

    ValueType& operator[](const KeyType& key) {
        const size_t index = hash(key) & MAP_NUM_BUCKET_MASK;
        LinkNode* node = bucket[index];
        LinkNode* prev = nullptr;

        while (node != nullptr) {
            if (node->entry.first == key) {
                return node->entry.second;
            }
            prev = node;
            node = node->next;
        }

        node = new LinkNode();
        node->entry.first = key;
        node->next = nullptr;
        if (prev == nullptr) {
            bucket[index] = node;
        } else {
            prev->next = node;
        }
        return node->entry.second;
    }

    class iterator {
        friend class Map;
    public:
        iterator& operator++() {
            LinkNode* next;

            next = node->next;
            if (next != nullptr) {
                node = next;
                return *this;
            }

            while (++bucket_index < MAP_NUM_BUCKET) {
                next = map->bucket[bucket_index];
                if (next != nullptr) {
                    node = next;
                    return *this;
                }
            }

            node = nullptr;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return node == other.node && bucket_index == other.bucket_index &&
                    map == other.map;
        }

        bool operator!=(const iterator& other) const {
            return node != other.node || bucket_index != other.bucket_index ||
                    map != other.map;
        }

        const MapEntry& operator*() const {
            return node->entry;
        }

    protected:
        iterator(size_t index, LinkNode* n, const Map* m) : bucket_index(index), node(n), map(m) {}

    private:
        size_t bucket_index;
        LinkNode* node;
        const Map* map;
    };

    iterator begin() const {
        for (size_t i = 0; i < MAP_NUM_BUCKET; i++) {
            LinkNode* node = bucket[i];
            if (node != nullptr) {
                return iterator(i, node, this);
            }
        }

        return end();
    }

    const iterator& end() const { return endIterator; }

    iterator begin() { return ((const Map*)this)->begin(); }

    const iterator& end() { return endIterator; }

    iterator find(const KeyType& key) const {
        const size_t index = hash(key) & MAP_NUM_BUCKET_MASK;
        LinkNode* node = bucket[index];

        while (node != nullptr) {
            if (node->entry.first == key) {
                return iterator(index, node, this);
            }
            node = node->next;
        }

        return end();
    }

private:
    size_t hash(const KeyType& key) const { return ((size_t)key) >> 4; }

    LinkNode* bucket[MAP_NUM_BUCKET];
    const iterator endIterator;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_MAP_H
