/**
 * @file heap.c
 * @brief Pairing Heap
 */
#include "coter/container/heap.h"

/* 合并两个堆 */
static ct_heap_node_t* heap_meld(ct_heap_node_t* a, ct_heap_node_t* b, ct_heap_compare_cb cmp) {
    if (!a) { return b; }
    if (!b) { return a; }

    if (cmp(a, b) < 0) { /* a 优先级更高，b 成为 a 的第一个子节点 */
        b->right_ = a->child_;
        if (a->child_) { a->child_->left_ = b; }
        a->child_ = b;
        b->left_  = a;
        return a;
    } else { /* b 优先级更高，a 成为 b 的第一个子节点 */
        a->right_ = b->child_;
        if (b->child_) { b->child_->left_ = a; }
        b->child_ = a;
        a->left_  = b;
        return b;
    }
}

/* 两两配对合并所有子节点 (防止深度过大导致栈溢出) */
static ct_heap_node_t* heap_merge_pairs_iter(ct_heap_node_t* first, ct_heap_compare_cb cmp) {
    if (!first) { return NULL; }

    /* 第一阶段：从左往右，两两配对合并 */
    ct_heap_node_t* curr      = first;
    ct_heap_node_t* tree_list = NULL;
    while (curr) {
        ct_heap_node_t* a = curr;
        ct_heap_node_t* b = curr->right_;
        if (b) {
            ct_heap_node_t* next = b->right_;
            a->left_ = a->right_ = b->left_ = b->right_ = NULL;
            ct_heap_node_t* paired                  = heap_meld(a, b, cmp);
            /* 暂时用 right 指针把配对后的树连成一个单向链表 */
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

    /* 第二阶段：从右往左 (由于上面是头插，现在 tree_list 就是反序的)，依次合并 */
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

    /* 解除子节点的父指针/兄弟指针 */
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

    /* 从兄弟/父子关系中剥离 */
    if (node->right_) { node->right_->left_ = node->left_; }
    if (node->left_->child_ == node) {
        node->left_->child_ = node->right_; /* node 是其父节点的第一个孩子 */
    } else {
        node->left_->right_ = node->right_; /* node 有左兄弟 */
    }

    ct_heap_node_t* children = node->child_;
    if (children) { children->left_ = NULL; }

    node->child_ = node->left_ = node->right_ = NULL;
    --heap->size_;

    /* 合并剥离出来的子树到根节点 */
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
