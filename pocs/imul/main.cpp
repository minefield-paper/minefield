#include <iterator>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


volatile uint64_t init = 0x5555555555555555; // 07ffffffffff;

extern "C"{
    volatile uint64_t __fault_pending = 0;
};

uint64_t target_faulted          = 0;
uint64_t target_and_trap_faulted = 0;
uint64_t trap_faulted            = 0;



static int      fd;
static uint64_t plane0_offset, plane0_return;
static uint64_t plane2_offset, plane2_return;


namespace undervolting {
void end();
static void signal_handler(int number) {
    end();
    puts("aborted!");
    exit(-1);
}

static uint64_t calculate_msr_value(int64_t offset, uint64_t plane) {
    uint64_t val;
    val = (offset * 1.024) - 0.5; // -0.5 to deal with rounding issues
    val = 0xFFE00000 & ((val & 0xFFF) << 21);
    val = val | 0x8000001100000000;
    val = val | (plane << 40);
    return (uint64_t)val;
}

static void set(uint64_t msr_value) {
    pwrite(fd, &msr_value, 8, 0x150);
}

void open(uint8_t core) {

    char path[20];
    snprintf(path, sizeof(path), "/dev/cpu/%d/msr", core);

    // open the register
    fd = ::open(path, O_RDWR);
    if ( fd == -1 ) {
        printf("UNDERVOLTING: Could not open %s\n", path);
        exit(-1);
    }
    plane0_return = calculate_msr_value(0, 0);
    plane2_return = calculate_msr_value(0, 2);

    signal(SIGINT, signal_handler);
}

void close() {
    if ( fd != -1 ) {
        ::close(fd);
    }
}

void set_undervolting_target(uint64_t offset) {
    plane0_offset = calculate_msr_value(offset, 0);
    plane2_offset = calculate_msr_value(offset, 2);
}

void begin() {
    //////////////////
    // drop voltage //
    //////////////////
    set(plane0_offset);
    set(plane2_offset);
}

void end() {
    ////////////////////
    // return voltage //
    ////////////////////
    set(plane0_return);
    set(plane2_return);
}

}



extern "C" void analyse(uint64_t final_reg, uint64_t *results, int64_t iterations, uint64_t factor) {
    
    uint64_t reg = init;
    for ( int64_t i = iterations; i != 0; --i ) {
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
        reg *= factor;
    }

    uint64_t errors = final_reg != reg;
    uint64_t detected = __fault_pending;
    
    target_faulted += (errors != 0);
    target_and_trap_faulted += ((errors != 0) && (detected != 0));
    trap_faulted += (detected != 0);
}

extern "C" __attribute__((noinline)) uint64_t experiment(uint64_t *results, int64_t iterations, uint64_t factor) {
    uint64_t reg = init;
    __fault_pending = 0;

    do {

        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));
        asm volatile("imulq %%rbx, %%rax" : "=a"(reg) : "a"(reg), "b"(factor));

        results[iterations] = reg;

    } while ( --iterations );

    return reg;
}

uint64_t results[1024 * 1024 * 100];

extern "C" void loop(int64_t iterations, uint64_t random_value) {

    //////////////////
    // drop voltage //
    //////////////////
    undervolting::begin();

    uint64_t final_reg = experiment(results, iterations, random_value);

    ////////////////////
    // return voltage //
    ////////////////////reference
    undervolting::end();

    analyse(final_reg, results, iterations, random_value);
}


extern "C" int _main(int argc, char **argv) {

    srand(0xdeadbeaf);

    if ( argc != 4 ) {
        printf("%s rounds offset core\n", argv[0]);
        exit(-1);
    }

    int64_t     iterations = 1024ull * 30; // * 1024ull;// * 10ull;
    int64_t     rounds     = strtol(argv[1], NULL, 10);
    int64_t     offset     = strtol(argv[2], NULL, 10);
    uint8_t     core       = strtol(argv[3], NULL, 10);

    undervolting::open(core);
    undervolting::set_undervolting_target(offset);

    printf("--- Experiment Started %3ld mV ---\n", offset);

    for ( int64_t i = 0; i < rounds; ++i ) {
        uint32_t factor = rand();

        loop(iterations, factor);

        double fscore  = (double)target_and_trap_faulted / (double)target_faulted;
        printf("%5lu: fscore= %5.6f = %8llu / %8llu (%8llu)\r", i, fscore, target_and_trap_faulted, target_faulted, trap_faulted);
    }

    printf("\n");

    double fscore  = (double)target_and_trap_faulted / (double)target_faulted;
    printf("%5lu: fscore= %5.6f = %8llu / %8llu (%8llu)\n", rounds, fscore, target_and_trap_faulted, target_faulted, trap_faulted);

    undervolting::close();

    return 0;
}
