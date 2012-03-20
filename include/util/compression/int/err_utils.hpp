/*-----------------------------------------------------------------------------
 *  err_utils.hpp - A header for err_utiils.c
 *
 *  Coding-Style:
 *      emacs) Mode: C, tab-width: 8, c-basic-offset: 8, indent-tabs-mode: nil
 *      vi) tabstop: 8, expandtab
 *
 *  Authors:
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *      Fabrizio Silvestri <fabrizio.silvestri_at_isti.cnr.it>
 *      Rossano Venturini <rossano.venturini_at_isti.cnr.it>
 *-----------------------------------------------------------------------------
 */

#ifndef ERR_UTILS_HPP
#define ERR_UTILS_HPP

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>

#define FILE_OUTPUT             0
#define CONSOLE_OUTPUT          1

#define eoutput(fmt, ...)               \
        do {                            \
                err_utils::flush_log(); \
                err_utils::err_print(__func__, __LINE__, fmt, ##__VA_ARGS__);   \
        } while (0)

/* Shortcuts for debug-prints */
#ifdef DEBUG
 #define __value(val)           \
        do {                    \
                cerr << "Value: " << #val << "=" << val <<   \
                        " (" << __func__ << ":" << __LINE__ << ")" << endl;     \
        } while (0)

 #define __array(ar, num)       \
        do {                    \
                int     n;      \
\
                n = (num == 0)? sizeof(ar) / sizeof(ar[0]) : num;       \
\
                cerr << "Array[]: " << #ar      \
                        " (" << __func__ << ":" << __LINE__ << ")" << endl;     \
                for (int i = 0; i < n; i++)             \
                        cerr << ar[i] << " ";           \
                cerr << endl;   \
        } while (0)

 #define __doutput(flag, fmt, ...)              \
        do {                                    \
                if ((flag) == FILE_OUTPUT) {    \
                        err_utils::push_log(__func__, __LINE__, fmt, ##__VA_ARGS__);    \
                } else if ((flag) == CONSOLE_OUTPUT) {                          \
                        fprintf(stderr, "%s(%d): ", __func__, __LINE__);        \
                        fprintf(stderr, fmt, ##__VA_ARGS__);                    \
                        fprintf(stderr, "\n");                                  \
                }               \
        } while (0)
#else
 #define __value(var)
 #define __array(ar, num)
 #define __doutput(flag, fmt, ...)
#endif /* DEBUG */

/* Some other utilities for debugs */
#ifdef DEBUG
 #define __validate(ptr, range) \
        do {                    \
                if (!err_utils::validate_address(ptr, range))   \
                        eoutput("Exception: A invalidated memory access");      \
        } while (0)

 #define __assert(cond)         assert(cond)
#else
 #define __validate(ptr, range)
 #define __assert(cond)
#endif /* DEBUG */

class err_utils {
        public:
                /* Debugging functions */
                static void sigsegv_handler(int sig);
                static int validate_address(void *ptr, uint32_t range);

                /* Logging functions */
                static void push_log(const char *func, int32_t line, const char *fmt, ...)
                        __attribute__ ((format (printf, 3, 4)));
                static void flush_log(void);

                /* Error functions */
                static void err_print(const char *func, int32_t line, const char *fmt, ...)
                        __attribute__ ((format (printf, 3, 4)));
};

#endif  /* ERR_UTILS_HPP */
