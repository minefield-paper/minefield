#include "enclave/enclave_u.h"

#include <sgx_urts.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static sgx_enclave_id_t eid = 0;

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


void ocall_print_string(const char *str) {
    puts(str);
}

void experiment() {
    //////////////////
    // drop voltage //
    //////////////////
    undervolting::begin();

    if ( enclave_run(eid) != SGX_SUCCESS ) {
        undervolting::end();

        puts("cannot run enclave");
        exit(-1);
    }

    ////////////////////
    // return voltage //
    ////////////////////
    undervolting::end();
}

extern "C" int main(int argc, char **argv) {

    srand(time(0));

    if ( argc != 4 ) {
        printf("%s rounds offset core\n", argv[0]);
        exit(-1);
    }

    int64_t     rounds = strtol(argv[1], NULL, 10);
    int64_t     offset = strtol(argv[2], NULL, 10);
    uint8_t     core   = strtol(argv[3], NULL, 10);

    char buffer[500];
    snprintf(buffer, 500, "%s", argv[0]);
    buffer[strlen(buffer) - 4] = 0;
    char buffer2[500];
    snprintf(buffer2, 500, "%s/enclave/enclave.so", buffer);

    int                updated = 0;
    sgx_launch_token_t token   = { 0 };
    if ( sgx_create_enclave(buffer2, /*debug=*/1, &token, &updated, &eid, NULL) != SGX_SUCCESS ) {
        puts("cannot open enclave");
        exit(-1);
    }

    undervolting::open(core);
    undervolting::set_undervolting_target(offset);

    for ( int64_t i = 0; i < rounds; ++i ) {
        experiment();
    }

    printf("done\n");
    
    undervolting::close();

    sgx_destroy_enclave(eid);

    return 0;
}
