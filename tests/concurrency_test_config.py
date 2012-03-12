CONFIG={
    "client": "../client",
    "host": "127.0.0.1",
    "port": "7999", # to simplify the process, I make the port to be string

    # Test single set
    "test_set_ids": range(1, 450),
    "test_key_start": 1100,
    "test_key_end": 1150, # excluded

    # Single set parallel test
    "thread_count": 50,
    "updates_per_thread": 100,
    "target_set": 1041
}

OPERATIONS={
    "ADD": "1",
    "REM": "2",
    "SIZE": "3",
    "GET": "4",
    "RANGE": "5"
}

