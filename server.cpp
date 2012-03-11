#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include "util.h"
#include "server.h"

using namespace std;

// -- Constants
const int ARGUMENT_OFFSET = 1;
static const int BUFFER_SIZE = 1024;

// -- Utilities
bool are_valid_values(Value* begin, Value* end) {
    while (begin != end) {
        if (*begin < 0)
            return false;
        ++begin;
    }
    return true;
}
void write_to_network(int fd, Value* buffer, int size) {
    to_network_order(buffer, buffer + size);
    write(fd, buffer, size * sizeof(Value));
}
// WriteBackInfo stores the information needed for the
// write_to_client function.
struct WriteBackInfo {
    int client;
    Value* buffer;
    int count;
    const int max_size;
};
void write_to_client(const SortedSet::IndexKey& indexKey, 
                     const Value& _, void* arg) {
    WriteBackInfo* p_info = (WriteBackInfo*) arg;
    
    // if the buffer is almost full, write it back 
    if (p_info->count >= p_info->max_size - 2) {
        write_to_network(p_info->client, p_info->buffer, p_info->count);
        p_info->count = 0;
    }
    p_info->buffer[p_info->count++] = (Value)indexKey;
    p_info->buffer[p_info->count++] = indexKey>>ValueBitSize;
}
void append_to_buffer(const SortedSet::IndexKey& indexKey, const Value& _, void* arg) {
    Value** pos_ptr = (Value**) arg;

    Value key =  (Value)indexKey;
    Value val = indexKey>>ValueBitSize;

    **pos_ptr = key;
    *pos_ptr += 1; // update the pos
    **pos_ptr = val;
    *pos_ptr += 1;
}

// -- SortedSetServer 
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
            if (pthread_create(&receive_thread, NULL, handle_request, 
                    new ThreadArgs(client_socket, &sorted_set)) < 0) {
                cerr<<"[ERROR] create thread fails"<<endl;
                close(client_socket);
            }
        }
    }
}

// -- HANDLE Client Requests
void* SortedSetServer::handle_request(void* args) {
    cout<<"[INFO] new request arrives"<<endl;
    ThreadArgs* arguments = (ThreadArgs*) args;

    Value buffer[BUFFER_SIZE] = { '\0' };
    int bufferRead = read(arguments->first, buffer, BUFFER_SIZE);
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

    to_host_order(buffer, buffer + bufferRead);
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
    commands[op - 1](arguments->first, buffer, argument_count, arguments->second);
    close_client_socket(arguments);
    cout<<"[INFO] Request Processed!"<<endl;
    return NULL;
}

void SortedSetServer::close_client_socket(ThreadArgs* args, const char* error_message) {
    int fd = args->first;
    if (error_message != NULL) {
        cerr<<"[Error] "<<error_message<<endl;
        write(fd, &NetworkOrderInvalidValue, sizeof(Value));
    }

    close(fd);
    delete args;
}

// -- Command handlers
// Please note all the command handlers will assume the external functions pass
// enough arguments so they will *NOT* check the numbers of arguments.
void SortedSetServer::add(int client, Value* buffer, int arg_count, SortedSet* set) {
    buffer += ARGUMENT_OFFSET; // skip the operator
    if (!are_valid_values(buffer, buffer + 3)) {
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }
    set->add(buffer[0], buffer[1], buffer[2]);
}

void SortedSetServer::remove(int client, Value* buffer, int arg_count, SortedSet* set) {
    buffer += ARGUMENT_OFFSET; // skip the operator
    if (!are_valid_values(buffer, buffer + 2)) {
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }
    set->remove(buffer[0], buffer[1]);
}
void SortedSetServer::size(int client, Value* buffer, int arg_count, SortedSet* set) {
    Value* pos = buffer + ARGUMENT_OFFSET; // let pos point to the first argument  
    if (!are_valid_values(pos, pos + 1)) {
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }
    // Add the result right after the last argument

    *(pos + 1) = set->size(pos[0]);

    write_to_network(client, buffer, pos + 2 - buffer);
}

void SortedSetServer::get(int client, Value* buffer, int arg_count, SortedSet* set) {
    Value* pos = buffer + ARGUMENT_OFFSET; // let pos point to the first argument  
    
    Value* response_pos = pos + 2;
    *response_pos = set->get(pos[0], pos[1]); // If the value doesn't exist, set->get() 
                                           // will return InvalidValue(-1)
    write_to_network(client, buffer, response_pos + 1 - buffer);
}
void SortedSetServer::get_range(int client, Value* buffer, 
                                int arg_count, SortedSet* set) {
    Value* set_id_begin = buffer + ARGUMENT_OFFSET;
    Value* pos = buffer + ARGUMENT_OFFSET;
    Value* buffer_end = buffer + ARGUMENT_OFFSET + arg_count;

    while (pos != buffer_end && *pos != InvalidValue) {
        ++pos;
    }
    Value* set_id_end = pos;

    // Missing the "-1" as the separator?
    if (*pos != InvalidValue) {
        cerr<<"[Error] Missing -1 as the separator"<<endl;
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }
    // check the validity of the set ids
    if (!are_valid_values(set_id_begin, set_id_end)) {
        cerr<<"[Error] There are inalid set id"<<endl;
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }

    ++pos; // skip the "-1" separator
    // check if there are enough room for "lower" and "upper"
    if (buffer_end - pos < 2) {
        cerr<<"[Error] invlaid upper and lower bound"<<endl;
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }
    // check the validity of the "lower" and "upper"
    if (!are_valid_values(pos, pos + 2)) {
        cerr<<"[Error] invlaid upper and lower bound"<<endl;
        write(client, &NetworkOrderInvalidValue, sizeof(Value));
        return;
    }

    Value lower = pos[0];
    Value upper = pos[1];
    // Skip the `lower` and `upper`; so after this pos will point to 
    // the position ready to append command results.
    pos += 2;
    write_to_network(client, buffer, pos - buffer);

    // NOTE: here I pass the address to the 'pos' pointer. (pointer to a pointer).
    //       Doing this allows the callback function to have enough information 
    //       update the latest available position for append.
    to_host_order(set_id_begin, set_id_end);

    int avalable_size = BUFFER_SIZE - (pos - buffer);
    WriteBackInfo info = {client, pos, 0, avalable_size};
    set->get_range(set_id_begin, set_id_end, lower, upper, 
                   write_to_client, &info);

    // write back the rest of the buffer
    write_to_network(client, info.buffer, info.count);

    Value returnVal = InvalidValue;
    write_to_network(client, &returnVal, 1);
}
