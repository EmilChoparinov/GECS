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
#define pred_f(T)   T##_predicate
#define unary_f(T)  T##_unary
#define binary_f(T) T##_binary
#define ret(T)      T


/*-------------------------------------------------------
 * Function Reference Macros
 *-------------------------------------------------------*/
#define fname(T, name)              T##_##name
#define fdecl(ret_T, T, name, body) ret(ret_T) fname(T, name) body

/*-------------------------------------------------------
 * Utilities
 *-------------------------------------------------------*/
#define EXPAND(x) x

#endif