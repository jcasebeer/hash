typedef struct {
    u64 *key; 
    u64 *val;
    u64 zero_val;
    u32 cap;
    u32 len;
    u8 flags;
}Map;

enum {
    MAP_HEAP = 1
};

u32 ceil_pow2(u32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void
map_init(Map *m, u32 n)
{
    u32 cap = ceil_pow2(n)*2;
    if (!cap)
        cap = 16;

    u64 *storage = calloc(cap*2, sizeof(*storage));
    m->key = storage;
    m->val = storage+cap;
    m->cap = cap;
    m->len = 0;
    m->flags |= MAP_HEAP;
}



void
map_free(Map *m)
{
    if (m->flags & MAP_HEAP == 0) return;
    if (m->key) free(m->key);
    m->val = NULL;
    m->cap = 0;
    m->len = 0;
}

u64
map_get(Map *map, u64 key)
{
    if (key == 0)
        return map->zero_val;

    u64 mask = map->cap - 1;
    u64 slot = key & mask;

    while(map->key[slot] && map->key[slot] != key)
        slot = (slot + 1) & mask;

    return map->val[slot];
}

void map_grow(Map *m);

void
map_put(Map *map, u64 key, u64 val)
{
    map_grow(map);

    if (key == 0)
        map->zero_val = val;

    u64 mask = map->cap - 1; 
    u64 slot = key & mask;
    while(map->key[slot] && map->key[slot] != key)
        slot = (slot + 1) & mask;

    map->key[slot] = key;
    map->val[slot] = val;
    map->len++;
}

void
map_grow(Map *m)
{
    // 75% load factor
    if (m->flags & MAP_HEAP && m->len >= (m->cap>>1)) {
        Map new = {0};
        new.zero_val = m->zero_val;
        map_init(&new, m->cap);
        for(u32 i = 0; i<m->cap; i++) {
            if (m->key[i])
                map_put(&new, m->key[i], m->val[i]);
        }
        map_free(m);
        *m = new;
    }
}

// fnv1a
u64
hash_str(char *str)
{
    u64 hash = 0xcbf29ce484222325; 
    for(; *str; str++)
        hash = (hash ^ *str) * 0x100000001B3;
    return hash;
}

// fnv1a
u64
hash_bytes(u8 *bytes, u64 length)
{
    u64 hash = 0xcbf29ce484222325; 
    for(u64 i = 0; i < length; i++)
        hash = (hash ^ bytes[i]) * 0x100000001B3;
    return hash;
}

// murmurhash 3 finalizer
u64
hash64(u64 key)
{
   key ^= (key >> 33);
   key *= 0xff51afd7ed558ccd;
   key ^= (key >> 33);
   key *= 0xc4ceb9fe1a85ec53;
   key ^= (key >> 33);
   return key;
}

void test_map()
{
    u64 keystore[32]={0};
    u64 valstore[32]={0};

    Map m = {
        .key = keystore,
        .val = valstore,
        .cap = 32
    };

    map_put(&m, 1, 2);
    map_put(&m, 2, 4);
    map_put(&m, 3, 6);
    map_put(&m, 4, 8);
    map_put(&m, 0, 0xDEADBEEF);
    map_put(&m, 32, 0xFEEDBEEF);

    assert(map_get(&m, 1) == 2);
    assert(map_get(&m, 2) == 4);
    assert(map_get(&m, 3) == 6);
    assert(map_get(&m, 4) == 8);
    assert(map_get(&m, 0) == 0xDEADBEEF);
    assert(map_get(&m, 32) == 0xFEEDBEEF);

    memset(keystore, 0, sizeof(keystore));
    memset(valstore, 0, sizeof(keystore));

    map_put(&m, hash_str("hello"), (u64)"world");
    map_put(&m, hash_str("peanut butter"), (u64)"jelly");
    map_put(&m, hash_str("silver"), (u64)"gold");
    map_put(&m, hash_str("DEAD"), (u64)"BEEF");

    assert(strcmp((char *)map_get(&m, hash_str("hello")), "world") == 0); 
    assert(strcmp((char *)map_get(&m, hash_str("peanut butter")), "jelly") == 0); 
    assert(strcmp((char *)map_get(&m, hash_str("silver")), "gold") == 0); 
    assert(strcmp((char *)map_get(&m, hash_str("DEAD")), "BEEF") == 0); 
    assert(hash_str("gMPflVXtwGDXbIhP73TX") == hash_str("LtHf1prlU1bCeYZEdqWf"));
}
