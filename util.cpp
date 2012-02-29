#include <iostream>

using namespace std;

bool can_continue(int result, const char* operation) {
    if (result < 0) {
        cerr<<"[ERROR] "<<operation<<" fails"<<endl;
        return false;
    }

    return true;
}
