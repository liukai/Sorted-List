#include <cassert>
#include <iostream>
#include "set_manager.h"

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

// TODO: add description
int main(int argc, char* argv[]) {
    set_manager_test();

    return 0;
}




