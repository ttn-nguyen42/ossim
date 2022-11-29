#pragma once

#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#define MAX_QUEUE_SIZE 10

/* Define necessary methods to compare the priority between processes */
class pcb_comparator {
public:
    bool operator()(std::shared_ptr<pcb_t> const &a, std::shared_ptr<pcb_t> const &b) {
        return (a->priority < b->priority);
    }
};

class queue_t {
private:
    std::priority_queue<std::shared_ptr<pcb_t>, std::vector<std::shared_ptr<pcb_t>>, pcb_comparator> q;
public:
    /* Add new process to queue */
    void enqueue(const std::shared_ptr<pcb_t>& proc);

    /* Dequeue the top priority process */
    std::shared_ptr<pcb_t> dequeue();

    /* Check if queue is empty */
    bool empty();
};

#endif

