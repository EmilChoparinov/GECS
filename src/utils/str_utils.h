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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Function that takes a string < 256 bytes long and chunks it into a 256 byte
   segment. Throws an assertion if given a string longer than 256. The remainder
   of the string is 0'd out. */
char *fix_str_256(char * str);

#endif