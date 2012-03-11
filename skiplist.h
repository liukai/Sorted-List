#ifndef QUORA_SKIP_LIST_H
#define QUORA_SKIP_LIST_H

#include <vector>
#include "lockable.h"
#include <ctime>
#include <cmath>
#include <cassert>
#include <set>

const int DEFAULT_LEVEL = 32;

template <class TKey, class TValue>
struct SkipNode : public Lockable {
    TKey key;
    TValue value;
    std::vector<SkipNode<TKey, TValue>*> next;

    SkipNode(int level, const TKey& key, const TValue& value): 
        key(key), value(value), next(level + 1) {
    }
};

template <class TKey, class TValue>
class SkipList {
    typedef SkipNode<TKey, TValue> Node;
    typedef std::vector<Node*> NodeList;
    typedef std::set<Node*> LockedNodes;
public:
    typedef void (*Callback)(const TKey&, const TValue&, void*);
    // @defaultKey: the header node is only a sentinel node, it 
    // should choose a "default key" that could differentiate 
    // itself from "normal" nodes.
    SkipList(const TKey& defaultKey = TKey(), 
             int max_level = DEFAULT_LEVEL): 
            MAX_LEVEL(max_level), PREV_LIST_SIZE(MAX_LEVEL + 2) {
        header = new Node(DEFAULT_LEVEL, defaultKey, TValue()); level = 0;
    }
    // In the deletion function we don't check the locks
    ~SkipList() {
        Node* node = header;
        while(node != NULL) {
            Node* next = node->next[0];
            delete node;
            node = next;
        }
    }

    // QUERIES
    bool containsKey(const TKey& key);
    bool get(const TKey &key, TValue& val);
    // range() will find the item with key ranging between [from, to].
    // Whenever an item is found, range() will invoke 'callback()' with 
    // 'args' as the callback parameter.
    void range(const TKey& from, const TKey& to,
               Callback callback, void* args);
    int size() const {
        return counter.get();
    }

    // COMMANDS
    void add(const TKey& key, const TValue& value);
    void remove(const TKey& key);
private:
    // -- Utilities
    // get_random_level() will generate the level at a "certain" probability
    // It will ensure the generated levels that could yield efficient structure.
    int get_random_level();
    // find_node_for_write() finds the node with specific key and
    // locks the key properly(with hand-in-hand lock).
    // @params prev: the initial "previous" node, whose key is guarantee to 
    //               be less than key.
    // @params prev_list: output parameter. After calling this method,
    //                    the prev_list will be filled all the previous
    //                    nodes of the serched node(if such node doesn't
    //                    exist, it will be the first node whose value is
    //                    greater than `key`)
    // @returns: return the node with the corresponding key; if such node
    //           doesn't exist, returns the first node whose value is
    //           greater than `key`.
    Node* find_node_for_write(const TKey&, Node* prev,
                         SkipList<TKey, TValue>::NodeList& prev_list,
                         LockedNodes& locked_nodes);
    
    // -- Lock helpers
    bool can_lock(NodeList& prev_list, Node* pos, int i);
    static void add_to_locked_nodes(LockedNodes& nodes, Node* node);
    static void release_locks(LockedNodes& nodes);
    static void remove_from_locked_nodes(LockedNodes& nodes, Node* node);

    // -- Fields
    Node *header;
    SafeCounter counter;
    const int MAX_LEVEL;
    const int PREV_LIST_SIZE;
    int level;
};

// -- Queries
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
            pos->next[i]->read_lock();
            pos->unlock();
            pos = pos->next[i];

            val = pos->value;
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

// -- Commands
template <class TKey, class TValue>
SkipNode<TKey, TValue>* SkipList<TKey, TValue>::find_node_for_write(
        const TKey& key, SkipNode<TKey, TValue>* prev,
        SkipList<TKey, TValue>::NodeList& prev_list,
        LockedNodes& locked_nodes) {
    Node* pos = NULL;

    for (int i = level; i >= 0; i--) {
        // Invariance: the header is locked
        pos = prev->next[i];
        assert(locked_nodes.find(prev) != locked_nodes.end());

        if (can_lock(prev_list, pos, i)) {
            pos->write_lock();
            add_to_locked_nodes(locked_nodes, pos);
        }

        int unlocked_exemption = 0;
        while (pos != NULL && pos->key < key) {
            assert(locked_nodes.find(pos) != locked_nodes.end());

            if (prev != prev_list[i + 1]) {
                prev->unlock();
                remove_from_locked_nodes(locked_nodes, prev);
            } else {
                unlocked_exemption++;
                assert(unlocked_exemption <= 1);
            }

            prev = pos;
            pos = pos->next[i];

            if (pos == NULL)
                break;

            if (can_lock(prev_list, pos, i)) {
                pos->write_lock();
                add_to_locked_nodes(locked_nodes, pos);
            }
            // Always keep locking two related nodes
        }

        assert(prev->key <= key);
        assert(pos == NULL || !(pos->key < key));
        prev_list[i] = prev; 
    }
    return pos;
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::add(const TKey& key, const TValue& value) {
    Node* prev = header;
    NodeList prev_list(PREV_LIST_SIZE, NULL);
    LockedNodes locked_nodes;

    // generate new level
    // WARNING: Please note if the new node has a level that is greater
    // the current one, we have to make sure to lock the header node 
    // now(and won't release it during the hand-in-hand locking).
    // Deadlock may occur if lock the header after calling 
    // find_node_for_write() -- IT IS IMPORTNAT to lock the node in 
    // ascending/descending order uniformly. 
    header->write_lock();
    int new_level = get_random_level();
    add_to_locked_nodes(locked_nodes, header);

    if (new_level > level) {
        for (int i = level + 1; i <= new_level; i++) {
            assert(header->next[i] == NULL);
            prev_list[i] = header;
        }
        level = new_level;
    }

    Node* pos = find_node_for_write(key, prev, prev_list, locked_nodes);

    // if the key already exists, then only update the value
    if (pos != NULL && pos->key == key) {
        pos->value = value;
        release_locks(locked_nodes);

        return;
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
    // -- Step 1: locate
    Node* prev = header;
    NodeList prev_list(PREV_LIST_SIZE, NULL);
    LockedNodes locked_nodes;
    
    header->write_lock();
    add_to_locked_nodes(locked_nodes, header);
    Node* pos = find_node_for_write(key, prev, prev_list, locked_nodes);

    if (pos != NULL && pos->key == key) {
        for (int i = 0; i <= level; i++) {
            if (prev_list[i]->next[i] != pos)
                break;
            prev_list[i]->next[i] = pos->next[i];
        }

        pos->unlock();
        remove_from_locked_nodes(locked_nodes, pos);
        delete pos;

        while (level > 0 && header->next[level] == NULL) {
            level--;
        }
        counter.decrease();
    }
    release_locks(locked_nodes);
}

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
    return level < MAX_LEVEL ? level : MAX_LEVEL;
}

// -- Lock Helpers
template <class TKey, class TValue>
bool SkipList<TKey, TValue>::can_lock(SkipList<TKey, 
                                      TValue>::NodeList& prev_list, 
                                      SkipList<TKey, TValue>::Node* pos, 
                                      int i) {
    assert(prev_list[i + 1] == NULL || 
           (int)prev_list[i + 1]->next.size() > i + 1);
    return (pos != NULL && 
        ((prev_list[i + 1] == NULL || 
         ((int)prev_list[i + 1]->next.size() > i + 1 && prev_list[i + 1]->next[i + 1] != pos))));
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::add_to_locked_nodes(
        SkipList<TKey, TValue>::LockedNodes& nodes, 
        SkipList<TKey, TValue>::Node* node) {
    assert(node != NULL);
    assert(nodes.find(node) == nodes.end());

    nodes.insert(node);
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::release_locks(
        SkipList<TKey, TValue>::LockedNodes& nodes) {
    for (typename LockedNodes::iterator pos = nodes.begin();
         pos != nodes.end(); ++pos) {
        (*pos)->unlock();
    }
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::remove_from_locked_nodes(
        SkipList<TKey, 
        TValue>::LockedNodes& nodes, 
        SkipList<TKey, TValue>::Node* node) {
    assert(node != NULL);
    assert(nodes.find(node) != nodes.end());

    nodes.erase(node);
}

#endif
