
#include "mem.h"

addr_t memory_t::alloc_mem(uint32_t size, pcb_t *proc) {
    std::unique_lock<std::mutex> lock(m_Lock);
    addr_t ret_mem = 0;
    /* Allocate [size] byte in the memory for the
     * process [proc] and save the address of the first
     * byte in the allocated memory region to [ret_mem].
     */

    //uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE :
    //                     size / PAGE_SIZE + 1; // Number of pages we will use
    uint32_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int mem_avail = 0; // We could allocate new memory region or not?

    /* First we must check if the amount of free memory in
     * virtual address space and physical address space is
     * large enough to represent the amount of required memory
     *
     * If so, set 1 to [mem_avail].
     * Hint: check [proc] bit in each page of _mem_stat
     * to know whether this page has been used by a process.
     * For virtual memory space, check bp (break pointer).
     */
    {
        uint32_t available_pages = 0;
        for (int i = 0; i < NUM_PAGES; i += 1) {
            if (_mem_stat[i].proc <= 0) {
                /* Not allocated */
                available_pages += 1;
            }
            if (available_pages >= num_pages) {
                break;
            }
        }
        /* Check if new memory region can be allocated
         *
         * On the physical address space, the number of pages must not be less than number of available pages
         * As for virtual address space, the size span from the breakpoint to its final segment must be less than the maximum address possible
         * (not more than 20 bits)
         */
        if (available_pages >= num_pages) {
            if (proc->bp + (num_pages * PAGE_SIZE) <= RAM_SIZE) {
                mem_avail = 1;
            }
        }
    }

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
        for (long phys_index = 0,
                 page_index = 0,
                 prev_index = -1;; phys_index += 1) {
            if (_mem_stat[phys_index].proc > 0) {
                /* Allocated, skip */
                continue;
            }

            /* Update the segment table */
            /* Calculate the virtual address */
            addr_t v_addr = ret_mem + (page_index * PAGE_SIZE);
            // printf("Virtual address: %d\n", v_addr);

            addr_t first_level_index = get_first_lv(v_addr);
            addr_t second_level_index = get_second_lv(v_addr);

            /* Update the level 1 segment */
            auto &first_level_entry = proc->seg_table.table.at(first_level_index);
            if (first_level_entry.v_index == 0) {
                first_level_entry.v_index = 1;
                first_level_entry.pages = std::make_shared<trans_table_t>();
            }

            /* Update the level 2 segment */
            auto &second_level_entry = first_level_entry.pages->table[second_level_index];
            second_level_entry.v_index = 1;
            second_level_entry.p_index = (addr_t) phys_index;
            first_level_entry.pages->size += 1;

            /* Update the memory status */
            if (prev_index >= 0) {
                /* May cause problem by casting to int */
                /* Phys index have type uint32_t with max value greater than int in _mem_stat
                 * However, ->next of the last page must be -1
                 * _mem_state entries will be long
                 */
                _mem_stat[prev_index].next = phys_index;
            }
            prev_index = phys_index;

            _mem_stat[phys_index].proc = proc->pid;
            _mem_stat[phys_index].index = page_index;

            page_index += 1;
            if (page_index >= num_pages) {
                /* Last page has next of (-1) */
                _mem_stat[phys_index].next = -1;
                break;
            }
        }
    }
    return ret_mem;
}

int memory_t::free_mem(addr_t address, pcb_t *proc) {
    std::unique_lock<std::mutex> lock(m_Lock);
    /* Release memory region allocated by [proc]. The first byte of
     * this region is indicated by [address]
     * 	- Set flag [proc] of physical page use by the memory block
     * 	  back to zero to indicate that it is free.
     * 	- Remove unused entries in segment table and page tables of
     * 	  the process [proc].
     * 	- Remember to use lock to protect the memory from other
     * 	  processes.  */
    addr_t physical_addr = translate(address, proc);
    if (physical_addr == INT32_MAX) {
        return 1;
    }

    addr_t physical_start = physical_addr >> OFFSET_LEN;
    for (long physical_index = physical_start,
             page_index = 0;
         physical_index != -1; page_index += 1) {
        addr_t virtual_addr = address + (page_index * PAGE_SIZE);
        addr_t first_level_index = get_first_lv(virtual_addr);
        addr_t second_level_index = get_second_lv(virtual_addr);

        /* Clean second level */
        auto &trans_table = proc->seg_table.table[first_level_index].pages;
        trans_table->table[second_level_index].v_index = 0;
        trans_table->size -= 1;

        /* Clean first level */
        if (trans_table->size == 0) {
            auto &page_table = proc->seg_table.table;
            page_table[first_level_index].pages.reset();
            page_table[first_level_index].v_index = 0;
        }

        /* Move to next mem_stat and clear */
        _mem_stat[physical_index].proc = 0;
        physical_index = _mem_stat[physical_index].next;
    }
    return 0;
}

int memory_t::read_mem(addr_t address, pcb_t *proc, BYTE *data) {
    addr_t physical_addr = translate(address, proc);
    if (physical_addr != INT32_MAX) {
        *data = _ram[physical_addr];
        return 0;
    } else {
        return 1;
    }
}

int memory_t::write_mem(addr_t address, pcb_t *proc, BYTE data) {
    addr_t physical_addr = translate(address, proc);
    // printf("At: %d\n", physical_addr);
    // printf("Data -> memory: %d\n", data);
    if (physical_addr != INT32_MAX) {
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
            printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03ld)\n",
                   /*
                    * i = 0
                    * Shift left 10 -> 0000 0000 0000 0000
                    *
                    * i = 1
                    * Shift left 10 -> 0000 0100 0000 0000
                    */
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
    /*
     * 0u = unsigned 0 -> 0x0000
     * ~0u = not 0u = 0xFFFF
     * ~0u << OFFSET_LEN = ~0u << 10 = 0xFF (1111 1100) (0000 0000) [shift left 10 bits]
     * ~(~0u << OFFSET_LEN) = (0000 0000) (0000 0000) (0000 0011) (1111 1111)
     *
     * addr & [...] means to leave the last 10 bits of addr and set the first (32-10) = 22 to be 0
     * From:    (1010 0010) (0101 0110) (0111 1111) (1101 1010)
     * To       (0000 0000) (0000 0000) (0000 0011) (1101 1010)
     */

    /* Get the last 10 bits of the provided virtual address */
    return addr & ~((~0u) << OFFSET_LEN);
}

addr_t memory_t::get_first_lv(addr_t addr) {
    /*
     * addr =       11111111111111111111
     * add >> 15 =  00000000000000011111
     */

    /* Get the first 5 bits in front */
    return addr >> (OFFSET_LEN + PAGE_LEN);
}

addr_t memory_t::get_second_lv(addr_t addr) {
    /*
     * addr =                           11111111111111111111
     * get_first_lv(addr)               00000000000000011111
     * get_first_lv(addr) << PAGE_LEN   00000000001111100000
     * addr >> OFFSET_LEN               00000000001111111111
     * [...]                            00000000000000011111
     */

    /* Get the second 5 bits in front */
    return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Translate virtual address to physical address */
addr_t memory_t::translate(addr_t virtual_addr, pcb_t *proc) {
    /* Offset of the virtual address */
    addr_t offset = get_offset(virtual_addr);

    /* The first layer index */
    addr_t first_lv = get_first_lv(virtual_addr);

    /* The second layer index */
    addr_t second_lv = get_second_lv(virtual_addr);
    /*
     * Example: 13535
     * 00000|01101|0011011111
     *
     */
    /*std::bitset<20> virtual_addr_bits(virtual_addr);
    std::bitset<10> offset_bits(offset);
    std::bitset<5> first_lv_bits(first_lv);
    std::bitset<5> second_lv_bits(second_lv);

    std::cout << "Virtual address: " << virtual_addr_bits << "     " << virtual_addr << std::endl;
    std::cout << "Offset bits:               " << offset_bits << "     " << offset << std::endl;
    std::cout << "First level:     " << first_lv_bits << "                    " << first_lv << std::endl;
    std::cout << "Second level:         " << second_lv_bits << "               " << second_lv << std::endl;*/

    /* Search in the first level */
    std::shared_ptr<trans_table_t> trans_table = proc->seg_table.table[first_lv].pages;
    if (proc->seg_table.table[first_lv].v_index) {
        if (trans_table->table[second_lv].v_index) {
            /* Concatenate the offset of the virtual addess
             * to [p_index] field of trans_table->table
             */

            /*
             * offset   = 0000000000|1100110101
             * addr     = 1111111111|1111111111
             * [...]    = 1111111111|1100110101
             */

            /* Shift left by OFFSET_LEN (in this case is 10) then perform bitwise OR with offset
             * to concatenate virtual address offset
             */
            return (trans_table->table[second_lv].p_index << OFFSET_LEN | offset);
        }
    }

    /* uint32_t has more number needed for indexing in this exercise */
    return INT32_MAX;
}
