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
#include "jk_hash.h"


static unsigned int jk_hash_buckets_size[] = {
    7,          13,         31,         61,         127,        251,
    509,        1021,       2039,       4093,       8191,       16381,
    32749,      65521,      131071,     262143,     524287,     1048575,
    2097151,    4194303,    8388607,    16777211,   33554431,   67108863,
    134217727,  268435455,  536870911,  1073741823, 2147483647, 0
};


static long jk_hash_default_hash(char *key, uint klen)
{
    long h = 0, g;
    char *kend = key + klen;

    while (key < kend) {
        h = (h << 4) + *key++;
        if ((g = (h & 0xF0000000))) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h;
}


jk_hash_t *jk_hash_new(unsigned int init_buckets, jk_hash_fn *hash,
    jk_hash_free *free)
{
    jk_hash_t *o;

    o = malloc(sizeof(*o));
    if (NULL == o) {
        return NULL;
    }

    if (init_buckets < JK_HASH_BUCKETS_MIN_SIZE) {
        init_buckets = JK_HASH_BUCKETS_INIT_SIZE;

    } else if (init_buckets > JK_HASH_BUCKETS_MAX_SIZE) {
        init_buckets = JK_HASH_BUCKETS_MAX_SIZE;
    }

    o->buckets = malloc(sizeof(void *) * init_buckets);
    if (NULL == o->buckets) {
        free(o);
        return NULL;
    }

    memset(o->buckets, 0, sizeof(void *) * init_buckets);

    if (!hash) {
        hash = &jk_hash_default_hash;
    }

    o->hash = hash;
    o->free = free;
    o->buckets_size = init_buckets;
    o->elm_nums = 0;

    return o;
}


int jk_hash_find(jk_hash_t *o, char *key, int klen, void **ret)
{
    jk_hash_entry_t *e;
    long hashval = o->hash(key, klen);
    int index = hashval % o->buckets_size;

    e = o->buckets[index];
    while (e) {
        if (e->hashval == hashval && e->klen == klen &&
            !strncmp(e->key, key, klen))
        {
            *ret = e->data;
            return JK_HASH_OK;
        }
        e = e->next;
    }
    return JK_HASH_ERR;
}


int jk_hash_insert(jk_hash_t *o, char *key, int klen, void *data, int replace)
{
    jk_hash_entry_t *en, **ei;
    long hashval = o->hash(key, klen);
    int index = hashval % o->buckets_size;

    ei = &o->buckets[index];
    while (*ei) {
        if (*ei->hashval == hashval && *ei->klen == klen &&
            !strncmp(*ei->key, key, klen)) /* found the key */
        {
            if (replace) {
                if (o->free) {
                    o->free(*ei->data);
                }
                *ei->data = data;
                return JK_HASH_OK;
            }
            return JK_HASH_DUPLICATE_KEY;
        }
        ei = &(*ei->next);
    }

    en = malloc(sizeof(*en) + klen);
    if (NULL == e) {
        return JK_HASH_ERR;
    }

    en->hashval = hashval;
    en->klen = klen;
    en->data = data;
    en->next = NULL;

    memcpy(en->key, key, klen);

    *ei = en; /* append to the last of hash list */

    o->elm_nums++;

    if (o->elm_nums * 1.5 > o->buckets_size) {
        jk_hash_rehash(o);
    }

    return JK_HASH_OK;
}


int jk_hash_remove(jk_hash_t *o, char *key, int klen)
{
    jk_hash_entry_t *e, *p;
    long hashval = o->hash(key, klen);
    int index = hashval % o->buckets_size;

    p = NULL;
    e = o->buckets[index];
    while (e) {
        if (e->hashval == hashval && e->klen == klen &&
            !strncmp(e->key, key, klen)) {
            break;
        }
        p = e;
        e = e->next;
    }

    if (!e) { /* not found */
        return JK_HASH_ERR;
    }

    if (!p) {
        o->buckets[index] = e->next;
    } else {
        p->next = e->next;
    }

    if (o->free) {
        o->free(e->data);
    }
    free(e);

    o->elm_nums--;

    return JK_HASH_OK;
}


static void jk_hash_rehash(jk_hash_t *o)
{
    unsigned int new_bucket_size;
    void **new_buckets;
    jk_hash_entry_t *e, *next;
    int i, index;

    for (i = 0; jk_hash_buckets_size[i] != 0; i++) {
        if (jk_hash_buckets_size[i] > o->buckets_size) {
            break;
        }
    }

    if (jk_hash_buckets_size[i] > 0) {
        new_bucket_size = jk_hash_buckets_size[i];
    } else {
        new_bucket_size = jk_hash_buckets_size[i - 1];
    }

    if (new_bucket_size == o->buckets_size) { /* already the max buckets size */
        return;
    }

    new_buckets = malloc(sizeof(void *) * new_bucket_size);
    if (NULL == new_buckets) { /* not enough memory */
        return;
    }

    memset(new_buckets, 0, sizeof(void *) * new_bucket_size);

    for (i = 0; i < o->buckets_size; i++) {

        e = o->buckets[i];
        while (e) {
            next = e->next; /* next process entry */

            index = e->hashval % new_bucket_size;
            e->next = new_buckets[index];
            new_buckets[index] = e;

            e = next;
        }
    }

    free(o->buckets); /* free old buckets */

    o->buckets = new_buckets;
    o->buckets_size = new_bucket_size;

    return;
}


void jk_hash_destroy(jk_hash_t *o)
{
    jk_hash_entry_t *e, *next;
    int i;

    for (i = 0; i < o->buckets_size; i++) {

        e = o->buckets[i];

        while (e) {
            next = e->next;
            if (o->free) {
                o->free(e->data);
            }
            free(e);
            e = next;
        }
    }

    free(o->buckets);
    free(o);

    return;
}
