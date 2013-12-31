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

#ifndef __JK_AIO_H
#define __JK_AIO_H

#include "jk_types.h"

#define  JK_AIO_WORKER_THREADS  15

typedef struct jk_aio_request_s jk_aio_request_t;
typedef void jk_aio_finish_fn(jk_aio_request_t *);

typedef enum {
    jk_aio_operate_read,
    jk_aio_operate_write,
    jk_aio_operate_open,
    jk_aio_operate_close,
    jk_aio_operate_mkdir,
    jk_aio_operate_rmdir
} jk_aio_operate_t;


typedef struct jk_aio_s {
    pthread_mutex_t lock;
    int pipe[2];
    jk_aio_request_t *rhead, *rtail; /* finish requests list */
    jk_uint64_t nreqs;
    jk_uint64_t ncoms;
} jk_aio_t;


struct jk_aio_request_s {
    jk_aio_operate_t type;   /* operate */
    int fd;                  /* read/write/close */
    int size;                /* read/write */
    char *buf;               /* read/write */
    char *path;              /* open/mkdir/rmdir */
    int flags;               /* open */
    mode_t mode;             /* open/mkdir */
    int result;              /* result */
    jk_aio_finish_fn *finish;
    jk_aio_request_t *next;
};


int jk_aio_init();
int jk_aio_poll();
int jk_aio_wait(struct timeval *tv);
int jk_aio_nreqs();
int jk_aio_read(int fd, char *buf, int size, jk_aio_finish_fn *finish);
int jk_aio_write(int fd, char *buf, int size, jk_aio_finish_fn *finish);
int jk_aio_open(char *path, int flags, mode_t mode, jk_aio_finish_fn *finish);
int jk_aio_close(int fd, jk_aio_finish_fn *finish);
int jk_aio_mkdir(char *path, mode_t mode, jk_aio_finish_fn *finish);
int jk_aio_rmdir(char *path, jk_aio_finish_fn *finish);


#define JK_AIO_RESULT(req)  (req)->result
#define JK_AIO_BUFFER(req)  (req)->buf


#endif
