#include <stdint.h>

#ifndef FACTOR
#define FACTOR 0xdeadbeaf
#endif

#ifndef INIT
#define INIT 0xdeadbeaf
#endif

extern "C" volatile uint64_t __fault_mul_init = INIT;
extern "C" volatile uint64_t __fault_mul_factor = FACTOR;

extern "C" int _main(int argc, char*argv[]);

int main(int argc, char *argv[]) {
    asm volatile("mov __fault_mul_init(%rip), %r13");
    asm volatile("mov __fault_mul_init(%rip), %r12");
    return _main(argc, argv);
}

