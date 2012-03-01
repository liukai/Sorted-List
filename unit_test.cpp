#include <cassert>
#include <iostream>
#include "set_manager.h"
#include "skiplist.h"

using namespace std;

void set_manager_test() {
    SetManager manager;

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
    assert(manager.get(1, 0) == SetManager::INVALID);
    // -- set doesn't exists
    assert(manager.get(0, 0) == SetManager::INVALID);

    // remove test
    // -- erase existing item
    manager.remove(1, 2);
    assert(manager.size(1) == 2);
    assert(manager.get(1, 2) == SetManager::INVALID);
    // -- erase set that doesn't exist
    manager.remove(0, 2);
    // -- erase key that doesn't exist
    manager.remove(1, 2);
    assert(manager.size(1) == 2);
    assert(manager.get(1, 2) == SetManager::INVALID);
}

void add_one(int key, int value, void* arg) {
    int* pCount = (int*) arg;
    (*pCount) += 1;
}
void test_skip_list_functionality() {
    SkipList<int, int> skip_list(-1);
    int expected_size = 0;
    // Insert some nodes in a reverse way ...
    for (int i = 300; i >= 0; --i) {
        const int* pVal = NULL;

        int key = i * 2;
        int val = i;
        skip_list.add(key, i);
        expected_size += 1;

        assert(skip_list.containsKey(key));
        assert(skip_list.get(key, pVal));
        assert(*pVal == val);
        assert(expected_size == skip_list.size());
    }

    // Check some non-existing items ...
    for (int i = 1000; i < 1100; ++i) {
        const int* pVal = NULL;
        assert(!skip_list.get(i, pVal));
        assert(!skip_list.containsKey(i));
    }

    // Test the range
    int count = 0;
    skip_list.range(10, 18, add_one, &count);
    assert(count == 5);
    count = 0;
    skip_list.range(9, 19, add_one, &count);
    assert(count == 5);
    count = 0;
    skip_list.range(10, 19, add_one, &count);
    assert(count == 5);
    count = 0;
    skip_list.range(9, 18, add_one, &count);
    assert(count == 5);

    count = 0;
    skip_list.range(7, 12, add_one, &count);
    assert(count == 3);

    // Test the remove
    for (int i = 0; i < 50; i += 3) {
        const int* pVal = NULL;
        int key = i * 2;
        skip_list.remove(key);
        assert(!skip_list.containsKey(key));
        assert(!skip_list.get(key, pVal));
    }
}

// TODO: add description
int main(int argc, char* argv[]) {
    set_manager_test();
    test_skip_list_functionality();

    return 0;
}




