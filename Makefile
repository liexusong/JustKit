
all:
	gcc jk_aio.c jk_avl.c jk_bloom_filter.c jk_event.c jk_hash.c jk_spinlock.c jk_thread_pool.c jk_timer.c -shared -o justkit.so
