/*==============================================================================
File:
    lib/core/arena.h
Purpose:
    Arena alloc structure provider. Allows for the collections of given objects
    to share a lifetime.
==============================================================================*/
#ifndef __HEADER_ARENA_H__
#define __HEADER_ARENA_H__

#define REGION_SIZE 512
#define TABLE_INIT_SIZE 32

#include "vector.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct arena arena;

/**
 * @brief Create a new arena on the heap.
 *
 * @return arena* The newly created arena that may be passed to the allocator.
 */
arena *arena_make(void);

/**
 * @brief Destroy/Free the arena. Will cleanup all memory being used in the
 * heap.
 *
 * @param arena The arena to destroy
 */
void arena_destroy(arena *arena);

/**
 * @brief Allocate `size` bytes out of `arena` and return a pointer to the
 * location.
 *
 * @param arena The arena to reserve bytes out of
 * @param bytes The amount of bytes to reserve
 * @return void* Pointer to the address of where those bytes were reserved.
 */
void *arena_alloc(arena *arena, size_t bytes);

/**
 * @brief Poll the remaining contiguous bytes left in the current region. Use
 * `arena_refresh` if an empty region is to be made.
 * 
 * @param arena 
 * @return int 
 */
int arena_poll(arena *arena);

/**
 * @brief Refresh the current region to have maximum space available.
 * 
 */
void arena_refresh(arena *a);

#endif