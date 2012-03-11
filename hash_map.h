#ifndef QUORA_HASH_TABLE_H
#define QUORA_HASH_TABLE_H

#include <algorithm>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include "lockable.h"

template <class TKey, class TValue>
class HashMap {
    typedef std::pair<TKey, TValue> Slot;
    typedef std::list<Slot> Bucket;
    typedef typename std::vector<Bucket> Table;
    typedef typename Bucket::iterator SlotPointer;
public:
    typedef int (*HashFunction) (const TKey&);
    typedef void (*ForeachAction) (const TKey&, TValue&);
    typedef TValue (*ValueGenerator)(void*);

    HashMap(int max_size, HashFunction fun): table(max_size), get_hash_code(fun) {
        locks = new Lockable[max_size];
    }
    ~HashMap() {
        delete[] locks;
    }

    // QUERIES
    bool containsKey(const TKey& key);
    bool get(const TKey &key, TValue& pVal); 
    void safe_get(const TKey &key, TValue& val, ValueGenerator defaultValueValueGenerator, void* arg); 
    int size() const {
        return counter.get();
    }

    // COMMANDS
    // -- the add() will return true if the key/value doesn't 
    // exist before; otherwise it returns false
    bool add(const TKey& key, const TValue& value);
    // -- the remove() will return true if the key exists
    // and the corresponding item has been deleted; otherwise
    // it returns false.
    bool remove(const TKey& key);

    // foreach() provide a way to travers all elements
    // in the HashMap
    void foreach(ForeachAction action);
private:
    int get_bucket_id(const TKey& key) {
        return get_hash_code(key) % table.size();
    }
    // find a slot with a specific key in a bucket, it will return
    // bucket.end() if nothing found
    SlotPointer find_slot(Bucket& bucket, const TKey& key) {
        for(SlotPointer pos = bucket.begin(); pos != bucket.end(); ++pos) {
            if (pos->first == key)
                return pos;
        }
        return bucket.end();
    }

    Table table;
    SafeCounter counter;
    HashFunction get_hash_code;
    Lockable* locks;
};

template <class TKey, class TValue>
bool HashMap<TKey, TValue>::containsKey(const TKey& key) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].read_lock();

    SlotPointer slot = find_slot(bucket, key);
    bool contained = (slot != bucket.end());

    locks[bucket_id].unlock();

    return contained;
}
template <class TKey, class TValue>
bool HashMap<TKey, TValue>::get(const TKey &key, TValue& val) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].read_lock();

    SlotPointer slot = find_slot(bucket, key);
    bool is_succeed = slot != bucket.end();
    if (is_succeed)
        val = slot->second;

    locks[bucket_id].unlock();

    return is_succeed;
}
template <class TKey, class TValue>
void HashMap<TKey, TValue>::safe_get(const TKey& key, TValue& value,
                                     HashMap::ValueGenerator defaultValueValueGenerator, void* arg) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].write_lock();

    SlotPointer slot = find_slot(bucket, key);

    if (slot == bucket.end()) { // Insert new item
        value = defaultValueValueGenerator(arg);
        bucket.push_back(std::make_pair(key, value));
        counter.increase();
    } else { // Update existing item
        value = slot->second;
    }

    locks[bucket_id].unlock();
}


template <class TKey, class TValue>
bool HashMap<TKey, TValue>::add(const TKey& key, const TValue& value) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].write_lock();

    SlotPointer slot = find_slot(bucket, key);
    bool added = slot == bucket.end();
    if (added) { // Insert new item
        bucket.push_back(std::make_pair(key, value));
        counter.increase();
    } else { // Update existing item
        slot->second = value;   
    }

    locks[bucket_id].unlock();
    return added;
}
template <class TKey, class TValue>
bool HashMap<TKey, TValue>::remove(const TKey& key) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].write_lock();

    SlotPointer slot = find_slot(bucket, key);
    // nothing will happen when the key doesn't exist
    bool found = slot != bucket.end();
    if (found) {
        bucket.erase(slot);
        counter.decrease();
    }
    locks[bucket_id].unlock();
    return found;
}

template <class TKey, class TValue>
void HashMap<TKey, TValue>::foreach(ForeachAction action) {
    for (size_t i = 0; i < table.size(); ++i) {
        Bucket& bucket = table[i];
        for (SlotPointer pos = bucket.begin(); pos != bucket.end(); ++pos) {
            action(pos->first, pos->second);
        }
    }
}

#endif
