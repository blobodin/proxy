#include "linked_list.h"
#include "buffer.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/* Initalizes node with given web key, web object, and timestamp. Returns it. */
node_t *node_create(char *key, buffer_t *data, clock_t timestamp) {
    node_t *new_node = (node_t *) malloc(sizeof(node_t));

    new_node->prev = NULL;
    new_node->next = NULL;
    new_node->key = key;
    new_node->data = data;
    new_node->timestamp = timestamp;

    return new_node;
}

/* Frees list by recursively iterating through all connected nodes. Ignores
 * call if head_node argument is null. */
void list_free(node_t *head_node) {
    if (head_node == NULL) {
        return;
    }

    list_free(head_node->prev);
    list_free(head_node->next);
    node_free(head_node);
}

/* Frees the key, web object data, and node struct. */
void node_free(node_t *node) {
    if (node == NULL) {
        return;
    }
    buffer_free(node->data);
    free(node->key);
    free(node);
}

/* Adds a new node to the front of the linked list.
 * Returns the new head_node of this linked list. */
node_t *node_add(node_t *head_node, char *key, buffer_t *data,
                 clock_t timestamp) {
    node_t *node = node_create(key, data, timestamp);
    node->next = head_node;
    if (head_node != NULL) {
        head_node->prev = node;
    }
    return node;
}

/* Retrieves the node specified by a key in the linked list. Updates
 * the timestamp. Returns null if node not found. */
node_t *node_get(node_t *head_node, char *key, clock_t timestamp) {
    node_t *curr_node = head_node;
    while (curr_node != NULL) {
        if (strcmp(curr_node->key, key) == 0) {
            curr_node->timestamp = timestamp;
            return curr_node;
        }
        curr_node = curr_node->next;
    }
    return NULL;
}

/* Removes a node from a linked list and returns the head node of the linked
 * list. Frees the node as well using the free node function. */
node_t *node_remove(node_t *head_node, node_t *node) {
    if (node == head_node) {
        node_free(node);
        return NULL;
    } else if (node->prev != NULL && node->next == NULL) {
        node->prev->next = NULL;
        node_free(node);
        return head_node;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node_free(node);
        return head_node;
    }
}
