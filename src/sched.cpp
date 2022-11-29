#include "sched.h"

bool mlq_scheduler_t::is_empty() {
    for (int i = 0; i < MAX_PRIO; i += 1) {
        if (!m_q_Ready[i].empty()) {
            return false;
        }
    }
    return true;
}

void mlq_scheduler_t::add_proc(const std::shared_ptr<pcb_t>& proc) {
    std::unique_lock lock(m_Lock);
    m_q_Ready[proc->priority].enqueue(proc);
}

std::shared_ptr<pcb_t> mlq_scheduler_t::get_proc() {
    std::unique_lock lock(m_Lock);
    /* TODO */
    /* Get process from MLQ */
    return nullptr;
}

