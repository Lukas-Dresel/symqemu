/*
 *  x86 integer helpers
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"
#include "qapi/error.h"
#include "qemu/guest-random.h"

#define SymExpr void*
#include "RuntimeCommon.h"

#include "int_helper_div128_utils.h"

#define DIVISION_NBITS 8
#define DIVISION_SIGNED 0
#define DIVISION_SUFFIX _8_unsigned
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 8
#define DIVISION_SIGNED 1
#define DIVISION_SUFFIX _8_signed
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 16
#define DIVISION_SIGNED 0
#define DIVISION_SUFFIX _16_unsigned
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 16
#define DIVISION_SIGNED 1
#define DIVISION_SUFFIX _16_signed
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 32
#define DIVISION_SIGNED 0
#define DIVISION_SUFFIX _32_unsigned
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 32
#define DIVISION_SIGNED 1
#define DIVISION_SUFFIX _32_signed
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 64
#define DIVISION_SIGNED 0
#define DIVISION_SUFFIX _64_unsigned
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX

#define DIVISION_NBITS 64
#define DIVISION_SIGNED 1
#define DIVISION_SUFFIX _64_signed
#include "int_helper_sym_div.h"
#undef DIVISION_NBITS
#undef DIVISION_SIGNED
#undef DIVISION_SUFFIX