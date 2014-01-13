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
#include <time.h>
#include "jk_bloom_filter.h"


static unsigned int __salts[8];


static void jk_bloom_filter_init_salts()
{
    int i = 0, j;
    unsigned int salt;

    srand((unsigned int)time(NULL));

    while (i < 8) {

        salt = ((unsigned int)rand() ^ (unsigned int)rand());

        for (j = 0; j < i; j++) { /* make sure all salts are different */
            if (salts[j] == salt) continue;
        }

        __salts[i] = salt;

        i++;
    }
}


static unsigned int jk_bloom_filter_default_hash(char *key, int len, int index)
{
    unsigned int h = __salts[index], g;
    char *end = key + len;

    while (key < end) {
        h = (h << 4) + *key++;

        if ((g = (h & 0xF0000000))) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h;
}


void jk_bloom_filter_init(jk_bloom_filter_t *filter,
    jk_bloom_filter_hash_fn *hash, char *bits, int size)
{
    if (!hash) {
        jk_bloom_filter_init_salts();
        hash = &jk_bloom_filter_default_hash;
    }

    filter->hash = hash;
    filter->size = size * 8; /* the size equals bits's size */
    filter->bits = bits;
}


jk_bloom_filter_t *jk_bloom_filter_new(jk_bloom_filter_hash_fn *hash,
    char *bits, int size)
{
    jk_bloom_filter_t *filter;

    filter = malloc(sizeof(*filter));
    if (NULL == filter) {
        return NULL;
    }

    memset(bits, 0, size);

    jk_bloom_filter_init(filter, hash, bits, size);

    return filter;
}


void jk_bloom_filter_set(jk_bloom_filter_t *filter, char *key, int len)
{
    int i, index;

    for (i = 0; i < 8; i++) {

        index = filter->hash(key, len, i) % filter->size; /* bits index */

        filter->bits[index / 8] |= 1 << (index % 8);
    }
}


int jk_bloom_filter_isset(jk_bloom_filter_t *filter, char *key, int len)
{
    int i, index;

    for (i = 0; i < 8; i++) {

        index = filter->hash(key, len, i) % filter->size; /* bits index */

        if (!(filter->bits[index / 8] & (1 << (index % 8)))) {
            return 0;
        }
    }

    return 1;
}


void jk_bloom_filter_free(jk_bloom_filter_t *filter)
{
    free(filter);
}

