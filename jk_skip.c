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
#include <math.h>
#include "jk_skip.h"


#define  JK_SKIP_MAX_LEVEL        32
#define  jk_skip_node_free(node)  free(node)


static int jk_skip_random_level(void)
{
    int level = 1;

    while ((random() & 0xFFFF) < (0.25 * 0xFFFF))
        level += 1;
    return level;
}


static jk_skip_node_t *jk_skip_node_new(jk_uint32_t level, jk_uint64_t key,
    void *value)
{
    jk_skip_node_t *node;
    int size = sizeof(*node) + (sizeof(jk_skip_node_t *) * (level-1));

    node = malloc(size);
    if (!node) {
        return NULL;
    }

    node->key = key;
    node->value = value;

    return node;
}


jk_skip_t *jk_skip_new()
{
    jk_skip_t *list;
    int i;

    list = malloc(sizeof(*list));
    if (!list) {
        return NULL;
    }

    list->header = jk_skip_node_new(JK_SKIP_MAX_LEVEL, 0, NULL);
    if (!list->header) {
        free(list);
        return NULL;
    }

    for (i = 0; i < JK_SKIP_MAX_LEVEL; i++) {
        list->header->forward[i] = NULL;
    }

    list->level = 1;
    list->length = 0;

    return list;
}


void jk_skip_free(jk_skip_t *list)
{
    jk_skip_node_t *node = list->header->forward[0], *next;

    /* free the header node first */
    jk_skip_node_free(list->header);

    while (node) {
        next = node->forward[0];
        jk_skip_node_free(node);
        node = next;
    }

    free(list);
}


int jk_skip_insert(jk_skip_t *list, jk_uint64_t key, void *value)
{
    jk_skip_node_t *update[JK_SKIP_MAX_LEVEL], *n;
    int i, level;

    n = list->header;
    for (i = list->level - 1; i >= 0; i--) {
        while (n->forward[i] && (n->forward[i]->key < key))
            n = n->forward[i];
        update[i] = n;
    }

    level = jk_skip_random_level();

    if (level > list->level) {
        for (i = list->level; i <level; i++)
            update[i] = list->header;
        list->level = level;
    }

    n = jk_skip_node_new(level, key, value);
    if (!n) {
        return -1;
    }

    for (i = 0; i < level; i++) {
        n->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = n;
    }

    list->length++;

    return 0;
}


int jk_skip_delete(jk_skip_t *list, jk_uint64_t key)
{
    jk_skip_node_t *update[JK_SKIP_MAX_LEVEL], *n;
    int i, level;

    n = list->header;
    for (i = list->level - 1; i >= 0; i--) {
        while (n->forward[i] && (n->forward[i]->key < key))
            n = n->forward[i];
        update[i] = n;
    }

    n = n->forward[0];

    if (n && (n->key == key)) {
        for (i = 0; i < list->level; i++) {
            if (update[i]->forward[i] != n)
                break;
            update[i]->forward[i] = n->forward[i];
        }

        jk_skip_node_free(n);

        while (list->level > 1 && 
               list->header->forward[list->level-1] == NULL)
        {
            list->level--;
        }

        list->length--;

        return 0;
    }

    return -1;
}


jk_skip_node_t *jk_skip_find_node(jk_skip_t *list, jk_int64_t key)
{
    jk_skip_node_t *node;
    int i;

    node = list->header;
    for (i = list->level - 1; i >= 0; i--) {
        while (node->forward[i] && node->forward[i]->key < key) {
            node = node->forward[i];
        }
    }

    node = node->forward[0];
    if (node && (node->key == key)) {
        return node;
    }

    return NULL;
}


int jk_skip_find(jk_skip_t *list, jk_int64_t key, void **ret)
{
    jk_skip_node_t *node;
    int i;

    node = jk_skip_find_node(list, key);
    if (!node) {
        return -1;
    }

    if (ret) {
        *ret = node->value;
    }
    return 0;
}

