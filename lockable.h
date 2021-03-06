#ifndef QUORA_LOCKABLE_H
#define QUORA_LOCKABLE_H

#include <pthread.h>
#include <cassert>

class Lockable {
public:
    Lockable() {
        pthread_rwlock_init(&lock, NULL);
    }
    virtual ~Lockable() {
        pthread_rwlock_destroy(&lock);
    }

    void read_lock() const {
        pthread_rwlock_rdlock(&lock);
    }
    void write_lock() const {
        pthread_rwlock_wrlock(&lock);
    }
    void unlock() const {
        pthread_rwlock_unlock(&lock);
    }
private:
    mutable pthread_rwlock_t lock;
};

class SafeCounter: public Lockable {
public:
    SafeCounter(): count(0) {}
    int get() const { 
        read_lock();
        int c = count;
        unlock();
        return c;
    }
    void increase() {
        write_lock();
        ++count;
        unlock();
    }
    void decrease() {
        write_lock();
        --count;
        unlock();
    }
private:
    int count;
};

#endif
