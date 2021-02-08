CFLAGS= -g -Os -Wall -march=native -std=c++17  -fno-fast-math 
LDFLAGS= 

PLACEMENT?=100
TYPE?=seqmul
STORE?=false
FACTOR?=0x2bbb871
INIT?=0x998dbe9
IMM?=false
T1?=mem
T2?=mem
RAX?=false
SEED?=0xdeadbeaf

MITIGATION_CFLAGS=$(CFLAGS)
MITIGATION_CFLAGS+= -mllvm -fh-enable=$(PLACEMENT)
MITIGATION_CFLAGS+= -mllvm -fh-type=$(TYPE)

MITIGATION_CFLAGS+= -mllvm -fh-imul1=$(T1) 
MITIGATION_CFLAGS+= -mllvm -fh-imul2=$(T2)
MITIGATION_CFLAGS+= -mllvm -fh-use-rax=$(RAX) 
MITIGATION_CFLAGS+= -mllvm -fh-seed=$(SEED) 

MITIGATION_CFLAGS+= -mllvm -fh-handeling=sticky
MITIGATION_CFLAGS+= -mllvm -fh-store=$(STORE)
MITIGATION_CFLAGS+= -mllvm -fh-use-imm=$(IMM)
MITIGATION_CFLAGS+= -mllvm -fh-factor=$(FACTOR) -DFACTOR=$(FACTOR) -DINIT=$(INIT)


DIR=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
COMPILER=$(DIR)/../../compiler/clang

all:
	g++ $(CFLAGS) entry.c $(LDFLAGS) -c -o entry.o
	$(COMPILER)++ $(MITIGATION_CFLAGS) main.cpp entry.c $(LDFLAGS) -o main -Wno-#warnings

clean:
	-rm main entry.c
