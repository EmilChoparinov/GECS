/* =========================================================================
    Author: E.D Choparinov, Amsterdam
    Related Files: logger.h
    Created On: May 06 2024
    Purpose:
        This library intended to be a stand-alone logging libary supports
        variable logging levels with pretty coloring.
========================================================================= */

#ifndef __HEADER_LOGGER_H__
#define __HEADER_LOGGER_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef __FUNCTION__
#ifdef WIN32 // WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else //*NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

/*-------------------------------------------------------
 * ENUMERATION OPTIONS AND COLOR DEFINITIONS
 *-------------------------------------------------------*/
enum { LOG_TRACE, LOG_INFO, LOG_DEBUG, LOG_WARN, LOG_ERROR };

#define GRN "\x1B[32m" /* LOG_TRACE */
#define BLU "\x1B[34m" /* LOG_INFO */
#define MAG "\x1B[35m" /* LOG_DEBUG */
#define YEL "\x1B[33m" /* LOG_WARN */
#define RED "\x1B[31m" /* LOG_ERROR */
#define RST "\x1B[0m"  /* Color reset macro. */

/*-------------------------------------------------------
 * LOGGING FUNCTIONS
 *-------------------------------------------------------*/
#define log_trace(...)                                                         \
  _cust_log(LOG_TRACE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define log_info(...)                                                          \
  _cust_log(LOG_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define log_debug(...)                                                         \
  _cust_log(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define log_warn(...)                                                          \
  _cust_log(LOG_WARN, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define log_error(...)                                                         \
  _cust_log(LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

/**
 * @brief Set the logging level. All levels below this level will be ignored
 *
 * @param LEVEL the level to set from the log enum values:
 *              LOG_TRACE, LOG_INFO, LOG_DEBUG, LOG_WARN, LOG_ERROR
 */
void log_set_level(int LEVEL);

/*-------------------------------------------------------
 * PRIVATE FUNCTIONS
 *-------------------------------------------------------*/
void _cust_log(int LEVEL, const char *FILE, const char *FUNC, int LINE_NUM,
               const char *FORMAT, ...);

#endif