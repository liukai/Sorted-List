#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <algorithm>
#include "util.h"
#include "commands.h"

using namespace std;
const char* opNames[] = {"None", "ADD", "REM", "SIZE", "GET", "RANGE"};

int parse_command(const char* input, int* buffer) {
    // check if the input is valid: all digits or spaces
    const char* pos = input;
    while (*pos != '\0') {
        if ((*pos >= '0' && *pos <= '9') || // digits
            *pos == ' ' || *pos == '\t' || *pos == '-') { // spaces
            ++pos;
        } else {
            cerr<<"# the input has invalid characters"<<endl;
            return -1;
        }
    }
    
    // get numbers
    pos = input;
    int* buffer_pos = buffer;
    while (*pos != '\0') {
        while (*pos == ' ' || *pos == '\t') {
            ++pos;
        }
        if (*pos == '\0')
            break;

        *buffer_pos++=  (int)atoi(pos);

        while ((*pos >= '0' && *pos <= '9') || *pos == '-') {
            ++pos;
        }
    }
    
    // validate the format
    CommandRules ruler;
    Command op = (Command)buffer[0];
    if (!ruler.is_valid_op(op)) {
        cerr<<"Invalid operation: "<<op<<endl;
        return -1;
    }

    int actual_argument_count = buffer_pos - buffer - 1;
    int expected_argument_count = ruler.get_argument_count(op);

    if (expected_argument_count != CommandRules::ARBITARY && 
        actual_argument_count != expected_argument_count) {
        cerr<<"the expected argument number is <"<<expected_argument_count<<
              "> but actual argument number is <"<<actual_argument_count<<">"<<endl;
        return -1;
    }
    return actual_argument_count + 1;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr<<"USAGE: client <host_address> <server_port> <operations>"<<endl;
        return 1;
    }
    const char* host = argv[1];
    int port = atoi(argv[2]);
    if (port == 0) {
        cerr<<"Invalid port format"<<endl;
    }

    const char* input = argv[3];

    // Create the socket
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!can_continue(sock, "socket()")) {
        return 1;
    }

    // Connect to the server
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host);
    server.sin_port = htons(port);

    int result = connect(sock, (sockaddr *) &server, sizeof(server));
    if (!can_continue(result, "connect()")) {
        return 1;
    }

    // Send the command to the server
    const int BUFFER_SIZE = 1024 * 24;
    int buffer[1024];
    int size = parse_command(input, buffer);

    if (size == -1) {
        cerr<<"Invalid input: "<<input<<endl;
        return 1;
    }

    to_network_order(buffer, buffer + size);
    int data_size = sizeof(int) * size;
    if (send(sock, buffer, data_size, 0) != data_size) {
        cerr<<"Send data fails!"<<endl;
        return 1;
    }
    bool first = true;
    int sepCount = 0;
    int op = 0;

    while (true) {
        int bytesRead = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        int receiveSize = bytesRead / sizeof(int);
        to_host_order(buffer, buffer + receiveSize);

        if (receiveSize != 0) {
            int i = 0;
            if (first) {
                op  = buffer[0];
                cout<<opNames[op];
                i = 1;
                first = false;
            }
            for (; i < receiveSize; ++i) {
                cout<<" "<<buffer[i];
            }
            cout.flush();

            Value* begin = buffer;
            Value* end = buffer + receiveSize;

            do {
                begin = find(begin, end, -1);
                if (begin != end) {
                    sepCount += 1;
                    begin += 1;
                }
            } while(begin < end);
        }
        if (op != 5 || sepCount >= 2) 
            break;
    }
    cout<<endl;
    cout<<endl;

    close(sock);
    return 0;
}

