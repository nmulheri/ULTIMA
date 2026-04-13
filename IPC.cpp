
#include <iostream>
#include <queue>
#include <vector>
#include <cstring>
#include <ctime>

using namespace std;

class ipc {
public:

    /*
    Message_Type structure
    Stores the type of message and it's description.
    Examples:
    0 = Text Message
    1 = Service Request
    2 = Notification
    */
    struct Message_Type {
        int message_type_id;
        char message_type_description[64];

        Message_Type() {
            message_type_id = 0;
            strcpy(message_type_description, "Text Message");
        }

        Message_Type(int id, const char *description) {
            message_type_id = id;
            strncpy(message_type_description, description, sizeof(message_type_description) - 1);
            message_type_description[sizeof(message_type_description) - 1] = '\0';
        }
    };

    /*
    Message structure
    Stores all important message data.
    */
    struct Message {
        int source_task_id;
        int destination_task_id;
        time_t message_arrival_time;
        Message_Type msg_type;
        int msg_size;
        char msg_text[33]; // 32 bytes + null terminator

        Message() {
            source_task_id = -1;
            destination_task_id = -1;
            message_arrival_time = time(nullptr);
            msg_type = Message_Type();
            msg_size = 0;
            msg_text[0] = '\0';
        }
    };

    /*
    Mailbox structure
    Each task has one mailbox.
    Each mailbox stores a queue of messages.
    */
    struct Mailbox {
        int task_id;
        queue<Message> messages;

        Mailbox() {
            task_id = -1;
        }

        Mailbox(int id) {
            task_id = id;
        }
    };

private:
    int max_tasks;
    vector<Mailbox> task_mailboxes;

public:
    // Constructor
    ipc(int max_tasks);

    // Helper functions
    int get_max_tasks() const;
    bool is_valid_task(int task_id) const;
    Mailbox* get_mailbox(int task_id);
    const Mailbox* get_mailbox(int task_id) const;

    // Output functions for setup testing
    void print_mailbox_setup() const;
};

/*
Constructor
Create one mailbox for each active task.
If max_tasks is invalid then max_tasks will be set to zero.
*/
ipc::ipc(int max_tasks) {
    if (max_tasks <= 0) {
        this->max_tasks = 0;
        return;
    }

    this->max_tasks = max_tasks;

    // Task IDs are made from 1 up to max_tasks
    for (int i = 1; i <= max_tasks; i++) {
        task_mailboxes.push_back(Mailbox(i));
    }
}

/*
Returns the number of active tasks.
*/
int ipc::get_max_tasks() const {
    return max_tasks;
}

/*
Checks if a task ID is valid.
*/
bool ipc::is_valid_task(int task_id) const {
    return task_id >= 1 && task_id <= max_tasks;
}

/*
Returns a pointer to the mailbox for a task ID.
Return nullptr if task ID is invalid.
*/
ipc::Mailbox* ipc::get_mailbox(int task_id) {
    if (!is_valid_task(task_id)) {
        return nullptr;
    }

    for (auto& mailbox : task_mailboxes) {
        if (mailbox.task_id == task_id) {
            return &mailbox;
        }
    }

    return nullptr;
}

/*
Const version of get_mailbox()
*/
const ipc::Mailbox* ipc::get_mailbox(int task_id) const {
    if (!is_valid_task(task_id)) {
        return nullptr;
    }

    for (const auto& mailbox : task_mailboxes) {
        if (mailbox.task_id == task_id) {
            return &mailbox;
        }
    }

    return nullptr;
}

/*
Prints mailbox setup information.
Used to check whether or not all task mailboxes have been created properly.
*/
void ipc::print_mailbox_setup() const {
    cout << "========================================" << endl;
    cout << "IPC Mailbox Setup" << endl;
    cout << "========================================" << endl;
    cout << "Number of Active Tasks: " << max_tasks << endl << endl;

    for (const auto& mailbox : task_mailboxes) {
        cout << "Task ID: " << mailbox.task_id << endl;
        cout << "Created Mailbox Successfully." << endl;
        cout << "Initial Number of Messages in Queue: " << mailbox.messages.size() << endl;
        cout << "----------------------------------------" << endl;
    }
}

/*
Basic setup test for Person 1
This test verifies the following:
- IPC constructor
- Creation of mailboxes
- Accessing mailboxes
- Validating tasks
*/
int main() {
    ipc messenger(4);

    messenger.print_mailbox_setup();

    cout << endl;
    cout << "========================================" << endl;
    cout << "Testing Access to Mailboxes" << endl;
    cout << "========================================" << endl;

    for (int i = 1; i <= messenger.get_max_tasks(); ++i) {
        ipc::Mailbox* mailbox = messenger.get_mailbox(i);

        if (mailbox != NULL) {
            cout << "Mailbox Located For Task Number " << mailbox->task_id << endl;
        } else {
            cout << "Mailbox Not Found For Task # " << i << endl;
        }
    }

    cout << endl;
    cout << "========================================" << endl;
    cout << "Test of Validity Of Task Numbers" << endl;
    cout << "========================================" << endl;

    int test_ids[] = {-1, 1, 2, 3, 4, 5};

    for (int id : test_ids) {
        if (messenger.is_valid_task(id)) {
            cout << "Valid Task Number " << id << endl;
        } else {
            cout << "Invalid Task Number " << id << endl;
        }
    }

    cout << endl;
cout << "========================================" << endl;
cout << "Testing Invalid Mailbox Access" << endl;
cout << "========================================" << endl;

int invalid_mailbox_ids[] = {0, -1, 5, 10};

for (int id : invalid_mailbox_ids) {
    ipc::Mailbox* mailbox = messenger.get_mailbox(id);

    if (mailbox == NULL) {
        cout << "Correct: No mailbox for task " << id << endl;
    } else {
        cout << "Error: Found mailbox for invalid task " << id << endl;
    }
}
    return 0;
}
