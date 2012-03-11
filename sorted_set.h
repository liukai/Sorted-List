#ifndef QUORA_SORTED_LIST_H
#define QUORA_SORTED_LIST_H

#include "hash_map.h"
#include "skiplist.h"
#include "util.h"
#include <cmath>

class SortedSet {
public:
    typedef long IndexKey;
    typedef HashMap<Value, Value> Set;
    typedef HashMap<Value, Set*> Sets;

    typedef SkipList<IndexKey, Value> Indexer;
    typedef HashMap<Value, Indexer*> IndexerList;
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
    IndexKey make_index_key(Value key, Value score) {
        IndexKey combinedKey = score;
        return (combinedKey << ValueBitSize) + key;
    }

    // Fields
    int max_element_size_in_set;
    Sets sets;
    IndexerList indexerList;
};

#endif
