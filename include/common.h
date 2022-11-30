#ifndef COMMON_H
#define COMMON_H

/* Define structs and routine could be used by every source files */

#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <pthread.h>
#include <mutex>
#include <bits/stdc++.h>

#define ADDRESS_SIZE    20
#define OFFSET_LEN    10
#define FIRST_LV_LEN    5
#define SECOND_LV_LEN    5
#define SEGMENT_LEN     FIRST_LV_LEN
#define PAGE_LEN        SECOND_LV_LEN

#define NUM_PAGES    (1 << (ADDRESS_SIZE - OFFSET_LEN))
#define PAGE_SIZE    (1 << OFFSET_LEN)

typedef char BYTE;
typedef uint32_t addr_t;

enum ins_opcode_t {
    CALC,    // Just perform calculation, only use CPU
    ALLOC,    // Allocate memory
    FREE,    // Deallocated a memory block
    READ,    // Write data to a byte on memory
    WRITE    // Read data from a byte on memory
};

/* instructions executed by the CPU */
struct inst_t {
    enum ins_opcode_t opcode;
    uint32_t arg_0; // Argument lists for instructions
    uint32_t arg_1;
    uint32_t arg_2;
};

struct code_seg_t {
    std::vector<inst_t> text;

    explicit code_seg_t(int code_size) : text(code_size) {}
};

/* Second layer */
struct trans_table_entry_t {
    addr_t v_index; // The index of virtual address
    addr_t p_index; // The index of physical address
};

struct trans_table_t {
    /* A row in the page table of the second layer */
    std::vector<trans_table_entry_t> table;

    trans_table_t() : table(1 << SECOND_LV_LEN) {}
};

/* Mapping virtual addresses and physical ones */
struct page_table_entry_t {
    addr_t v_index{};    // Virtual index
    std::shared_ptr<trans_table_t> pages{};
};

struct page_table_t {
    /* Translation table for the first layer */
    std::vector<page_table_entry_t> table;

    page_table_t() : table(1 << FIRST_LV_LEN) {}
};

/* PCB, describe information about a process */
struct pcb_t {
    uint32_t pid;    // PID
    uint32_t priority; /* Task with higher priority runs first */
    code_seg_t code;    // Code segment
    addr_t regs[10]{}; // Registers, store address of allocated regions
    uint32_t pc{}; // Program pointer, point to the next instruction
    page_table_t seg_table; // Page table
    uint32_t bp{PAGE_SIZE};    // Break pointer
    uint32_t prio{};

    /* Constructor for initialization */
    pcb_t(uint32_t pid, uint32_t priority, int code_size) : code(code_size) {
        this->pid = pid;
        this->priority = priority;
    }
};

#endif

