#include "enclave_t.h"

#include <stdint.h>
uint64_t volatile __fault_count   = 0;
uint64_t volatile __fault_pending = 0;

uint64_t volatile __fault_mul = 0xdaedbeaf;

extern "C" void __attribute__((naked)) __fault_abort(void) {
    asm volatile("ud2");
}

void enclave_run() {

}
