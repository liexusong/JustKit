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

#include <stdlib.h>
#include "jk_event.h"


static int jk_event_context_init(jk_event_t *ev);
static void jk_event_context_free(jk_event_t *ev);
static int jk_event_context_add(jk_event_t *ev, int fd, int event);
static int jk_event_context_del(jk_event_t *ev, int fd, int event);
static int jk_event_context_process(jk_event_t *ev, struct timeval *tv);


jk_event_t *jk_event_new(int max_events)
{
    jk_event_t *ev;

    ev = malloc(sizeof(*ev));
    if (NULL == ev) {
        return NULL;
    }

    ev->events = calloc(max_events, sizeof(jk_event_node_t));
    if (NULL == ev->events) {
        free(ev);
        return NULL;
    }

    ev->max_events = max_events;

    if (jk_event_context_init(ev) == -1) {
        free(ev->events);
        free(ev);
        return NULL;
    }

    return ev;
}


void jk_event_free(jk_event_t *ev)
{
    jk_event_context_free(ev);
    free(ev->events);
    free(ev);
}


int jk_event_add(jk_event_t *ev, int fd, int event,
    jk_event_callback_fn *handler, void *data)
{
    jk_event_node_t *node;

    if (fd >= ev->max_events) {
        return -1;
    }

    if (jk_event_context_add(ev, fd, event) == -1) {
        return -1;
    }

    node = &ev->events[fd];
    node->event |= event;
    node->data = data;

    if (event & JK_EVENT_REVENT) {
        node->rev_handler = handler;
    }

    if (event & JK_EVENT_WEVENT) {
        node->wev_handler = handler;
    }

    return 0;
}


int jk_event_del(jk_event_t *ev, int fd, int event)
{
    jk_event_node_t *node;

    if (fd >= ev->max_events) {
        return -1;
    }

    if (jk_event_context_del(ev, fd, event)) {
        return -1;
    }

    node = &ev->events[fd];
    node->event = node->event & (~event);

    return 0;
}


int jk_event_process(jk_event_t *ev, struct timeval *tv)
{
    return jk_event_context_process(ev, tv);
}



/* I/O Event APIs */

#if defined(linux)

#include <sys/epoll.h>

struct jk_event_context {
    int epfd;
    struct epoll_event *events;
};

static int jk_event_context_init(jk_event_t *ev)
{
    struct jk_event_context *ctx = malloc(sizeof(*ctx));

    if (!ctx) return -1;

    ctx->events = malloc(sizeof(struct epoll_event) * ev->max_events);
    if (!ctx->events) {
        free(ctx);
        return -1;
    }

    ctx->epfd = epoll_create(1024); /* 1024 is just an hint for the kernel */
    if (ctx->epfd == -1) {
        free(ctx->events);
        free(ctx);
        return -1;
    }

    ev->ctx = ctx;

    return 0;
}


static void jk_event_context_free(jk_event_t *ev)
{
    struct jk_event_context *ctx = ev->ctx;

    close(ctx->epfd);
    free(ctx->events);
    free(ctx);
}


static int jk_event_context_add(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;
    struct epoll_event ee;
    int op = (ev->events[fd].event == JK_EVENT_NEVENT ?
                             EPOLL_CTL_ADD : EPOLL_CTL_MOD);

    ee.events = 0;

    event |= ev->events[fd].event;

    if (event & JK_EVENT_REVENT) ee.events |= EPOLLIN;
    if (event & JK_EVENT_WEVENT) ee.events |= EPOLLOUT;

    ee.data.u64 = 0;  /* avoid valgrind warning */
    ee.data.fd = fd;

    if (epoll_ctl(ctx->epfd, op, fd, &ee) == -1)
        return -1;
    return 0;
}


static int jk_event_context_del(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;
    struct epoll_event ee;

    event = ev->events[fd].event & (~event);
    ee.events = 0;

    if (event & JK_EVENT_REVENT) ee.events |= EPOLLIN;
    if (event & JK_EVENT_WEVENT) ee.events |= EPOLLOUT;

    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;

    if (event != JK_EVENT_NEVENT) {
        return epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, fd, &ee);
    } else {
        return epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, fd, &ee);
    }
}


static int jk_event_context_process(jk_event_t *ev, struct timeval *tvp)
{
    struct jk_event_context *ctx = ev->ctx;
    jk_event_node_t *node;
    int nevents = 0, i;

    nevents = epoll_wait(ctx->epfd, ctx->events, ev->max_events,
                         tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);

    if (nevents > 0) {

        for (i = 0; i < nevents; i++) {

            struct epoll_event *e = ctx->events + i;
            int fd = e->data.fd;
            int rfired = 0;

            node = &ev->events[fd];

            if ((e->events & EPOLLIN) && (node->event & JK_EVENT_REVENT)) {
                node->rev_handler(ev, fd, node->data);
                rfired = 1;
            }

            if ((e->events & EPOLLOUT) && (node->event & JK_EVENT_WEVENT)) {

                if (!rfired || node->rev_handler != node->wev_handler) {
                    node->wev_handler(ev, fd, node->data);
                }
            }
        }
    }

    return nevents;
}

#elif defined(__FreeBSD__)

#include <sys/event.h>

struct jk_event_context {
    int kqfd;
    struct kevent *events;
};


static int jk_event_context_init(jk_event_t *ev)
{
    struct jk_event_context *ctx = malloc(sizeof(*ctx));

    if (!ctx) return -1;

    ctx->events = malloc(sizeof(struct kevent) * ev->max_events);
    if (!ctx->events) {
        free(ctx);
        return -1;
    }

    ctx->kqfd = kqueue();
    if (ctx->kqfd == -1) {
        free(ctx->events);
        free(ctx);
        return -1;
    }

    ev->ctx = ctx;

    return 0;    
}


static void jk_event_context_free(jk_event_t *ev)
{
    struct jk_event_context *ctx = ev->ctx;

    close(ctx->kqfd);
    free(ctx->events);
    free(ctx);
}


static int jk_event_context_add(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;
    struct kevent ke;

    if (event & JK_EVENT_REVENT) {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(ctx->kqfd, &ke, 1, NULL, 0, NULL) == -1)
            return -1;
    }

    if (event & JK_EVENT_WEVENT) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(ctx->kqfd, &ke, 1, NULL, 0, NULL) == -1)
            return -1;
    }

    return 0;
}


static int jk_event_context_del(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;
    struct kevent ke;

    if (event & JK_EVENT_REVENT) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        if (kevent(ctx->kqfd, &ke, 1, NULL, 0, NULL) == -1)
            return -1;
    }

    if (event & JK_EVENT_WEVENT) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        if (kevent(ctx->kqfd, &ke, 1, NULL, 0, NULL) == -1)
            return -1;
    }

    return 0;
}


static int jk_event_context_process(jk_event_t *ev, struct timeval *tvp)
{
    struct jk_event_context *ctx = ev->ctx;
    jk_event_node_t *node;
    int retval, nevents = 0;

    if (tvp != NULL) {
        struct timespec timeout;

        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;

        retval = kevent(ctx->kqfd, NULL, 0, ctx->events, ev->max_events,
              &timeout);

    } else {
        retval = kevent(ctx->kqfd, NULL, 0, ctx->events, ev->max_events, NULL);
    }    

    if (retval > 0) {
        int i;

        nevents = retval;

        for(i = 0; i < nevents; i++) {

            struct kevent *e = ctx->events + i;
            int fd = e->ident;
            int rfired = 0;

            node = &ev->events[fd];

            if ((node->event & JK_EVENT_REVENT) && e->filter == EVFILT_READ) {
                node->rev_handler(ev, fd, node->data);
                rfired = 1;
            }

            if ((node->event & JK_EVENT_WEVENT) && e->filter == EVFILT_WRITE) {

                if (!rfired && node->rev_handler != node->wev_handler) {
                    node->wev_handler(ev, fd, node->data);
                }
            }
        }
    }

    return nevents;
}

#else

#include <string.h>
#include <sys/select.h>

struct jk_event_context {
    fd_set  rfds,  wfds;
    fd_set _rfds, _wfds;
    int max_fd;
};


static int jk_event_context_init(jk_event_t *ev)
{
    struct jk_event_context *ctx = malloc(sizeof(*ctx));

    if (!ctx) return -1;

    FD_ZERO(&ctx->rfds);
    FD_ZERO(&ctx->wfds);

    ctx->max_fd = -1;
    ev->ctx = ctx;

    return 0;
}


static void jk_event_context_free(jk_event_t *ev)
{
    free(ev->ctx);
}


static int jk_event_context_add(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;

    if (event & JK_EVENT_REVENT) FD_SET(fd, &ctx->rfds);
    if (event & JK_EVENT_WEVENT) FD_SET(fd, &ctx->wfds);

    if (fd > ctx->max_fd) ctx->max_fd = fd;

    return 0;
}


static int jk_event_context_del(jk_event_t *ev, int fd, int event)
{
    struct jk_event_context *ctx = ev->ctx;

    if (event & JK_EVENT_REVENT) FD_CLR(fd, &ctx->rfds);
    if (event & JK_EVENT_WEVENT) FD_CLR(fd, &ctx->wfds);

    /* if the fd not in read set and write set,
     * and the fd was the max fd, so update the max fd. */
    if (fd == ctx->max_fd && !FD_ISSET(fd, &ctx->rfds) && 
        !FD_ISSET(fd, &ctx->wfds))
    {
        int i;

        for (i = ev->max_events - 1; i >= 0; i--) {
            if (ev->events[i].event != JK_EVENT_NEVENT) {
                break;
            }
        }

        ctx->max_fd = i;
    }

    return 0;
}


static int jk_event_context_process(jk_event_t *ev, struct timeval *tvp)
{
    struct jk_event_context *ctx = ev->ctx;
    int retval, i, nevents = 0;

    memcpy(&ctx->_rfds, &ctx->rfds, sizeof(fd_set));
    memcpy(&ctx->_wfds, &ctx->wfds, sizeof(fd_set));

    retval = select(ctx->max_fd + 1, &ctx->_rfds, &ctx->_wfds, NULL, tvp);

    if (retval > 0) {

        for (i = 0; i <= ctx->max_fd; i++) {

            int rfired = 0;
            jk_event_node_t *node = &ev->events[i];

            if ((node->event & JK_EVENT_REVENT) && FD_ISSET(i, &ctx->_rfds)) {
                node->rev_handler(ev, i, node->data);
                rfired = 1;
            }

            if ((node->event & JK_EVENT_WEVENT) && FD_ISSET(i, &ctx->_wfds)) {

                if (!rfired || node->rev_handler != node->wev_handler) {
                    node->wev_handler(ev, i, node->data);
                }
            }

            nevents++;
        }
    }

    return nevents;
}

#endif

