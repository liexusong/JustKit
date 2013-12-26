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

#include <pthread.h>
#include "jk_thread_pool.h"
#include "jk_aio.h"

static void jk_aio_execute(void *arg);
static void jk_aio_fininsh(void *arg);


static jk_aio_t aio;
static jk_thread_pool_t *worker_pool;


int jk_aio_init()
{
    worker_pool = jk_thread_pool_new(JK_AIO_WORKER_THREADS);
    if (!worker_pool) {
        return -1;
    }

    if (pthread_mutex_init(&aio.lock, NULL) == -1) {
        return -1;
    }

    aio.finish = NULL;
    aio.count = 0;

    return 0;
}


int jk_aio_poll()
{
    
}


int jk_aio_submit(jk_aio_request_t *req)
{
    if (jk_thread_pool_push(worker_pool, jk_aio_execute, (void *)req,
        jk_aio_fininsh)) {
        return -1;
    }
    return 0;
}


static void jk_aio_execute(void *arg)
{
    jk_aio_request_t *req = arg;

    switch (req->type) {
    case jk_aio_operate_read:
    case jk_aio_operate_write:
    case jk_aio_operate_flush:
    case jk_aio_operate_close:
    case jk_aio_operate_open:
    case jk_aio_operate_mkdir:
    }
}


static void jk_aio_fininsh(void *arg)
{
    jk_aio_request_t *req = arg;

    pthread_mutex_lock(&aio.lock);

    req->next = aio.finish;
    aio.finish = req;
    aio.count++;

    pthread_mutex_unlock(&aio.lock);
}
