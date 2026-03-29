#include "Sema.h"
#include <iostream>

using namespace std;

string stateToString(State s) {
    switch (s) {
        case RUNNING: return "RUNNING";
        case READY: return "READY";
        case BLOCKED: return "BLOCKED";
        case DEAD: return "DEAD";
        default: return "UNKNOWN";
    }
}

void printTaskStates(TCB& t1, TCB& t2, TCB& t3) {
    cout << "\n--- Task States ---\n";
    cout << t1.task_name << ": " << stateToString(t1.state) << endl;
    cout << t2.task_name << ": " << stateToString(t2.state) << endl;
    cout << t3.task_name << ": " << stateToString(t3.state) << endl;
}

int main() {
    TCB t1 = {1, "Task1", READY};
    TCB t2 = {2, "Task2", READY};
    TCB t3 = {3, "Task3", READY};

    Semaphore sem("Printer", 1);

    cout << "Initial State:\n";
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nTask1 tries down():\n";
    sem.down(&t1);
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nTask2 tries down():\n";
    sem.down(&t2);
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nTask3 tries down():\n";
    sem.down(&t3);
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nCalling up():\n";
    sem.up();
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nCalling up():\n";
    sem.up();
    sem.dump();
    printTaskStates(t1, t2, t3);

    cout << "\nCalling up():\n";
    sem.up();
    sem.dump();
    printTaskStates(t1, t2, t3);

    return 0;
}