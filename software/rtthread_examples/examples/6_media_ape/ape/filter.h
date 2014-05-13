/*

libdemac - A Monkey's Audio decoder

$Id: filter.h 25005 2010-03-03 21:20:13Z amiconn $

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

#ifndef _APE_FILTER_H
#define _APE_FILTER_H

#include "demac_config.h"

void init_filter_16_11(filter_int* buf);
void apply_filter_16_11(int fileversion, int channel,
                        int32_t* decoded, int count);

void init_filter_64_11(filter_int* buf);
void apply_filter_64_11(int fileversion, int channel,
                        int32_t* decoded, int count);

void init_filter_32_10(filter_int* buf);
void apply_filter_32_10(int fileversion, int channel,
                        int32_t* decoded, int count);

void init_filter_256_13(filter_int* buf);
void apply_filter_256_13(int fileversion, int channel,
                         int32_t* decoded, int count);

void init_filter_1280_15(filter_int* buf);
void apply_filter_1280_15(int fileversion, int channel,
                          int32_t* decoded, int count);

#endif
