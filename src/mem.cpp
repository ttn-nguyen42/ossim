
#include "mem.h"
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <cstdio>

/* Search for page table table from the a segment table */
static struct trans_table_t *get_trans_table(
    addr_t index,    // Segment level index
    page_table_t *page_table) { // first level table

    /*
     * TODO: Given the Segment index [index], you must go through each
     * row of the segment table [page_table] and check if the v_index
     * field of the row is equal to the index
     *
     * */

    int i;
    for (i = 0; i < page_table->table.size(); i++) {
        // Enter your code here
    }
    return nullptr;

}

addr_t memory_t::alloc_mem(uint32_t size, pcb_t *proc) {
    std::unique_lock<std::mutex> lock(m_Lock);
    addr_t ret_mem = 0;
    /* TODO: Allocate [size] byte in the memory for the
     * process [proc] and save the address of the first
     * byte in the allocated memory region to [ret_mem].
     * */

    uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE :
                         size / PAGE_SIZE + 1; // Number of pages we will use
    int mem_avail = 0; // We could allocate new memory region or not?

    /* First we must check if the amount of free memory in
     * virtual address space and physical address space is
     * large enough to represent the amount of required
     * memory. If so, set 1 to [mem_avail].
     * Hint: check [proc] bit in each page of _mem_stat
     * to know whether this page has been used by a process.
     * For virtual memory space, check bp (break pointer).
     * */

    if (mem_avail) {
        /* We could allocate new memory region to the process */
        ret_mem = proc->bp;
        proc->bp += num_pages * PAGE_SIZE;
        /* Update status of physical pages which will be allocated
         * to [proc] in _mem_stat. Tasks to do:
         * 	- Update [proc], [index], and [next] field
         * 	- Add entries to segment table page tables of [proc]
         * 	  to ensure accesses to allocated memory slot is
         * 	  valid. */
    }
    return ret_mem;
}

int memory_t::free_mem(addr_t address, struct pcb_t *proc) {
    /*TODO: Release memory region allocated by [proc]. The first byte of
 * this region is indicated by [address]. Task to do:
 * 	- Set flag [proc] of physical page use by the memory block
 * 	  back to zero to indicate that it is free.
 * 	- Remove unused entries in segment table and page tables of
 * 	  the process [proc].
 * 	- Remember to use lock to protect the memory from other
 * 	  processes.  */
    return 0;
}

int memory_t::read_mem(addr_t address, struct pcb_t *proc, BYTE *data) {
    addr_t physical_addr;
    if (translate(address, &physical_addr, proc)) {
        *data = _ram[physical_addr];
        return 0;
    } else {
        return 1;
    }
}

int memory_t::write_mem(addr_t address, struct pcb_t *proc, BYTE data) {
    addr_t physical_addr;
    if (translate(address, &physical_addr, proc)) {
        _ram[physical_addr] = data;
        return 0;
    } else {
        return 1;
    }
}

void memory_t::dump() {
    int i;
    for (i = 0; i < NUM_PAGES; i++) {
        if (_mem_stat[i].proc != 0) {
            printf("%03d: ", i);
            printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
                   i << OFFSET_LEN,
                   ((i + 1) << OFFSET_LEN) - 1,
                   _mem_stat[i].proc,
                   _mem_stat[i].index,
                   _mem_stat[i].next
            );
            int j;
            for (j = i << OFFSET_LEN;
                 j < ((i + 1) << OFFSET_LEN) - 1;
                 j++) {

                if (_ram[j] != 0) {
                    printf("\t%05x: %02x\n", j, _ram[j]);
                }

            }
        }
    }
}

addr_t memory_t::get_offset(addr_t addr) {
    return addr & ~((~0U) << OFFSET_LEN);
}

addr_t memory_t::get_first_lv(addr_t addr) {
    return addr >> (OFFSET_LEN + PAGE_LEN);
}

addr_t memory_t::get_second_lv(addr_t addr) {
    return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

int memory_t::translate(addr_t virtual_addr, addr_t *physical_addr, pcb_t *proc) {
    /* Offset of the virtual address */
    addr_t offset = get_offset(virtual_addr);
    /* The first layer index */
    addr_t first_lv = get_first_lv(virtual_addr);
    /* The second layer index */
    addr_t second_lv = get_second_lv(virtual_addr);

    /* Search in the first level */
    struct trans_table_t *trans_table = nullptr;
    trans_table = get_trans_table(first_lv, &proc->seg_table);
    if (trans_table == nullptr) {
        return 0;
    }

    int i;
    for (i = 0; i < trans_table->table.size(); i++) {
        if (trans_table->table[i].v_index == second_lv) {
            /* TODO: Concatenate the offset of the virtual addess
             * to [p_index] field of trans_table->table[i] to
             * produce the correct physical address and save it to
             * [*physical_addr]  */
            return 1;
        }
    }
    return 0;
}
