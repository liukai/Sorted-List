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
}
void SortedSet::remove(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        // Nothing to remove
        return;
    }

    set->remove(key);
}
Value SortedSet::get(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        return INVALID;
    }
    Value val = INVALID;
    return set->get(key, val) ? val : INVALID;
}
Value SortedSet::size(Value set_id){
    Set* set = NULL;
    return sets.get(set_id, set) ? set->size() : 0;
}

