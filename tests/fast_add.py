
from concurrency_test_config import *
import threading
import commands
import random
import sys

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

def parallel_add(set_id, count):
    for i in range(count):
        call("ADD", (set_id, i, i))

def parallel_rem(set_id, count):
    for i in range(count):
        call("REM", (set_id, i))

thread_count = 60
count = 400

threads = []
"""
for i in range(thread_count):
    print "ADD", i
    thread = threading.Thread(target=parallel_add,
                              args=(i, count))
    threads.append(thread)
    thread.start()

for t in threads:
    t.join()
"""

for i in range(thread_count):
    print "REM", i
    thread = threading.Thread(target=parallel_rem,
                              args=(i, count))
    threads.append(thread)
    thread.start()

for t in threads:
    t.join()
