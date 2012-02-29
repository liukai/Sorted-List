#ifndef QUORA_SET_MANAGER_H
#define QUORA_SET_MANAGER_H

#include <map>

typedef unsigned short Value;

// Concurrency control
class SetManager {
    typedef std::map<Value, Value> Set;
    typedef std::map<Value, Set> SetCollection;    
public:
    static const Value INVALID = (Value)(-1);

    void add(Value set_id, Value key, Value score) {
        set_collection[set_id][key] = score;
    }
    void remove(Value set_id, Value key) {
        SetCollection::iterator set_pos = set_collection.find(set_id);
        if (set_pos == set_collection.end()) {
            return; // nothing needs to be removed
        }
        set_pos->second.erase(key);
    }
    Value get(Value set_id, Value key) const {
        SetCollection::const_iterator set_pos = set_collection.find(set_id);
        if (set_pos == set_collection.end())
            return INVALID;
        Set::const_iterator key_pos = set_pos->second.find(key);
        if (key_pos == set_pos->second.end())
            return INVALID;
        return key_pos->second;
    }
    Value size(Value set_id) const {
        SetCollection::const_iterator pos = set_collection.find(set_id);
        return pos == set_collection.end() ? 0 : (Value)pos->second.size();
    }
    // get_range();
    private:
    SetCollection set_collection;
};

#endif
