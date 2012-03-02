#ifndef SORTED_SET_COMMANDS
#define SORTED_SET_COMMANDS

enum Command { ADD = 1, REM = 2, SIZE, GET, GETRANGE };

class CommandRules {
public:
    static const int ARBITARY = -1;

    CommandRules();
    int get_argument_count(Command op);
    bool is_valid_op(Command op) const { return op >= ADD && op <= GETRANGE; }

private:
    int argument_count[GETRANGE - ADD + 1];
};

#endif
