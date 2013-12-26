#ifndef __JK_AIO_H
#define __JK_AIO_H


#define  JK_AIO_WORKER_THREADS  15

typedef struct jk_aio_request_s jk_aio_request_t;

typedef enum {
    jk_aio_operate_read,
    jk_aio_operate_write,
    jk_aio_operate_flush,
    jk_aio_operate_open,
    jk_aio_operate_close,
    jk_aio_operate_mkdir
} jk_aio_operate_t;


typedef struct jk_aio_s {
    pthread_mutex_t lock;
    jk_aio_request_t *finish;
    int count;
} jk_aio_t;


struct jk_aio_request_s {
    jk_aio_operate_t type;   /* operate */
    int fd;     /* read/write/flush/close */
    int size;   /* read/write */
    char *buf;  /* read/write */
    char *path; /* open/mkdir */
    int ret;    /* return */
    union {
        int fd;     /* open */
        int bytes;  /* read/write */
    } ctx;
    jk_aio_request_t *next;
};


#endif
