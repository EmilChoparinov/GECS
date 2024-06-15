/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: arena.h arena.c
    Created On: May 31 2024
    Purpose:
        Stand-alone allocation provider. Allows for a collection of
        objects to share a lifetime
========================================================================= */

#ifndef __HEADER_ARENA_H__
#define __HEADER_ARENA_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

/*-------------------------------------------------------
 * ARENA CONFIGURATIONS
 *-------------------------------------------------------*/

/* Note: Allocation requests of greater than REGION_SIZE will be given a
         specialized chunk of exactly the requested size. */
#define REGION_SIZE 4096 /* The total memory allocated per chunk. */

typedef struct arena_t arena_t;

/*-------------------------------------------------------
 * CONTAINER OPERATIONS
 *-------------------------------------------------------*/
arena_t *arena_init(void);
void     arena_free(arena_t *ar);

/*-------------------------------------------------------
 * ELEMENT OPERATIONS
 *-------------------------------------------------------*/
void  *arena_alloc(arena_t *ar, size_t bytes);
void  *arena_realloc(arena_t *ar, void *ptr, size_t bytes);
void   arena_push_region(arena_t *ar, size_t chunk_size);
size_t arena_poll(arena_t *ar);

#endif