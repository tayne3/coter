/**
 * @file heap.h
 * @brief Min-Heap (Pairing Heap)
 */
#ifndef COTER_HEAP_H
#define COTER_HEAP_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Min-Heap Node
typedef struct ct_heap_node {
	struct ct_heap_node* child;  // Pointer to the first child node
	struct ct_heap_node* left;   // Pointer to the left sibling, or the parent if it's the first child
	struct ct_heap_node* right;  // Pointer to the right sibling
} ct_heap_node_t;

// Min-Heap Compare Callback (<0 indicates that a has a higher priority than b)
typedef int (*ct_heap_compare_cb)(const ct_heap_node_t* a, const ct_heap_node_t* b);

// Min-Heap Structure
typedef struct ct_heap {
	ct_heap_node_t*    _root;
	uint32_t           _size;
	ct_heap_compare_cb _cmp;
} ct_heap_t;

#define ct_heap_size(h)     ((h)->_size)
#define ct_heap_is_empty(h) ((h)->_size == 0)

// Initialize the heap
COTER_API void ct_heap_init(ct_heap_t* heap, ct_heap_compare_cb cmp);

// Insert a node
COTER_API void ct_heap_insert(ct_heap_t* heap, ct_heap_node_t* node);

// Get and remove the top node
COTER_API ct_heap_node_t* ct_heap_pop(ct_heap_t* heap);

// Get the top node (without removing)
COTER_API ct_heap_node_t* ct_heap_top(const ct_heap_t* heap);

// Remove an arbitrary node (for priority updates)
COTER_API void ct_heap_remove(ct_heap_t* heap, ct_heap_node_t* node);

// Update a node's priority (if modified)
COTER_API void ct_heap_update(ct_heap_t* heap, ct_heap_node_t* node);

#ifdef __cplusplus
}
#endif
#endif  // COTER_HEAP_H
