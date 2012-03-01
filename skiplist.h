#ifndef QUORA_SKIP_LIST_H
#define QUORA_SKIP_LIST_H

#include <vector>
#include "lockable.h"

const int MAX_LEVEL = 8;

template <class TKey, class TValue>
struct SkipNode : Lockable {
    TKey key;
    TValue value;
    std::vector<SkipNode<TKey, TValue>*> next; // pointer to the next neighbors

    SkipNode(int level, const TKey& key, const TValue& value)
                : key(key), value(value), next(level + 1) {
    }
};

template <class TKey, class TValue>
class SkipList {
public:
    typedef void Callback(TKey, TValue,void*);
    // TODO: this is not a good idea
    SkipList(const TKey& defaultKey = TKey()): count(0) {
        header = new SkipNode<TKey, TValue>(MAX_LEVEL, defaultKey, TValue());
        level = 0;

        pthread_rwlock_init(&count_lock, NULL);
    }
    ~SkipList() {
        delete header;
        pthread_rwlock_destroy(&count_lock);
    }

    // QUERY
    bool containsKey(const TKey& key) const;
    bool get(const TKey &key, const TValue*& pVal) const;
    void range(const TKey& from, const TKey& to, Callback callback, void* args) const;
    int size() const {
        pthread_rwlock_rdlock(&count_lock);
        int size = count;
        pthread_rwlock_unlock(&count_lock);

        return size; 
    }

    // COMMAND
    void add(const TKey& key, const TValue& value);
    void print() const;
    void remove(const TKey& key);
private:
    const SkipNode<TKey, TValue>* find(const TKey& key) const;
    SkipNode<TKey, TValue> *header;
    int level;

    mutable pthread_rwlock_t count_lock;
    int count;
};

// Since for template class, theoretically, should be in the header file 
#include "skiplist.cpp"

#endif
