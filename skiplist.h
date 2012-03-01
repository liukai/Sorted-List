#ifndef QUORA_SKIP_LIST_H
#define QUORA_SKIP_LIST_H

#include <vector>
#include "lockable.h"

const int DEFAULT_LEVEL = 16;

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
    SkipList(const TKey& defaultKey = TKey(), int max_level = DEFAULT_LEVEL): count(0), max_level(max_level) {
        header = new SkipNode<TKey, TValue>(DEFAULT_LEVEL, defaultKey, TValue()); level = 0;

        pthread_rwlock_init(&count_lock, NULL);
    }
    ~SkipList() {
        delete header;
        pthread_rwlock_destroy(&count_lock);
    }

    // QUERIES
    bool containsKey(const TKey& key) const;
    bool get(const TKey &key, const TValue*& pVal) const;
    void range(const TKey& from, const TKey& to, Callback callback, void* args) const;
    int size() const {
        pthread_rwlock_rdlock(&count_lock);
        int size = count;
        pthread_rwlock_unlock(&count_lock);

        return size; 
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value);
    void print() const;
    void remove(const TKey& key);
private:
    const SkipNode<TKey, TValue>* find(const TKey& key) const;
    int get_random_level();

    mutable pthread_rwlock_t count_lock;
    SkipNode<TKey, TValue> *header;
    int count;
    int max_level;
    int level;
};

// Since for template class, theoretically, should be in the header file 
#include "skiplist.cpp"

#endif
