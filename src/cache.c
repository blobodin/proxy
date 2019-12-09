#include "cache.h"
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/* Hashes a web link key into a chaining hash table index. */
size_t hash(char *key){
    size_t sum = 0;
    for (ssize_t i = strlen(key) - 1; i >= 0; i--) {
        sum += key[i];
        sum *= 31;
    }
    return sum % NUM_BUCKETS;
}

/* Initializes a cache via malloc. Sets size to 0, initializes the read-write
 * lock, and sets all head nodes in the chaining hash dictionary to null. */
cache_t *cache_create() {
    cache_t *cache = malloc(sizeof(size_t) + sizeof(pthread_rwlock_t) +
                            ((sizeof(node_t *) * NUM_BUCKETS)));
    assert(cache != NULL);

    cache->size = 0;
    pthread_rwlock_init(&(cache->lock), NULL);
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        cache->buckets[i] = NULL;
    }
    return cache;
}

/* Frees all of the linked lists of a chaining hash dictionary and frees the
 * cache struct. */
void cache_free(cache_t *cache) {
    if (cache == NULL) {
        return;
    }

    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        if (cache->buckets[i] != NULL) {
            list_free(cache->buckets[i]);
        }
    }
    free(cache);
}

/* Returns the number of bytes of web objects stored in the cache. */
size_t cache_size(cache_t *cache) {
    return cache->size;
}

/* Takes a cache and finds the node with the lowest timestamp.*/
node_t *get_lru(cache_t *cache) {
    clock_t curr_time = 0;
    node_t *lru_node;
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        if (cache->buckets[i] != NULL) {
            node_t *curr_node = cache->buckets[i];
            while (curr_node != NULL) {
                if (curr_time == 0 || curr_node->timestamp < curr_time) {
                    curr_time = curr_node->timestamp;
                    lru_node = curr_node;
                }
                curr_node = curr_node->next;
            }
        }
    }
    return lru_node;
}

/* Returns the web object stored in the node denoted by key. The node's
 * timestamp is updated to the provided one since it was just used.*/
buffer_t *cache_get(cache_t *cache, char *key, clock_t timestamp) {
    /* A read lock is used so many threads can access values in the cache
     * at the same time. */
    pthread_rwlock_rdlock(&(cache->lock));
    node_t *bucket = cache->buckets[hash(key)];
    if (bucket == NULL) {
        pthread_rwlock_unlock(&(cache->lock));
        return NULL;
    }
    node_t *node = node_get(bucket, key, timestamp);
    if (node == NULL) {
        pthread_rwlock_unlock(&(cache->lock));
        return NULL;
    }
    /* A copy of the buffer found is returned in order to account for the
     * race condition where something removes/frees the web object after
     * another thread obtains it. */
    buffer_t *buf_cpy = buffer_copy(node->data);
    pthread_rwlock_unlock(&(cache->lock));
    return buf_cpy;
}

/* Add a node to the cache with the given, key, web object data, and
 * timestamp. */
void cache_add(cache_t *cache, char *key, buffer_t *data, clock_t timestamp) {
    /*A write lock is used so that only one thread can alter the cache at a
    * time. */
    pthread_rwlock_wrlock(&(cache->lock));
    /* Removes nodes from the cache if the maximum size would be reached by
    *  the addition of this new we object. */
    while (cache_size(cache) + buffer_length(data) > MAX_CACHE_SIZE) {
        cache_remove(cache, get_lru(cache));
    }
    cache->size += buffer_length(data);
    node_t *bucket = cache->buckets[hash(key)];
    node_t *new_bucket = node_add(bucket, key, data, timestamp);
    cache->buckets[hash(key)] = new_bucket;
    pthread_rwlock_unlock(&(cache->lock));
}

/* Removes the specified node from the cache. Because this function is only
 * called from within the cache_add function, it does not need its own lock.
 * the add function is already locked. */
void cache_remove(cache_t *cache, node_t *node) {
    if (node == NULL) {
        return;
    }
    cache->size -= buffer_length(node->data);
    ssize_t index = hash(node->key);
    node_t *bucket = cache->buckets[index];
    node_t *new_bucket = node_remove(bucket, node);
    cache->buckets[index] = new_bucket;
}
