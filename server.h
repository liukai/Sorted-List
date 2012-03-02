#ifndef QUORA_SORTED_SET_SERVER_H
#define QUORA_SORTED_SET_SERVER_H

#include "sorted_set.h"
#include "commands.h"
#include "sorted_set.h"

class SortedSetServer {
    typedef void (*CommandHandler)(int client, Value* buffer, 
                                   int buffer_size, SortedSet* set);
    typedef std::pair<int, SortedSet*> ThreadArgs;
public:
    // Arguments:
    //   @max_connection: maximal parellel connection.
    //   @set_size: the size of the sorted set.
    SortedSetServer(int port, int max_connection = 10, int set_size = 4513):
            port(port), max_connection(max_connection),
            server_socket(-1), sorted_set(set_size, set_size) {
    }
    ~SortedSetServer();

    int run();

private:
    int initialze_socket();
    void start_server();

    // handle_request() will handle each incoming request.
    static void* handle_request(void* args);
    // Description: close_client_socket() is a helper function to close a client socket 
    //   If error_message is not NULL, then:
    //     1. send error message(-1) to client to indicate something wrong happens.
    //     2. Log error message
    static void close_client_socket(ThreadArgs* args, const char* error_message = NULL);

    // -- Commands handlers
    static void add(int client, Value* buffer, int buffer_size, SortedSet* set);
    static void remove(int client, Value* buffer, int buffer_size, SortedSet* set);
    static void size(int client, Value* buffer, int buffer_size, SortedSet* set);
    static void get(int client, Value* buffer, int buffer_size, SortedSet* set);
    static void get_range(int client, Value* buffer, int buffer_size, SortedSet* set);

    static CommandHandler commands[];
    static const int PORT_SIZE = 10;
    static CommandRules ruler;

    int port;
    int max_connection; // how many concurrent connection are supported
    int server_socket;
    SortedSet sorted_set;
};

#endif
