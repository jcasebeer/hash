#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

struct multihash {
    uint32_t   *key;
    uint32_t   *val;
    uint32_t unused; 
    size_t      cap;
};

uint32_t hash32(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

struct multihash multihash_init(uint32_t *key, uint32_t *val, size_t cap, uint32_t unused)
{
    struct multihash h = {0};
    size_t i;
    for(i = 0; i<cap; i++)
        key[i] = unused;
    h.key = key;
    h.val = val;
    h.unused = unused;
    h.cap = cap;
    return h;
}

void multihash_set(struct multihash *h, uint32_t key, uint32_t val)
{
    size_t slot = hash32(key) % (h->cap);
    while(h->key[slot] != h->unused)
        slot = (slot + 1) % (h->cap);
    h->key[slot] = key;
    h->val[slot] = val;
}   

size_t multihash_get(struct multihash *h, uint32_t key, uint32_t *out)
{
    size_t slot = hash32(key) % (h->cap);
    size_t found = 0;
    while(h->key[slot] != h->unused) {
        if (h->key[slot] == key)
            out[found++] = h->val[slot];
        slot = (slot + 1) % (h->cap);
    }
    return found;
}

void multihash_get_bitset(struct multihash *h, uint32_t key, uint64_t *out)
{
    size_t slot = hash32(key) % (h->cap);
    while(h->key[slot] != h->unused) {
        if (h->key[slot] == key) {
            uint64_t val = h->val[slot];
            out[val>>6] = out[val>>6] | (1 << (val & 63));
        }
        slot = (slot + 1) % (h->cap);
    }
}

enum {
    MAX_AABB=256,
    HASH_SIZE=MAX_AABB*4*2,
    WORLD_WIDTH=128,
    WORLD_HEIGHT=64,
    GRID_SQUARE_WIDTH = 16,
    GRID_SQUARE_HEIGHT =16 
};

typedef struct v2 {
    int x; 
    int y; 
}v2;

struct aabb {
    v2 min;
    v2 max;
};

int aabb_contains(struct aabb box, v2 point)
{
    return (point.x >= box.min.x && 
            point.x <  box.max.x && 
            point.y >= box.min.y &&
            point.y <  box.max.y);
}

uint32_t grid_location(int x, int y)
{
    return (x | ((y) << 16));
}

struct aabb grid_span(struct aabb box)
{
    box.min.x = box.min.x / GRID_SQUARE_WIDTH;
    box.min.y = box.min.y / GRID_SQUARE_HEIGHT;
    box.max.x = box.max.x / GRID_SQUARE_WIDTH;
    box.max.y = box.max.y / GRID_SQUARE_HEIGHT;
    return box;
}

#if 0

size_t multihash_get_span(struct multihash *h, struct aabb span, uint32_t *out)
{
    size_t num = 0;
    // supports id's as high as 1024
    uint64_t set[16] = {0};
    for(int x = span.min.x; x<=span.max.x; x++)
        for(int y = span.min.y; y<=span.max.y; y++) {
            multihash_get_bitset(h, grid_location(x,y), set); 
        }

    for(int i = 0; i<16; i++) {
        uint32_t bitset = set[i];
        uint32_t mag = i<<6;
        while(bitset != 0) {
            out[num++] = mag + __builtin_ctzll(bitset); 
            bitset &= (bitset - 1);
        }
    }
    return num;
}

#endif

size_t combine_sorted(uint32_t *a, size_t alen, uint32_t *b, size_t blen, uint32_t *out)
{
    size_t ac = 0, bc = 0, len = 0; 
    while(ac<alen && bc<blen) {
        if (a[ac] < b[bc])
            out[len++] = a[ac++];
        else
        if (a[ac] > b[bc])
            out[len++] = b[bc++];
        else {
            out[len++] = a[ac++];
            bc++;
        }
    }
    while(ac<alen)
        out[len++] = a[ac++];

    while(bc<blen)
        out[len++] = b[bc++];
    return len;
}

size_t merge_into(uint32_t *in, size_t inlen, uint32_t *out, size_t outlen)
{
    int end = inlen + outlen;
    int inc = inlen - 1;
    int outc = outlen - 1;

    while(inc >= 0 && outc >= 0) {
        if (in[inc] > out[outc]) 
            out[--end] = in[inc--];    
        else 
            out[--end] = out[outc--];
    }
    while(inc>=0)
        out[--end] = in[inc--];
    while(outc>=0)
        out[--end] = out[outc--];
    return inlen+outlen;
}

size_t multihash_get_span(struct multihash *h, struct aabb span, uint32_t *out)
{
    size_t num = 0;
    uint32_t temp[MAX_AABB];

    for(int x = span.min.x; x<=span.max.x; x++)
        for(int y = span.min.y; y<=span.max.y; y++) {
            size_t len = multihash_get(h, grid_location(x,y), temp);
            num = merge_into(temp, len, out, num);
        }

    return num;
}

int main()
{
    uint32_t     keys[HASH_SIZE] = {0};
    uint32_t     vals[HASH_SIZE] = {0};
    struct aabb dudes[MAX_AABB]  = {0};

    uint32_t bucket[HASH_SIZE] = {0};
    struct multihash map = multihash_init(keys, vals, HASH_SIZE, (uint32_t)-1);

    int i;
    for(i = 0; i<MAX_AABB; i++) {
        struct aabb dude = {0};          
        dude.min.x = rand() & (WORLD_WIDTH  - 1);
        dude.min.y = rand() & (WORLD_HEIGHT - 1);
        dude.max.x = dude.min.x + 4;
        dude.max.y = dude.min.y + 4;
        dudes[i] = dude;
        struct aabb span = grid_span(dude);
        for(int x = span.min.x; x<=span.max.x; x++)
            for(int y = span.min.y; y<=span.max.y; y++)
                multihash_set(&map, grid_location(x,y), i);
    }

    int y, x;
    for(y = 0; y<WORLD_HEIGHT; y++) {
        for(x = 0; x<WORLD_WIDTH; x++){
            int found_dude = 0;
            size_t num_aabbs = multihash_get(&map, grid_location(x/GRID_SQUARE_WIDTH,y/GRID_SQUARE_HEIGHT), bucket);
            for(i=0; i<num_aabbs; i++) {
                struct aabb dude = dudes[bucket[i]]; 
                if (aabb_contains(dude, (v2) {x,y})) {
                    found_dude=1;
                    break;
                }
            }
            if (found_dude)
                printf("O");
            else
                printf(" ");

        }
        printf("\n");
    }
//#if 0
    for(i = 0; i<HASH_SIZE; i++) {
        //printf("slot %8d, key: %8d, val: %8d\n", i, map.key[i], map.val[i]);
//        printf("%8x %8d\n", map.key[i], map.val[i]);
    }
//#endif
printf("%d\n", sizeof(keys) + sizeof(vals));
    return 0;
}
