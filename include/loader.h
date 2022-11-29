#ifndef LOADER_H
#define LOADER_H

#include <memory>
#include "common.h"

std::shared_ptr<pcb_t> load(const char * path);

#endif

