/**
 * @file slist.h
 * @author krad2
 * @date June 28 2020
 * @brief An intrusive singly linked list implementation in C.
 * @see https://en.wikipedia.org/wiki/Linked_list
 */

#ifndef DS_SLIST_H_
#define DS_SLIST_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct slist_node {
	struct slist_node *next;
} slist_node_t;

struct slist {
    slist_node_t *head;
};

void slist_init(struct slist *list);

bool slist_is_empty(struct slist *list);


void slist_prepend(struct slist *list, struct slist_element *element);


void slist_element_insert_at(struct slist_element *after, struct slist_element *element);


void slist_element_remove_at(struct slist_element *after);


void slist_append(struct slist *list, struct slist_element *element);

int slist_remove(struct slist *list, struct slist_element *element);

int slist_remove_head(struct slist *list, struct slist_element **element);

struct slist_iterator {
    /**\brief pointer to current element*/
    struct slist_element *current;
};


int slist_iterator_init(struct slist_iterator *it, struct slist *list);


int slist_iterator_next(struct slist_iterator *it, struct slist_element **element);

static inline void list_add(list_node_t *node, list_node_t *new_node) {
	new_node->next = node->next;
	node->next = new_node;
}

static inline list_node_t *list_remove_head(list_node_t *list) {
	list_node_t *head = list->next;

	if (head) {
		list->next = head->next;
	}
	return head;
}

static inline list_node_t *list_remove(list_node_t *list, list_node_t *node) {
	while (list->next) {
		if (list->next == node) {
			list->next = node->next;
			return node;
		}
		list = list->next;
	}
	return list->next;
}

#ifdef __cplusplus
}
#endif

#endif /* DS_SLIST_H_ */
