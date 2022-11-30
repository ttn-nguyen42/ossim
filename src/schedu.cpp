#include "schedu.h"

bool mlq_scheduler_t::is_empty() {
    for (int i = 0; i < MAX_PRIO; i += 1) {
        if (!m_q_Ready[i].empty()) {
            return false;
        }
    }
    return true;
}

void mlq_scheduler_t::add_proc(const std::shared_ptr<pcb_t> &proc) {
    std::unique_lock lock(m_Lock);
    /* O(log n) */
    m_q_Ready[(MAX_PRIO - 1) - proc->prio].enqueue(proc);
#ifdef OPTIMIZED_SCH
    m_q_Access.push(proc->prio);
#endif
}

/*
 * Processes running on the higher priority queues
 * have absolute overridability over process of lower priority
 *
 * NAIVE APPROACH:      From highest priority queues to lowest, if the previous one is
 *                      empty, go to lower priority queues, then take away top priority process from that queue
 *
 *      DISADVANTAGE:   Go through the whole queue stack every process extraction
 *
 *      COMPLEXITY:     C priority_queue pop elements at O(log n)
 *                      perform queue size check at O(1)
 *                      Process search are done in a loop, O(n) where n = number of levels
 *                      |
 *                      |__> Generally O(n) : n = number of levels
 *
 * OPTIMIZATION:        Add another standalone priority queue
 *                      Added processes's prio level (uint32_t) go into that queue, priority comparison are done based on the ASSIGNED QUEUE LEVEL (prio)
 *                      Processes of highest priority goes in front
 *                      Next time get_proc() is accessed, pop() from the priority queue to get the level where most prioritize process just got put into
 *                      then queue[level].pop() to get the level
 *
 *      WHY:            Avoid traversal of queue levels, reduces the complexity to less than O(n)
 *
 *      TRADEOFF:       More memory. uint32_t generally have an allocated size of 4 byte, at any given point of 10000 process, it consumes 40KB more than the naive approach
 */
std::shared_ptr<pcb_t> mlq_scheduler_t::get_proc() {
    std::unique_lock lock(m_Lock);
    /* Naive approach */
    /* Avoid using mlq_scheduler_t.empty(), which makes naive approach O(2*n) */
#ifdef OPTIMIZED_SCH
    /* O(1) */
    if (!m_q_Access.empty()) {
        uint32_t prioritized_next_level = m_q_Access.top();
        /* O(log n) : n is the number of processes */
        m_q_Access.pop();
        if (!m_q_Ready[MAX_PRIO - prioritized_next_level].empty()) {
            /* O(log n) : n is the size of this level queue */
            return m_q_Ready[MAX_PRIO - prioritized_next_level].dequeue();
        }
    }
#else
    for (uint32_t i = 0; i < MAX_PRIO; i += 1) {
        if (!m_q_Ready[i].empty()) {
            return m_q_Ready[i].dequeue();
        }
    }
#endif
    return nullptr;
}

