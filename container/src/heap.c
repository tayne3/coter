/**
 * @file heap.c
 * @brief Pairing Heap
 */
#include "coter/container/heap.h"

/* 合并两个堆 */
static ct_heap_node_t* heap__meld(ct_heap_node_t* a, ct_heap_node_t* b, ct_heap_compare_cb cmp) {
    if (!a) { return b; }
    if (!b) { return a; }

    if (cmp(a, b) < 0) { /* a 优先级更高，b 成为 a 的第一个子节点 */
        b->right = a->child;
        if (a->child) { a->child->left = b; }
        a->child = b;
        b->left  = a;
        return a;
    } else { /* b 优先级更高，a 成为 b 的第一个子节点 */
        a->right = b->child;
        if (b->child) { b->child->left = a; }
        b->child = a;
        a->left  = b;
        return b;
    }
}

/* 两两配对合并所有子节点 (防止深度过大导致栈溢出) */
static ct_heap_node_t* heap__merge_pairs_iter(ct_heap_node_t* first, ct_heap_compare_cb cmp) {
    if (!first) { return NULL; }

    /* 第一阶段：从左往右，两两配对合并 */
    ct_heap_node_t* curr      = first;
    ct_heap_node_t* tree_list = NULL;
    while (curr) {
        ct_heap_node_t* a = curr;
        ct_heap_node_t* b = curr->right;
        if (b) {
            ct_heap_node_t* next = b->right;
            a->left = a->right = b->left = b->right = NULL;
            ct_heap_node_t* paired                  = heap__meld(a, b, cmp);
            /* 暂时用 right 指针把配对后的树连成一个单向链表 */
            paired->right = tree_list;
            tree_list     = paired;
            curr          = next;
        } else {
            a->left = a->right = NULL;
            a->right           = tree_list;
            tree_list          = a;
            curr               = NULL;
        }
    }

    /* 第二阶段：从右往左 (由于上面是头插，现在 tree_list 就是反序的)，依次合并 */
    ct_heap_node_t* root = NULL;
    while (tree_list) {
        ct_heap_node_t* next = tree_list->right;
        tree_list->right     = NULL;
        root                 = heap__meld(root, tree_list, cmp);
        tree_list            = next;
    }

    return root;
}

void ct_heap_init(ct_heap_t* heap, ct_heap_compare_cb cmp) {
    if (!heap) { return; }
    heap->_root = NULL;
    heap->_size = 0;
    heap->_cmp  = cmp;
}

void ct_heap_clear(ct_heap_t* heap) {
    if (!heap) { return; }
    heap->_root = NULL;
    heap->_size = 0;
}

void ct_heap_insert(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node) { return; }
    node->child = node->left = node->right = NULL;
    heap->_root                            = heap__meld(heap->_root, node, heap->_cmp);
    ++heap->_size;
}

ct_heap_node_t* ct_heap_pop(ct_heap_t* heap) {
    if (!heap || !heap->_root) { return NULL; }

    ct_heap_node_t* old_root = heap->_root;
    ct_heap_node_t* children = old_root->child;

    /* 解除子节点的父指针/兄弟指针 */
    if (children) { children->left = NULL; }

    heap->_root = heap__merge_pairs_iter(children, heap->_cmp);
    --heap->_size;

    old_root->child = old_root->left = old_root->right = NULL;
    return old_root;
}

ct_heap_node_t* ct_heap_top(const ct_heap_t* heap) {
    return heap ? heap->_root : NULL;
}

void ct_heap_remove(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node || heap->_size == 0) { return; }

    if (node == heap->_root) {
        ct_heap_pop(heap);
        return;
    }

    /* 从兄弟/父子关系中剥离 */
    if (node->right) { node->right->left = node->left; }
    if (node->left->child == node) {
        node->left->child = node->right; /* node 是其父节点的第一个孩子 */
    } else {
        node->left->right = node->right; /* node 有左兄弟 */
    }

    ct_heap_node_t* children = node->child;
    if (children) { children->left = NULL; }

    node->child = node->left = node->right = NULL;
    --heap->_size;

    /* 合并剥离出来的子树到根节点 */
    ct_heap_node_t* remeld = heap__merge_pairs_iter(children, heap->_cmp);
    heap->_root            = heap__meld(heap->_root, remeld, heap->_cmp);
}

void ct_heap_update(ct_heap_t* heap, ct_heap_node_t* node) {
    if (!heap || !node) { return; }
    ct_heap_remove(heap, node);
    ct_heap_insert(heap, node);
}

void ct_heap_move(ct_heap_t* dst, ct_heap_t* src) {
    if (!dst || !src || src->_size == 0) { return; }

    if (dst->_size == 0) {
        dst->_root = src->_root;
        dst->_size = src->_size;
    } else {
        dst->_root = heap__meld(dst->_root, src->_root, dst->_cmp);
        dst->_size += src->_size;
    }

    src->_root = NULL;
    src->_size = 0;
}
