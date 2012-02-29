#ifndef QUORA_UTIL_H
#define QUORA_UTIL_H

// This method will check the return value of Unix-style APIs(where result < 0
// indicates failure otherwise success) and display corresponding error message
bool can_continue(int result, const char* operation);


#endif

