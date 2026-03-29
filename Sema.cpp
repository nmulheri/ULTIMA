#include "Sema.h"
#include <iostream>

using namespace std;

Semaphore::Semaphore(string name, int value) {
    resource_name = name;
    sema_value = value;
}

void Semaphore::down(TCB* task) {
    if (sema_value == 1) {
        sema_value = 0;
        task->state = RUNNING;
        cout << task->task_name << " acquired " << resource_name << endl;
    } else {
        task->state = BLOCKED;
        sema_queue.push(task);
        cout << task->task_name << " BLOCKED and added to queue\n";
    }
}

TCB* Semaphore::up() {
    if (!sema_queue.empty()) {
        TCB* next = sema_queue.front();
        sema_queue.pop();
        next->state = READY;
        cout << next->task_name << " UNBLOCKED and moved to READY\n";
        return next;
    } else {
        sema_value = 1;
        cout << resource_name << " released and now AVAILABLE\n";
        return nullptr;
    }
}

void Semaphore::dump() const {
    cout << "\n--- Semaphore Dump ---\n";
    cout << "Resource: " << resource_name << endl;
    cout << "Sema Value: " << sema_value << endl;

    cout << "Queue: ";
    if (sema_queue.empty()) {
        cout << "EMPTY";
    } else {
        queue<TCB*> temp = sema_queue;
        while (!temp.empty()) {
            cout << temp.front()->task_id;
            temp.pop();
            if (!temp.empty()) cout << " -> ";
        }
    }
    cout << endl;
}