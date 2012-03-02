#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include "util.h"
#include "server.h"
using namespace std;

CommandRules SortedSetServer::ruler;
SortedSetServer::CommandHandler SortedSetServer::commands[] = {
    add, remove, size, get, get_range
};

SortedSetServer::~SortedSetServer() {
    if (server_socket != InvalidValue) {
        close(server_socket);
    }
}

// -- Running Server
int SortedSetServer::run() {
    int result = initialze_socket();
    if (result < 0) {
        return result;
    }

    start_server();
    return 0;
}

int SortedSetServer::initialze_socket() {
    // -- Get the address info
    char port_text[PORT_SIZE];
    sprintf(port_text, "%d", port);

    struct addrinfo* serv = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(NULL, port_text, &hints, &serv);
    if (!can_continue(result, "getaddrinfo()")) {
        freeaddrinfo(serv);
        return result;
    }   

    // -- create the socket
    server_socket = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
    if (!can_continue(server_socket, "socket()")) {
        freeaddrinfo(serv);
        return server_socket;
    }   

    int on = 1;
    result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (!can_continue(result, "setsockopt()")) {
        freeaddrinfo(serv);
        return result;
    }   

    // -- bind
    result = bind(server_socket, serv->ai_addr, serv->ai_addrlen);
    if (!can_continue(result, "bind()")) {
        freeaddrinfo(serv);
        return result;
    }   
    freeaddrinfo(serv); // destroy the serv as it is no longer needed.

    // -- listen
    result = listen(server_socket, max_connection);
    if(!can_continue(result, "listen()")) {
        return result;
    }

    return 0;
}

void SortedSetServer::start_server() {
    pthread_t receive_thread;

    sockaddr_in client_address;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    while(true) {
        int client_socket = accept(server_socket, (sockaddr *) &client_address, &sin_size);
        ThreadArgs arg = ThreadArgs(client_socket, &sorted_set);
        if(can_continue(client_socket, "accept()")) {
            pthread_create(&receive_thread, NULL, handle_request, new int(client_socket));
        }
    }
}

// -- HANDLE Client Requests
void* SortedSetServer::handle_request(void* args) {
    ThreadArgs* arguments = (ThreadArgs*) args;

    static const int BUFFER_SIZE = 1024;
    Value buffer[BUFFER_SIZE] = { '\0' };
    int bufferRead = robust_read(arguments->first, buffer, BUFFER_SIZE);
    if (bufferRead <= 0) {
        close_client_socket(arguments,
                            "[Error] error occurs while reading data from client");
        return NULL;
    }

    // TODO: here I implicitly made the assumption that all requests will be fit
    // in the buffer; not batch operations allowed.
    int argument_count = bufferRead / sizeof(Value) - 1;
    if (argument_count <= 0) {
        close_client_socket(arguments,
                            "[Error] Abort handling request: "
                            "The data from client is invalid ...");
        return NULL;
    }

    Command op = (Command)buffer[0];
    if (!ruler.is_valid_op(op)) {
        close_client_socket(arguments, "Invalid operator");
        return NULL;
    }

    int expected_argument_count = ruler.get_argument_count(op);
    // if the arguments are more than we want, we simply ignore the additional arguments.
    if (argument_count < expected_argument_count) {
        close_client_socket(arguments,
                            "The data from client doesn't have enough argumetns");
        return NULL;
    }
    
    // Dispatch the command
    commands[op - 1](arguments->first, buffer + 1, argument_count, arguments->second);
    close_client_socket(arguments);
    return NULL;
}

void SortedSetServer::close_client_socket(ThreadArgs* args, const char* error_message) {
    int fd = args->first;
    if (error_message == NULL) {
        write(fd, &InvalidValue, sizeof(Value));
        cerr<<"[Error] "<<error_message<<endl;
    }

    close(fd);
    delete args;
}

// Command handlers
void SortedSetServer::add(int client, Value* buffer, int buffer_size, SortedSet* set) {
    set->add(buffer[0], buffer[1], buffer[2]);
}
void SortedSetServer::remove(int client, Value* buffer, int buffer_size, SortedSet* set) {
    set->remove(buffer[0], buffer[1]);
}
void SortedSetServer::size(int client, Value* buffer, int buffer_size, SortedSet* set) {
    Value size = set->size(buffer[0]);
    write(client, &size, sizeof(Value));
}
void SortedSetServer::get(int client, Value* buffer, int buffer_size, SortedSet* set) {
    // If the value doesn't exist, set->get() will return InvalidValue(-1)
    Value val = set->get(buffer[0], buffer[1]);
    write(client, &val, sizeof(Value));
}
void write_back(const SortedSet::IndexKey& indexKey, const Value& _, void* arg) {
    int fd = *((int*)arg);
    write(fd, &indexKey.key, sizeof(Value));
    write(fd, &indexKey.score, sizeof(Value));
}
void SortedSetServer::get_range(int client, Value* buffer, 
                                int buffer_size, SortedSet* set) {
    Value* set_id_start = buffer;
    Value* buffer_end = buffer + buffer_size;

    while (buffer != buffer_end && *buffer != InvalidValue) {
        ++buffer;
    }
    Value* set_id_end = buffer;

    // Missing the "-1" as the separator?
    if (*buffer != InvalidValue) {
        write(client, &InvalidValue, sizeof(Value));
        return;
    }

    ++buffer; // skip the "-1" separator
    // check if there are enough room for "lower" and "upper"
    if (buffer_end - buffer < 2) {
        write(client, &InvalidValue, sizeof(Value));
        return;
    }
    Value lower = buffer[0];
    Value upper = buffer[1];
    set->get_range(set_id_start, set_id_end, lower, upper, write_back, &client);
    write(client, &InvalidValue, sizeof(Value));
}
