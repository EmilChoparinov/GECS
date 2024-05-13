/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: gecs_utils.h gec_utils.c
    Created On: April 23 2024
    Purpose:
        This file contains immutable functions that are generally helpful
        for the framework.
========================================================================= */

#ifndef __HEADER_GECS_UTILS_H__
#define __HEADER_GECS_UTILS_H__

/*------------------------------------------------------
 * Text Processing Functions
 *-------------------------------------------------------
 * These functions are intended to be called with the strings being null
 * terminated. Make sure that all strings have at least one null byte before
 * calling */
const unsigned long hash(const char *str);

#endif