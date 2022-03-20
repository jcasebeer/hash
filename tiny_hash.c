#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

struct hash {
    uint32_t *key;
    uint32_t *val;
    uint32_t zero_val;
    size_t cap;
};

struct str_hash {
    const char **key;
    const char **val;
    size_t cap;
};

uint32_t hash32(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t hash_string(const char *s)
{
    uint32_t hash = 0x811c9dc5;
    while(*s) {
        hash = hash ^ (uint32_t)(*s);
        hash = hash * 0x01000193;
        s++;
    }
    return hash;
}

struct str_hash str_hash_init(const char **keys, const char **vals, size_t size)
{
    struct str_hash hash = {keys, vals, size}; 
    return hash;
}

void str_hash_set(struct str_hash *h, const char *key, const char *val)
{
    size_t i = hash_string(key) & (h->cap - 1);
    while(h->key[i] != NULL && strcmp(h->key[i], key) != 0)
        i = (i+1) & (h->cap - 1);
    h->key[i] = key;
    h->val[i] = val;
}

const char *str_hash_get(struct str_hash *h, const char *key)
{
    size_t i = hash_string(key) & (h->cap - 1); 
    while(h->key[i] != NULL && strcmp(h->key[i], key) != 0)
        i = (i+1) & (h->cap - 1);
    return h->val[i]; 
}

struct hash hash_init(uint32_t *keys, uint32_t *vals, size_t size)
{
    struct hash hash = {keys, vals, 0, size};
    return hash;
}

void hash_set(struct hash *h, uint32_t key, uint32_t val)
{
    if (key == 0) {
        h->zero_val = val;
        return;
    }
    size_t i = hash32(key) & (h->cap - 1);
    while(h->key[i] != 0 && h->key[i] != key)
        i = (i+1) & (h->cap - 1);
    h->key[i] = key; 
    h->val[i] = val; 
}

uint32_t hash_get(struct hash *h, uint32_t key)
{
    if (key==0)
        return h->zero_val;
    size_t i = hash32(key) & (h->cap - 1);
    while(h->key[i] != 0 && h->key[i] != key)
        i = (i+1) & (h->cap - 1);
    return h->key[i] == key ? h->val[i] : 0;
}

void print_spatial_hash(struct hash *bhash, int width, int height)
{
    for(int row = 0; row<height; row++) {
        for(int col = 0; col<width; col++) {
            uint32_t coord = (row<<16) | col;
            if (hash_get(bhash, coord))
                printf("O");
            else
                printf(" ");
        }
        printf("\n");
    }
}

void clear_screen()
{ 
    printf("\e[1;1H\e[2J");
}

void reset_cursor()
{
    printf("\e[1;1H");
}


int main()
{
    srand(time(NULL));
    
    uint32_t keys[32] = {0};
    uint32_t vals[32] = {0};
    struct hash hash = hash_init(keys, vals, 32);

    for(uint32_t i = 0; i<16; i++) {
        uint32_t val = i*i;
        hash_set(&hash, i, val);
        assert(hash_get(&hash, i) == val);
    }

    const char *str_keys[32] = {0};
    const char *str_vals[32] = {0};
    struct str_hash str_hash = str_hash_init(str_keys, str_vals, 32);

    str_hash_set(&str_hash, "hello", "world");
    str_hash_set(&str_hash, "hello darkness", "my old friend");
    str_hash_set(&str_hash, "peanut butter", "jelly");
    str_hash_set(&str_hash, "gold", "silver");
    char hello_key[] = {'h','e','l','l','o', '\0'};
    assert(strcmp(str_hash_get(&str_hash, hello_key), "world") == 0);
    assert(strcmp(str_hash_get(&str_hash, "hello darkness"), "my old friend") == 0);
    assert(strcmp(str_hash_get(&str_hash, "peanut butter"), "jelly") == 0);
    assert(strcmp(str_hash_get(&str_hash, "gold"), "silver") == 0);
    

    uint32_t block_keys[1024] = {0};
    uint32_t block_vals[1024] = {0};
    struct hash bhash = hash_init(block_keys, block_vals, 1024);

    const int blocks = 512;
    const int width = 60;
    const int height = 40;
    int x = width/2;
    int y = height/2;

    struct coord {
        int x,y;
    };
    
    struct coord cardinals[4] = {
        { 0, 1},
        { 0,-1},
        { 1, 0},
        {-1, 0},
    };
    
    clear_screen();
    for(int i = 0; i<blocks; i++) {
        uint32_t coord = (y<<16) | x;
        if (hash_get(&bhash, coord))
            i--;
        hash_set(&bhash, coord, 1);
        struct coord diff = cardinals[rand() % 4];
        x += diff.x;
        y += diff.y;
        while (x<0 || y<0 || x>=width || y>=height) {
           x = rand() % width; 
           y = rand() % height; 
        }
        reset_cursor();
        print_spatial_hash(&bhash, width, height);
        usleep(1000);
    }

    return 0;
}
