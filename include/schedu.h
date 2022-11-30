#pragma once

#ifndef SCHEDULER_H
#define SCHEDULER_H

#define OPTIMIZED_SCH

#include "queue.h"

#define MAX_PRIO 5

class mlq_scheduler_t {
private:
    queue_t m_q_Ready[MAX_PRIO];
#ifdef OPTIMIZED_SCH
    std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<>> m_q_Access;
#endif
    std::mutex m_Lock;
public:
    /* Check if ready queue is currently empty */
    bool is_empty();

    /* Extract processes from the priority queue */
    std::shared_ptr<pcb_t> get_proc();

    /* Add process to MLQ scheduler */
    void add_proc(const std::shared_ptr<pcb_t> &proc);
};

#endif /* SCHEDULER_H */


