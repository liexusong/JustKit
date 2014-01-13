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
#include <unistd.h>


static int ncpu;


void jk_spinlock_init()
{
    ncpu = sysconf(_SC_NPROCESSORS_ONLN); /* linux only */
    if (ncpu <= 0) {
        ncpu = 1;
    }
}


void jk_spinlock(int *lock, int which)
{
    int i, n;

    for ( ;; ) {

        if (*lock == 0 && 
            __sync_bool_compare_and_swap(lock, 0, which)) {
            return;
        }

        if (ncpu > 1) {

            for (n = 1; n < 129; n << 1) {

                for (i = 0; i < n; i++) {
                    __asm__("pause");
                }
    
                if (*lock == 0 && 
                    __sync_bool_compare_and_swap(lock, 0, which)) {
                    return;
                }
            }
        }

        sched_yield();
    }
}


void jk_spinunlock(int *lock, int which)
{
    __sync_bool_compare_and_swap(lock, which, 0);
}

