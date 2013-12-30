#ifndef __JK_TIMER_H
#define __JK_TIMER_H

#include "jk_avl.h"
#include "jk_types.h"


typedef jk_uint64_t jk_timer_callback_fn(void *);
typedef void jk_timer_free_fn(void *);


typedef struct jk_timer_node_s {
    jk_timer_callback_fn *cb;
    jk_timer_free_fn *free;
    void *data;
    jk_avl_node_t timer_node;
} jk_timer_node_t;


typedef struct jk_timer_s {
    jk_avl_node_t *root;
} jk_timer_t;


void jk_timer_init(jk_timer_t *timer);
jk_timer_t *jk_timer_new();
int jk_timer_push(jk_timer_t *timer, jk_uint64_t timeout,
    jk_timer_callback_fn *cb, jk_timer_free_fn *free, void *data);
jk_timer_node_t *jk_timer_min_node(jk_timer_t *timer);
void jk_timer_wait(jk_timer_t *timer);
void jk_timer_destroy(jk_timer_t *timer);
void jk_timer_free(jk_timer_t *timer);

#endif
