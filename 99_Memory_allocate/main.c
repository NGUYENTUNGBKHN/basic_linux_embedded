/******************************************************************************/
/*! @addtogroup Group2
    @file       main.c
    @brief      
    @date       2025/07/23
    @author     Development Dept at Tokyo (nguyen-thanh-tung@jcm-hq.co.jp)
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#include <stdio.h>
#include <string.h>

#define ALIGN_TYPE int
#define ALIGN_SIZE sizeof(ALIGN_TYPE)

char test_space[50];

typedef struct mm_pool_s
{
    int mm_pool_id;
    char *mm_pool_list;     // Points to the next available free block
    char *mm_pool_search;   // Not explicitly used in the current allocation, could be for searching fragmented blocks
    char *mm_pool_start;    // Beginning of the memory pool
    int mm_pool_size;       // Total size of the memory pool
    struct mm_pool_s *next;
    struct mm_pool_s *prev;
}mm_pool_t, *mm_pool_p;

mm_pool_p main_pool;

mm_pool_p _mm_head = NULL; // Initialize to NULL for safety
int _mm_byte_count = 0;

int mm_pool_allocate(mm_pool_p pool_ptr, void **mm_ptr, int mm_size, int wait_op)
{
    // Basic error checking
    if (pool_ptr == NULL || mm_ptr == NULL || mm_size <= 0)
    {
        return -1; // Indicate an error
    }

    // Align the requested size to ALIGN_TYPE
    // This ensures that allocated memory blocks start at an aligned address.
    int aligned_mm_size = (mm_size + ALIGN_SIZE - 1) / ALIGN_SIZE * ALIGN_SIZE;

    // Check if the pool itself is valid
    if (pool_ptr->mm_pool_start == NULL || pool_ptr->mm_pool_size <= 0)
    {
        return -1; // Indicate an error if pool is not properly initialized
    }

    // Check for available space
    // This is a very simple "first-fit" approach, assuming mm_pool_list always
    // points to the next free block. It doesn't handle fragmentation.
    if ((pool_ptr->mm_pool_list - pool_ptr->mm_pool_start) + aligned_mm_size > pool_ptr->mm_pool_size)
    {
        // Not enough contiguous space available from the current list pointer.
        // In a more sophisticated allocator, you would search for a free block.
        *mm_ptr = NULL; // Indicate failure to allocate
        return -2;      // Indicate no memory available
    }

    // Allocate the memory
    *mm_ptr = (void *)pool_ptr->mm_pool_list;
    pool_ptr->mm_pool_list += aligned_mm_size; // Move the list pointer to the next available spot

    return 0; // Success
}

void mm_pool_create(mm_pool_p pool_ptr, int mm_size, void *start_address)
{
    mm_pool_p next;
    mm_pool_p prev;

    // Correction: start_address should NOT be NULL for a valid pool creation.
    // If start_address is NULL, you can't allocate from it.
    if (NULL == start_address || pool_ptr == NULL || mm_size <= 0)
    {
        return; // Invalid input, do not proceed
    }

    /* Initialize the byte pool control block to all zeros. */
    memset(pool_ptr, 0, (sizeof(mm_pool_t)));
    
    // Ensure the pool size is a multiple of ALIGN_TYPE for proper alignment
    mm_size = (mm_size / ALIGN_SIZE) * ALIGN_SIZE;
    if (mm_size == 0) { // If original mm_size was less than ALIGN_SIZE
        return;
    }
    
    pool_ptr->mm_pool_start = start_address;
    pool_ptr->mm_pool_size = mm_size;

    // Initially, the entire pool is available, so mm_pool_list points to the start
    pool_ptr->mm_pool_list = pool_ptr->mm_pool_start;
    pool_ptr->mm_pool_search = pool_ptr->mm_pool_start; // Initialize search pointer as well

    if (_mm_byte_count == 0)
    {
        _mm_head = pool_ptr;
        // For a single pool, it points to itself for circular list (next and prev)
        _mm_head->next = _mm_head;
        _mm_head->prev = _mm_head;
    }
    else
    {
        // This logic assumes a circular doubly linked list of pools.
        // It inserts the new pool_ptr into the list.
        next = _mm_head;         // The new pool's next is the current head
        prev = _mm_head->prev;   // The new pool's prev is the current head's prev

        pool_ptr->next = next;
        pool_ptr->prev = prev;
        
        next->prev = pool_ptr;  // Update the old head's prev to the new pool
        prev->next = pool_ptr;  // Update the old head's prev's next to the new pool
    }
    _mm_byte_count ++;
}

void mm_pool_free()
{
    // This function is currently empty.
    // In a real memory allocator, this would involve marking a freed block
    // as available, potentially coalescing adjacent free blocks.
}

int main()
{
    mm_pool_t my_pool_block; // The control block for our pool
    
    // Create a memory pool using test_space
    // The size of the pool is sizeof(test_space)
    mm_pool_create(&my_pool_block, sizeof(test_space), test_space);

    main_pool = &my_pool_block; // Assign to main_pool if you intend to use it globally

    void *ptr1 = NULL;
    void *ptr2 = NULL;
    void *ptr3 = NULL;

    printf("Initial mm_pool_list offset: %ld\n", (long)(main_pool->mm_pool_list - main_pool->mm_pool_start));

    // Allocate some memory
    int status1 = mm_pool_allocate(main_pool, &ptr1, 10, 0); // Allocate 10 bytes
    if (status1 == 0 && ptr1 != NULL) {
        printf("Allocated 10 bytes at offset: %ld\n", (long)((char*)ptr1 - main_pool->mm_pool_start));
        printf("New mm_pool_list offset: %ld\n", (long)(main_pool->mm_pool_list - main_pool->mm_pool_start));
        memset(ptr1, 'A', 10); // Use the allocated memory
    } else {
        printf("Failed to allocate 10 bytes. Status: %d\n", status1);
    }

    int status2 = mm_pool_allocate(main_pool, &ptr2, 25, 0); // Allocate 25 bytes
    if (status2 == 0 && ptr2 != NULL) {
        printf("Allocated 25 bytes at offset: %ld\n", (long)((char*)ptr2 - main_pool->mm_pool_start));
        printf("New mm_pool_list offset: %ld\n", (long)(main_pool->mm_pool_list - main_pool->mm_pool_start));
        memset(ptr2, 'B', 25);
    } else {
        printf("Failed to allocate 25 bytes. Status: %d\n", status2);
    }

    int status3 = mm_pool_allocate(main_pool, &ptr3, 20, 0); // Try to allocate 20 bytes (should fail)
    if (status3 == 0 && ptr3 != NULL) {
        printf("Allocated 20 bytes at offset: %ld\n", (long)((char*)ptr3 - main_pool->mm_pool_start));
        printf("New mm_pool_list offset: %ld\n", (long)(main_pool->mm_pool_list - main_pool->mm_pool_start));
    } else {
        printf("Failed to allocate 20 bytes (expected). Status: %d\n", status3);
    }

    printf("\nContents of test_space after allocations:\n");
    for (int i = 0; i < sizeof(test_space); ++i) {
        if (test_space[i] >= ' ' && test_space[i] <= '~') { // Printable characters
            printf("%c ", test_space[i]);
        } else {
            printf(". "); // Non-printable characters
        }
        if ((i + 1) % 10 == 0) {
            printf("\n");
        }
    }
    printf("\n");


    // You would typically call mm_pool_free() here for ptr1, ptr2, etc.,
    // but that function is not yet implemented.

    return 0;
}