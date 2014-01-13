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

#ifndef __JK_BLOOM_FILTER_H
#define __JK_BLOOM_FILTER_H

typedef unsigned int jk_bloom_filter_hash_fn(char *key, int len, int index);

typedef struct jk_bloom_filter_s {
    jk_bloom_filter_hash_fn *hash;
    int size;
    char *bits;
} jk_bloom_filter_t;

void jk_bloom_filter_init(jk_bloom_filter_t *filter,
    jk_bloom_filter_hash_fn *hash, char *bits, int size);
jk_bloom_filter_t *jk_bloom_filter_new(jk_bloom_filter_hash_fn *hash,
    char *bits, int size);
void jk_bloom_filter_set(jk_bloom_filter_t *filter, char *key, int len);
int jk_bloom_filter_isset(jk_bloom_filter_t *filter, char *key, int len);
void jk_bloom_filter_free(jk_bloom_filter_t *filter);

#endif
