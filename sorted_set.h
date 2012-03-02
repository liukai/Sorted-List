#ifndef QUORA_SORTED_LIST_H
#define QUORA_SORTED_LIST_H

#include "hash_map.h"
#include "util.h"

typedef unsigned short Value;

class SortedSet {
    typedef HashMap<Value, Value> Set;
    typedef HashMap<Value, Set*> Sets;
public:
    static const Value INVALID = (Value)(-1);

    SortedSet(int max_size, int max_set_size): sets(max_size, naive_hash) {
        this->max_size = max_size;
        this->max_set_size = max_set_size;
    }
    ~SortedSet();

    void add(Value set_id, Value key, Value score);
    void remove(Value set_id, Value key);
    Value get(Value set_id, Value key) ;
    Value size(Value set_id);
    // get_range();
private:
    static void remove_set(const Value& key, Set*& val) {
        delete val;
    }
    int max_size;
    int max_set_size;
    Sets sets;
};

#endif
