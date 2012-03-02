#include <cassert>
#include <ctime>
#include <cmath>

#include <iostream>
#include "sorted_set.h"
#include "hash_map.h"
#include "skiplist.h"
#include "util.h"

using namespace std;

void test_sorted_set() {
    SortedSet manager(5209, 5209);

    // test the add operation
    assert(manager.size(1) == 0);

    manager.add(1, 2, 3);
    assert(manager.size(1) == 1);
    assert(manager.get(1, 2) == 3);

    manager.add(1, 1, 4);
    assert(manager.size(1) == 2);
    assert(manager.get(1, 1) == 4);

    manager.add(1, 3, 2);
    assert(manager.size(1) == 3);
    assert(manager.get(1, 3) == 2);

    manager.add(1, 2, 5);
    assert(manager.size(1) == 3);
    assert(manager.get(1, 2) == 5);

    // test invalid "get" and "size" operation
    // -- set exists, key doesn't exist
    assert(manager.size(0) == 0);
    assert(manager.get(1, 0) == SortedSet::INVALID);
    // -- set doesn't exists
    assert(manager.get(0, 0) == SortedSet::INVALID);

    // remove test
    // -- erase existing item
    manager.remove(1, 2);
    assert(manager.size(1) == 2);
    assert(manager.get(1, 2) == SortedSet::INVALID);
    // -- erase set that doesn't exist
    manager.remove(0, 2);
    // -- erase key that doesn't exist
    manager.remove(1, 2);
    assert(manager.size(1) == 2);
    assert(manager.get(1, 2) == SortedSet::INVALID);
}

void add_one(int key, int value, void* arg) {
    int* pCount = (int*) arg;
    (*pCount) += 1;
}

template <class TDict>
void test_dict_functionality(TDict& dict) {
    int expected_size = 0;
    // Insert some nodes in a reverse way ...
    for (int i = 300; i >= 0; --i) {
        int actual_val = NULL;

        int key = i * 2;
        int val = i;
        assert(!dict.containsKey(key));
        dict.add(key, i);
        expected_size += 1;

        assert(dict.containsKey(key));
        assert(dict.get(key, actual_val));
        assert(actual_val == val);
        assert(expected_size == dict.size());

        // duplicate add
        val = i * 2 + 1;
        dict.add(key, val);
        assert(dict.containsKey(key));
        assert(dict.get(key, actual_val));
        assert(actual_val == val);
        assert(expected_size == dict.size());
    }

    // Check some non-existing items ...
    for (int i = 1000; i < 1100; ++i) {
        int actual_val = NULL;
        assert(!dict.get(i, actual_val));
        assert(!dict.containsKey(i));
    }

    // Test the remove
    for (int i = 0; i < 50; i += 3) {
        int actual_val = NULL;
        int key = i * 2;
        dict.remove(key);
        --expected_size;
        assert(!dict.containsKey(key));
        assert(!dict.get(key, actual_val));
        assert(expected_size == dict.size());

        dict.remove(key);
        assert(!dict.containsKey(key));
        assert(!dict.get(key, actual_val));
        assert(expected_size == dict.size());
    }
    
    // add back the removed items
    for (int i = 0; i < 50; i += 3) {
        int actual_val = NULL;
        int key = i * 2;
        int val = i - 1;
        dict.add(key, val);
        ++expected_size;
        assert(dict.containsKey(key));
        assert(dict.get(key, actual_val));
        assert(actual_val == val);
        assert(expected_size == dict.size());
    }
}

void test_skip_list() {
    SkipList<int, int> dict(-1);
    test_dict_functionality(dict);

    // Test the range
    int count = 0;
    dict.range(10, 18, add_one, &count);
    assert(count == 5);
    count = 0;
    dict.range(9, 19, add_one, &count);
    assert(count == 5);
    count = 0;
    dict.range(10, 19, add_one, &count);
    assert(count == 5);
    count = 0;
    dict.range(9, 18, add_one, &count);
    assert(count == 5);

    count = 0;
    dict.range(9, 9, add_one, &count);
    cout<<count<<endl;
    assert(count == 0);

    count = 0;
    dict.range(7, 12, add_one, &count);
    assert(count == 3);
}
void test_hash_set() {
    // http://primes.utm.edu/lists/small/100000.txt
    HashMap<int, int> hash_map(1299827, &naive_hash<int>);
    test_dict_functionality(hash_map);
}

// TODO: add description
int main(int argc, char* argv[]) {
    test_skip_list();
    // test_hash_set();
    // test_sorted_set();

    return 0;
}
