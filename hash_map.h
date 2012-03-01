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

    typedef typename std::pair<TKey, TValue> Slot;
    typedef std::list<Slot> Bucket;
    typedef typename std::vector<Bucket> Table;
    typedef typename Bucket::iterator SlotPointer;
public:
    typedef int (*HashFunction) (const TKey&);
    HashMap(int max_size, HashFunction fun): table(max_size), get_hash_code(fun)
                                             {
        locks = new Lockable[max_size];
    }

    // QUERIES
    bool containsKey(const TKey& key);
    bool get(const TKey &key, TValue*& pVal); 
    int size() const {
        return counter.get();
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value);
    void remove(const TKey& key);

private:
    int get_bucket_id(const TKey& key) {
        return get_hash_code(key) % table.size();
    }
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
    //std::vector<Lockable> locks;
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
bool HashMap<TKey, TValue>::get(const TKey &key, TValue*& pVal) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].read_lock();

    SlotPointer slot = find_slot(bucket, key);
    bool is_succeed = slot != bucket.end();
    pVal = is_succeed ? &slot->second : pVal;
    pVal = &slot->second;

    locks[bucket_id].unlock();

    return is_succeed;
}
template <class TKey, class TValue>
void HashMap<TKey, TValue>::add(const TKey& key, const TValue& value) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].write_lock();

    SlotPointer slot = find_slot(bucket, key);
    if (slot == bucket.end()) { // Insert new item
        bucket.push_back(make_pair(key, value));
    } else { // Update existing item
        slot->second = value;   
    }

    locks[bucket_id].unlock();

    counter.increase();
}
template <class TKey, class TValue>
void HashMap<TKey, TValue>::remove(const TKey& key) {
    int bucket_id = get_bucket_id(key);
    Bucket& bucket = table[bucket_id];
    locks[bucket_id].write_lock();

    SlotPointer slot = find_slot(bucket, key);
    if (slot != bucket.end()) { // Insert new item
        bucket.erase(slot);
    }
    locks[bucket_id].unlock();
    counter.decrease();
}

#endif
