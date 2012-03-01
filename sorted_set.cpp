#include "sorted_set.h"

void SortedSet::add(Value set_id, Value key, Value score) {
    pthread_mutex_lock(&set_collection_lock);
    set_collection[set_id][key] = score;
    pthread_mutex_unlock(&set_collection_lock);
}
void SortedSet::remove(Value set_id, Value key) {
    pthread_mutex_lock(&set_collection_lock);
    SetCollection::iterator set_pos = set_collection.find(set_id);
    if (set_pos == set_collection.end()) {
        pthread_mutex_unlock(&set_collection_lock);
        return; // nothing needs to be removed
    }
    set_pos->second.erase(key);
    pthread_mutex_unlock(&set_collection_lock);
}
Value SortedSet::get(Value set_id, Value key) const {
    SetCollection::const_iterator set_pos = set_collection.find(set_id);
    if (set_pos == set_collection.end())
        return INVALID;
    Set::const_iterator key_pos = set_pos->second.find(key);
    if (key_pos == set_pos->second.end())
        return INVALID;
    return key_pos->second;
}
Value SortedSet::size(Value set_id) const {
    SetCollection::const_iterator pos = set_collection.find(set_id);
    return pos == set_collection.end() ? 0 : (Value)pos->second.size();
}
