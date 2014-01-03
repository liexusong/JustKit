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
#include "jk_avl.h"


#define JK_AVL_MAX_HEIGHT       41
#define JK_AVL_HEIGHT_OF(n)    ((n) == jk_avl_empty_node ? 0 : (n)->height)


static void jk_avl_rebalance(jk_avl_node_t ***nodeplaces_ptr, int count)
{
    for ( ; count > 0 ; count--) {

        jk_avl_node_t ** nodeplace = *--nodeplaces_ptr;
        jk_avl_node_t * node = *nodeplace;
        jk_avl_node_t * nodeleft = node->left;
        jk_avl_node_t * noderight = node->right;

        int heightleft = JK_AVL_HEIGHT_OF(nodeleft);
        int heightright = JK_AVL_HEIGHT_OF(noderight);

        if (heightright + 1 < heightleft) {

            jk_avl_node_t * nodeleftleft = nodeleft->left;
            jk_avl_node_t * nodeleftright = nodeleft->right;
            int heightleftright = JK_AVL_HEIGHT_OF(nodeleftright);

            if (JK_AVL_HEIGHT_OF(nodeleftleft) >= heightleftright) {

                node->left = nodeleftright;
                nodeleft->right = node;
                node->height = 1 + heightleftright;
                nodeleft->height = 1 + node->height;
                *nodeplace = nodeleft;

            } else {
                nodeleft->right = nodeleftright->left;
                node->left = nodeleftright->right;
                nodeleftright->left = nodeleft;
                nodeleftright->right = node;
                nodeleft->height = node->height = heightleftright;
                nodeleftright->height = heightleft;
                *nodeplace = nodeleftright;
            }

        } else if (heightleft + 1 < heightright) {

            jk_avl_node_t * noderightright = noderight->right;
            jk_avl_node_t * noderightleft = noderight->left;
            int heightrightleft = JK_AVL_HEIGHT_OF(noderightleft);

            if (JK_AVL_HEIGHT_OF(noderightright) >= heightrightleft) {
                node->right = noderightleft; noderight->left = node;
                node->height = 1 + heightrightleft;
                noderight->height = 1 + node->height;
                *nodeplace = noderight;

            } else {
                noderight->left = noderightleft->right;
                node->right = noderightleft->left;
                noderightleft->right = noderight;
                noderightleft->left = node;
                noderight->height = node->height = heightrightleft;
                noderightleft->height = heightright;
                *nodeplace = noderightleft;
            }

        } else {
            int height = (heightleft < heightright ?
                                       heightright : heightleft) + 1;
            if (height == node->height) /* no need rebalance */
                break;
            node->height = height;
        }
    }
}


jk_avl_node_t *jk_avl_find_min(jk_avl_node_t *root)
{
    jk_avl_node_t *node = root;

    while (node) {
        if (node->left == jk_avl_empty_node) {
            break;
        }
        node = node->left;
    }

    return node;
}


jk_avl_node_t *jk_avl_find_node(jk_avl_node_t *root, jk_uint64_t key)
{
    jk_avl_node_t *node = root;

    while (node) {
        if (node->key == key) {
            return node;
        } else if (node->key > key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return NULL;
}


void *jk_avl_find(jk_avl_node_t *root, jk_uint64_t key)
{
    jk_avl_node_t *node;

    node = jk_avl_find_node(root, key);
    if (node) {
        return node->data;
    }
    return NULL;
}


void jk_avl_insert(jk_avl_node_t *new_node, jk_avl_node_t **ptree)
{
    jk_uint64_t key = new_node->key;
    jk_avl_node_t ** nodeplace = ptree;
    jk_avl_node_t ** stack[JK_AVL_MAX_HEIGHT];
    int stack_count = 0;
    jk_avl_node_t *** stack_ptr = &stack[0];

    for (;;) {
        jk_avl_node_t * node = *nodeplace;
        if (node == jk_avl_empty_node)
            break;

        *stack_ptr++ = nodeplace;
        stack_count++;

        if (key < node->key)
            nodeplace = &node->left;
        else
            nodeplace = &node->right;
    }

    new_node->left = jk_avl_empty_node;
    new_node->right = jk_avl_empty_node;
    new_node->height = 1;
    *nodeplace = new_node;

    jk_avl_rebalance(stack_ptr, stack_count);
}


void jk_avl_remove(jk_avl_node_t *node_to_delete, jk_avl_node_t **ptree)
{
    jk_uint64_t key = node_to_delete->key;
    jk_avl_node_t ** nodeplace = ptree;
    jk_avl_node_t ** stack[JK_AVL_MAX_HEIGHT];
    int stack_count = 0;
    jk_avl_node_t *** stack_ptr = &stack[0];
    jk_avl_node_t ** nodeplace_to_delete;

    for (;;) {
        jk_avl_node_t * node = *nodeplace;

        *stack_ptr++ = nodeplace;
        stack_count++;
        if (key == node->key)
            break;
        if (key < node->key)
            nodeplace = &node->left;
        else
            nodeplace = &node->right;
    }

    nodeplace_to_delete = nodeplace;

    if (node_to_delete->left == jk_avl_empty_node) {
        *nodeplace_to_delete = node_to_delete->right;
        stack_ptr--;
        stack_count--;

    } else {
        jk_avl_node_t *** stack_ptr_to_delete = stack_ptr;
        jk_avl_node_t ** nodeplace = &node_to_delete->left;
        jk_avl_node_t * node;

        for (;;) {
            node = *nodeplace;
            if (node->right == jk_avl_empty_node)
                break;
            *stack_ptr++ = nodeplace;
            stack_count++;
            nodeplace = &node->right;
        }

        *nodeplace = node->left;

        node->left = node_to_delete->left;
        node->right = node_to_delete->right;
        node->height = node_to_delete->height;
        *nodeplace_to_delete = node;
        *stack_ptr_to_delete = &node->left;
    }

    jk_avl_rebalance(stack_ptr, stack_count);
}

