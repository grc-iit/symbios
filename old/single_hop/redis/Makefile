CFLAGS+=-fopenmp -O3
CFLAGS+=-I/home/kfeng/install/include
CFLAGS+=-I/home/kfeng/pkg_src/hiredis-vip
LDFLAGS+=-L/home/kfeng/install/lib -lpthread -lcrypto -lm
LDFLAGS+=-L/home/kfeng/pkg_src/hiredis-vip -lhiredis_vip
#LDFLAGS+=-L/home/kfeng/install/lib -lpthread -lcrypto -llmpe -lmpe -ltmpe -lm
MPICC=/home/kfeng/MPICH/bin/mpicc
CC=gcc
PWD=$(shell pwd)

all: redis_create_shadow redis_create_shadow_mt

redis_create_shadow: redis_create_shadow.o link_redis.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

redis_create_shadow_mt: redis_create_shadow_mt.o link_redis.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o redis_create_shadow redis_create_shadow_mt
