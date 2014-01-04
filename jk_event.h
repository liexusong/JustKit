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

#ifndef __JK_EVENT_H
#define __JK_EVENT_H

#include <sys/time.h>

#define JK_EVENT_NEVENT  0
#define JK_EVENT_REVENT  1
#define JK_EVENT_WEVENT  2


typedef struct jk_event_s jk_event_t;
typedef void jk_event_callback_fn(jk_event_t *ev, int fd, void *data);


typedef struct jk_event_node_s {
    int event;
    jk_event_callback_fn *rev_handler;
    jk_event_callback_fn *wev_handler;
    void *data;
} jk_event_node_t;


struct jk_event_s {
    int max_events;
    jk_event_node_t *events;
    void *ctx;
};


jk_event_t *jk_event_new(int max_events);
void jk_event_free(jk_event_t *ev);
int jk_event_add(jk_event_t *ev, int fd, int event,
    jk_event_callback_fn *handler, void *data);
int jk_event_del(jk_event_t *ev, int fd, int event);
int jk_event_process(jk_event_t *ev, struct timeval *tv);


#endif
