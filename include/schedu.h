#pragma once

#ifndef SCHEDULER_H
#define SCHEDULER_H

// #define OPTIMIZED_SCH

#include "queue.h"

#define MAX_PRIO 512

#ifdef MLQ_SCHED
class mlq_scheduler_t {
private:
    queue_t m_q_Ready[MAX_PRIO];
#ifdef OPTIMIZED_SCH
    std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<>> m_q_Access;
#endif
    std::mutex m_Lock;
public:
    /* Extract processes from the priority queue */
    std::shared_ptr<pcb_t> get_proc();

    /* Add process to MLQ scheduler */
    void add_proc(const std::shared_ptr<pcb_t> &proc);
};
#else
class scheduler_t {
private:
    queue_t m_q_Ready;
    queue_t m_q_Run;
    std::mutex m_Lock;
public:
    /* Extract processes from the priority queue */
    std::shared_ptr<pcb_t> get_proc();

    /* Add process to ready queue */
    void add_proc(const std::shared_ptr<pcb_t> &proc);

    /* Add process to run queue */
    void put_proc(const std::shared_ptr<pcb_t> &proc);
};
#endif

#endif /* SCHEDULER_H */


