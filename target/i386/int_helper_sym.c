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

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/exec-all.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"
#include "qapi/error.h"
#include "qemu/guest-random.h"

#define SymExpr void*
#include "RuntimeCommon.h"

//#define DEBUG_MULDIV

/* modulo 9 table */
// static const uint8_t rclb_table[32] = {
//     0, 1, 2, 3, 4, 5, 6, 7,
//     8, 0, 1, 2, 3, 4, 5, 6,
//     7, 8, 0, 1, 2, 3, 4, 5,
//     6, 7, 8, 0, 1, 2, 3, 4,
// };

// /* modulo 17 table */
// static const uint8_t rclw_table[32] = {
//     0, 1, 2, 3, 4, 5, 6, 7,
//     8, 9, 10, 11, 12, 13, 14, 15,
//     16, 0, 1, 2, 3, 4, 5, 6,
//     7, 8, 9, 10, 11, 12, 13, 14,
// };

/* division, flags are undefined */

void* HELPER(sym_div)(target_ulong nbits, target_ulong is_signed,
                     target_ulong numerator, void *num_expr,
                     target_ulong denominator, void *denom_expr)
{
    assert((nbits == 8) || (nbits == 16) || (nbits == 32) || (nbits == 64));
    if (num_expr == NULL && denom_expr == NULL) {
        return NULL;
    }
    if (num_expr == NULL) {
        num_expr = _sym_build_integer(numerator, nbits);
        if (num_expr == NULL) {
            // the backend forces us to concretize (probably expression pruning)
            return NULL;
        }
    }
    if (denom_expr == NULL) {
        denom_expr = _sym_build_integer(denominator, nbits);
        if (denom_expr == NULL) {
            // the backend forces us to concretize (probably expression pruning)
            return NULL;
        }
    }

    assert(_sym_bits_helper(num_expr) >= nbits * 2);
    assert(_sym_bits_helper(denom_expr) >= nbits * 2);
    if (_sym_bits_helper(num_expr) > nbits * 2) {
        num_expr = _sym_extract_helper(num_expr, nbits*2-1, 0);
    }
    if (_sym_bits_helper(denom_expr) > nbits * 2) {
        denom_expr = _sym_extract_helper(denom_expr, nbits*2-1, 0);
    }
    assert(_sym_bits_helper(num_expr) == nbits * 2);
    assert(_sym_bits_helper(denom_expr) == nbits * 2);

    void* bitmask_half = _sym_build_integer((1<<nbits)-1, nbits*2);
    void* bitmask_full = _sym_build_integer((1<<(nbits*2))-1, nbits*2);

    unsigned int num, den, q;

    num = (numerator & 0xffff);
    void* num_expr_masked = _sym_extract_helper(_sym_build_and(num_expr, bitmask_full), nbits * 2 - 1, 0);
    den = (denominator & 0xff);
    void* denom_expr_masked = _sym_extract_helper(_sym_build_and(denom_expr, bitmask_half), nbits * 2 - 1, 0);

    uintptr_t site_id_div_by_zero = (0x1337ul << 16) + nbits + (is_signed ? 2 : 0) + 0x40;
    void* constraint_div_by_zero = _sym_build_equal(denom_expr_masked, _sym_build_integer(0, nbits*2));
    if (den == 0) {
        _sym_push_path_constraint(constraint_div_by_zero, true, site_id_div_by_zero);
        return NULL;
    }
    _sym_push_path_constraint(constraint_div_by_zero, false, site_id_div_by_zero);

    q = (num / den);
    void* quotient_expr;
    if (is_signed) {
        quotient_expr = _sym_build_signed_div(num_expr_masked, denom_expr_masked);
    } else {
        quotient_expr = _sym_build_unsigned_div(num_expr_masked, denom_expr_masked);
    }
    void* constraint_overflow = _sym_build_unsigned_greater_than(quotient_expr, _sym_build_integer((1<<nbits)-1, nbits*2));
    uintptr_t site_id_overflow = site_id_div_by_zero + 1;
    if (q > 0xff) {
        assert((1<<nbits)-1 == 0xff);
        _sym_push_path_constraint(constraint_overflow, true, site_id_overflow);
        return NULL;
    }
    _sym_push_path_constraint(constraint_overflow, false, site_id_overflow);

    quotient_expr = _sym_build_and(quotient_expr, bitmask_half);
    // q &= 0xff;

    void* remainder_expr = _sym_build_unsigned_rem(num_expr_masked, denom_expr_masked);
    // r = (num % den) & 0xff;

    // env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | (r << 8) | q;
    // we delegate the handling of the old value to the caller
    void* result_expr = _sym_build_or(_sym_build_shift_left(remainder_expr, _sym_build_integer(nbits, nbits*2)), quotient_expr);
    return result_expr;
}

// void helper_divw_AX(CPUX86State *env, target_ulong t0)
// {
//     unsigned int num, den, q, r;

//     num = (env->regs[R_EAX] & 0xffff) | ((env->regs[R_EDX] & 0xffff) << 16);
//     den = (t0 & 0xffff);
//     if (den == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q = (num / den);
//     if (q > 0xffff) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q &= 0xffff;
//     r = (num % den) & 0xffff;
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | q;
//     env->regs[R_EDX] = (env->regs[R_EDX] & ~0xffff) | r;
// }

// void helper_idivw_AX(CPUX86State *env, target_ulong t0)
// {
//     int num, den, q, r;

//     num = (env->regs[R_EAX] & 0xffff) | ((env->regs[R_EDX] & 0xffff) << 16);
//     den = (int16_t)t0;
//     if (den == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q = (num / den);
//     if (q != (int16_t)q) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q &= 0xffff;
//     r = (num % den) & 0xffff;
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | q;
//     env->regs[R_EDX] = (env->regs[R_EDX] & ~0xffff) | r;
// }

// void helper_divl_EAX(CPUX86State *env, target_ulong t0)
// {
//     unsigned int den, r;
//     uint64_t num, q;

//     num = ((uint32_t)env->regs[R_EAX]) | ((uint64_t)((uint32_t)env->regs[R_EDX]) << 32);
//     den = t0;
//     if (den == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q = (num / den);
//     r = (num % den);
//     if (q > 0xffffffff) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     env->regs[R_EAX] = (uint32_t)q;
//     env->regs[R_EDX] = (uint32_t)r;
// }

// void helper_idivl_EAX(CPUX86State *env, target_ulong t0)
// {
//     int den, r;
//     int64_t num, q;

//     num = ((uint32_t)env->regs[R_EAX]) | ((uint64_t)((uint32_t)env->regs[R_EDX]) << 32);
//     den = t0;
//     if (den == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     q = (num / den);
//     r = (num % den);
//     if (q != (int32_t)q) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     env->regs[R_EAX] = (uint32_t)q;
//     env->regs[R_EDX] = (uint32_t)r;
// }

// /* bcd */

// /* XXX: exception */
// void helper_aam(CPUX86State *env, int base)
// {
//     int al, ah;

//     al = env->regs[R_EAX] & 0xff;
//     ah = al / base;
//     al = al % base;
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | al | (ah << 8);
//     CC_DST = al;
// }

// void helper_aad(CPUX86State *env, int base)
// {
//     int al, ah;

//     al = env->regs[R_EAX] & 0xff;
//     ah = (env->regs[R_EAX] >> 8) & 0xff;
//     al = ((ah * base) + al) & 0xff;
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | al;
//     CC_DST = al;
// }

// void helper_aaa(CPUX86State *env)
// {
//     int icarry;
//     int al, ah, af;
//     int eflags;

//     eflags = cpu_cc_compute_all(env, CC_OP);
//     af = eflags & CC_A;
//     al = env->regs[R_EAX] & 0xff;
//     ah = (env->regs[R_EAX] >> 8) & 0xff;

//     icarry = (al > 0xf9);
//     if (((al & 0x0f) > 9) || af) {
//         al = (al + 6) & 0x0f;
//         ah = (ah + 1 + icarry) & 0xff;
//         eflags |= CC_C | CC_A;
//     } else {
//         eflags &= ~(CC_C | CC_A);
//         al &= 0x0f;
//     }
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | al | (ah << 8);
//     CC_SRC = eflags;
// }

// void helper_aas(CPUX86State *env)
// {
//     int icarry;
//     int al, ah, af;
//     int eflags;

//     eflags = cpu_cc_compute_all(env, CC_OP);
//     af = eflags & CC_A;
//     al = env->regs[R_EAX] & 0xff;
//     ah = (env->regs[R_EAX] >> 8) & 0xff;

//     icarry = (al < 6);
//     if (((al & 0x0f) > 9) || af) {
//         al = (al - 6) & 0x0f;
//         ah = (ah - 1 - icarry) & 0xff;
//         eflags |= CC_C | CC_A;
//     } else {
//         eflags &= ~(CC_C | CC_A);
//         al &= 0x0f;
//     }
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xffff) | al | (ah << 8);
//     CC_SRC = eflags;
// }

// void helper_daa(CPUX86State *env)
// {
//     int old_al, al, af, cf;
//     int eflags;

//     eflags = cpu_cc_compute_all(env, CC_OP);
//     cf = eflags & CC_C;
//     af = eflags & CC_A;
//     old_al = al = env->regs[R_EAX] & 0xff;

//     eflags = 0;
//     if (((al & 0x0f) > 9) || af) {
//         al = (al + 6) & 0xff;
//         eflags |= CC_A;
//     }
//     if ((old_al > 0x99) || cf) {
//         al = (al + 0x60) & 0xff;
//         eflags |= CC_C;
//     }
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xff) | al;
//     /* well, speed is not an issue here, so we compute the flags by hand */
//     eflags |= (al == 0) << 6; /* zf */
//     eflags |= parity_table[al]; /* pf */
//     eflags |= (al & 0x80); /* sf */
//     CC_SRC = eflags;
// }

// void helper_das(CPUX86State *env)
// {
//     int al, al1, af, cf;
//     int eflags;

//     eflags = cpu_cc_compute_all(env, CC_OP);
//     cf = eflags & CC_C;
//     af = eflags & CC_A;
//     al = env->regs[R_EAX] & 0xff;

//     eflags = 0;
//     al1 = al;
//     if (((al & 0x0f) > 9) || af) {
//         eflags |= CC_A;
//         if (al < 6 || cf) {
//             eflags |= CC_C;
//         }
//         al = (al - 6) & 0xff;
//     }
//     if ((al1 > 0x99) || cf) {
//         al = (al - 0x60) & 0xff;
//         eflags |= CC_C;
//     }
//     env->regs[R_EAX] = (env->regs[R_EAX] & ~0xff) | al;
//     /* well, speed is not an issue here, so we compute the flags by hand */
//     eflags |= (al == 0) << 6; /* zf */
//     eflags |= parity_table[al]; /* pf */
//     eflags |= (al & 0x80); /* sf */
//     CC_SRC = eflags;
// }

// #ifdef TARGET_X86_64
// static void add128(uint64_t *plow, uint64_t *phigh, uint64_t a, uint64_t b)
// {
//     *plow += a;
//     /* carry test */
//     if (*plow < a) {
//         (*phigh)++;
//     }
//     *phigh += b;
// }

// static void neg128(uint64_t *plow, uint64_t *phigh)
// {
//     *plow = ~*plow;
//     *phigh = ~*phigh;
//     add128(plow, phigh, 1, 0);
// }

// /* return TRUE if overflow */
// static int div64(uint64_t *plow, uint64_t *phigh, uint64_t b)
// {
//     uint64_t q, r, a1, a0;
//     int i, qb, ab;

//     a0 = *plow;
//     a1 = *phigh;
//     if (a1 == 0) {
//         q = a0 / b;
//         r = a0 % b;
//         *plow = q;
//         *phigh = r;
//     } else {
//         if (a1 >= b) {
//             return 1;
//         }
//         /* XXX: use a better algorithm */
//         for (i = 0; i < 64; i++) {
//             ab = a1 >> 63;
//             a1 = (a1 << 1) | (a0 >> 63);
//             if (ab || a1 >= b) {
//                 a1 -= b;
//                 qb = 1;
//             } else {
//                 qb = 0;
//             }
//             a0 = (a0 << 1) | qb;
//         }
// #if defined(DEBUG_MULDIV)
//         printf("div: 0x%016" PRIx64 "%016" PRIx64 " / 0x%016" PRIx64
//                ": q=0x%016" PRIx64 " r=0x%016" PRIx64 "\n",
//                *phigh, *plow, b, a0, a1);
// #endif
//         *plow = a0;
//         *phigh = a1;
//     }
//     return 0;
// }

// /* return TRUE if overflow */
// static int idiv64(uint64_t *plow, uint64_t *phigh, int64_t b)
// {
//     int sa, sb;

//     sa = ((int64_t)*phigh < 0);
//     if (sa) {
//         neg128(plow, phigh);
//     }
//     sb = (b < 0);
//     if (sb) {
//         b = -b;
//     }
//     if (div64(plow, phigh, b) != 0) {
//         return 1;
//     }
//     if (sa ^ sb) {
//         if (*plow > (1ULL << 63)) {
//             return 1;
//         }
//         *plow = -*plow;
//     } else {
//         if (*plow >= (1ULL << 63)) {
//             return 1;
//         }
//     }
//     if (sa) {
//         *phigh = -*phigh;
//     }
//     return 0;
// }

// void helper_divq_EAX(CPUX86State *env, target_ulong t0)
// {
//     uint64_t r0, r1;

//     if (t0 == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     r0 = env->regs[R_EAX];
//     r1 = env->regs[R_EDX];
//     if (div64(&r0, &r1, t0)) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     env->regs[R_EAX] = r0;
//     env->regs[R_EDX] = r1;
// }

// void helper_idivq_EAX(CPUX86State *env, target_ulong t0)
// {
//     uint64_t r0, r1;

//     if (t0 == 0) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     r0 = env->regs[R_EAX];
//     r1 = env->regs[R_EDX];
//     if (idiv64(&r0, &r1, t0)) {
//         raise_exception_ra(env, EXCP00_DIVZ, GETPC());
//     }
//     env->regs[R_EAX] = r0;
//     env->regs[R_EDX] = r1;
// }
// #endif

// #if TARGET_LONG_BITS == 32
// # define ctztl  ctz32
// # define clztl  clz32
// #else
// # define ctztl  ctz64
// # define clztl  clz64
// #endif

// target_ulong helper_pdep(target_ulong src, target_ulong mask)
// {
//     target_ulong dest = 0;
//     int i, o;

//     for (i = 0; mask != 0; i++) {
//         o = ctztl(mask);
//         mask &= mask - 1;
//         dest |= ((src >> i) & 1) << o;
//     }
//     return dest;
// }

// target_ulong helper_pext(target_ulong src, target_ulong mask)
// {
//     target_ulong dest = 0;
//     int i, o;

//     for (o = 0; mask != 0; o++) {
//         i = ctztl(mask);
//         mask &= mask - 1;
//         dest |= ((src >> i) & 1) << o;
//     }
//     return dest;
// }

// #define SHIFT 0
// #include "shift_helper_template.h"
// #undef SHIFT

// #define SHIFT 1
// #include "shift_helper_template.h"
// #undef SHIFT

// #define SHIFT 2
// #include "shift_helper_template.h"
// #undef SHIFT

// #ifdef TARGET_X86_64
// #define SHIFT 3
// #include "shift_helper_template.h"
// #undef SHIFT
// #endif

// /* Test that BIT is enabled in CR4.  If not, raise an illegal opcode
//    exception.  This reduces the requirements for rare CR4 bits being
//    mapped into HFLAGS.  */
// void helper_cr4_testbit(CPUX86State *env, uint32_t bit)
// {
//     if (unlikely((env->cr[4] & bit) == 0)) {
//         raise_exception_ra(env, EXCP06_ILLOP, GETPC());
//     }
// }

// target_ulong HELPER(rdrand)(CPUX86State *env)
// {
//     Error *err = NULL;
//     target_ulong ret;

//     if (qemu_guest_getrandom(&ret, sizeof(ret), &err) < 0) {
//         qemu_log_mask(LOG_UNIMP, "rdrand: Crypto failure: %s",
//                       error_get_pretty(err));
//         error_free(err);
//         /* Failure clears CF and all other flags, and returns 0.  */
//         env->cc_src = 0;
//         return 0;
//     }

//     /* Success sets CF and clears all others.  */
//     env->cc_src = CC_C;
//     return ret;
// }
