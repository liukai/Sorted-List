#ifndef QUORA_SORTED_SET_SERVER_H
#define QUORA_SORTED_SET_SERVER_H

#include "set_manager.h"
#include "operations.h"

class SortedSetServer {
public:
    SortedSetServer(int port, int max_connection = 10):
        port(port), max_connection(max_connection), server_socket(-1) {}
    ~SortedSetServer();

    int run();

private:
    int initialze_socket();
    void start_server();
    static void* handle_request(void* args);
    // TODO:
    static void close_client_socket(int* pFd, const char* error_message = NULL);

    static const Value INVALID = Value(-1);
    static const int PORT_SIZE = 10;
    static OperationRuler ruler;

    int port;
    int max_connection; // how many concurrent connection are supported
    int server_socket;
};


#endif
