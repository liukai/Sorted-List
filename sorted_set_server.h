#ifndef QUORA_SORTED_SET_SERVER
#define QUORA_SORTED_SET_SERVER

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

    static const int INVALID = -1;
    static const int PORT_SIZE = 10;

    int port;
    int max_connection; // how many concurrent connection are supported
    int server_socket;
};


#endif
