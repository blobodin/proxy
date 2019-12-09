#ifndef CACHE_H
#define CACHE_H

#include "buffer.h"
#include "linked_list.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

/* This cache is a thread safe chaining-hash dictionary that approximately
 * employs LRU to evict nodes when it reaches capacity.
 * Race conditions resulting from get, add, and remove functions require
 * read/write locks. In this way, multiple threads can read from the cache at
 * the same time, but only one thread can add or remove at a time.
 *
 * Timestamps are employed to keep track of the least recently used node,
 * are always updated whenever the get function is called. It is only
 * approximately least recently used because multiple threads can update the
 * timestamps of nodes at the same time, since the get function only has a
 * read lock. This is okay, though, because timestamps that are overwritten
 * by concurrent threads were approximately the same timestamp value as those
 * that overwrote them.
 */

/* Max amount of web object data that can be stored in cache. */
#define MAX_CACHE_SIZE 1048576
/* Max amount of web object data that can be stored in a node. */
#define MAX_OBJECT_SIZE 100000
/* Number of buckets in chaining hash dictionary for cache. */
#define NUM_BUCKETS 53

/* Cache struct for the proxy server. Its purpose is to store
 * Recently accessed data so that it can be quickly retrieved if a client
 * request it again.
 */
typedef struct cache {
    /* Size of the cache's web objects. Exlcudes meta data. */
    size_t size;
    pthread_rwlock_t lock;
    /* Chaining hash dictionary that stores web objects. */
    node_t *buckets[];
} cache_t;

/* Hash function used to obtain an index from a string. */
size_t hash(char *key);
/* Initializes an empty cache and all its fields. */
cache_t *cache_create();
/* Frees a cache and all of its fields. */
void cache_free(cache_t *cache);
/* Retrieves the web object specified the key. Updates the node of the web
 * object with a timestamp. */
buffer_t *cache_get(cache_t *cache, char *key, time_t timestamp);
/* Adds a web object to the cache with its key and timestamp. */
void cache_add(cache_t *cache, char *key, buffer_t *data,
              time_t timestamp);
/* Removes a web object from the cache. */
void cache_remove(cache_t *cache, node_t *node);
/* Returns the amount of web data in bytes */
size_t cache_size(cache_t *cache);
/* Returns the least recently used node, i.e. the node with the lowest
 * timestamp. */
node_t *get_lru(cache_t *cache);

#endif // CACHE_H
