#ifndef QUORA_UTIL_H
#define QUORA_UTIL_H


typedef int Value;
const int InvalidValue = -1;
// NetworkOrderInvalidValue is the network ordered representation
// of InvalidValue
const int NetworkOrderInvalidValue = htonl(InvalidValue);
const int MinValue = 0;
const int MaxValue = 2147483647;

// This method will check the return value of Unix-style APIs(where result < 0
// indicates failure otherwise success) and display corresponding error message
bool can_continue(int result, const char* operation);

size_t robust_read(int fd, void* buffer, size_t bufferSize);
size_t robust_write(int fd, void* buffer, size_t bufferSize);

template <class TData>
void to_host_order(TData* begin, TData* end) {
    while (begin != end) {
        *begin= ntohl(*begin);
        ++begin;
    }
}
template <class TData>
void to_network_order(TData* begin, TData* end) {
    while (begin != end) {
        *begin= htonl(*begin);
        ++begin;
    }
}

template <class T>
inline int naive_hash(const T& item) { return (int)item; }

#endif

