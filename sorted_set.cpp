#include "sorted_set.h"

SortedSet::~SortedSet() {
    // by calling the sets' foreach(), we can delete
    // all the allocated Set* pointer in sets.
    sets.foreach(&remove_set<Set*>);
    indexerList.foreach(&remove_set<Indexer*>);
}

void SortedSet::add(Value set_id, Value key, Value score) {
    // Check if the set exists
    Set* set = NULL;
    sets.safe_get(set_id, set, create_set, this);
    Value old_score = InvalidValue;

    // Update the index
    if (set->get(key, old_score)) {
        // Update index only if (1) the key already exists and
        // (2) the updated value is not the same as the old one
        if (old_score != score) {
            IndexKey old_key = make_index_key(key, old_score);

            Indexer* indexer = NULL;
            indexerList.get(set_id, indexer);
            assert(indexer);

            indexer->remove(old_key);
            indexer->add(make_index_key(key, score), key);
        }
    } else {
        Indexer* indexer = NULL;
        indexerList.safe_get(set_id, indexer, 
                             create_skip_list, this);
        // the key doesn't exist before
        indexer->add(make_index_key(key, score), key);
    }

    set->add(key, score);
}
void SortedSet::remove(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        // Nothing to remove
        return;
    }

    Value score = InvalidValue;
    if(set->get(key, score)) {
        set->remove(key);
        Indexer* indexer = NULL;
        indexerList.get(set_id, indexer);
        // TODO: COULD THERE BE SOME inconsistency and potential 
        // harm here?
        assert(indexer); 
        
        indexer->remove(make_index_key(key, score));
    }
}
Value SortedSet::get(Value set_id, Value key) {
    Set* set = NULL;
    if (!sets.get(set_id, set)) {
        return InvalidValue;
    }
    Value val = InvalidValue;
    return set->get(key, val) ? val : InvalidValue;
}
Value SortedSet::size(Value set_id) {
    Set* set = NULL;
    return sets.get(set_id, set) ? set->size() : 0;
}
void SortedSet::get_range(Value* setBegin, Value* setEnd, 
                          Value lower, Value upper, 
                          SortedSet::Indexer::Callback callback,
                          void* args) {
    assert(lower <= upper);
    IndexKey low = make_index_key(0, lower);
    IndexKey high = make_index_key(MaxValue, upper);

    for (Value* setPos = setBegin; setPos != setEnd; ++setPos) {
        Indexer* indexer = NULL;
        if (!indexerList.get(*setPos, indexer)) {
            continue;
        }

        indexer->range(low, high, callback, args);
    }
}
