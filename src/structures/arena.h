/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: arena.h arena.c
    Created On: April 11 2024
    Purpose:
        Stand-alone allocation provider. Allows for a collection of
        objects to share a lifetime.
========================================================================= */

#ifndef __HEADER_ARENA_H__
#define __HEADER_ARENA_H__

/*-------------------------------------------------------
 * ARENA CONFIGURATION
 *-------------------------------------------------------
 * There are two macros that can be edited as desired by
 * the developer:
 *     REGION_SIZE: The total memory that should be 
 *                  allocated for a chunk
 *      TABLE_INIT_SIZE: The */
#define REGION_SIZE 4096

/*-------------------------------------------------------
 * RETURN CODES
 *-------------------------------------------------------
 * These return codes should not EVER be touched and 
 * have */
#define ARENA_OK 0
#define ARENA_FAIL -1

#include "short_vector.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-------------------------------------------------------
 * FUNCTIONS
 *-------------------------------------------------------*/

/* Hidden arena structure. Check arena.c for details. */
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