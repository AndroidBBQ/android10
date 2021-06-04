#ifndef ANDROID_RENDERSCRIPT_LIST_H
#define ANDROID_RENDERSCRIPT_LIST_H

namespace android {
namespace renderscript {

namespace {

constexpr size_t BUFFER_SIZE = 64;

}  // anonymous namespace

template <class T>
class List {
private:
    class LinkedBuffer {
    public:
        LinkedBuffer() : next(nullptr) {}

        union {
            char raw[BUFFER_SIZE - sizeof(LinkedBuffer*)];
            T typed;
        } data;
        LinkedBuffer* next;
    };

public:
    class iterator;

    List() : last(nullptr), first(&firstBuffer.data.typed),
             beginIterator(this, &firstBuffer, const_cast<T*>(first)),
             _size(0) {
        current = const_cast<T*>(first);
        currentBuffer = &firstBuffer;
    }

    template <class InputIterator>
    List(InputIterator first, InputIterator last) : List() {
        for (InputIterator it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    ~List() {
        LinkedBuffer* p = firstBuffer.next;
        LinkedBuffer* next;
        while (p != nullptr) {
            next = p->next;
            delete p;
            p = next;
        }
    }

    void push_back(const T& value) {
        last = current;
        *current++ = value;
        _size++;
        if ((void*)current >= (void*)&currentBuffer->next) {
            LinkedBuffer* newBuffer = new LinkedBuffer();
            currentBuffer->next = newBuffer;
            currentBuffer = newBuffer;
            current = &currentBuffer->data.typed;
        }
    }

    class iterator {
        friend class List;
    public:
        iterator& operator++() {
            p++;
            if ((void*)p >= (void*)&buffer->next) {
                buffer = buffer->next;
                if (buffer != nullptr) {
                    p = &buffer->data.typed;
                } else {
                    p = nullptr;
                }
            }
            return *this;
        }

        bool operator==(const iterator& other) const {
            return p == other.p && buffer == other.buffer && list == other.list;
        }

        bool operator!=(const iterator& other) const {
            return p != other.p || buffer != other.buffer || list != other.list;
        }

        const T& operator*() const { return *p; }

        T* operator->() { return p; }

    protected:
        explicit iterator(const List* list_) : list(list_) {}
        iterator(const List* list_, LinkedBuffer* buffer_, T* p_) :
            p(p_), buffer(buffer_), list(list_) {}

    private:
        T* p;
        LinkedBuffer* buffer;
        const List* list;
    };

    const iterator& begin() const { return beginIterator; }

    iterator end() const { return iterator(this, currentBuffer, current); }

    bool empty() const { return current == first; }

    T& front() const { return *const_cast<T*>(first); }

    T& back() const { return *last; }

    size_t size() const { return _size; }

private:
    T* current;
    T* last;
    LinkedBuffer* currentBuffer;
    LinkedBuffer firstBuffer;
    const T* first;
    const iterator beginIterator;
    size_t _size;
};

}  // namespace renderscript
}  // namespace android

#endif  //  ANDROID_RENDERSCRIPT_LIST_H
