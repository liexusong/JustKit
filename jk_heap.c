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

#include "jk_heap.h"

#define jk_heap_parent(pos)  ((int)(((pos) - 1) / 2))
#define jk_heap_left(pos)    ((pos) * 2 + 1)
#define jk_heap_right(pos)   ((pos) * 2 + 2)


int jk_heap_init(jk_heap_t *heap, jk_heap_comp_fn *comp,
    jk_heap_destroy_fn *destroy)
{
    heap->tree = malloc(sizeof(void *) * JK_HEAP_INIT_SIZE);
    if (heap->tree == NULL) {
        return -1;
    }

    heap->size = JK_HEAP_INIT_SIZE;
    heap->used = 0;
    heap->comp = comp;
    heap->destroy = destroy;

    return 0;
}


jk_heap_t *jk_heap_create(jk_heap_comp_fn *comp,
    jk_heap_destroy_fn *destroy)
{
    jk_heap_t *heap;

    heap = malloc(sizeof(*heap));
    if (heap != NULL) {
        if (jk_heap_init(heap, comp, destroy) == -1) {
            free(heap);
            return NULL;
        }
    }
    return heap;
}


void jk_heap_destroy(jk_heap_t *heap)
{
    int i;

    if (heap->destroy != NULL) {
        for (i = 0; i < heap->used; i++) {
            heap->destroy(heap->tree[i]);
    	}
    }
    free(heap->tree);
}


void jk_heap_free(jk_heap_t *heap)
{
    jk_heap_destroy(heap);
    free(heap);
}


int jk_heap_insert(jk_heap_t *heap, void *data)
{
    int ipos, ppos;
    void *temp;

    /* extpand */
    if (heap->used >= heap->size) {
        int nsize = heap->size + JK_HEAP_INCR_SIZE;

        temp = (void **)realloc(heap->tree, sizeof(void *) * nsize);
        if (temp == NULL) {
            return -1;
        }
        heap->tree = temp;
        heap->size = nsize;
    }

    heap->tree[jk_heap_length(heap)] = data; /* insert the last */

    ipos = jk_heap_length(heap);
    ppos = jk_heap_parent(ipos);

    while (ipos > 0 && 
           heap->comp(heap->tree[ppos], heap->tree[ipos]) < 0)
    {
        temp = heap->tree[ipos];
        heap->tree[ipos] = heap->tree[ppos];
        heap->tree[ppos] = temp;

        ipos = ppos;
        ppos = jk_heap_parent(ipos);
    }

    heap->used++;

    return 0;
}


int jk_heap_extract(jk_heap_t *heap, void **data)
{
    void *save, *temp;
    int nsize;
    int ipos, lpos, rpos, mpos;

    if (jk_heap_length(heap) == 0) {
        return -1;
    }

    if (data != NULL) {
        *data = heap->tree[0];
    }

    heap->used--; /* adjust the used of the heap */

    save = heap->tree[jk_heap_length(heap)];

    if (heap->used == 0) {
        nsize = JK_HEAP_INIT_SIZE;
    } else if (heap->size > heap->used * 1.5) {
        nsize = heap->used * 1.5;
    } else {
        nsize = heap->size;
    }

    if (nsize != heap->size) {
        temp = realloc(heap->tree, sizeof(void *) * nsize);
        if (temp != NULL) {
            heap->tree = temp;
            heap->size = nsize;
        }
    }

    if (jk_heap_length(heap) == 0) {
        return 0;
    }

    heap->tree[0] = save;

    for (ipos = 0; ; ) {
        lpos = jk_heap_left(ipos);
        rpos = jk_heap_right(ipos);
        mpos = ipos;

        if (lpos < jk_heap_length(heap) &&
            heap->comp(heap->tree[lpos], heap->tree[ipos]) > 0)
        {
            mpos = lpos;
        }

        if (rpos < jk_heap_length(heap) &&
            heap->comp(heap->tree[rpos], heap->tree[ipos]) > 0)
        {
            mpos = rpos;
        }

        if (mpos == ipos) {
            break;
        } else {
            temp = heap->tree[mpos];
            heap->tree[mpos] = heap->tree[ipos];
            heap->tree[ipos] = temp;
            ipos = mpos;
        }
    }

    return 0;
}

