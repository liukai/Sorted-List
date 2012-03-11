#ifndef QUORA_SORTED_LIST_H
#define QUORA_SORTED_LIST_H

#include "hash_map.h"
#include "skiplist.h"
#include "util.h"
#include <cmath>

class SortedSet {
public:
    // TODO to delete
    struct IndexKey {
        Value set_id;
        Value score;
        Value key;

        IndexKey(Value set_id = -1, Value score = -1, Value key = -1):
                set_id(set_id), score(score), key(key) {
        }
        bool operator== (const IndexKey other) const {
            return set_id == other.set_id &&
                   score == other.score &&
                   key == other.key;
        }
        bool operator< (const IndexKey other) const {
            if (set_id != other.set_id)
                return set_id < other.set_id;
            if (score != other.score)
                return score < other.score;
            return key < other.key;
        }
    };

    // TODO: rename this
    typedef long Key;
    typedef HashMap<Value, Value> Set;
    typedef HashMap<Value, Set*> Sets;

    typedef SkipList<Key, Value> Indexer;
    typedef HashMap<Value, Indexer*> IndexerList;

    // TODO: delete this
    typedef SkipList<IndexKey, Value> ScoreIndexer;
public:
    // @params max_set_size specifies the maximal set numbers.
    // @params max_element_size_in_set specifies how many elements, 
    //         at most, can be held in a single set.
    SortedSet(int max_set_size, int max_element_size_in_set):
              sets(max_set_size, &naive_hash),
              indexerList(max_set_size, &naive_hash) {
        this->max_element_size_in_set = max_element_size_in_set;
    }
    ~SortedSet();

    void add(Value set_id, Value key, Value score);
    void remove(Value set_id, Value key);
    Value get(Value set_id, Value key) ;
    Value size(Value set_id);
    void get_range(Value* setBegin, Value* setEnd, 
                             Value lower, Value upper, 
                             Indexer::Callback callback,
                             void* args);
private:
    // Set creation and destruction
    static Set* create_set(void* arg) {
        SortedSet* pThis = (SortedSet*) arg;
        return new Set(pThis->max_element_size_in_set, 
                           naive_hash);
    }
    static Indexer* create_skip_list(void* arg) {
        SortedSet* pThis = (SortedSet*) arg;
        // skiplist of level max_level can hold
        // 2^max_level elements
        int max_level = log(pThis->max_element_size_in_set) / log(2) + 1;
        return new Indexer(InvalidValue, max_level);
                           
    }
    template <class TValue>
    static void remove_set(const Value& key, TValue& val) {
        delete val;
    }

    // Index Keys
    Key make_index_key(Value key, Value score) {
        Key combinedKey = score;
        return (combinedKey << ValueBitSize) + key;
    }

    // Fields
    int max_element_size_in_set;
    Sets sets;
    IndexerList indexerList;
    ScoreIndexer indexer;
};

#endif
