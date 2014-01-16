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

#ifndef __JK_AVL_H
#define __JK_AVL_H

#include "jk_types.h"

#define jk_avl_empty_node    ((jk_avl_node_t *)NULL)

typedef struct jk_avl_node_s jk_avl_node_t;

struct jk_avl_node_s {
    jk_uint64_t key;
    void *data;
    short height;
    jk_avl_node_t *left;
    jk_avl_node_t *right;
};


jk_avl_node_t *jk_avl_find_min(jk_avl_node_t *root);
jk_avl_node_t *jk_avl_find_max(jk_avl_node_t *root);
jk_avl_node_t *jk_avl_find_node(jk_avl_node_t *root, jk_uint64_t key);
void *jk_avl_find(jk_avl_node_t *root, jk_uint64_t key);
void jk_avl_insert(jk_avl_node_t *new_node, jk_avl_node_t **ptree);
void jk_avl_remove(jk_avl_node_t *node_to_delete, jk_avl_node_t **ptree);


#endif
