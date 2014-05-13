/*

libdemac - A Monkey's Audio decoder

$Id: demac_config.h 29239 2011-02-06 23:18:30Z amiconn $

Copyright (C) Dave Chapman 2007

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA

*/

#ifndef _DEMAC_CONFIG_H
#define _DEMAC_CONFIG_H

/* Build-time choices for libdemac.
 * Note that this file is included by both .c and .S files. */

#ifdef ROCKBOX

#include "config.h"

#ifndef __ASSEMBLER__
#include "codeclib.h"
#include <codecs.h>
#endif

#define APE_OUTPUT_DEPTH 29

/* On ARMv4, using 32 bit ints for the filters is faster. */
#if defined(CPU_ARM) && (ARM_ARCH == 4)
#define FILTER_BITS 32
#endif

#if !defined(CPU_PP) && !defined(CPU_S5L870X)
#define FILTER256_IRAM
#endif

#if CONFIG_CPU == PP5002 || defined(CPU_S5L870X)
/* Code and data IRAM for speed (PP5002 has a broken cache), not enough IRAM
 * for the insane filter buffer. Reciprocal table for division in IRAM. */
#define ICODE_SECTION_DEMAC_ARM   .icode
#define ICODE_ATTR_DEMAC          ICODE_ATTR
#define ICONST_ATTR_DEMAC         ICONST_ATTR
#define IBSS_ATTR_DEMAC           IBSS_ATTR
#define IBSS_ATTR_DEMAC_INSANEBUF

#elif CONFIG_CPU == PP5020
/* Code and small data in DRAM for speed (PP5020 IRAM isn't completely single
 * cycle). Insane filter buffer not in IRAM in favour of reciprocal table for
 * divison. Decoded data buffers should be in IRAM (defined by the caller). */
#define ICODE_SECTION_DEMAC_ARM   .text
#define ICODE_ATTR_DEMAC
#define ICONST_ATTR_DEMAC
#define IBSS_ATTR_DEMAC
#define IBSS_ATTR_DEMAC_INSANEBUF

#elif CONFIG_CPU == PP5022
/* Code in DRAM, data in IRAM. Insane filter buffer not in IRAM in favour of
 * reciprocal table for divison */
#define ICODE_SECTION_DEMAC_ARM   .text
#define ICODE_ATTR_DEMAC
#define ICONST_ATTR_DEMAC         ICONST_ATTR
#define IBSS_ATTR_DEMAC           IBSS_ATTR
#define IBSS_ATTR_DEMAC_INSANEBUF

#else
/* Code in DRAM, data in IRAM, including insane filter buffer. */
#define ICODE_SECTION_DEMAC_ARM   .text
#define ICODE_ATTR_DEMAC
#define ICONST_ATTR_DEMAC         ICONST_ATTR
#define IBSS_ATTR_DEMAC           IBSS_ATTR
#define IBSS_ATTR_DEMAC_INSANEBUF IBSS_ATTR
#endif

#else /* !ROCKBOX */

#define APE_OUTPUT_DEPTH (ape_ctx->bps)

#define MEM_ALIGN_ATTR __attribute__((aligned(16)))
        /* adjust to target architecture for best performance */

#define ICODE_ATTR_DEMAC
#define ICONST_ATTR_DEMAC
#define IBSS_ATTR_DEMAC
#define IBSS_ATTR_DEMAC_INSANEBUF

/* Use to give gcc hints on which branch is most likely taken */
#if defined(__GNUC__) && __GNUC__ >= 3
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#endif /* !ROCKBOX */

/* Defaults */

#ifndef FILTER_HISTORY_SIZE
#define FILTER_HISTORY_SIZE 512
#endif

#ifndef PREDICTOR_HISTORY_SIZE
#define PREDICTOR_HISTORY_SIZE 512
#endif     

#ifndef FILTER_BITS
#define FILTER_BITS 16
#endif


#ifndef __ASSEMBLER__

//#if defined(CPU_ARM) && (ARM_ARCH < 5 || defined(USE_IRAM))
///* optimised unsigned integer division for ARMv4, in IRAM */
//unsigned udiv32_arm(unsigned a, unsigned b);
//#define UDIV32(a, b) udiv32_arm(a, b)
//#else
///* default */
#define UDIV32(a, b) (a / b)
//#endif

//unsigned udiv32_arm(unsigned a, unsigned b);
//#define UDIV32(a, b) udiv32_arm(a, b)

#include <inttypes.h>
#if FILTER_BITS == 32
typedef int32_t filter_int;
#elif FILTER_BITS == 16
typedef int16_t filter_int;
#endif
#endif

#endif /* _DEMAC_CONFIG_H */
