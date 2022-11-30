#include "queue.h"

void queue_t::enqueue(std::shared_ptr<pcb_t> proc) {
    /* Enqueue new process */
    if (q.size() == MAX_QUEUE_SIZE) {
        perror("Queue overflow while enqueue-ing\n");
        return;
    }
    q.push(proc);
}

std::shared_ptr<pcb_t> queue_t::dequeue() {
    /* Returns the process of highest priority */
    if (empty()) {
        perror("Dequeue-ing from empty queue\n");
        return nullptr;
    }
    std::shared_ptr<pcb_t> top = q.top();
    q.pop();
    return top;
}

bool queue_t::empty() {
    return q.empty();
}
