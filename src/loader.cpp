
#include "loader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

static uint32_t avail_pid = 1;

#define OPT_CALC        "calc"
#define OPT_ALLOC       "alloc"
#define OPT_FREE        "free"
#define OPT_READ        "read"
#define OPT_WRITE       "write"

static enum ins_opcode_t get_opcode(char *opt) {
    if (!strcmp(opt, OPT_CALC)) {
        return CALC;
    } else if (!strcmp(opt, OPT_ALLOC)) {
        return ALLOC;
    } else if (!strcmp(opt, OPT_FREE)) {
        return FREE;
    } else if (!strcmp(opt, OPT_READ)) {
        return READ;
    } else if (!strcmp(opt, OPT_WRITE)) {
        return WRITE;
    } else {
        printf("Opcode: %s\n", opt);
        exit(1);
    }
}

std::shared_ptr<pcb_t> load(const char *path) {
    std::ifstream descriptor(path);
    if (!descriptor) {
        printf("Process descriptor not found: %s\n", path);
        exit(1);
    }
    char* opcode = (char*)"";
    int code_size, priority = 0;
    descriptor >> priority >> code_size;
    avail_pid += 1;
    std::shared_ptr<pcb_t> proc = std::make_shared<pcb_t>(avail_pid, priority, code_size);
    for (inst_t& it : proc->code.text) {
        descriptor >> opcode;
        it.opcode = get_opcode(opcode);
        switch (it.opcode) {
            case CALC:
                break;
            case ALLOC:
                descriptor >> it.arg_0 >> it.arg_1;
                break;
            case FREE:
                descriptor >> it.arg_0;
                break;
            case READ:
                break;
            case WRITE:
                descriptor >> it.arg_0 >> it.arg_1 >> it.arg_2;
                break;
            default:
                printf("Invalid opcode: %s\n", opcode);
                exit(1);
        }
    }


}



