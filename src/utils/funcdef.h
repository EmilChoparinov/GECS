/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: funcdef.h
    Created On: June 01 2024
    Purpose:
        The purpose of this file is to generate support macros for
        container generation. Only typename utilities
========================================================================= */
#ifndef __HEADER_FUNCDEF_H__
#define __HEADER_FUNCDEF_H__

/*-------------------------------------------------------
 * Atomic Function Types
 *-------------------------------------------------------*/
#define pred_f(T)          T##_predicate
#define unary_f(T)         T##_unary
#define binary_f(T)        T##_binary
#define compare_f(T)       T##_compare
#define ret(T)             T
#define wrap_type(T, with) wrap_##with##_##T

/*-------------------------------------------------------
 * Function Reference Macros
 *-------------------------------------------------------*/
#define fname(T, name)              T##_##name
#define fdecl(ret_T, T, name, body) ret(ret_T) fname(T, name) body

/*-------------------------------------------------------
 * Utilities
 *-------------------------------------------------------*/
#define EXPAND(x)    x
#define CONCAT(a, b) a##b
#define UNIQUE(x)    CONCAT(x, __LINE__)

#endif