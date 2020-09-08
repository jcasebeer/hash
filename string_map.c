#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

struct string_map
{
    char   **keys;
    char   **vals;
    size_t *hash;
    size_t cap;
    size_t len;
};

size_t hash_string(char *str)
{
    size_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; 

    return hash;
}

void string_map_put(struct string_map *map, char *key, char *val)
{
    size_t slot = hash_string(key);
    size_t hash = slot;
    for(;;) {
        slot = slot & (map->cap - 1);
        if (map->keys[slot] == 0) {
            map->keys[slot] = key;
            map->vals[slot] = val;
            map->hash[slot] = hash;
            map->len++;
            return;
        } else if (map->hash[slot] == hash && strcmp(map->keys[slot], key) == 0) {
            map->vals[slot] = val;
            return;
        }
        slot++;
    }
}   
char *string_map_get(struct string_map *map, char *key)
{
    size_t slot = hash_string(key);
    size_t hash = slot;
    for(;;) {
        slot = slot & (map->cap - 1);
        if (map->keys[slot] == 0) {
            return NULL;
        } else if (map->hash[slot] == hash && strcmp(key, map->keys[slot]) == 0) {
            return map->vals[slot];
        }
        slot++;
    }
}

int main()
{
    size_t  hash_store[16] = {0};
    char    *key_store[16] = {0};
    char    *val_store[16] = {0};

    struct string_map map = {0};
    map.hash = hash_store;
    map.keys = key_store;
    map.vals = val_store;
    map.cap = 16;

    char *keys[] = {
        "wow",
        "peanut butter",
        "fish",
        "alvin",
        "1+1",
        "2+2",
        "3+3",
        "4+4"
    };

    char *vals[] = {
        "whoa",
        "jelly",
        "chips",
        "chipmunks",
        "2",
        "4",
        "6",
        "8"
    };

    int i;
    int key_count = sizeof(keys)/sizeof(*keys);

    for(i = 0; i<key_count; i++) {
        string_map_put(&map, keys[i], vals[i]);
    } 

    for(i = 0; i<key_count; i++) {
        assert(strcmp(string_map_get(&map, keys[i]), vals[i]) == 0);
    }
    assert(string_map_get(&map, "we never inserted this key") == 0);
    string_map_put(&map, "wow", "bazinga!");
    assert(strcmp(string_map_get(&map, "wow"),"bazinga!") == 0);

    for(i = 0; i<key_count; i++) {
        printf("%14s = %9s\n", keys[i], string_map_get(&map, keys[i]));
    }

    return 0;
}
