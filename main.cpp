#include "Sched.h"
#include "Sema.h"
#include <iostream>

using namespace std;

int main() {
    Scheduler sched;
    Semaphore sem("Printer", 1);

    // task creation
    int id1 = sched.create_task("Task1");
    int id2 = sched.create_task("Task2");
    int id3 = sched.create_task("Task3");
    sched.dump(1);

    // round-robin yield through all three
    sched.yield();
    sched.dump(0);
    sched.yield();
    sched.dump(0);
    sched.yield(); // wraps back to Task1
    sched.dump(0);

    // semaphore contention Task1 acquires, Task2 and Task3 block
    TCB* t1 = sched.find_task(id1);
    TCB* t2 = sched.find_task(id2);
    TCB* t3 = sched.find_task(id3);

    sem.down(t1);
    sem.down(t2);
    sem.down(t3);
    sem.dump();
    sched.dump(1);

    // release to unblock in FIFO order
    sem.up();
    sem.up();
    sem.up();
    sem.dump();
    sched.dump(1);

    // scheduling resumes after unblock
    sched.yield();
    sched.yield();
    sched.dump(0);

    // kill and garbage collect
    sched.kill_task(id2);
    sched.garbage_collect();
    sched.dump(1);

    sched.kill_task(id1);
    sched.kill_task(id3);
    sched.garbage_collect();
    sched.dump(1);

    return 0;
}