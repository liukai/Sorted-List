#ifndef QUORA_SET_MANAGER_H
#define QUORA_SET_MANAGER_H

#include <map>
#include <pthread.h>

typedef unsigned short Value;

// Concurrency control
class SetManager {
    typedef std::map<Value, Value> Set;
    typedef std::map<Value, Set> SetCollection;    
public:
    static const Value INVALID = (Value)(-1);

    SetManager() { pthread_mutex_init(&set_collection_lock, NULL); };
    ~SetManager() {
        pthread_mutex_destroy(&set_collection_lock);
    }

    void add(Value set_id, Value key, Value score);
    void remove(Value set_id, Value key);
    Value get(Value set_id, Value key) const;
    Value size(Value set_id) const;
    // get_range();
private:
    pthread_mutex_t set_collection_lock;
    SetCollection set_collection;
};

#endif
