/* 
 * File:   list.h
 * Author: mone
 *
 * Created on 24 febbraio 2012, 12.22
 */

/* subset of the Linux Kernel source file: "include/linux/list.h"
CPLv2 */
#ifndef _LISTX_H
#define _LISTX_H

#include <stdlib.h>

typedef unsigned int size_t;

/**#define container_of(ptr, type, member) ({			\
                const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
                (type *)( (char *)__mptr - offsetof(type,member) );})*/

#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static void __list_add(struct list_head * n, struct list_head *prev, struct list_head *next) {
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;

}

static void list_add(struct list_head * n, struct list_head *head) {
    __list_add(n, head, head->next);
}

static void list_add_tail(struct list_head * n, struct list_head *head) {
    __list_add(n, head->prev, head);
}

static void __list_del(struct list_head * prev, struct list_head * next) {
    next->prev = prev;
    prev->next = next;
}

static void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}

static int list_empty(const struct list_head *head) {
    return head->next == head;
}

static struct list_head *list_next(const struct list_head *current) {
    if (list_empty(current))
        return NULL;
    else
        return current->next;
}

static struct list_head *list_prev(const struct list_head *current) {
    if (list_empty(current))
        return NULL;
    else
        return current->prev;
}

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_entry(pos, head, member, type)                          \
	for (pos = container_of((head)->next, type, member);      \
	&pos->member != (head);        \
	pos = container_of(pos->member.next, type, member))

#define list_for_each_entry_reverse(pos, head, member)                  \
	for (pos = container_of((head)->prev, typeof(*pos), member);      \
	&pos->member != (head);        \
	pos = container_of(pos->member.prev, typeof(*pos), member))

#endif


