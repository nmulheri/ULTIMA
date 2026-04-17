#include "Sched.h"
#include "Sema.h"
#include "MMU.h"
#include <iostream>

using namespace std;

int main() {
    Scheduler sched;
    MMU mmu(1024, '.', 64);

    int id1 = sched.create_task("Task1");
    int id2 = sched.create_task("Task2");
    int id3 = sched.create_task("Task3");

    TCB* t1 = sched.find_task(id1);
    TCB* t2 = sched.find_task(id2);
    TCB* t3 = sched.find_task(id3);

    cout << "\n=== Initial Scheduler Dump ===" << endl;
    sched.dump(1);

    cout << "\n=== Allocate Memory ===" << endl;
    t1->memory_handle = mmu.Mem_Alloc(t1->task_id, 100);
    t2->memory_handle = mmu.Mem_Alloc(t2->task_id, 200);
    t3->memory_handle = mmu.Mem_Alloc(t3->task_id, 500);

    cout << "Task1 handle: " << t1->memory_handle << endl;
    cout << "Task2 handle: " << t2->memory_handle << endl;
    cout << "Task3 handle: " << t3->memory_handle << endl;

    mmu.List_Dump();
    mmu.Mem_Dump(0, 256);

    cout << "\n=== Write Data ===" << endl;
    mmu.Mem_Write(t1->task_id, t1->memory_handle, 0, 16, "this is task one");
    mmu.Mem_Write(t2->task_id, t2->memory_handle, 0, 16, "this is task two");
    mmu.Mem_Write(t3->task_id, t3->memory_handle, 0, 18, "this is task three");

    mmu.Mem_Dump(0, 512);

    cout << "\n=== Read Data ===" << endl;
    char buffer[32];

    if (mmu.Mem_Read(t1->task_id, t1->memory_handle, 0, 16, buffer) == 1) {
        cout << "Read from Task1 memory: " << buffer << endl;
    }

    if (mmu.Mem_Read(t2->task_id, t2->memory_handle, 0, 16, buffer) == 1) {
        cout << "Read from Task2 memory: " << buffer << endl;
    }

    cout << "\n=== Failed Allocation Example ===" << endl;
    int fail_handle = mmu.Mem_Alloc(t1->task_id, 300);
    cout << "Task1 extra allocation handle: " << fail_handle << endl;

    cout << "\n=== Free Task2 Memory ===" << endl;
    mmu.Mem_Free(t2->task_id, t2->memory_handle);
    t2->memory_handle = -1;

    mmu.List_Dump();
    mmu.Mem_Dump(0, 512);

    cout << "\n=== Memory Status ===" << endl;
    cout << "Memory Left: " << mmu.Mem_Left() << endl;
    cout << "Largest Free Segment: " << mmu.Mem_Largest() << endl;
    cout << "Smallest Free Segment: " << mmu.Mem_Smallest() << endl;

    cout << "\n=== Free Remaining Memory ===" << endl;
    mmu.Mem_Free(t1->task_id, t1->memory_handle);
    mmu.Mem_Free(t3->task_id, t3->memory_handle);

    t1->memory_handle = -1;
    t3->memory_handle = -1;

    mmu.List_Dump();
    mmu.Mem_Dump(0, 256);

    return 0;
}