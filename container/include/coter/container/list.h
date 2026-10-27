/**
 * @file list.h
 * @brief Intrusive doubly linked list.
 */
#ifndef COTER_CONTAINER_LIST_H
#define COTER_CONTAINER_LIST_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ct_list;

/**
 * @brief Intrusive doubly linked list node and sentinel.
 *
 * A list head is a sentinel node whose next_/prev_ links point to the first
 * and last elements. An empty list points back to the sentinel through both
 * links.
 */
typedef struct ct_list {
    struct ct_list* prev_;  ///< Previous node.
    struct ct_list* next_;  ///< Next node.
} ct_list_t, ct_list_buf_t[1];

/**
 * @brief Get the containing object from an embedded list node.
 * @param node_ Pointer to the list node.
 * @param type_ Containing object type.
 * @param member_ Name of the list member inside @p type_.
 */
#define ct_list_entry(node_, type_, member_) CT_CONTAINER_OF(node_, type_, member_)

#define ct_list_isempty(head_)          ((head_)->next_ == (void*)(head_))
#define ct_list_first(head_)            ((head_)->next_)
#define ct_list_last(head_)             ((head_)->prev_)
#define ct_list_is_first(head_, node_)  ((node_)->next_ == (void*)(head_))
#define ct_list_is_last(head_, node_)   ((node_)->prev_ == (void*)(head_))
#define ct_list_next(pos_)              ((pos_)->next_)
#define ct_list_prev(pos_)              ((pos_)->prev_)

#define ct_list_first_entry(head_, type_, member_) ct_list_entry((head_)->next_, type_, member_)
#define ct_list_last_entry(head_, type_, member_)  ct_list_entry((head_)->prev_, type_, member_)

#define ct_list_next_entry(pos_, type_, member_) \
    !(pos_) ? NULL : ct_list_entry(((ct_list_t*)(&(pos_)->member_))->next_, type_, member_)
#define ct_list_prev_entry(pos_, type_, member_) \
    !(pos_) ? NULL : ct_list_entry(((ct_list_t*)(&(pos_)->member_))->prev_, type_, member_)

/**
 * @brief Iterate over list nodes.
 *
 * Nodes and the head sentinel must not be modified during iteration.
 */
#define ct_list_foreach(pos_, head_) \
    for (ct_list_t* pos_ = (head_)->next_; pos_ != (void*)(head_); pos_ = pos_->next_)

/**
 * @brief Iterate over list entries.
 *
 * Nodes and the head sentinel must not be modified during iteration.
 */
#define ct_list_foreach_entry(pos_, head_, type_, member_)            \
    for (type_* pos_ = ct_list_first_entry((head_), type_, member_); \
         pos_ && (void*)(&(pos_)->member_) != (void*)(head_); pos_ = ct_list_next_entry(pos_, type_, member_))

/**
 * @brief Iterate over list entries starting at a given position.
 *
 * Nodes and the head sentinel must not be modified during iteration.
 */
#define ct_list_foreach_entry_from(pos_, head_, type_, member_) \
    for (; pos_ && (void*)(&(pos_)->member_) != (void*)(head_); \
         (pos_) = ct_list_next_entry((pos_), type_, member_))

/**
 * @brief Iterate over list entries while allowing the current node to be removed.
 */
#define ct_list_foreach_entry_safe(pos_, head_, type_, member_)                  \
    for (type_* pos_ = ct_list_first_entry(head_, type_, member_),               \
                 *next_node_ = ct_list_next_entry(pos_, type_, member_);         \
         pos_ && (void*)(&(pos_)->member_) != (void*)(head_);                     \
         pos_ = next_node_, next_node_ = ct_list_next_entry(next_node_, type_, member_))

/**
 * @brief Initialize or reset a list head.
 * @param self List head.
 */
CT_API void ct_list_init(ct_list_t* self);

/**
 * @brief Get the number of elements in the list.
 * @param self List head.
 * @return Number of elements.
 */
CT_API size_t ct_list_size(const ct_list_t* self);

/**
 * @brief Append a node at the tail.
 * @param self List head.
 * @param node Node to append.
 */
CT_API void ct_list_append(ct_list_t* self, ct_list_t* node);

/**
 * @brief Prepend a node at the head.
 * @param self List head.
 * @param node Node to prepend.
 */
CT_API void ct_list_prepend(ct_list_t* self, ct_list_t* node);

/**
 * @brief Insert a node before the target node.
 * @param target Existing list node.
 * @param node Node to insert.
 */
CT_API void ct_list_before(ct_list_t* target, ct_list_t* node);

/**
 * @brief Insert a node after the target node.
 * @param target Existing list node.
 * @param node Node to insert.
 */
CT_API void ct_list_after(ct_list_t* target, ct_list_t* node);

/**
 * @brief Remove a node from its list.
 * @param node Node to remove.
 */
CT_API void ct_list_remove(ct_list_t* node);

/**
 * @brief Splice another list before this list.
 * @param self Destination list.
 * @param list Source list; it becomes empty.
 */
CT_API void ct_list_splice_prev(ct_list_t* self, ct_list_t* list);

/**
 * @brief Splice another list after this list.
 * @param self Destination list.
 * @param list Source list; it becomes empty.
 */
CT_API void ct_list_splice_next(ct_list_t* self, ct_list_t* list);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_LIST_H
