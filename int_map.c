#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h> 

struct map {
    uint32_t *keys;
    uint32_t *vals;
    uint32_t zero_val; // allows the use of "0" as a key
    uint32_t len;
    uint32_t cap;
};

void print_debug_map(struct map map)
{
    printf("len %d, cap %d\n", map.len, map.cap);
    int i;
    printf("Slot: aux Key: %4d, Val: %4d\n", 0, map.zero_val); 
    for (i = 0; i<map.cap; i++) {
        printf("Slot: %3d Key: %4d, Val: %4d\n", i, map.keys[i], map.vals[i]);
    }
}

uint32_t hash32(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

void map_put(struct map *map, uint32_t key, uint32_t val)
{
    if (key == 0) {
        map->zero_val = val;
        map->len++;
        return;
    }
    uint32_t slot = hash32(key);
    for(;;) {
        // map->cap MUST be a power of 2
        slot = slot & (map->cap - 1);
        if (map->keys[slot] == 0) {
            map->keys[slot] = key;
            map->vals[slot] = val;
            map->len++;
            return;
        } else if (map->keys[slot] == key) {
            map->vals[slot] = val;
            return;
        }
        slot++;
    }
}   

uint32_t map_get(struct map *map, uint32_t key)
{
    if (key == 0) {
        return map->zero_val;
    }
    uint32_t slot = hash32(key);
    for(;;) {
        slot = slot & (map->cap - 1);
        if (map->keys[slot] == key) {
            return map->vals[slot];
        } else if (map->keys[slot] == 0) {
            return 0;
        }
        slot++;
    }
}

void map_remove(struct map *map, uint32_t key)
{
    if (key == 0) {
        map->zero_val = 0;
        map->len--;
        return;
    }
    uint32_t slot = hash32(key);
    for(;;) {
        slot = slot & (map->cap - 1);
        if (map->keys[slot] == key) {
            map->keys[slot] = 0;
            map->vals[slot] = 0;
            map->len--;
            slot++;
            for(;;) {
                slot = slot & (map->cap - 1); 
                if (map->keys[slot] == 0) {
                    return;
                } else {
                    uint32_t old_key = map->keys[slot];
                    uint32_t old_val = map->vals[slot];
                    map->keys[slot] = 0;
                    map->vals[slot] = 0;
                    map->len--;
                    map_put(map, old_key, old_val);
                }
                slot++;
            }
        } else if (map->keys[slot] == 0) {
            return;
        }
        slot++;
    }
}

int main()
{
    int key_storage[32] = {0};
    int val_storage[32] = {0};

    struct map map = {0}; 
    map.keys = key_storage;
    map.vals = val_storage;
    map.cap  = 32;

    int i;
    for(i = 0; i<16; i++) {
        map_put(&map, i, (i+1)*2);
    }
    print_debug_map(map);
    for(i = 0; i<16; i++) {
        assert(map_get(&map, i) == (i+1)*2);
    }
    print_debug_map(map);
    for(i = 0; i<16; i++) {
        map_remove(&map, i);
        if (i < 15) {
            assert(map_get(&map, i+1) == (i+2)*2);
        }
    }
    print_debug_map(map);

    return 0;
}
