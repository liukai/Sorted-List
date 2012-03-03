#ifndef QUORA_SORTED_LIST_H
#define QUORA_SORTED_LIST_H

#include "hash_map.h"
#include "skiplist.h"
#include "util.h"

class SortedSet {
public:
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

    typedef HashMap<Value, Value> Set;
    typedef HashMap<Value, Set*> Sets;
    typedef SkipList<IndexKey, Value> ScoreIndexer;

public:
    static const Value INVALID = (Value)(-1);
    SortedSet(int max_set_size, int max_set_size_element_size):
                sets(max_set_size, naive_hash), indexer() {
        this->max_set_size_element_size = max_set_size_element_size;
    }
    ~SortedSet();

    void add(Value set_id, Value key, Value score);
    void remove(Value set_id, Value key);
    Value get(Value set_id, Value key) ;
    Value size(Value set_id);
    void get_range(Value* setBegin, Value* setEnd, 
                   Value lower, Value upper, ScoreIndexer::Callback callback, void* args) {
        for (Value* setPos = setBegin; setPos != setEnd; ++setPos) {
            IndexKey low(*setPos, lower, MinValue);
            IndexKey high(*setPos, upper, MaxValue);
            indexer.range(low, high, callback, args);
        }
    }
private:
    static void remove_set(const Value& key, Set*& val) {
        delete val;
    }
    int max_set_size_element_size;
    Sets sets;
    ScoreIndexer indexer;
};

#endif
