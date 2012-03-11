CONFIG={
    "client": "../client",
    "host": "127.0.0.1",
    "port": "7999", # to simplify the process, I make the port to be string

    # Test single set
    "test_set_ids": range(1, 150),
    "test_key_start": 100,
    "test_key_end": 150, # excluded

    # Single set parallel test
    "thread_count": 40,
    "updates_per_thread": 500,
    "target_set": 1051
}

OPERATIONS={
    "ADD": "1",
    "REM": "2",
    "SIZE": "3",
    "GET": "4",
    "RANGE": "5"
}

