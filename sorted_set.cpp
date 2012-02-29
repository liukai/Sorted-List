#include "sorted_set_server.h"

#include "set_manager.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    SortedSetServer server(7999);
    server.run();
    return 0;
}




