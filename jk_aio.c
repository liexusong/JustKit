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
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "jk_thread_pool.h"
#include "jk_aio.h"

static void jk_aio_execute(void *arg);
static void jk_aio_fininsh(void *arg);
static jk_aio_request_t *jk_aio_request(int type, jk_aio_finish_fn *finish);
static int jk_aio_submit(jk_aio_request_t *req);
static void jk_aio_destory(jk_aio_request_t *req);


static jk_aio_t aio;
static jk_thread_pool_t *worker_pool;


int jk_aio_init()
{
    int flags;

    if (pthread_mutex_init(&aio.lock, NULL) == -1) {
        return -1;
    }

    aio.rhead = NULL;
    aio.rtail = NULL;
    aio.nreqs = 0;
    aio.ncoms = 0;

    if (pipe(aio.pipe) == -1) {
        pthread_mutex_destroy(&aio.lock);
        return -1;
    }

    if ((flags = fcntl(aio.pipe[0], F_GETFL, 0)) < 0 ||
         fcntl(aio.pipe[0], F_SETFL, flags | O_NONBLOCK) < 0)
    {
        pthread_mutex_destroy(&aio.lock);
        close(aio.pipe[0]);
        close(aio.pipe[1]);
        return -1;
    }

    if ((flags = fcntl(aio.pipe[1], F_GETFL, 0)) < 0 ||
         fcntl(aio.pipe[1], F_SETFL, flags | O_NONBLOCK) < 0)
    {
        pthread_mutex_destroy(&aio.lock);
        close(aio.pipe[0]);
        close(aio.pipe[1]);
        return -1;
    }

    worker_pool = jk_thread_pool_new(JK_AIO_WORKER_THREADS);
    if (!worker_pool) {
        pthread_mutex_destroy(&aio.lock);
        close(aio.pipe[0]);
        close(aio.pipe[1]);
        return -1;
    }

    return 0;
}


int jk_aio_nreqs()
{
    int waits;

    pthread_mutex_lock(&aio.lock);

    waits = aio.nreqs - aio.ncoms;

    pthread_mutex_unlock(&aio.lock);

    return waits;
}


int jk_aio_wait(struct timeval *tv)
{
    fd_set sets;
    int ret, count = 0;
    char c;

    FD_ZERO(&sets);
    FD_SET(aio.pipe[0], &sets);

    ret = select(aio.pipe[0] + 1, &sets, NULL, NULL, tv);
    if (ret < 0) {
        return -1;
    } else if (ret == 0) {
        return 0;
    }

    while (1) {
        ret = read(aio.pipe[0], &c, 1);
        if (ret > 0) {
            count++;
        } else {
            break;
        }
    }

    return count;
}


int jk_aio_poll()
{
    jk_aio_request_t *req;
    int processed = 0;

    for (;;) {

        pthread_mutex_lock(&aio.lock);

        req = aio.rhead;
        if (!req) {
            pthread_mutex_unlock(&aio.lock);
            return processed;
        }

        aio.rhead = req->next;
        if (!aio.rhead) {
            aio.rtail = NULL;
        }

        aio.ncoms++;

        pthread_mutex_unlock(&aio.lock);

        if (req->finish) {
            req->finish(req);
        }

        jk_aio_destory(req);

        processed++;
    }
}


static jk_aio_request_t *jk_aio_request(int type, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = malloc(sizeof(*req));
    if (NULL == req) {
        return NULL;
    }

    memset(req, 0, sizeof(*req));

    req->type = type;
    req->finish = finish;
    req->next = NULL;

    return req;
}


static int jk_aio_submit(jk_aio_request_t *req)
{
    pthread_mutex_lock(&aio.lock);
    aio.nreqs++;
    pthread_mutex_unlock(&aio.lock);

    if (jk_thread_pool_push(worker_pool, jk_aio_execute, (void *)req,
        jk_aio_fininsh))
    {
        pthread_mutex_lock(&aio.lock);
        aio.nreqs--;
        pthread_mutex_unlock(&aio.lock);
        return -1;
    }
    return 0;
}


static void jk_aio_destory(jk_aio_request_t *req)
{
    if (req->path) {
        free(req->path);
    }
    free(req);
}


static void jk_aio_execute(void *arg)
{
    jk_aio_request_t *req = arg;

    switch (req->type) {
    case jk_aio_operate_read:
        req->result = read(req->fd, req->buf, req->size);
        break;

    case jk_aio_operate_write:
        req->result = write(req->fd, req->buf, req->size);
        break;

    case jk_aio_operate_close:
        req->result = close(req->fd);
        break;

    case jk_aio_operate_open:
        req->result = open(req->path, req->flags, req->mode);
        break;

    case jk_aio_operate_mkdir:
        req->result = mkdir(req->path, req->mode);
        break;

    case jk_aio_operate_rmdir:
        req->result = rmdir(req->path);
        break;

    case jk_aio_operate_unlink:
        req->result = unlink(req->path);
        break;
    }

    req->errno = errno;
}


static void jk_aio_fininsh(void *arg)
{
    jk_aio_request_t *req = arg;

    pthread_mutex_lock(&aio.lock);

    req->next = NULL;

    if (!aio.rhead) {
        aio.rhead = req;
    }

    if (aio.rtail) {
        aio.rtail->next = req;
    }

    aio.rtail = req;

    write(aio.pipe[1], "\0", 1);

    pthread_mutex_unlock(&aio.lock);
}


int jk_aio_read(int fd, char *buf, int size, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_read, finish);
    if (!req) {
        return -1;
    }

    req->fd = fd;
    req->buf = buf;
    req->size = size;

    return jk_aio_submit(req);
}


int jk_aio_write(int fd, char *buf, int size, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_write, finish);
    if (!req) {
        return -1;
    }

    req->fd = fd;
    req->buf = buf;
    req->size = size;

    return jk_aio_submit(req);
}


int jk_aio_open(char *path, int flags, mode_t mode, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_open, finish);
    if (!req) {
        return -1;
    }

    req->path = strdup(path);
    if (!req->path) {
        jk_aio_destory(req);
        return -1;
    }

    req->flags = flags;
    req->mode = mode;

    return jk_aio_submit(req);
}


int jk_aio_close(int fd, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_close, finish);
    if (!req) {
        return -1;
    }

    req->fd = fd;

    return jk_aio_submit(req);
}


int jk_aio_mkdir(char *path, mode_t mode, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_mkdir, finish);
    if (!req) {
        return -1;
    }

    req->path = strdup(path);
    if (!req->path) {
        jk_aio_destory(req);
        return -1;
    }

    req->mode = mode;

    return jk_aio_submit(req);
}


int jk_aio_rmdir(char *path, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_rmdir, finish);
    if (!req) {
        return -1;
    }

    req->path = strdup(path);
    if (!req->path) {
        jk_aio_destory(req);
        return -1;
    }

    return jk_aio_submit(req);
}


int jk_aio_unlink(char *path, jk_aio_finish_fn *finish)
{
    jk_aio_request_t *req = jk_aio_request(jk_aio_operate_unlink, finish);
    if (!req) {
        return -1;
    }

    req->path = strdup(path);
    if (!req->path) {
        jk_aio_destory(req);
        return -1;
    }

    return jk_aio_submit(req);
}

