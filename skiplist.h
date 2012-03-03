#ifndef QUORA_SKIP_LIST_H
#define QUORA_SKIP_LIST_H

#include <vector>
#include "lockable.h"
#include <ctime>
#include <cmath>
#include <cassert>
#include <set>

const int DEFAULT_LEVEL = 16;

template <class TKey, class TValue>
struct SkipNode : public Lockable {
    TKey key;
    TValue value;
    std::vector<SkipNode<TKey, TValue>*> next;

    SkipNode(int level, const TKey& key, const TValue& value)
                : key(key), value(value), next(level + 1) {
    }
};

template <class TKey, class TValue>
class SkipList {
    typedef SkipNode<TKey, TValue> Node;
    typedef std::vector<Node*> NodeList;
public:
    typedef void (*Callback)(const TKey&, const TValue&, void*);
    // @defaultKey: the header node is only a sentinel node, it 
    // should choose a "default key" that could differentiate 
    // itself from "normal" nodes.
    // TODO: this design can be improved
    SkipList(const TKey& defaultKey = TKey(), 
             int max_level = DEFAULT_LEVEL): max_level(max_level) {
        header = new Node(DEFAULT_LEVEL, defaultKey, TValue()); level = 0;
    }
    ~SkipList() {
        delete header;
    }

    // QUERIES
    bool containsKey(const TKey& key);
    bool get(const TKey &key, TValue& val);
    // range() will find the item with key ranging between [from, to].
    // Whenever an item is found, range() will invoke 'callback()' with 
    // 'args' as the callback parameter.
    void range(const TKey& from, const TKey& to, Callback callback, void* args);
    int size() const {
        return counter.get();
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value);
    void remove(const TKey& key);
private:
    Node* find(const TKey& key);
    // get_random_level() will generate the level at a "certain" probability
    // It will ensure the generated levels that could yield efficient structure.
    int get_random_level();
    static void release_locks(std::set<Node*>& nodes) {
        for (typename std::set<Node*>::iterator pos = nodes.begin();
             pos != nodes.end(); ++pos) {
            (*pos)->unlock();
        }
    }
    static void add_lock_if_unlocked(std::set<Node*>& nodes, Node* node) {
        if (node != NULL && nodes.find(node) == nodes.end()) {
            node->write_lock();
            nodes.insert(node);
        }
    }

    Node *header;
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

    // generate the level with a certain probability
    float rand_num = (float) rand() / RAND_MAX;
    int level = (int)(log(rand_num) / log(1 - P));
    return level < max_level ? level : max_level;
}

template <class TKey, class TValue>
SkipNode<TKey, TValue>* SkipList<TKey, TValue>::find(const TKey &key) {
    Node *pos = header;

    pos->read_lock();
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            Node* next = pos->next[i];
            next->read_lock();

            pos->unlock();
            pos = next;
        }
        if (pos->next[i] != NULL && pos->next[i]->key == key) {
            Node* next = pos->next[i];
            next->read_lock();
            pos->unlock();

            return next;
        }
    }
    pos->unlock();
    return NULL;
}

template <class TKey, class TValue>
bool SkipList<TKey, TValue>::containsKey(const TKey &key) {
    TValue val;
    return get(key, val);
}
template <class TKey, class TValue>
bool SkipList<TKey, TValue>::get(const TKey &key, TValue& val) {
    Node *pos = header;

    pos->read_lock();
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            Node* next = pos->next[i];
            next->read_lock();

            pos->unlock();
            pos = next;
        }
        if (pos->next[i] != NULL && pos->next[i]->key == key) {
            val = pos->next[i]->value;
            pos->unlock();

            return true;
        }
    }
    pos->unlock();
    return false;
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::range(const TKey& from, const TKey& to, 
        Callback callback, void* args) {
    const Node* pos = header;    
    pos->read_lock();

    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < from) {
            Node* next = pos->next[i];
            next->read_lock();

            pos->unlock();
            pos = next;
        }
    }

    if (pos->next[0] == NULL) {
        pos->unlock();
        return;
    }
    // left bound found
    Node* left = pos->next[0];
    left->read_lock();
    pos->unlock();
    pos = left;
    
    while (pos != NULL && (pos->key < to || pos->key == to)) {
        callback(pos->key, pos->value, args);

        Node* next = pos->next[0];
        if (next == NULL)
            break;
        next->read_lock();
        pos->unlock();
        pos = next;
    }
    pos->unlock();
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::add(const TKey& key, const TValue& value) {
    Node* pos = header;
    NodeList prev_list(max_level + 1, NULL);
    set<Node*> locked_nodes;

    // Lock the first two elements
    add_lock_if_unlocked(locked_nodes, pos);
    prev_list[level] = pos;

    for (int i = level; i >= 0; i--) {
        add_lock_if_unlocked(locked_nodes, pos->next[i]);
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            assert(locked_nodes.find(pos) != locked_nodes.end());
            pos->unlock();
            locked_nodes.erase(pos);

            pos = pos->next[i];
            assert(locked_nodes.find(pos) != locked_nodes.end());
            // Always keep locking two related nodes
            add_lock_if_unlocked(locked_nodes, pos->next[i]);
        }

        // if the key already exists
        if (pos->next[i] != NULL && pos->next[i]->key == key) {
            pos->next[i]->value = value;
            release_locks(locked_nodes);

            return;
        }

        prev_list[i] = pos; 
    }

    pos = pos->next[0];
    assert(pos == NULL || !(pos->key == key));

    int new_level = get_random_level();
    if (new_level > level) {
        for (int i = level + 1; i <= new_level; i++) {
            prev_list[i] = header;
        }
        level = new_level;
    }

    Node* new_node = new Node(new_level, key, value);
    for (int i = 0; i <= new_level; i++) {
        new_node->next[i] = prev_list[i]->next[i];
        prev_list[i]->next[i] = new_node;
    }
    release_locks(locked_nodes);
    counter.increase();
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::remove(const TKey &key) {
    Node *pos = header;    
    NodeList prev_list(max_level + 1);
    set<Node*> locked_nodes;
    add_lock_if_unlocked(locked_nodes, pos);

    for (int i = level; i >= 0; i--) {
        add_lock_if_unlocked(locked_nodes, pos->next[i]);
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            assert(locked_nodes.find(pos) != locked_nodes.end());
            pos->unlock();
            locked_nodes.erase(pos);

            pos = pos->next[i];
            assert(locked_nodes.find(pos) != locked_nodes.end());
            // Always keep locking two related nodes
            add_lock_if_unlocked(locked_nodes, pos->next[i]);
            // pos = pos->next[i];
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
    release_locks(locked_nodes);
}

#endif
