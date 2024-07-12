/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: str_utils.h str_utils.c
    Created On: June 18 2024
    Purpose:
        The purpose of this file is to house a bunch of string manipulation
        routines I needed to store somewhere for the library, so here they
        went.
========================================================================= */
#ifndef _HEADER_STR_UTILS_H__
#define _HEADER_STR_UTILS_H__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "csdsa.h"

uint64_t hash_vector(vec *v);

#endif