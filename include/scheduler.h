#pragma once

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "queue.h"

#define MAX_PRIO 5

class mlq_scheduler_t {
private:
    queue_t m_q_Ready[MAX_PRIO];
    std::mutex m_Lock;
public:
    /* Check if ready queue is currently empty */
    bool is_empty();

    std::shared_ptr<pcb_t> get_proc();

    /* Add process to MLQ scheduler */
    void add_proc(const std::shared_ptr<pcb_t>& proc);
};

#endif /* SCHED_H */


