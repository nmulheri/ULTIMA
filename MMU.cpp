#include "MMU.h"
#include <cstring>
#include <iomanip>

MMU::MMU(int size, char default_initial_value, int page_size)
    : total_size(size),
      block_size(page_size),
      default_value(default_initial_value),
      next_handle(1),
      head(nullptr),
      mem_sem("Core Memory", 1) {

    core_memory = new unsigned char[total_size];

    for (int i = 0; i < total_size; i++) {
        core_memory[i] = default_value;
    }

    head = new MemBlock;
    head->is_free = true;
    head->memory_handle = 0;
    head->start = 0;
    head->end = total_size - 1;
    head->size = total_size;
    head->current_location = 0;
    head->task_id = -1;
    head->next = nullptr;
}

MMU::~MMU() {
    delete[] core_memory;

    MemBlock* current = head;
    while (current != nullptr) {
        MemBlock* temp = current;
        current = current->next;
        delete temp;
    }
}

int MMU::round_up_block(int size) {
    if (size <= 0) {
        return 0;
    }

    if (size % block_size == 0) {
        return size;
    }

    return ((size / block_size) + 1) * block_size;
}

MMU::MemBlock* MMU::find_block_by_handle(int memory_handle) {
    MemBlock* current = head;

    while (current != nullptr) {
        if (!current->is_free && current->memory_handle == memory_handle) {
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

MMU::MemBlock* MMU::find_block_by_handle_and_task(int memory_handle, int task_id) {
    MemBlock* current = head;

    while (current != nullptr) {
        if (!current->is_free &&
            current->memory_handle == memory_handle &&
            current->task_id == task_id) {
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

void MMU::split_block(MemBlock* block, int needed_size, int handle, int task_id) {
    if (block->size == needed_size) {
        block->is_free = false;
        block->memory_handle = handle;
        block->task_id = task_id;
        block->current_location = block->start;
        return;
    }

    MemBlock* new_free_block = new MemBlock;
    new_free_block->is_free = true;
    new_free_block->memory_handle = 0;
    new_free_block->start = block->start + needed_size;
    new_free_block->end = block->end;
    new_free_block->size = block->size - needed_size;
    new_free_block->current_location = 0;
    new_free_block->task_id = -1;
    new_free_block->next = block->next;

    block->is_free = false;
    block->memory_handle = handle;
    block->end = block->start + needed_size - 1;
    block->size = needed_size;
    block->current_location = block->start;
    block->task_id = task_id;
    block->next = new_free_block;
}

int MMU::Mem_Alloc(int task_id, int size) {
    if (size <= 0) {
        return -1;
    }

    int needed_size = round_up_block(size);

    TCB temp_task;
    temp_task.task_id = task_id;
    temp_task.task_name = "Task" + to_string(task_id);
    temp_task.state = READY;
    temp_task.next = nullptr;
    temp_task.memory_handle = -1;

    mem_sem.down(&temp_task);

    MemBlock* current = head;
    while (current != nullptr) {
        if (current->is_free && current->size >= needed_size) {
            int handle = next_handle++;
            split_block(current, needed_size, handle, task_id);
            mem_sem.up();
            return handle;
        }
        current = current->next;
    }

    mem_sem.up();
    return -1;
}

int MMU::Mem_Free(int task_id, int memory_handle) {
    TCB temp_task;
    temp_task.task_id = task_id;
    temp_task.task_name = "Task" + to_string(task_id);
    temp_task.state = READY;
    temp_task.next = nullptr;
    temp_task.memory_handle = -1;

    mem_sem.down(&temp_task);

    MemBlock* block = find_block_by_handle_and_task(memory_handle, task_id);

    if (block == nullptr) {
        mem_sem.up();
        return -1;
    }

    for (int i = block->start; i <= block->end; i++) {
        core_memory[i] = '#';
    }

    block->is_free = true;
    block->memory_handle = 0;
    block->task_id = -1;
    block->current_location = 0;

    Mem_Coalesce();
    mem_sem.up();
    return 1;
}

int MMU::Mem_Read(int task_id, int memory_handle, char* ch) {
    MemBlock* block = find_block_by_handle_and_task(memory_handle, task_id);

    if (block == nullptr || ch == nullptr) {
        return -1;
    }

    if (block->current_location > block->end) {
        return -1;
    }

    *ch = core_memory[block->current_location];
    block->current_location++;

    return 1;
}

int MMU::Mem_Write(int task_id, int memory_handle, char ch) {
    MemBlock* block = find_block_by_handle_and_task(memory_handle, task_id);

    if (block == nullptr) {
        return -1;
    }

    if (block->current_location > block->end) {
        return -1;
    }

    core_memory[block->current_location] = ch;
    block->current_location++;

    return 1;
}

int MMU::Mem_Read(int task_id, int memory_handle, int offset_from_beg, int text_size, char* text) {
    MemBlock* block = find_block_by_handle_and_task(memory_handle, task_id);

    if (block == nullptr || text == nullptr || text_size <= 0) {
        return -1;
    }

    int start_pos = block->start + offset_from_beg;
    int end_pos = start_pos + text_size - 1;

    if (start_pos < block->start || end_pos > block->end) {
        return -1;
    }

    for (int i = 0; i < text_size; i++) {
        text[i] = core_memory[start_pos + i];
    }

    text[text_size] = '\0';
    return 1;
}

int MMU::Mem_Write(int task_id, int memory_handle, int offset_from_beg, int text_size, const char* text) {
    MemBlock* block = find_block_by_handle_and_task(memory_handle, task_id);

    if (block == nullptr || text == nullptr || text_size <= 0) {
        return -1;
    }

    int start_pos = block->start + offset_from_beg;
    int end_pos = start_pos + text_size - 1;

    if (start_pos < block->start || end_pos > block->end) {
        return -1;
    }

    for (int i = 0; i < text_size; i++) {
        core_memory[start_pos + i] = text[i];
    }

    return 1;
}

int MMU::Mem_Left() const {
    int total_free = 0;
    MemBlock* current = head;

    while (current != nullptr) {
        if (current->is_free) {
            total_free += current->size;
        }
        current = current->next;
    }

    return total_free;
}

int MMU::Mem_Largest() const {
    int largest = 0;
    MemBlock* current = head;

    while (current != nullptr) {
        if (current->is_free && current->size > largest) {
            largest = current->size;
        }
        current = current->next;
    }

    return largest;
}

int MMU::Mem_Smallest() const {
    int smallest = total_size + 1;
    bool found_free = false;
    MemBlock* current = head;

    while (current != nullptr) {
        if (current->is_free) {
            found_free = true;
            if (current->size < smallest) {
                smallest = current->size;
            }
        }
        current = current->next;
    }

    if (!found_free) {
        return 0;
    }

    return smallest;
}

int MMU::Mem_Coalesce() {
    int count = 0;
    MemBlock* current = head;

    while (current != nullptr && current->next != nullptr) {
        if (current->is_free && current->next->is_free) {
            MemBlock* next_block = current->next;

            current->end = next_block->end;
            current->size = current->size + next_block->size;
            current->next = next_block->next;

            for (int i = current->start; i <= current->end; i++) {
                core_memory[i] = '.';
            }

            delete next_block;
            count++;
        } else {
            current = current->next;
        }
    }

    return count;
}

int MMU::Mem_Dump(int starting_from, int num_bytes) const {
    if (starting_from < 0 || starting_from >= total_size || num_bytes <= 0) {
        return -1;
    }

    int end = starting_from + num_bytes;
    if (end > total_size) {
        end = total_size;
    }

    cout << "\n--- Memory CORE Dump ---" << endl;

    for (int i = starting_from; i < end; i++) {
        cout << core_memory[i];
        if ((i + 1) % 64 == 0) {
            cout << endl;
        }
    }

    cout << endl;
    return 1;
}

void MMU::List_Dump() const {
    cout << "\n--- Memory Usage (Linked List) ---" << endl;
    cout << left
         << setw(10) << "Status"
         << setw(15) << "Handle"
         << setw(15) << "Start"
         << setw(15) << "End"
         << setw(15) << "Size"
         << setw(18) << "Current Loc"
         << setw(10) << "Task-ID" << endl;

    MemBlock* current = head;

    while (current != nullptr) {
        cout << left
             << setw(10) << (current->is_free ? "Free" : "Used")
             << setw(15) << current->memory_handle
             << setw(15) << current->start
             << setw(15) << current->end
             << setw(15) << current->size;

        if (current->is_free) {
            cout << setw(18) << "NA";
            cout << setw(10) << "MMU";
        } else {
            cout << setw(18) << current->current_location;
            cout << setw(10) << current->task_id;
        }

        cout << endl;
        current = current->next;
    }
}