#!/usr/bin/python

from concurrency_test_config import *
import threading
import commands
import random
import sys

"""
Description:

"""

def key_to_value(key):
    return 2 * key + 1
def wait_to_finish(threads):
    for thread in threads:
        thread.join()
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
                       expected_keys, sets):
    arguments = sets
    arguments.append(-1)
    arguments.append(begin)
    arguments.append(end)

    result = call("RANGE", arguments).split()[len(arguments) + 1: -1]

    if not assert_equal(len(expected_keys), len(result) / 2, "Range size unmatch", True):
        return False

    for index in range(0, len(result), 2):
        key = result[index]
        val = result[index + 1]

        exp_key = expected_keys[index / 2]
        exp_val = key_to_value(exp_key)
        assert_equal(str(exp_key), key, "key unmatch", True)
        assert_equal(str(exp_val), val, "value unmatch", True)


# Threads
def run_parallel_commands(key_start, key_end, set_id, op,
                          is_valid = lambda key: True,
                          to_args = lambda set_id, key : (set_id, key)):
    threads = []

    # Shuffle
    key_range = random.shuffle(range(key_start, key_end))

    for key in xrange(key_start, key_end):
        if not is_valid(key):
            continue
        thread = threading.Thread(target=call,
                                  args=(op, to_args(set_id, key)))
        thread.start()
        threads.append(thread)

    wait_to_finish(threads)

def test_single_set(set_id):
    """ This function will run the currency test on a single set in the sorted list"""
    key_start = CONFIG["test_key_start"]
    key_end = CONFIG["test_key_end"]

    # -- Phase 1: Add a series of key/value pairs
    run_parallel_commands(key_start, key_end, set_id, "ADD",
                          to_args = lambda set_id, key: [set_id, key, key_to_value(key)])

    # -- Phase 2: Validate the results
    # Test size
    assert_size_equal(key_end - key_start, set_id)
    for key in xrange(key_start, key_end):
        expected_result = "GET %d %d %d" % (set_id, key, key_to_value(key))
        actual_result = call("GET", [set_id, key])
        assert_equal(expected_result, actual_result)

    assert_range_equal(key_to_value(key_start), key_to_value(key_end),
                       key_end - key_start,
                       range(key_start, key_end), [set_id])

    run_parallel_commands(key_start, key_end, set_id, "ADD",
                          to_args = lambda set_id, key: [set_id, key, key_to_value(key)])

    # Test size
    assert_size_equal(key_end - key_start, set_id)
    for key in xrange(key_start, key_end):
        expected_result = "GET %d %d %d" % (set_id, key, key_to_value(key))
        actual_result = call("GET", [set_id, key])
        assert_equal(expected_result, actual_result)

    assert_range_equal(key_to_value(key_start), key_to_value(key_end),
                       key_end - key_start,
                       range(key_start, key_end), [set_id])

    # -- Phase 3: Remove the odd elements

    run_parallel_commands(key_start, key_end, set_id, "REM",
                          is_valid = lambda key: key % 2  == 0)

    # Phase 4: Validate the results again
    assert_size_equal((key_end - key_start) / 2, set_id)
    for key in xrange(key_start, key_end):
        if (key % 2 == 0):
            expected_result = "GET %d %d %d" % (set_id, key, -1)
        else:
            expected_result = "GET %d %d %d" % (set_id, key, key_to_value(key))
        actual_result = call("GET", [set_id, key])
        assert_equal(expected_result, actual_result)

    assert_range_equal(key_to_value(key_start), key_to_value(key_end),
                       (key_end - key_start) / 2,
                       range(key_start + 1, key_end, 2), [set_id])

# test_single_set(1)
for i in range(500):
    print "call %d" % i
    # test_single_set(1)
    test_single_set(i + 2048)
