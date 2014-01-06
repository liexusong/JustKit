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
#include <sys/time.h>
#include "jk_avl.h"
#include "jk_types.h"
#include "jk_timer.h"


static jk_uint64_t jk_current_time()
{
    struct timeval tv;
    jk_uint64_t ret;

    if (gettimeofday(&tv, NULL) == -1) {
        return 0;
    }
    
    ret = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return ret;
}


void jk_timer_init(jk_timer_t *timer)
{
    timer->root = jk_avl_empty_node;
}


jk_timer_t *jk_timer_new()
{
    jk_timer_t *timer;

    timer = malloc(sizeof(*timer));
    if (!timer) {
        return NULL;
    }

    jk_timer_init(timer);

    return timer;
}


int jk_timer_push(jk_timer_t *timer, jk_uint64_t timeout,
    jk_timer_callback_fn *cb, jk_timer_free_fn *free, void *data)
{
    jk_timer_node_t *node;

    node = malloc(sizeof(*node));
    if (!node) {
        return -1;
    }

    node->cb = cb;
    node->free = free;
    node->data = data;
    node->timer_node.key = timeout + jk_current_time();
    node->timer_node.data = node;

    jk_avl_insert(&node->timer_node, &timer->root);

    return 0;
}


jk_timer_node_t *jk_timer_min_node(jk_timer_t *timer)
{
    jk_avl_node_t *node = jk_avl_find_min(timer->root);

    if (node) {
        return node->data;
    }

    return NULL;
}


int jk_timer_wait(jk_timer_t *timer)
{
    jk_timer_node_t *node;
    jk_uint64_t current, retval, nevents = 0;

    for (;;) {

        node = jk_timer_min_node(timer);
        if (!node) {
            break;
        }

        current = jk_current_time();

        if (node->timer_node.key <= current) {

            jk_avl_remove(&node->timer_node, &timer->root);

            retval = node->cb(node->data);
            if (retval > 0) {
                node->timer_node.key = retval + jk_current_time(); /* new key */
                jk_avl_insert(&node->timer_node, &timer->root);

            } else {
                if (node->free) {
                    node->free(node->data);
                }
                free(node);
            }

            nevents++;

        } else {
            jk_uint64_t remain = node->timer_node.key - current;
            struct timeval tv;

            tv.tv_sec = remain / 1000;
            tv.tv_usec = (remain % 1000) * 1000;

            select(0, NULL, NULL, NULL, &tv);
        }
    }

    return nevents;
}


int jk_timer_process_timeout(jk_timer_t *timer)
{
    jk_timer_node_t *node;
    jk_uint64_t current;
    int nevents = 0, retval;

    for (;;) {

        current = jk_current_time();

        node = jk_timer_min_node(timer);
        if (!node || node->timer_node.key > current) { /* no elements timeout */
            break;
        }

        jk_avl_remove(&node->timer_node, &timer->root);

        retval = node->cb(node->data);
        if (retval > 0) {
            node->timer_node.key = retval + jk_current_time(); /* new key */
            jk_avl_insert(&node->timer_node, &timer->root);

        } else {
            if (node->free) {
                node->free(node->data);
            }
            free(node);
        }

        nevents++;
    }

    return nevents;
}


static void jk_timer_tree_node_destroy(jk_avl_node_t *tree)
{
    jk_timer_node_t *node;

    if (!tree) {
        return;
    }

    jk_timer_tree_node_destroy(tree->left);
    jk_timer_tree_node_destroy(tree->right);

    node = tree->data;
    if (node->free) {
        node->free(node->data);
    }
    free(node);
}


void jk_timer_destroy(jk_timer_t *timer)
{
    jk_timer_tree_node_destroy(timer->root);
}


void jk_timer_free(jk_timer_t *timer)
{
    jk_timer_destroy(timer);
    free(timer);
}

