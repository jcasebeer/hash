# hash
a collection of hash tables

# types
- struct map: uint32_t to uint32_t hash table
- struct string_map: char * to char * hash table.

# limitations
- hash tables are fixed size and pointers for storing their keys, values, and
(for string_map) hashes must be provided.
- the maximum capacity (cap member) of the hash tables must be a power of two
- for best performance tables shouldn't exceed 50% capacity
- trying to retrieve a non-existant key from a full table will result in an infinite loop

# todo
- string_map_remove function
