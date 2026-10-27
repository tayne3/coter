/**
 * @file heap.c
 * @brief Pairing Heap
 */
#include "coter/container/heap.h"

/* Merge two heaps. */
static ct_heap_node_t* heap_meld(ct_heap_node_t* a, ct_heap_node_t* b, ct_heap_compare_cb cmp) {
    if (!a) { return b; }
    if (!b) { return a; }

    if (cmp(a, b) < 0) { /* a has higher priority; make b its first child. */
        b->right_ = a->child_;
        if (a->child_) { a->child_->left_ = b; }
        a->child_ = b;
        b->left_  = a;
        return a;
    } else { /* b has higher priority; make a its first child. */
        a->right_ = b->child_;
        if (b->child_) { b->child_->left_ = a; }
        b->child_ = a;
        a->left_  = b;
        return b;
    }
}

/* Merge child trees pairwise to avoid deep recursion. */
static ct_heap_node_t* heap_merge_pairs_iter(ct_heap_node_t* first, ct_heap_compare_cb cmp) {
    if (!first) { return NULL; }

    /* First pass: pair adjacent trees from left to right. */
    ct_heap_node_t* curr      = first;
    ct_heap_node_t* tree_list = NULL;
    while (curr) {
        ct_heap_node_t* a = curr;
        ct_heap_node_t* b = curr->right_;
        if (b) {
            ct_heap_node_t* next = b->right_;
            a->left_ = a->right_ = b->left_ = b->right_ = NULL;
            ct_heap_node_t* paired                  = heap_meld(a, b, cmp);
            /* Use right_ links to chain the paired trees temporarily. */
            paired->right_ = tree_list;
            tree_list     = paired;
            curr          = next;
        } else {
            a->left_ = a->right_ = NULL;
            a->right_           = tree_list;
            tree_list          = a;
            curr               = NULL;
        }
    }

    /* Second pass: merge the reversed chain from right to left. */
    ct_heap_node_t* root = NULL;
    while (tree_list) {
        ct_heap_node_t* next = tree_list->right_;
        tree_list->right_     = NULL;
        root                 = heap_meld(root, tree_list, cmp);
        tree_list            = next;
    }

    return root;
}

void ct_heap_init(ct_heap_t* heap, ct_heap_compare_cb cmp) {
    if (!heap) { return; }
    heap->root_ = NULL;
    heap->size_ = 0;
    heap->cmp_  = cmp;
}

void ct_heap_clear(ct_heap_t* heap) {
    if (!heap) { return; }
    heap->root_ = NULL;
    heap->size_ = 0;
}

void ct_heap_insert(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node) { return; }
    node->child_ = node->left_ = node->right_ = NULL;
    heap->root_                            = heap_meld(heap->root_, node, heap->cmp_);
    ++heap->size_;
}

ct_heap_node_t* ct_heap_pop(ct_heap_t* heap) {
    if (!heap || !heap->root_) { return NULL; }

    ct_heap_node_t* old_root = heap->root_;
    ct_heap_node_t* children = old_root->child_;

    /* Detach the child list from the removed root. */
    if (children) { children->left_ = NULL; }

    heap->root_ = heap_merge_pairs_iter(children, heap->cmp_);
    --heap->size_;

    old_root->child_ = old_root->left_ = old_root->right_ = NULL;
    return old_root;
}

ct_heap_node_t* ct_heap_top(const ct_heap_t* heap) {
    return heap ? heap->root_ : NULL;
}

void ct_heap_remove(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node || heap->size_ == 0) { return; }

    if (node == heap->root_) {
        ct_heap_pop(heap);
        return;
    }

    /* Detach the node from its sibling and parent links. */
    if (node->right_) { node->right_->left_ = node->left_; }
    if (node->left_->child_ == node) {
        node->left_->child_ = node->right_; /* node is the first child of its parent. */
    } else {
        node->left_->right_ = node->right_; /* node has a left sibling. */
    }

    ct_heap_node_t* children = node->child_;
    if (children) { children->left_ = NULL; }

    node->child_ = node->left_ = node->right_ = NULL;
    --heap->size_;

    /* Merge the detached children back into the root. */
    ct_heap_node_t* remeld = heap_merge_pairs_iter(children, heap->cmp_);
    heap->root_            = heap_meld(heap->root_, remeld, heap->cmp_);
}

void ct_heap_update(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node) { return; }
    ct_heap_remove(heap, node);
    ct_heap_insert(heap, node);
}

void ct_heap_move(ct_heap_t* dst, ct_heap_t* src) {
    if (!dst || !src || src->size_ == 0) { return; }

    if (dst->size_ == 0) {
        dst->root_ = src->root_;
        dst->size_ = src->size_;
    } else {
        dst->root_ = heap_meld(dst->root_, src->root_, dst->cmp_);
        dst->size_ += src->size_;
    }

    src->root_ = NULL;
    src->size_ = 0;
}
