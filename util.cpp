#include <iostream>
#include <errno.h>

using namespace std;

bool can_continue(int result, const char* operation) {
    if (result < 0) {
        cerr<<"[ERROR] "<<operation<<" fails"<<endl;
        return false;
    }

    return true;
}

size_t robust_read(int fd, void* buffer, size_t bufferSize) {
    size_t left = bufferSize;
    size_t dataRead;
    char* pos = (char*)buffer;

    while (left > 0) {
        if ((dataRead = read(fd, pos, left)) < 0) {
            if (errno == EINTR)
                dataRead = 0;
            else
                return -1;
        }   
        else if (dataRead == 0) // file reading finished
            break;
        left -= dataRead;
        pos += dataRead;
    }   
    return (bufferSize - left);
}

size_t robust_write(int fd, void* buffer, size_t bufferSize) {
    size_t left = bufferSize;
    size_t dataWrite;
    char* pos = (char*) buffer;

    while (left > 0) {
        if ((dataWrite = write(fd, pos, left)) <= 0) {
            if (errno == EINTR)
                dataWrite = 0;
            else
                return -1;
        }
        left -= dataWrite;
        pos += dataWrite;
    }
    return bufferSize;
}

