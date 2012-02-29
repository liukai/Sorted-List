#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include "util.h"
#include "sorted_set_server.h"
using namespace std;

OperationRuler SortedSetServer::ruler;

SortedSetServer::~SortedSetServer() {
    if (server_socket != INVALID) {
        close(server_socket);
    }
}

int SortedSetServer::run() {
    int result = initialze_socket();
    if (result < 0) {
        return result;
    }

    start_server();
    return 0;
}

// TODO: comments are needed
int SortedSetServer::initialze_socket() {
    // TODO: what is on?
    int on = 1;

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

void* SortedSetServer::handle_request(void* args) {
    int* client_socket = (int*) args;

    static const int BUFFER_SIZE = 1024;
    Value buffer[BUFFER_SIZE] = { '\0' };
    int bufferRead = robust_read(*client_socket, buffer, BUFFER_SIZE);
    if (bufferRead <= 0) {
        close_client_socket(client_socket,
                            "[Error] error occurs while reading data from client");
        return NULL;
    }

    // TODO: here I implicitly made the assumption that all requests will be fit
    // in the buffer
    int argument_count = bufferRead / sizeof(Value) - 1;
    if (argument_count <= 0) {
        close_client_socket(client_socket,
                            "[Error] Abort handling request: "
                            "The data from client is invalid ...");
        return NULL;
    }

    Opeation op = (Opeation)buffer[0];
    if (!ruler.is_valid_op(op)) {
        close_client_socket(client_socket, "Invalid operator");
        return NULL;
    }

    int expected_argument_count = ruler.get_argument_count(op);
    // if the arguments are more than we want, we simply ignore it.
    if (argument_count < expected_argument_count) {
        close_client_socket(client_socket,
                            "The data from client doesn't have enough argumetns");
        return NULL;
    }
    
    close_client_socket(client_socket);
    return NULL;
}

void SortedSetServer::start_server() {
    pthread_t receive_thread;
    signal(SIGPIPE, SIG_IGN); // TODO: what is this

    sockaddr_in client_address;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    while(true) {
        int client_socket = accept(server_socket, (sockaddr *) &client_address, &sin_size);
        if(can_continue(client_socket, "accept()")) {
            pthread_create(&receive_thread, NULL, handle_request, new int(client_socket));
        }
    }
}

void SortedSetServer::close_client_socket(int* pFd, const char* error_message) {
    // TODO: A WORKAROUND
    static Value error = INVALID;
    if (error_message == NULL) {
        write(*pFd, &error, sizeof(Value));
        cerr<<"[Error] "<<error_message<<endl;
    }

    close(*pFd);
    delete pFd;
}
