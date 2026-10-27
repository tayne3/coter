/**
 * @file list.c
 * @brief Intrusive doubly linked list implementation.
 */
#include "coter/container/list.h"

void ct_list_init(ct_list_t* self) {
    if (!self) { return; }
    self->prev_ = self->next_ = self;
}

size_t ct_list_size(const ct_list_t* self) {
    if (!self) { return 0; }
    size_t size = 0;
    ct_list_foreach(node, self) {
        ++size;
    }
    return size;
}

void ct_list_append(ct_list_t* self, ct_list_t* node) {
    if (!self || !node) { return; }
    node->prev_        = self->prev_;
    node->next_        = self;
    self->prev_->next_ = node;
    self->prev_        = node;
}

void ct_list_prepend(ct_list_t* self, ct_list_t* node) {
    if (!self || !node) { return; }
    node->prev_        = self;
    node->next_        = self->next_;
    self->next_->prev_ = node;
    self->next_        = node;
}

void ct_list_before(ct_list_t* target, ct_list_t* node) {
    if (!target || !node) { return; }
    node->prev_          = target->prev_;
    node->next_          = target;
    target->prev_->next_ = node;
    target->prev_        = node;
}

void ct_list_after(ct_list_t* target, ct_list_t* node) {
    if (!target || !node) { return; }
    node->prev_          = target;
    node->next_          = target->next_;
    target->next_->prev_ = node;
    target->next_        = node;
}

void ct_list_remove(ct_list_t* node) {
    if (!node) { return; }
    node->prev_->next_ = node->next_;
    node->next_->prev_ = node->prev_;
}

void ct_list_splice_prev(ct_list_t* self, ct_list_t* list) {
    if (!self || !list) { return; }
    if (ct_list_isempty(list)) { return; }
    ct_list_last(list)->next_  = ct_list_first(self);
    ct_list_first(self)->prev_ = ct_list_last(list);
    ct_list_first(list)->prev_ = self;
    ct_list_first(self)        = ct_list_first(list);
    ct_list_init(list);
}

void ct_list_splice_next(ct_list_t* self, ct_list_t* list) {
    if (!self || !list) { return; }
    if (ct_list_isempty(list)) { return; }
    ct_list_first(list)->prev_ = ct_list_last(self);
    ct_list_last(self)->next_  = ct_list_first(list);
    ct_list_last(list)->next_  = self;
    ct_list_last(self)         = ct_list_last(list);
    ct_list_init(list);
}
