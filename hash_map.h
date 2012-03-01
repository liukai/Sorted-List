#ifndef QUORA_HASH_TABLE_H
#define QUORA_HASH_TABLE_H

#include <algorithm>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include "lockable.h"

template <class TKey, class TValue>
class HashMap{
    typedef std::pair<TKey, TValue> Slot;
    typedef std::list<Slot> Bucket;
    typedef std::vector<Bucket> Table;
public:
    typedef int (*HashFunction) (const TKey&);
    HashMap(int max_size, HashFunction fun): table(max_size), get_hash_code(fun) {
    }

    // QUERIES
    bool containsKey(const TKey& key) const {
        Bucket& bucket = get_bucket(key);
        bucket.read_lock();
        Slot* pSlot = find_slot(bucket, key);
        bool contained = pSlot != NULL;
        bucket.unlock();

        return contained;
    }
    bool get(const TKey &key, const TValue*& pVal) const {
        Bucket& bucket = get_bucket(key);
        bucket.read_lock();
        Slot* pSlot = find_slot(bucket, key);
        if (pSlot == NULL)
            return false;
        pVal = pSlot.second;
        bucket.unlock();

        return true;
    }
    int size() const {
        return counter.get();
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value) {
        Bucket& bucket = get_bucket(key);
        bucket.write_lock();
        Bucket::iterator slot = find_slot(bucket, key);
        if (slot == bucket.end()) { // Insert new item
            bucket.append(make_pair(key, value));
        } else { // Update existing item
            slot.second = value;   
        }
        bucket.unlock();
    }
    void remove(const TKey& key) {
        Bucket& bucket = get_bucket(key);
        bucket.write_lock();
        Bucket::iterator slot = find_slot(bucket, key);
        if (slot != bucket.end()) { // Insert new item
            bucket.erase(slot);
        }
        bucket.unlock();
    }
private:
    slot_compare()
    Bucket& get_bucket(const TKey& key) {
        return table[get_hash_code(key) % table.size()];
    }
    Bucket::iterator find_slot(Bucket& bucket, const TKey& key) {
        for(Bucket::iterator pos = bucket.begin(); pos != bucket.end(); ++pos) {
            if (pos->first == key)
                return pos;
        }
        return bucket.end();
    }
    Table table;
    SafeCounter counter;
    HashFunction get_hash_code;
};

#endif
