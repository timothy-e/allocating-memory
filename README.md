# Allocating-Memory
Run test-pool.c
This testing client only allows for 26 different allocations (one per lower case letter), although the actual implementation is only limited by memory availability.


# Usage
`LET` is a lowercase letter from a..z to identify each allocation

`create INT`: create the pool of size `INT`
`destroy`: destroy the current pool
`alloc LET INT`: allocate memory to `LET` of size `INT`
`free LET`: free memory associated with `LET`
`realloc LET INT`: reallocate memory associated with `LET` to size `INT`
`active`: print active
`available`: print available
`exit`: quit
