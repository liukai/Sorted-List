#ifndef SORTED_SET_OPERATIONS
#define SORTED_SET_OPERATIONS

enum Opeation { ADD = 1, REM = 2, SIZE, GET, GETRANGE };

// OperationRuler represents the operation-related rules
class OperationRuler {
public:
    static const int ARBITARY = -1;

    OperationRuler();
    int get_argument_count(Opeation op);
    bool is_valid_op(Opeation op) const { return op >= ADD && op <= GETRANGE; }

private:
    int argument_count[GETRANGE - ADD + 1];
};

#endif
