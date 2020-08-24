CFLAGS+=-fopenmp -O3
CFLAGS+=-I/home/kfeng/install/include
CFLAGS+=-I/home/kfeng/pkg_src/hiredis-vip
CFLAGS+=-I/home/kfeng/install/include/libmongoc-1.0
CFLAGS+=-I/home/kfeng/install/include/libbson-1.0
LDFLAGS+=-L/home/kfeng/install/lib64 -lmongoc-1.0 -lbson-1.0
LDFLAGS+=-L/home/kfeng/install/lib -lpthread -lcrypto -lm
LDFLAGS+=-L/home/kfeng/pkg_src/hiredis-vip -lhiredis_vip
MPICC=/home/kfeng/MPICH/bin/mpicc
CC=gcc
PWD=$(shell pwd)

all: test_link_redis test_link_mongo

test_link_symbios: test_link_symbios.o io_redis.o io_mongo.o common.o common.h
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_link_redis: test_link_redis.o io_redis.o common.o common.h
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_link_mongo: test_link_mongo.o io_mongo.o common.o common.h
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c common.h
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o test_link_redis test_link_mongo test_link_symbios
