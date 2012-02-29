#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "util.h"
#include "sorted_set_server.h"

#include <iostream>
using namespace std;

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
    cout<<"new client"<<endl;
    return NULL;
}
void SortedSetServer::start_server() {
    pthread_t receive_thread;
    signal(SIGPIPE, SIG_IGN); // TODO: what is this

    sockaddr_in client_address;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    while(true) {
        int message_socket = accept(server_socket, (sockaddr *) &client_address, &sin_size);
        if(can_continue(message_socket, "accept()")) {
            /*
            receive_args* args = malloc(sizeof(receive_args));
            args->server = server;
            args->incoming_socket = message_socket;
            args->client_address = client_address;
            */
            pthread_create(&receive_thread, NULL, handle_request, NULL);
        }
    }
}
