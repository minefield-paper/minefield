PLACEMENT?=100

MITIGATION_CFLAGS=
MITIGATION_CFLAGS+= -mllvm -fh-enable=$(PLACEMENT)
MITIGATION_CFLAGS+= -mllvm -fh-type=doublemul
MITIGATION_CFLAGS+= -mllvm -fh-trap=imul 
MITIGATION_CFLAGS+= -mllvm -fh-handeling=count
MITIGATION_CFLAGS+= -mllvm -fh-store=false
MITIGATION_CFLAGS+= -mllvm -fh-use-imm=false
MITIGATION_CFLAGS+= -mllvm -fh-factor=0x2bbb871 -DFACTOR=0x2bbb871 -DINIT=0x998dbe9

INCLUDE_DIRS=-I$(SGX_SDK)/include -Ienclave
LIBRARY_DIRS=-Lenclave

CFLAGS= $(INCLUDE_DIRS) -Wpedantic -Wall -O3 -march=native -std=c++17 

LDFLAGS= $(LIBRARY_DIRS) -lenclave_proxy -lsgx_urts -lm -lsgx_uae_service -pthread -L$(SGX_SDK)/lib64/ \

DIR=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
COMPILER=$(DIR)/../../../compiler/llvm/build/bin/clang

all:
	make -C enclave CC=$(COMPILER) MITIGATION_CFLAGS="$(MITIGATION_CFLAGS)" all
	g++ $(CFLAGS) main.cpp enclave/enclave_u.c $(LDFLAGS) -o main 

clean:
	-rm main
