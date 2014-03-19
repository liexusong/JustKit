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

#ifndef __JK_SKIP_H
#define __JK_SKIP_H

#include "jk_types.h"

typedef struct jk_skip_node_s jk_skip_node_t;

struct jk_skip_node_s {
    jk_uint64_t key;
    void *value;
    jk_skip_node_t *forward[1];
};

typedef struct jk_skip_s {
    jk_skip_node_t *header;
    jk_uint32_t length;
    jk_uint32_t level;
} jk_skip_t;


#define  jk_skip_length(list)  (list)->length

jk_skip_t *jk_skip_new();
void jk_skip_free(jk_skip_t *list);
int jk_skip_insert(jk_skip_t *list, jk_uint64_t key, void *value);
int jk_skip_delete(jk_skip_t *list, jk_uint64_t key);
jk_skip_node_t *jk_skip_find_node(jk_skip_t *list, jk_int64_t key);
int jk_skip_find(jk_skip_t *list, jk_int64_t key, void **ret);

#endif
