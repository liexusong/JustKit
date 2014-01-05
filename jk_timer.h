/*
 * Copyright (c) 2012 - 2013, Liexusong <280259971@qq.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
