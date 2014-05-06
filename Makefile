# Copyright (c) Liexusong

CC=gcc

OBJ = jk_aio.o jk_avl.o jk_bloom_filter.o jk_event.o jk_hash.o jk_spinlock.o jk_thread_pool.o jk_timer.o jk_skip.o jk_heap.o
PRGNAME = libjustkit.a

all: $(OBJ)
	ar -r $(PRGNAME) $(OBJ)

install:
	mkdir /usr/include/justkit
	cp jk_aio.h /usr/include/justkit
	cp jk_avl.h /usr/include/justkit
	cp jk_bloom_filter.h /usr/include/justkit
	cp jk_event.h /usr/include/justkit
	cp jk_hash.h /usr/include/justkit
	cp jk_spinlock.h /usr/include/justkit
	cp jk_thread_pool.h /usr/include/justkit
	cp jk_timer.h /usr/include/justkit
	cp jk_skip.h /usr/include/justkit
	cp jk_list.h /usr/include/justkit
	cp jk_heap.h /usr/include/justkit
	cp jk_types.h /usr/include/justkit
	cp $(PRGNAME) /usr/local/lib

jk_aio.o: jk_aio.c jk_aio.h
	$(CC) -c jk_aio.c

jk_avl.o: jk_avl.c jk_avl.h
	$(CC) -c jk_avl.c

jk_bloom_filter.o: jk_bloom_filter.c jk_bloom_filter.h
	$(CC) -c jk_bloom_filter.c

jk_event.o: jk_event.c jk_event.h
	$(CC) -c jk_event.c

jk_hash.o: jk_hash.c jk_hash.h
	$(CC) -c jk_hash.c

jk_spinlock.o: jk_spinlock.c jk_spinlock.h
	$(CC) -c jk_spinlock.c

jk_thread_pool.o: jk_thread_pool.c jk_thread_pool.h
	$(CC) -c jk_thread_pool.c

jk_timer.o: jk_timer.c jk_timer.h
	$(CC) -c jk_timer.c

jk_skip.o: jk_skip.c jk_skip.h
	$(CC) -c jk_skip.c

jk_heap.o: jk_heap.c jk_heap.h
	$(CC) -c jk_heap.c

clean:
	rm -rf $(PRGNAME) *.o
	rm -r /usr/include/justkit
	rm /usr/local/lib/$(PRGNAME)
