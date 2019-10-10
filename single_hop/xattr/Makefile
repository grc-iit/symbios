PVFS_DIR=/opt/ohpc/pub/orangefs
CFLAGS+=-fopenmp -O3 -g
CFLAGS+=-I/home/kfeng/install/include
CFLAGS+=-Wl,-rpath=$(PVFS_DIR)/lib
#CFLAGS+=-I/home/kfeng/pkg_src/hiredis-vip
LDFLAGS+=-L$(PVFS_DIR)/lib -lorangefsposix
LDFLAGS+=-L/home/kfeng/install/lib -lpthread -lcrypto -lm
LDFLAGS+=-L/home/kfeng/pkg_src/hiredis-vip -lhiredis_vip
#LDFLAGS+=-L/home/kfeng/install/lib -lpthread -lcrypto -llmpe -lmpe -ltmpe -lm
MPICC=/home/kfeng/MPICH_331_CH4/bin/mpicc
CC=/home/kfeng/MPICH_331_CH4/bin/mpicc
#CC=gcc
PWD=$(shell pwd)

all: setxattr getxattr header header_mpi

setxattr: setxattr.o get_num.o error_functions.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

header: header.o get_num.o error_functions.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

header_mpi: header_mpi.o get_num.o error_functions.o
	$(MPICC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

getxattr: getxattr.o get_num.o error_functions.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o setxattr getxattr header header_mpi
