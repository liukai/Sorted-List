Overview
=========================
The program is primarily composed of 2 parts:

* Network communication: This part handles the upper level network communication, such as transfer command/data between server and client, parse the incoming request, etc.
* Concurrent Data Structure: I implement 2 basic concurrent data structure: 
    - HashMap is used for fast item access (but with no range search support).
    - SkipList is used for O(logN) range search.
    - SortedList is the facade data sturcutre that wraps HashMap and SkipList.

Concurrent Data Structure
=========================
Hash Table
----------------
In concurrent hash table, items of the same hash code will be placed in a bucket(a linked list), which is also the basic unit for concurrency control. Each bucket will acquire a read lock for read operation and write lock for modification.

In this program, each `set` is a hash map, and there is also a "set of sets" hash table that is used for fast access of each set.

I use the "set id" and "key" as the hash code directly. My assumption is that the set ids and keys already have some random distribution. Of course, if such assumption isn't valid, we can also change the `get_hash_code` for the HashTable.

Also, for simplicity the HashMap is not scaleabl: it will not resize if there are too many elements.

Skip List
----------------
The skip list is a data structure that have complexity of O(logN) for adding, removing and accessing nodes. To get full introduction please view http://en.wikipedia.org/wiki/Skip_list.

Especially, for range search, the skip list can locate the node of smallest value with time complexity O(`logM`), where `M` is the size of the skip list. Since skip list is already sorted, so after we get the "lower" node, we can iterate forward the skip list until we're out of the "upper" bound. The total time complexity is O(`logM`) + `N`, where `N` is the number of nodes within a certain range.

The concurrency control of skip list is somehow like singly linked list. If we want to make some modification, we have to:

* lock each node and release only after we acquired the lock of next node.
* Every moment we will try to hold 2 locks, one for the previous node and one for current node. Doing this prevents a node's neighbors from being modified while we're trying to make some change to current node.
* For skip list we adopt the similiar idea, but the differnce is that in skip list we often have more than `n` neighbors, where `n > 0` and `n < log(N)` and `N` is the amount of all nodes.

In this program, skip list is used for indexing the `score`, the key of the skip list of a triple (set_id, score, key), so if we want to find the elements of set `20` whose score is in the range of `[100, 200]`, we essentially looking for the elements in between of `(20, 100, 0) and (20, 200, INT_MAX)`. The "key" in the triple's role is only to prevent duplicate hash keys(since the (set_id, score) pair is not unique).

SortedList
----------------
SortedList is a facade that combines both advantage of hash table and skip list. But the sorted list doesn't implement any mechanism to ensure the concurrency between hash table(storing sets) and skip list(indexing the scores). So it is possible that the set has update itself but the skip list is still keeping the out-of-date information(for a short time).

How to run the program
=========================
compile: `make`

run server: `./server`, the port number of the server is 7999.

run client: `./client <host> <port> <commands>`. For example you can call `./client 127.0.0.1 7999 "1 20 10 123`, the first number of the third argument indicate the operation(ADD = 1, REM = 2, SIZE = 3, GET = 4, GETRANGE = 5.).

run demo: `./demo.sh`. demo.sh is an example interaction.

Misc
=========================
there is also a executable file file called `unit_test`, but I'm sorry this code is not well written. To avoid use external unit test libraries I simply use the basic `assert` as the testing facility. In the make file every time when the `server` is re-compiled the `unit_test` will be executed(unit_test will output nothing if there is no error).

