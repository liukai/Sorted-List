#ifndef QUORA_UTIL
#define QUORA_UTIL

// This method will check the return value of Unix-style APIs(where result < 0
// indicates failure otherwise success) and display corresponding error message
bool can_continue(int result, const char* operation);


#endif

