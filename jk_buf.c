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
#include "jk_buf.h"


jk_buf_t *jk_buf_new(int min, int max, jk_buf_callback_fn *cb, void *ctx)
{
    jk_buf_t *buf;
    char *tmp;

    buf = malloc(sizeof(*buf));
    if (NULL == buf) {
        return NULL;
    }

    tmp = malloc(min);
    if (NULL == tmp) {
        free(buf);
        return NULL;
    }

    buf->start = tmp;
    buf->pos = tmp;
    buf->last = tmp;
    buf->end = tmp + min;
    buf->min = min;
    buf->max = max;
    buf->size = min;
    buf->cb = cb;
    buf->ctx = ctx;

    return buf;
}


int jk_buf_read(jk_buf_t *buf, int fd, int nread)
{
    int bytes, size;

    size = buf->end - buf->last >= nread ? nread : buf->end - buf->last;

    if (size <= 0) { /* not enough memory */

        if (buf->pos > buf->start) { /* move pos to start position */

            int nmove = buf->last - buf->pos;

            memmove(buf->start, buf->pos, nmove);

            buf->pos = buf->start;
            buf->last = buf->start + nmove;

            size = buf->end - buf->last >= nread ? nread :
                                                   buf->end - buf->last;

        } else { /* do resize */

            if (buf->size < buf->max) {

                int pos = buf->pos - buf->start;
                int last = buf->last - buf->start;
                int nsize = buf->size << 1;
                char *tmp;

                if (nsize > buf->max) {
                    nsize = buf->max;
                }

                tmp = realloc(buf->start, nsize);
                if (NULL == tmp) {
                    return -1;
                }

                buf->start = tmp;
                buf->pos = tmp + pos;
                buf->last = tmp + last;
                buf->end = tmp + nsize;
                buf->size = nsize;

                size = buf->end - buf->last >= nread ? nread :
                                                       buf->end - buf->last;

            } else { /* can not resize */
                return -1;
            }
        }
    }

    bytes = read(fd, buf->last, size);

    if (bytes > 0) {

        buf->last += bytes;

        if (buf->cb) {
            buf->pos = buf->cb(buf->pos, buf->last - buf->pos, buf->ctx);
        }
    }

    return bytes;
}


void jk_buf_clean(jk_buf_t *buf)
{
    if (buf->size > buf->min) { /* resize buffer */

        char *tmp = realloc(buf->start, buf->min);

        if (NULL != tmp) {
            buf->start = tmp;
            buf->end = tmp + buf->min;
            buf->size = buf->min;
        }
    }

    /* reset pos and last pointer */
    buf->pos = buf->start;
    buf->last = buf->start;
}


void jk_buf_free(jk_buf_t *buf)
{
    free(buf->start);
    free(buf);
}

