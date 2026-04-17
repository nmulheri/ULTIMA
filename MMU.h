#ifndef MMU_H
#define MMU_H

#include "Sema.h"
#include <iostream>

using namespace std;

class MMU {
private:
    struct MemBlock {
        bool is_free;
        int memory_handle;
        int start;
        int end;
        int size;
        int current_location;
        int task_id;
        MemBlock* next;
    };

    unsigned char* core_memory;
    int total_size;
    int block_size;
    char default_value;
    int next_handle;

    MemBlock* head;
    Semaphore mem_sem;

    int round_up_block(int size);
    MemBlock* find_block_by_handle(int memory_handle);
    MemBlock* find_block_by_handle_and_task(int memory_handle, int task_id);
    void split_block(MemBlock* block, int needed_size, int handle, int task_id);

public:
    MMU(int size = 1024, char default_initial_value = '.', int page_size = 64);
    ~MMU();

    int Mem_Alloc(int task_id, int size);
    int Mem_Free(int task_id, int memory_handle);

    int Mem_Read(int task_id, int memory_handle, char* ch);
    int Mem_Write(int task_id, int memory_handle, char ch);

    int Mem_Read(int task_id, int memory_handle, int offset_from_beg, int text_size, char* text);
    int Mem_Write(int task_id, int memory_handle, int offset_from_beg, int text_size, const char* text);

    int Mem_Left() const;
    int Mem_Largest() const;
    int Mem_Smallest() const;
    int Mem_Coalesce();
    int Mem_Dump(int starting_from = 0, int num_bytes = 1024) const;
    void List_Dump() const;
};

#endif