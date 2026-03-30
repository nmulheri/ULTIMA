#include "Sched.h"
#include <iostream>

using namespace std;

static string stateToString(State s) {
    switch (s) {
        case RUNNING: return "RUNNING";
        case READY:   return "READY";
        case BLOCKED: return "BLOCKED";
        case DEAD:    return "DEAD";
        default:      return "UNKNOWN";
    }
}

Scheduler::Scheduler() {
    process_table = nullptr;
    running = nullptr;
    next_id = 1;
}

Scheduler::~Scheduler() {
    TCB* cur = process_table;
    while (cur) {
        TCB* tmp = cur;
        cur = cur->next;
        delete tmp;
    }
}

int Scheduler::create_task(const string& task_name) {
    TCB* task = new TCB;
    task->task_id   = next_id++;
    task->task_name = task_name;
    task->state     = READY;
    task->next      = nullptr;

    // first task becomes RUNNING
    if (!process_table) {
        task->state = RUNNING;
        running = task;
        process_table = task;
    } else {
        TCB* cur = process_table;
        while (cur->next)
            cur = cur->next;
        cur->next = task;
    }

    cout << "Created " << task->task_name
         << " (ID: " << task->task_id << ")" << endl;
    return task->task_id;
}

void Scheduler::kill_task(int task_id) {
    TCB* task = find_task(task_id);
    if (!task) {
        cout << "Task ID " << task_id << " not found" << endl;
        return;
    }
    task->state = DEAD;
    if (running == task)
        running = nullptr;
    cout << task->task_name << " set to DEAD" << endl;
}

void Scheduler::yield() {
    if (!process_table) return;

    // no running task scan from head for first READY
    if (!running) {
        TCB* cur = process_table;
        while (cur) {
            if (cur->state == READY) {
                cur->state = RUNNING;
                running = cur;
                cout << running->task_name << " now RUNNING" << endl;
                return;
            }
            cur = cur->next;
        }
        cout << "No READY tasks available" << endl;
        return;
    }

    running->state = READY;

    // round-robin to scan from next node, wrap to head
    TCB* start = running->next ? running->next : process_table;
    TCB* cur = start;
    bool wrapped = false;

    while (true) {
        if (cur->state == READY) {
            cur->state = RUNNING;
            running = cur;
            cout << running->task_name << " now RUNNING" << endl;
            return;
        }
        cur = cur->next;
        if (!cur) {
            if (wrapped) break;
            cur = process_table;
            wrapped = true;
        }
        if (cur == start && wrapped) break;
    }

    // no other READY task keep current
    running->state = RUNNING;
    cout << running->task_name << " continues RUNNING (no other READY tasks)" << endl;
}

void Scheduler::garbage_collect() {
    int removed = 0;

    while (process_table && process_table->state == DEAD) {
        TCB* tmp = process_table;
        process_table = process_table->next;
        cout << "Garbage collected " << tmp->task_name << endl;
        delete tmp;
        removed++;
    }

    TCB* prev = process_table;
    while (prev && prev->next) {
        if (prev->next->state == DEAD) {
            TCB* tmp = prev->next;
            prev->next = tmp->next;
            cout << "Garbage collected " << tmp->task_name << endl;
            delete tmp;
            removed++;
        } else {
            prev = prev->next;
        }
    }

    if (removed == 0)
        cout << "Nothing to collect" << endl;
}

TCB* Scheduler::get_running() {
    return running;
}

TCB* Scheduler::find_task(int task_id) {
    TCB* cur = process_table;
    while (cur) {
        if (cur->task_id == task_id)
            return cur;
        cur = cur->next;
    }
    return nullptr;
}

int Scheduler::task_count() {
    int count = 0;
    TCB* cur = process_table;
    while (cur) {
        count++;
        cur = cur->next;
    }
    return count;
}

void Scheduler::dump(int level) {
    cout << "\n--- Process Table Dump ---" << endl;

    if (!process_table) {
        cout << "(empty)" << endl;
        return;
    }

    cout << "Task Name\tTask ID\t\tState" << endl;
    cout << "---------\t-------\t\t-----" << endl;
    TCB* cur = process_table;
    while (cur) {
        cout << cur->task_name << "\t\t"
             << cur->task_id   << "\t\t"
             << stateToString(cur->state) << endl;
        cur = cur->next;
    }

    if (level >= 1) {
        int ready = 0, run = 0, blocked = 0, dead = 0;
        cur = process_table;
        while (cur) {
            switch (cur->state) {
                case READY:   ready++;   break;
                case RUNNING: run++;     break;
                case BLOCKED: blocked++; break;
                case DEAD:    dead++;    break;
            }
            cur = cur->next;
        }
        cout << "\nSummary: "
             << task_count() << " total | "
             << run     << " running | "
             << ready   << " ready | "
             << blocked << " blocked | "
             << dead    << " dead" << endl;
    }

    if (level >= 2) {
        if (running)
            cout << "Active: " << running->task_name
                 << " (ID: " << running->task_id << ")" << endl;
        else
            cout << "Active: (none)" << endl;
    }
}