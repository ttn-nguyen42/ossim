
#include "cpu.h"
#include "timer.h"
#include "schedu.h"
#include "loader.h"

#define MLQ_SCHED
static int time_slot;
static int num_cpus;
static int done = 0;

static mlq_scheduler_t g_Scheduler;

static struct ld_args {
    char **path;
    unsigned long *start_time;
#ifdef MLQ_SCHED
    unsigned long *prio;
#endif
} ld_processes;
int num_processes;

struct cpu_args {
    struct timer_id_t *timer_id;
    int id;
};

static void cpu_routine(timer_id_t* timer_id, int id) {
    // struct timer_id_t *timer_id = ((struct cpu_args *) args)->timer_id;
    // int id = ((struct cpu_args *) args)->id;
    /* Check for new process in ready queue */
    int time_left = 0;
    std::shared_ptr<pcb_t> proc;
    while (true) {
        /* Check the status of current process */
        if (!proc) {
            /* No process is running, then we load new process from
             * ready queue */
            proc = g_Scheduler.get_proc();
            if (!proc) {
                next_slot(timer_id);
                continue; /* First load failed. skip dummy load */
            }
        } else if (proc->pc == proc->code.text.size()) {
            /* The process has finish it job */
            printf("\tCPU %d: Processed %2d has finished\n",
                   id, proc->pid);
            proc = g_Scheduler.get_proc();
            time_left = 0;
        } else if (time_left == 0) {
            /* The process has done its job in current time slot */
            printf("\tCPU %d: Put process %2d to run queue\n",
                   id, proc->pid);
            g_Scheduler.add_proc(proc);
            proc = g_Scheduler.get_proc();
        }

        /* Recheck process status after loading new process */
        if (!proc && done) {
            /* No process to run, exit */
            printf("\tCPU %d stopped\n", id);
            break;
        } else if (!proc) {
            /* There may be new processes to run in
             * next time slots, just skip current slot */
            next_slot(timer_id);
            continue;
        } else if (time_left == 0) {
            printf("\tCPU %d: Dispatched process %2d\n",
                   id, proc->pid);
            time_left = time_slot;
        }

        /* Run current process */
        run(proc.get());
        time_left--;
        next_slot(timer_id);
    }
    detach_event(timer_id);
    pthread_exit(nullptr);
}

static void ld_routine(timer_id_t* timer_id) {
    // auto *timer_id = (struct timer_id_t *) args;
    int i = 0;
    while (i < num_processes) {
        std::shared_ptr<pcb_t> proc = load(ld_processes.path[i]);
        proc->prio = ld_processes.prio[i];
        while (current_time() < ld_processes.start_time[i]) {
            next_slot(timer_id);
        }
        printf("\tLoaded a process at %s, PID: %d PRIO: %ld\n",
               ld_processes.path[i], proc->pid, ld_processes.prio[i]);
        g_Scheduler.add_proc(proc);
        free(ld_processes.path[i]);
        i++;
        next_slot(timer_id);
    }
    free(ld_processes.path);
    free(ld_processes.start_time);
    done = 1;
    detach_event(timer_id);
    pthread_exit(nullptr);
}

static void read_config(const char *path) {
    FILE *file;
    if ((file = fopen(path, "r")) == nullptr) {
        printf("Cannot find configure file at %s\n", path);
        exit(1);
    }
    fscanf(file, "%d %d %d\n", &time_slot, &num_cpus, &num_processes);
    ld_processes.path = (char **) malloc(sizeof(char *) * num_processes);
    ld_processes.start_time = (unsigned long *)
        malloc(sizeof(unsigned long) * num_processes);
#ifdef MLQ_SCHED
    ld_processes.prio = (unsigned long *)
        malloc(sizeof(unsigned long) * num_processes);
#endif
    int i;
    for (i = 0; i < num_processes; i++) {
        ld_processes.path[i] = (char *) malloc(sizeof(char) * 100);
        ld_processes.path[i][0] = '\0';
        strcat(ld_processes.path[i], "input/proc/");
        char proc[100];
#ifdef MLQ_SCHED
        fscanf(file, "%lu %s %lu\n", &ld_processes.start_time[i], proc, &ld_processes.prio[i]);
#else
        fscanf(file, "%lu %s\n", &ld_processes.start_time[i], proc);
#endif
        strcat(ld_processes.path[i], proc);
    }
}

int main(int argc, char *argv[]) {
    /* Read config */
    if (argc != 2) {
        printf("Usage: os [path to configure file]\n");
        return 1;
    }
    char path[100];
    path[0] = '\0';
    strcat(path, "input/");
    strcat(path, argv[1]);
    read_config(path);

    /* Memory leaks here */
    // auto *cpu = (pthread_t *) malloc(num_cpus * sizeof(pthread_t));
    // auto *args = (struct cpu_args *) malloc(sizeof(struct cpu_args) * num_cpus);
    // pthread_t ld;
    std::vector<std::thread> cpu;
    std::vector<timer_id_t*> args;
    std::thread ld;

    /* Init timer */
    int i;
    for (i = 0; i < num_cpus; i++) {
        args.push_back(attach_event());
    }
    struct timer_id_t *ld_event = attach_event();
    start_timer();

    /* Run CPU and loader */
    ld = std::thread(ld_routine, ld_event);
    for (i = 0; i < num_cpus; i++) {
        cpu.emplace_back(cpu_routine, args.at(i), i);
    }

    /* Wait for CPU and loader finishing */
    for (i = 0; i < num_cpus; i++) {
        cpu.at(i).join();
    }
    ld.join();

    /* Stop timer */
    stop_timer();

    return 0;

}



