#include <cassert>
#include <ctime>
#include <cmath>

#include <iostream>
#include "sorted_set.h"
#include "hash_map.h"
#include "skiplist.h"
#include "util.h"

using namespace std;

// -- UTILITIES
void add_one(const int& key, const int& value, void* arg) {
    int* pCount = (int*) arg;
    (*pCount) += 1;
}
void add_one_for_index_key(const long& key, const int& value, 
                           void* arg) {
    int* pCount = (int*) arg;
    (*pCount) += 1;
}

void test_sorted_set() {
    SortedSet set(5209, 5209);

    // test the add operation
    assert(set.size(1) == 0);

    set.add(1, 2, 3);
    assert(set.size(1) == 1);
    assert(set.get(1, 2) == 3);

    set.add(1, 1, 4);
    assert(set.size(1) == 2);
    assert(set.get(1, 1) == 4);

    set.add(1, 3, 2);
    assert(set.size(1) == 3);
    assert(set.get(1, 3) == 2);

    set.add(1, 2, 5);
    assert(set.size(1) == 3);
    assert(set.get(1, 2) == 5);

    // test invalid "get" and "size" operation
    // -- set exists, key doesn't exist
    assert(set.size(0) == 0);
    assert(set.get(1, 0) == InvalidValue);
    // -- set doesn't exists
    assert(set.get(0, 0) == InvalidValue);

    // remove test
    // -- erase existing item
    set.remove(1, 2);
    assert(set.size(1) == 2);
    assert(set.get(1, 2) == InvalidValue);
    // -- erase set that doesn't exist
    set.remove(0, 2);
    // -- erase key that doesn't exist
    set.remove(1, 2);
    assert(set.size(1) == 2);
    assert(set.get(1, 2) == InvalidValue);

    // range test
    int count = 0;
    int set_id = 1;

    // update the value and see if the indexer changes
    for (int i = 100; i <= 105; ++i) {
        count = 0;
        set.add(1, 20, i);
        set.get_range(&set_id, &set_id + 1, 100, 105, add_one_for_index_key, &count);
        assert(1 == count);
    }
}
void test_sorted_set_range() {
    SortedSet set(5209, 5209);
    int set_ids[] = {1, 2, 3, 4};

    int count = 0;
    for (int i = 0; i < 100; ++i) {
        set.add(1, i, i);
    }
    set.get_range(set_ids, set_ids +1, 10, 20, add_one_for_index_key, &count);
    assert(count == 11);

    count = 0;
    for (int i = 0; i < 100; ++i) {
        int score = i * 2 + 1;
        set.add(2, i, score);
    }
    set.get_range(set_ids + 1, set_ids + 2, 10, 20, add_one_for_index_key, &count);
    assert(count == 5);

    count = 0;
    set.get_range(set_ids + 2, set_ids + 3, 10, 20, add_one_for_index_key, &count);
    assert(count == 0);

    count = 0;
    set.get_range(set_ids, set_ids + 3, 10, 20, add_one_for_index_key, &count);
    assert(count == 5 + 11);
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
    assert(count == 0);

    count = 0;
    dict.range(7, 12, add_one, &count);
    assert(count == 3);
}
void test_hash_set() {
    HashMap<int, int> hash_map(1299827, &naive_hash<int>);
    test_dict_functionality(hash_map);
}

int main(int argc, char* argv[]) {
    test_skip_list();
    test_hash_set();
    test_sorted_set();
    test_sorted_set_range();

    return 0;
}
