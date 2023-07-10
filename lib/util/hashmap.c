/* hashmap.c - Hashmap.
 * Copyright (C) 2023 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "util/hashmap.h"
#include "util/alloc.h"
#include <stdbool.h>
#include <string.h>

#define LOAD_FACTOR 0.75

static inline uint64_t hash(uint64_t key, uint64_t capacity)
{
    return key % capacity;
}

void hashmap_init(hashmap_t *map, uint64_t size)
{
    map->buckets = (hashmap_bucket_t *) weak_calloc(sizeof (hashmap_bucket_t), size);
    map->capacity = size;
    map->size = 0;

    for (uint64_t i = 0; i < map->capacity; i++) {
        map->buckets[i].key = 0;
        map->buckets[i].is_occupied = 0;
        map->buckets[i].is_deleted  = 0;
    }
}

void hashmap_destroy(hashmap_t *map)
{
    weak_free(map->buckets);
    map->size = 0;
    map->capacity = 0;
    memset(map, 0, sizeof (*map));
}

static void hashmap_resize(hashmap_t *map)
{
    uint64_t old_capacity = map->capacity;
    hashmap_bucket_t *old_buckets = map->buckets;

    map->capacity *= 2;
    map->buckets = (hashmap_bucket_t *) weak_calloc(sizeof (hashmap_bucket_t), map->capacity);

    for (uint64_t i = 0; i < map->capacity; i++) {
        map->buckets[i].key = 0;
        map->buckets[i].is_occupied = false;
        map->buckets[i].is_deleted = false;
    }

    map->size = 0;

    for (uint64_t i = 0; i < old_capacity; i++) {
        if (old_buckets[i].is_occupied && !old_buckets[i].is_deleted) {
            hashmap_bucket_t *entry = &old_buckets[i];
            hashmap_put(map, entry->key, entry->val);
        }
    }

    free(old_buckets);
}

void hashmap_put(hashmap_t *map, uint64_t key, uint64_t val)
{
    if (map->size >= map->capacity * LOAD_FACTOR) {
        hashmap_resize(map);
    }

    unsigned long index = hash(key, map->capacity);

    while (map->buckets[index].is_occupied) {
        if (!map->buckets[index].is_deleted && map->buckets[index].key == key) {
            map->buckets[index].val = val;
            map->size++;
            return;
        }

        index = (index + 1) % map->capacity;
    }

    map->buckets[index].key = key;
    map->buckets[index].val = val;
    map->buckets[index].is_occupied = true;
    map->buckets[index].is_deleted = false;
    map->size++;
}

int64_t hashmap_get(hashmap_t *map, uint64_t key)
{
    uint64_t index = hash(key, map->capacity);

    while (map->buckets[index].is_occupied) {
        if (!map->buckets[index].is_deleted && map->buckets[index].key == key) {
            return map->buckets[index].val;
        }

        index = (index + 1) % map->capacity;
    }

    return -1;  // Key not found
}

bool hashmap_remove(hashmap_t *map, uint64_t key)
{
    uint64_t index = hash(key, map->capacity);

    while (map->buckets[index].is_occupied) {
        if (!map->buckets[index].is_deleted && map->buckets[index].key == key) {
            map->buckets[index].is_deleted = true;
            map->size--;
            return 1;
        }

        index = (index + 1) % map->capacity;
    }

    return 0;  // Key not found
}
