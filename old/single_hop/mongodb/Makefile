CFLAGS+=-fopenmp -O3
CFLAGS+=-I/home/kfeng/install/include/libmongoc-1.0
CFLAGS+=-I/home/kfeng/install/include/libbson-1.0
LDFLAGS+=-L/home/kfeng/install/lib64 -lmongoc-1.0 -lbson-1.0
MPICC=/home/kfeng/MPICH/bin/mpicc
CC=gcc
PWD=$(shell pwd)

all: insert query insert_mpi

insert: insert.o link_mongo.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

query: query.o link_mongo.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

insert_mpi: insert_mpi.o link_mongo.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o insert query
