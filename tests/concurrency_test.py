#!/usr/bin/python

from concurrency_test_config import *
import threading
import commands
import random
import sys


# -- Commands
def make_command(op, args):
    prefix = " ".join([CONFIG["client"], CONFIG["host"], str(CONFIG["port"])])
    arg_list = [OPERATIONS[op]]
    arg_list.extend(args)
    suffix = " ".join([str(item) for item in arg_list])
    return "%s '%s'" % (prefix, suffix)

def call(op, args):
    command = make_command(op, args)
    result = commands.getstatusoutput(command)
    max_trial = 5
    while (result[0] != 0) and max_trial > 0:
        print "Error occurs while calling: ", command
        print "\tDetails: ", result
        print "\tRedo(%s): " % max_trial, command
        result = commands.getstatusoutput(command)
        max_trial -= 1
    if max_trial == 0:
        exit(1)
    return result[1][:-1]
def run_sequence_commands(keys, set_id, op,
                          to_args = lambda set_id, key : (set_id, key)):
    for index, key in enumerate(keys):
        print "sequence command:", index
        call(op, to_args(set_id, key))

def run_parallel_commands(key_start, key_end, set_id, op,
                          is_valid = lambda key: True,
                          to_args = lambda set_id, key : (set_id, key)):
    # Shuffle
    key_range = range(key_start, key_end)
    random.shuffle(key_range)

    threads = []
    for key in xrange(key_start, key_end):
        if not is_valid(key):
            continue
        thread = threading.Thread(target=call,
                                  args=(op, to_args(set_id, key)))
        thread.start()
        threads.append(thread)

    wait_to_finish(threads)

# -- Unit Testing
def assert_equal(expected, actual, text = "", exit_on_error = True):
    if expected != actual:
        print "[UN-MATCHED] %s: expected <%s> but actual <%s>" % \
               (text, str(expected), str(actual))
        if exit_on_error:
            sys.exit(1)
    return expected == actual
def assert_size_equal(expected_size, set_id):
    return assert_equal("SIZE %d %d" % (set_id, expected_size),
                        call("SIZE", [set_id]))
def assert_range_equal(begin, end, expected_size,
                       expected_keys, sets,
                       key_to_value_fn):
    arguments = sets
    arguments.append(-1)
    arguments.append(begin)
    arguments.append(end)

    command = make_command("RANGE", arguments)

    print "before ... ", make_command("RANGE", arguments)
    result = call("RANGE", arguments).split()[len(arguments) + 1: -1]
    print "after ..."

    if not assert_equal(
            len(expected_keys), len(result) / 2,
            "Range size unmatch: " + command,
            True):
        return False

    for index in range(0, len(result), 2):
        key = result[index]
        val = result[index + 1]

        exp_key = expected_keys[index / 2]
        exp_val = key_to_value_fn(exp_key)
        assert_equal(str(exp_key), key, "key unmatch", True)
        assert_equal(str(exp_val), val, "value unmatch", True)

# -- Misc
def key_to_value_1(key):
    return 2 * key + 1
def key_to_value_2(key):
    return 2 * key + 1

def wait_to_finish(threads):
    for thread in threads:
        thread.join()

# -- main tests
def validate_results(set_id, key_start, key_end,
                     key_to_value_fn, check_range = False):
    # Test size
    assert_size_equal(key_end - key_start, set_id)

    # Test the range operation
    if check_range:
        assert_range_equal(key_to_value_fn(key_start),
                           key_to_value_fn(key_end),
                           key_end - key_start,
                           range(key_start, key_end), [set_id],
                           key_to_value_fn)

    # Test correctness of each element
    for key in xrange(key_start, key_end):
        expected_result = "GET %d %d %d" % \
                          (set_id, key, key_to_value_fn(key))
        actual_result = call("GET", [set_id, key])
        assert_equal(expected_result, actual_result)

def test_single_set(set_id, key_start, key_end):
    """ This function will run the currency test on a single set in the sorted list"""
    # -- Phase 1: Add a series of key/value pairs
    run_parallel_commands(key_start, key_end, set_id, "ADD",
                          to_args = lambda set_id,
                                    key: [set_id, key, key_to_value_1(key)])

    validate_results(set_id, key_start, key_end, key_to_value_1)

    # -- Phase 2: Update the values
    run_parallel_commands(key_start, key_end, set_id, "ADD",
                          to_args = lambda set_id, key: [set_id, key, key_to_value_2(key)])

    validate_results(set_id, key_start, key_end, key_to_value_2)

    # -- Phase 3: Remove the odd elements
    run_parallel_commands(key_start, key_end, set_id, "REM",
                          is_valid = lambda key: key % 2  == 0)

    # Phase 4: Validate the results again
    assert_size_equal((key_end - key_start) / 2, set_id)
    for key in xrange(key_start, key_end):
        if (key % 2 == 0):
            expected_result = "GET %d %d %d" % (set_id, key, -1)
        else:
            expected_result = "GET %d %d %d" % \
                              (set_id, key, key_to_value_2(key))
        actual_result = call("GET", [set_id, key])
        assert_equal(expected_result, actual_result)

    assert_range_equal(key_to_value_2(key_start), key_to_value_2(key_end),
                       (key_end - key_start) / 2,
                       range(key_start + 1, key_end, 2), [set_id],
                       key_to_value_2)

def batch_sets_test():
    key_start = CONFIG["test_key_start"]
    key_end = CONFIG["test_key_end"]

    for i in CONFIG["test_set_ids"]:
        print "Test single set: %d" % i
        test_single_set(i, key_start, key_end)

def single_set_parallel_test():
    """ Add many items to a single set """
    thread_count = CONFIG["thread_count"]
    count = CONFIG["updates_per_thread"]
    set_id = CONFIG["target_set"]

    keys = range(thread_count * count)
    random.shuffle(keys)

    # Batch add
    threads = []
    for i in xrange(thread_count):
        begin = i * count
        end = (i + 1) * count
        thread = threading.Thread(
                    target = run_sequence_commands,
                    args = (
                        keys[begin: end], set_id,
                        "ADD",
                        lambda set_id, key:
                                [set_id, key, key_to_value_1(key)]))
        threads.append(thread)
        thread.start()
    wait_to_finish(threads)

    print "Validating ..."
    validate_results(set_id, 0, thread_count * count,
                     key_to_value_1, True)

    # Batch removal
    threads = []
    for i in xrange(thread_count):
        begin = i * count
        end = (i + 1) * count
        thread = threading.Thread(
                    target = run_sequence_commands,
                    args = (
                        keys[begin: end], set_id,
                        "REM",
                        lambda set_id, key: [set_id, key]))
        threads.append(thread)
        thread.start()
    wait_to_finish(threads)

    print "Validating ..."
    assert_size_equal(0, set_id)

if "__main__" == __name__:
    single_set_parallel_test()
    # batch_sets_test()
