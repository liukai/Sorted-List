#ifndef QUORA_SKIP_LIST_H
#define QUORA_SKIP_LIST_H

#include <vector>
#include "lockable.h"
#include <ctime>
#include <cmath>

const int DEFAULT_LEVEL = 16;

template <class TKey, class TValue>
struct SkipNode : public Lockable {
    TKey key;
    TValue value;
    std::vector<SkipNode<TKey, TValue>*> next; // pointer to the next neighbors

    SkipNode(int level, const TKey& key, const TValue& value)
                : key(key), value(value), next(level + 1) {
    }
};

template <class TKey, class TValue>
class SkipList {
public:
    typedef void (*Callback)(TKey, TValue,void*);
    // TODO: this is not a good idea
    SkipList(const TKey& defaultKey = TKey(), 
             int max_level = DEFAULT_LEVEL): max_level(max_level) {
        header = new SkipNode<TKey, TValue>(DEFAULT_LEVEL, defaultKey, TValue()); level = 0;
    }
    ~SkipList() {
        delete header;
    }

    // QUERIES
    bool containsKey(const TKey& key);
    bool get(const TKey &key, TValue& val);
    void range(const TKey& from, const TKey& to, Callback callback, void* args);
    int size() const {
        return counter.get();
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value);
    void remove(const TKey& key);
private:
    SkipNode<TKey, TValue>* find(const TKey& key);
    int get_random_level();

    SkipNode<TKey, TValue> *header;
    SafeCounter counter;
    int max_level;
    int level;
};

template <class TKey, class TValue>
int SkipList<TKey, TValue>::get_random_level() {
    static const float P = 0.5;
    static bool first_time = true;

    // initialize the random seed
    if (first_time) {
        srand((unsigned) time(NULL));
        first_time = false;
    }

    // TODO: make sense of this
    // generate the level with a certain probability
    float rand_num = (float) rand() / RAND_MAX;
    int level = (int)(log(rand_num) / log(1 - P));
    return level < max_level ? level : max_level;
}

template <class TKey, class TValue>
SkipNode<TKey, TValue>* SkipList<TKey, TValue>::find(const TKey &key) {
    SkipNode<TKey, TValue> *pos = header;
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            pos = pos->next[i];
        }
    }
    return pos->next[0] == NULL ? NULL :
        (pos->next[0]->key == key) ? pos->next[0] :
        pos;
}
template <class TKey, class TValue>
bool SkipList<TKey, TValue>::containsKey(const TKey &key) {
    const SkipNode<TKey, TValue>* node = find(key);
    return node != NULL && node->key == key;
}
template <class TKey, class TValue>
bool SkipList<TKey, TValue>::get(const TKey &key, TValue& val) {
    SkipNode<TKey, TValue>* node = find(key);
    if (node == NULL)
        return false;

    if (key == node->key) {
        val = node->value;
        return true;
    } else {
        return false;
    }
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::range(const TKey& from, const TKey& to, 
        Callback callback, void* args) {
    const SkipNode<TKey, TValue>* pos = header;    
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < from) {
            pos = pos->next[i];
        }
    }

    // left bound found
    // TODO: it is possible that pos is NULL
    const SkipNode<TKey, TValue>* left = pos->next[0];
    if (left == NULL)
        return;

    for (int i = pos->next.size() - 1; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key <= to) {
            pos = pos->next[i];
        }
    }
    SkipNode<TKey, TValue>* right = pos->next[0];
    for (;left != right; left = left->next[0]) {
        callback(left->key, left->value, args);
    }
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::add(const TKey& key, const TValue& value) {
    SkipNode<TKey, TValue> *pos = header;
    std::vector<SkipNode<TKey, TValue>*> prev_list(max_level + 1);

    // Find all the neighbors
    // TODO: should be refactored
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            pos = pos->next[i];
        }
        prev_list[i] = pos; 
    }

    pos = pos->next[0];
    if (pos == NULL || pos->key != key) {        
        int new_level = get_random_level();

        if (new_level > level) {
            for (int i = level + 1; i <= new_level; i++) {
                prev_list[i] = header;
            }
            level = new_level;
        }

        pos = new SkipNode<TKey, TValue>(new_level, key, value);
        for (int i = 0; i <= new_level; i++) {
            pos->next[i] = prev_list[i]->next[i];
            prev_list[i]->next[i] = pos;
        }
        counter.increase();
    } else if(pos != NULL)  {
        // update the value
        pos->value = value;
    }
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::remove(const TKey &key) {
    SkipNode<TKey, TValue> *pos = header;    
    std::vector<SkipNode<TKey, TValue>*> prev_list(max_level + 1);

    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            pos = pos->next[i];
        }
        prev_list[i] = pos; 
    }
    pos = pos->next[0];

    if (pos->key == key) {
        for (int i = 0; i <= level; i++) {
            if (prev_list[i]->next[i] != pos)
                break;
            prev_list[i]->next[i] = pos->next[i];
        }

        delete pos;
        while (level > 0 && header->next[level] == NULL) {
            level--;
        }
        counter.decrease();
    }

}

#endif
