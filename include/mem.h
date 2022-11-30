#pragma once

#ifndef MEM_H
#define MEM_H

#include "common.h"

#define RAM_SIZE    (1 << ADDRESS_SIZE)

struct mem_stat_t {
    uint32_t proc;  // ID of process currently uses this page
    uint32_t index;    // Index of the page in the list of pages allocated to the process.
    long next;    // The next page in the list. -1 if it is the last page.
};

class memory_t {
private:
    std::mutex m_Lock;
    std::vector<mem_stat_t> _mem_stat;
    std::vector<BYTE> _ram;

    /* get offset of the virtual address */
    static addr_t get_offset(addr_t addr);

    /* get the first layer index */
    static addr_t get_first_lv(addr_t addr);

    /* get the second layer index */
    static addr_t get_second_lv(addr_t addr);

    /* Translate virtual address to physical address. If [virtual_addr] is valid,
     * return 1 and write its physical counterpart to [physical_addr].
     * Otherwise, return 0 */
    static addr_t translate(addr_t virtual_addr,      // Given virtual address
                     pcb_t *proc);             // Process uses given virtual address

public:
    memory_t() : _mem_stat(NUM_PAGES), _ram(RAM_SIZE) {}

    /* Allocate [size] bytes for process [proc] and return its virtual address.
     * If we cannot allocate new memory region for this process, return 0 */
    addr_t alloc_mem(uint32_t size, pcb_t *proc);

    /* Free a memory block having the first byte at [address] used by
     * process [proc]. Return 0 if [address] is valid. Otherwise, return 1 */
    int free_mem(addr_t address, pcb_t *proc);

    /* Read 1 byte memory pointed by [address] used by process [proc] and
     * save it to [data].
     * If the given [address] is valid, return 0. Otherwise, return 1 */
    int read_mem(addr_t address, pcb_t *proc, BYTE *data);

    /* Write [data] to 1 byte on the memory pointed by [address] of process
     * [proc]. If given [address] is valid, return 0. Otherwise, return 1 */
    int write_mem(addr_t address, pcb_t *proc, BYTE data);

    void dump();
};


#endif


