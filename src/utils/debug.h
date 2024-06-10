#ifndef __HEADER_DEBUG_H__
#define __HEADER_DEBUG_H__

#include <signal.h>

#define BREAK() raise(SIGTRAP)

#endif