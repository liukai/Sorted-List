#ifndef QUORA_LOCKABLE_H
#define QUORA_LOCKABLE_H

#include <pthread.h>

class Lockable {
public:
    Lockable() {
        pthread_mutex_init(&mutex, NULL);
    }
    virtual ~Lockable() {
        pthread_mutex_destroy(&mutex);
    }
    void lock() {
        pthread_mutex_lock(&mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex);
    }
private:
    pthread_mutex_t mutex;
};

#endif
