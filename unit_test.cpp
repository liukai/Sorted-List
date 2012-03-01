#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_LEVEL = 8;

template <class TKey, class TValue>
struct SkipNode {
    TKey key;
    TValue value;
    vector<SkipNode<TKey, TValue>*> next; // pointer to the next neighbors

    SkipNode(int level, const TKey& key, const TValue& value)
                            : key(key), value(value), next(level + 1) {
        
    }
};

template <class TKey, class TValue>
class SkipList {
public:
    SkipList() {
        header = new SkipNode<TKey, TValue>(MAX_LEVEL, TKey(), TValue());
        level = 0;
    }
    ~SkipList() {
        delete header;
    }

    void print() const;
    bool contains(const TKey &key) const;
    void add(const TKey& key, const TValue& value);
    void range(const TKey& from, const TKey& to);
    void remove(const TKey& key);
private:
    SkipNode<TKey, TValue> *header;
    int level;
};


int random_level() {
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

template <class TKey, class TValue>
void SkipList<TKey, TValue>::print() const {
    const SkipNode<TKey, TValue> *pos = header->next[0];
    cout << "{";
    while (pos != NULL) {
        /*
        cout<<"("<<pos->key<<","<<pos->value<<")";
        pos = pos->next[0];
        if (pos != NULL)
            cout << ",";
            */
        cout<<pos->next.size()<<", ";
        pos = pos->next[0];
    }    
    cout << "}" << endl;
}

template <class TKey, class TValue>
bool SkipList<TKey, TValue>::contains(const TKey &search_key) const {
    const SkipNode<TKey, TValue> *pos = header;
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < search_key) {
            pos = pos->next[i];
        }
    }
    pos = pos->next[0];

    return pos != NULL && pos->key == search_key;
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::range(const TKey& from, const TKey& to) {
    SkipNode<TKey, TValue>* pos = header;    
    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < from) {
            pos = pos->next[i];
        }
    }

    // left bound found
    // TODO: it is possible that pos is NULL
    SkipNode<TKey, TValue>* left = pos->next[0];
    cout<<left->key<<endl;

    for (int i = pos->next.size() - 1; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key <= to) {
            pos = pos->next[i];
        }
    }
    SkipNode<TKey, TValue>* right = pos;
    cout<<right->key<<endl;
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::add(const TKey& key, const TValue& value) {
    SkipNode<TKey, TValue> *pos = header;
    SkipNode<TKey, TValue> *prev_list[MAX_LEVEL + 1];
    memset(prev_list, 0, sizeof(SkipNode<TKey, TValue>*)*(MAX_LEVEL + 1));

    for (int i = level; i >= 0; i--) {
        while (pos->next[i] != NULL && pos->next[i]->key < key) {
            pos = pos->next[i];
        }
        prev_list[i] = pos; 
    }
    pos = pos->next[0];

    if (pos == NULL || pos->key != key) {        
        int lvl = random_level();

        if (lvl > level) {
            for (int i = level + 1; i <= lvl; i++) {
                prev_list[i] = header;
            }
            level = lvl;
        }

        pos = new SkipNode<TKey, TValue>(lvl, key, value);
        for (int i = 0; i <= lvl; i++) {
            pos->next[i] = prev_list[i]->next[i];
            prev_list[i]->next[i] = pos;
        }

    }
}

template <class TKey, class TValue>
void SkipList<TKey, TValue>::remove(const TKey &key) {
    SkipNode<TKey, TValue> *pos = header;    
    vector<SkipNode<TKey, TValue>*> prev_list(MAX_LEVEL + 1);

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

    }
}

int main() {

    SkipList<int, int> ss;
    ss.print();

    for (int i = 0; i < 100; ++i) {
        ss.add(i * 2, i * 2);
    }

    if (ss.contains(8)) {
        cout << "8 is in the list" << endl;
    }

    ss.print();

    ss.remove(8);
    ss.print();

    ss.range(7, 14);
    ss.range(7, 15);
    ss.range(1, 15);

    if (!ss.contains(8)) {
        cout << "7 has been deleted" << endl;
    }
    return 0;
}
