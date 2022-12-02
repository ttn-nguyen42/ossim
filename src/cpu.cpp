
#include "cpu.h"
#include "mem.h"

/* Defined in paging.cpp */
memory_t g_Memory;

static int calc(struct pcb_t *proc) {
    return ((unsigned long) proc & 0UL);
}

static int alloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index) {
    addr_t addr = g_Memory.alloc_mem(size, proc);
    if (addr == 0) {
        return 1;
    } else {
        proc->regs[reg_index] = addr;
        return 0;
    }
}

static int free_data(struct pcb_t *proc, uint32_t reg_index) {
    return g_Memory.free_mem(proc->regs[reg_index], proc);
}

static int read(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source, // Index of source register
    uint32_t offset, // Source address = [source] + [offset]
    uint32_t destination) { // Index of destination register

    BYTE data;
    if (g_Memory.read_mem(proc->regs[source] + offset, proc, &data)) {
        proc->regs[destination] = data;
        return 0;
    } else {
        return 1;
    }
}

static int write(
    struct pcb_t *proc, // Process executing the instruction
    BYTE data, // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset) {    // Destination address =
    // [destination] + [offset]
    return g_Memory.write_mem(proc->regs[destination] + offset, proc, data);
}

int run(struct pcb_t *proc) {
    /* Check if Program Counter point to the proper instruction */
    if (proc->pc >= proc->code.text.size()) {
        return 1;
    }

    struct inst_t ins = proc->code.text[proc->pc];
    proc->pc++;
    int stat = 1;
    switch (ins.opcode) {
        case CALC:
//            printf("calc\n");
            stat = calc(proc);
            break;
        case ALLOC:
//            printf("alloc %d %d\n", ins.arg_0, ins.arg_1);
            stat = alloc(proc, ins.arg_0, ins.arg_1);
            break;
        case FREE:
//            printf("free %d\n", ins.arg_0);
            stat = free_data(proc, ins.arg_0);
            break;
        case READ:
//            printf("read %d %d %d\n", ins.arg_0, ins.arg_1, ins.arg_2);
            stat = read(proc, ins.arg_0, ins.arg_1, ins.arg_2);
            break;
        case WRITE:
//            printf("write %d %d %d\n", ins.arg_0, ins.arg_1, ins.arg_2);
            stat = write(proc, ins.arg_0, ins.arg_1, ins.arg_2);
            break;
        default:
            stat = 1;
    }
    return stat;

}


