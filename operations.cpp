#include "operations.h"
#include <limits.h>

OperationRuler::OperationRuler() {
    argument_count[ADD - 1] = 3;
    argument_count[REM - 1] = 2;
    argument_count[SIZE - 1] = 1;
    argument_count[GET - 1] = 2;
    argument_count[GETRANGE - 1] = ARBITARY;
}

int OperationRuler::get_argument_count(Opeation op) {
    if (!is_valid_op(op))
        return -1;
    return argument_count[op - 1];
}
