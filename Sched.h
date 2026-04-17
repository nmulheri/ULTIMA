#ifndef SCHED_H
#define SCHED_H

#include "Sema.h"
#include <string>

class Scheduler {
private:
    TCB* process_table;
    TCB* running;
    int next_id;

public:
    Scheduler();
    ~Scheduler();

    int create_task(const std::string& task_name);
    void kill_task(int task_id);
    void yield();
    void garbage_collect();
    void dump(int level);

    TCB* get_running();
    TCB* find_task(int task_id);
    int task_count();
};

#endif