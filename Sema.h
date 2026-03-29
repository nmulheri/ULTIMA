#ifndef SEMA_H
#define SEMA_H

#include <string>
#include <queue>

enum State { RUNNING, READY, BLOCKED, DEAD };

struct TCB {
    int task_id;
    std::string task_name;
    State state;
};

class Semaphore {
private:
    std::string resource_name;
    int sema_value;
    std::queue<TCB*> sema_queue;

public:
    Semaphore(std::string name, int value = 1);

    void down(TCB* task);
    TCB* up();
    void dump() const;
};

#endif