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

#ifndef __JK_LIST_H
#define __JK_LIST_H

typedef struct jk_list_s jk_list_t;

struct jk_list_s {
    jk_list_t *prev;
    jk_list_t *next;
};


#define jk_list_for_each(node, head)                                    \
    for (node = (head)->next; node != (head); node = node->next)

#define jk_list_for_each_safe(node, n, head)                            \
    for (node = (head)->next, n = node->next; node != (head);           \
         node = n, n = node->next)

#define jk_list_for_each2(node, head)                                   \
    for (node = (head)->prev; node != (head); node = node->prev)

#define jk_list_for_each2_safe(node, n, head)                           \
    for (node = (head)->prev, n = node->prev; node != (head);           \
         node = n, n = node->prev)


void inline jk_list_init(jk_list_t *head)
{
    head->prev = head->next = head;
}


void inline jk_list_add(jk_list_t *head, jk_list_t *node)
{
    node->prev = head->prev; /* point to the last node */
    node->next = head;       /* point to the head node */
    head->prev->next = node;
    head->prev = node;
}


void inline jk_list_del(jk_list_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

#endif
