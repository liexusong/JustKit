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

#ifndef __JK_HEAP_H
#define __JK_HEAP_H

#define  JK_HEAP_INIT_SIZE  10
#define  JK_HEAP_INCR_SIZE  10

typedef int jk_heap_comp_fn(void *data1, void *data2);
typedef void jk_heap_destroy_fn(void *data);

typedef struct jk_heap_s {
    void           **tree;
    int              size;
    int              used;
    jk_heap_comp_fn *comp;
    jk_heap_destroy_fn *destroy;
} jk_heap_t;

#define  jk_heap_length(heap)  ((heap)->used)
#define  jk_heap_thetop(heap)  ((heap)->tree[0])

int jk_heap_init(jk_heap_t *heap, jk_heap_comp_fn *comp,
    jk_heap_destroy_fn *destroy);
jk_heap_t *jk_heap_create(jk_heap_comp_fn *comp,
    jk_heap_destroy_fn *destroy);
void jk_heap_destroy(jk_heap_t *heap);
void jk_heap_free(jk_heap_t *heap);
int jk_heap_insert(jk_heap_t *heap, void *data);
int jk_heap_extract(jk_heap_t *heap, void **data);

#endif

