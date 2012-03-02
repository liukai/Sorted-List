#include "sorted_set.h"

// TODO: USED FOR TEST ONLY
#include <iostream>

SortedSet::~SortedSet() {
    sets.foreach(remove_set);
}

void SortedSet::add(Value set_id, Value key, Value score) {
    // Check if the set exists
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        set = new Set(max_set_size, naive_hash);
        sets.add(set_id, set);
    }
    
    set->add(key, score);    

    IndexKey indexKey(set_id, score, key);
    indexer.add(indexKey, key);
}
void SortedSet::remove(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        // Nothing to remove
        return;
    }

    Value score = INVALID;
    if(set->get(key, score)) {
        set->remove(key);
        IndexKey indexKey(set_id, score, key);
        indexer.remove(indexKey);
    }
}
Value SortedSet::get(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        return INVALID;
    }
    Value val = INVALID;
    return set->get(key, val) ? val : INVALID;
}
Value SortedSet::size(Value set_id) {
    Set* set = NULL;
    return sets.get(set_id, set) ? set->size() : 0;
}

