
#include "mem.h"
#include "cpu.h"
#include "loader.h"

extern memory_t g_Memory;

int main(int argc, char ** argv) {
	if (argc < 2) {
		printf("Cannot find input process\n");
		exit(1);
	}
	std::shared_ptr<pcb_t> proc = load(argv[1]);
	unsigned int i;
	for (i = 0; i < proc->code.text.size(); i++) {
		run(proc.get());
	}
    g_Memory.dump();
	return 0;
}

