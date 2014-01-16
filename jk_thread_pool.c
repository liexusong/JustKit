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
#include "jk_thread_pool.h"


static void *jk_thread_loop(void *arg)
{
    jk_thread_pool_t *thd = arg;
    jk_thread_task_t *tsk;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    pthread_mutex_lock(&thd->wait_lock);
    thd->wait_threads--;
    pthread_cond_signal(&thd->wait_cond);
    pthread_mutex_unlock(&thd->wait_lock);

    for (;;) { /* forever loop */

        pthread_mutex_lock(&thd->lock);

        while (thd->task_nums <= 0) {
            pthread_cond_wait(&thd->cond, &thd->lock); /* here maybe destroy */
        }

        /* Get lock here */
        tsk = thd->wait_tasks;
        thd->wait_tasks = tsk->next;
        thd->task_nums--;

        pthread_mutex_unlock(&thd->lock);

        tsk->call(tsk->arg);

        if (tsk->finish) {
            tsk->finish(tsk->arg);
        }
        free(tsk);
    }

    return NULL;
}


jk_thread_pool_t *jk_thread_pool_new(int thread_nums)
{
    jk_thread_pool_t *thd;
    pthread_t tid;
    int i;

    thd = malloc(sizeof(*thd));
    if (NULL == thd) {
        return NULL;
    }

    thd->wait_tasks = NULL;
    thd->task_nums = 0;
    thd->worker_threads = thread_nums;
    thd->wait_threads = thread_nums;

    /* save all worker threads id */
    thd->tids = malloc(sizeof(pthread_t) * thread_nums);
    if (!thd->tids) {
        free(thd);
        return NULL;
    }

    pthread_mutex_init(&thd->lock, NULL);
    pthread_cond_init(&thd->cond, NULL);

    pthread_mutex_init(&thd->wait_lock, NULL);
    pthread_cond_init(&thd->wait_cond, NULL);

    for (i = 0; i < thread_nums; i++) {
        if (pthread_create(&thd->tids[i], NULL, jk_thread_loop, thd) == -1) {
            free(thd->tids);
            free(thd);
            return NULL;
        }
    }

    pthread_mutex_lock(&thd->wait_lock);
    while (thd->wait_threads > 0) {
        pthread_cond_wait(&thd->wait_cond, &thd->wait_lock);
    }
    pthread_mutex_unlock(&thd->wait_lock);

    return thd;
}


int jk_thread_pool_push(jk_thread_pool_t *thd, jk_thread_call_fn *call,
    void *arg, jk_thread_finish_fn *finish)
{
    jk_thread_task_t *tsk;

    tsk = malloc(sizeof(*tsk));
    if (NULL == tsk) {
        return -1;
    }

    tsk->call = call;
    tsk->finish = finish;
    tsk->arg = arg;

    pthread_mutex_lock(&thd->lock);    /* lock task's queue */

    tsk->next = thd->wait_tasks;
    thd->wait_tasks = tsk;
    thd->task_nums++;

    pthread_cond_signal(&thd->cond);  /* signal worker thread */
    pthread_mutex_unlock(&thd->lock); /* unlock task's queue */

    return 0;
}


void jk_thread_pool_destroy(jk_thread_pool_t *thd)
{
    jk_thread_task_t *task, *next;
    int i;

    /* free all tasks */
    pthread_mutex_lock(&thd->lock);

    task = thd->wait_tasks;
    while (task) {
        next = task->next;
        free(task);
        task = next;
        thd->task_nums--;
    }

    pthread_mutex_unlock(&thd->lock);

    for (i = 0; i < thd->worker_threads; i++) {
        pthread_cancel(thd->tids[i]); /* cancel all worker threads */
    }

    /* destroy lock and free memory */
    pthread_mutex_destroy(&thd->lock);
    pthread_cond_destroy(&thd->cond);
    pthread_mutex_destroy(&thd->wait_lock);
    pthread_cond_destroy(&thd->wait_cond);

    free(thd->tids);
    free(thd);

    return;
}

