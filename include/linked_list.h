#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "buffer.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

/* Nodes of the doubley linked list */
typedef struct node {
    struct node *prev;
    struct node *next;
    /* Key/URL of web object */
    char *key;
    /* Buffer_t that stores web object. */
    buffer_t *data;
    clock_t timestamp;
} node_t;

/* Creates new node with given parameters */
node_t *node_create(char *key, buffer_t *data, clock_t timestamp);
/* Frees entire list given a head node */
void list_free(node_t *head_node);
void node_free(node_t *node);
/* Adds new node to list with given parameters */
node_t *node_add(node_t *head_node, char *key, buffer_t *data,
                 clock_t timestamp);
/* Finds and returns node of given key */
node_t *node_get(node_t *head_node, char *key, clock_t timestamp);
/* Removes node corresponding to key */
node_t *node_remove(node_t *head_node, node_t *node);

#endif // LINKED_LIST_H
