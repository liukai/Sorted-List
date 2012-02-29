#include "sorted_set_server.h"

#include "set_manager.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    SetManager manager;
    manager.add(1, 2, 3);
    manager.add(1, 1, 4);
    manager.add(1, 3, 2);
    manager.add(1, 2, 3);

    cout<<manager.get(1, 2)<<endl;
    cout<<manager.size(1)<<endl;
        /*
    SortedSetServer server(7999);
    server.run();*/
    return 0;
}




