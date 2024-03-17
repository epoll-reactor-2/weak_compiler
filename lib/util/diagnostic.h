/* diagnostic.h - Diagnostics engine.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef FCC_UTIL_DIAGNOSTICS_H
#define FCC_UTIL_DIAGNOSTICS_H

#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdnoreturn.h>
#include <stdio.h>

struct diag_config {
    bool ignore_warns;
    bool show_location;
};

/** Jump buffer for handling errors. As warning/errors emit
    functions, can be used in normal "real-world" compiler mode
    as well as in testing mode.
   
    \note Driver code should do similar thing
   
    \code{.c}
    if (!setjmp(fatal_error_buf)) {
        // Normal code.
    } else {
        // Fallback on error inside normal code.
    }
    \endcode */
extern jmp_buf fcc_fatal_error_buf;

/** \defgroup fcc_diagnostic_streams
   
    Streams being FILE * used for debug purposes. There are
    two cases:
    - streams are NULL, then all compiler outputs written to the screen and program
      terminates on compile error;
    - streams are set, then all compiler outputs written to it.
   
    \code{c}
    void *diag_error_memstream;
    void *diag_warn_memstream;
    \endcode */
extern void *diag_error_memstream;
extern void *diag_warn_memstream;

/** Override default config.

    Default config has:
    - ignore_warns  = 1
    - show_location = 0
 */
void fcc_diag_set_config(struct diag_config *new_config);

/** \brief Set source code location being analyzed. Used to display
           warns and errors. */
void fcc_set_source_filename(const char *filename);
void fcc_set_source_stream(FILE *stream);

/** \brief Emit compile error according to \ref fcc_diagnostic_streams rule
           and go out from executor function of any depth. */
noreturn
void fcc_compile_error(uint16_t line_no, uint16_t col_no, const char *fmt, ...);
/** \brief Emit compile warning according to \ref fcc_diagnostic_streams rule. */
void fcc_compile_warn (uint16_t line_no, uint16_t col_no, const char *fmt, ...);

#endif // FCC_UTIL_DIAGNOSTICS_H