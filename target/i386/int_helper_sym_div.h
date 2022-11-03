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

#define nbits DIVISION_NBITS
#define is_signed DIVISION_SIGNED

void* HELPER(glue(sym_div, DIVISION_SUFFIX))(CPUX86State *env,
                      void *eax_expr,           // or rax in 64bit case
                      void *edx_expr,           // or rdx in 64bit case
                      target_ulong denominator,
                      void *denominator_expr)
{
    unsigned long long mask_half = (unsigned long long)-1;
    mask_half >>= (64 - nbits);
    // printf("%s: Division by %lx\n", __func__, denominator);
    if (eax_expr == NULL && denominator_expr == NULL && (nbits == 8 || edx_expr == NULL)) {
        // printf("\tfully concrete\n");
        return NULL;
    }
    if (eax_expr == NULL) {
        eax_expr = _sym_build_integer(env->regs[R_EAX], TARGET_LONG_BITS);
        if (eax_expr == NULL) {
            // the backend forces us to concretize (probably expression pruning)
            // printf("\teax expression was pruned by the backend\n");
            return NULL;
        }
    }
    if (nbits != 8 && edx_expr == NULL) {
        edx_expr = _sym_build_integer(env->regs[R_EDX], TARGET_LONG_BITS);
        if (edx_expr == NULL) {
            // the backend forces us to concretize (probably expression pruning)
            // printf("\tedx expression was pruned by the backend\n");
            return NULL;
        }
    }
    if (denominator_expr == NULL) {
        denominator_expr = _sym_build_integer(denominator, nbits);
        if (denominator_expr == NULL) {
            // printf("\tdenominator expression was pruned by the backend\n");
            // the backend forces us to concretize (probably expression pruning)
            return NULL;
        }
    }
    // printf("\teax: %p, edx: %p, denominator: %p\n", eax_expr, edx_expr, denominator_expr);

    assert(_sym_bits_helper(eax_expr) == TARGET_LONG_BITS);
    assert(nbits == 8 || (_sym_bits_helper(edx_expr) == TARGET_LONG_BITS));
    assert(_sym_bits_helper(denominator_expr) == TARGET_LONG_BITS);

    void* numerator_expr = NULL;
    target_ulong numerator_concrete_low = 0;
    target_ulong numerator_concrete_high = 0;
    target_ulong denominator_concrete = 0;

    // we assume target_(u)long is at least as large as target_type
    #define EXTEND_CONCRETE(target_type, src_type_signed, src_type_unsigned, value) \
        (is_signed) \
            ? ((target_type)(target_long)(src_type_signed)(value)) \
            : ((target_type)(target_ulong)(src_type_unsigned)(value))

    #define EXTEND_SYMBOLIC_BY(expr, n) is_signed \
                                        ? _sym_build_sext((expr), (n)) \
                                        : _sym_build_zext((expr), (n))

    #if nbits == 8
    {
        // numerator is 16 bits from ax
        numerator_concrete_low = EXTEND_CONCRETE(target_ulong, short, unsigned short, env->regs[R_EAX]);
        numerator_expr = _sym_extract_helper(eax_expr, nbits * 2 - 1, 0);
        //denominator is 8 bits from the argument, extended to 16 bits for the division
        denominator_concrete = EXTEND_CONCRETE(target_ulong, char, unsigned char, denominator);
        // extend by another nbits to get to the full double width
        denominator_expr = _sym_extract_helper(denominator_expr, nbits - 1, 0);
        denominator_expr = EXTEND_SYMBOLIC_BY(denominator_expr, nbits);
    }
    #elif nbits == 16 || nbits == 32
    {
        // numerator = EDX:EAX
        int64_t combined_numerator = ((env->regs[R_EDX] & mask_half) << nbits) | (env->regs[R_EAX] & mask_half);

        numerator_concrete_low = (nbits == 16)
                                ? EXTEND_CONCRETE(target_ulong, int32_t, uint32_t, combined_numerator)
                                : EXTEND_CONCRETE(target_ulong, int64_t, uint64_t, combined_numerator);
        numerator_expr = _sym_concat_helper(
                _sym_extract_helper(edx_expr, nbits - 1, 0),
                _sym_extract_helper(eax_expr, nbits - 1, 0)
                );
        assert(_sym_bits_helper(numerator_expr) == nbits * 2);
        denominator_expr = _sym_extract_helper(denominator_expr, nbits - 1, 0);
        denominator_expr = EXTEND_SYMBOLIC_BY(denominator_expr, nbits);
        #if nbits == 16
        denominator_concrete = EXTEND_CONCRETE(target_ulong, short, unsigned short, denominator);
        #else
        denominator_concrete = EXTEND_CONCRETE(target_ulong, int, unsigned int, denominator);
        #endif
    }
    #elif nbits == 64
    {
        numerator_concrete_high = env->regs[R_EDX];
        numerator_concrete_low = env->regs[R_EAX];
        numerator_expr = _sym_concat_helper(
                _sym_extract_helper(edx_expr, nbits - 1, 0),
                _sym_extract_helper(eax_expr, nbits - 1, 0)
                );
        assert(_sym_bits_helper(numerator_expr) == nbits * 2 && nbits * 2 == 128);
        denominator_expr = _sym_extract_helper(denominator_expr, nbits - 1, 0);
        denominator_expr = EXTEND_SYMBOLIC_BY(denominator_expr, nbits);
        assert(_sym_bits_helper(denominator_expr) == nbits * 2 && nbits * 2 == 128);
        denominator_concrete = denominator;
    }
    #else
    #error "Unsupported nbits" nbits
    #endif

    #undef EXTEND_CONCRETE
    #undef EXTEND_SYMBOLIC_BY

    if (numerator_expr == NULL || denominator_expr == NULL) {
        // the backend forces us to concretize (probably expression pruning)
        // printf("\tconcretizing due to backend pruning of numerator or denominator: numerator=%p, denominator=%p\n",
        //         numerator_expr, denominator_expr);
        return NULL;
    }

    // now numerator_expr and denominator_expr are both nbits*2 long
    assert(_sym_bits_helper(numerator_expr) == nbits * 2);
    assert(_sym_bits_helper(denominator_expr) == nbits * 2);

    uintptr_t site_id_div_by_zero = (0x1337ul << 16) + nbits + (is_signed ? 2 : 0) + 0x40;
    void* constraint_div_by_zero = _sym_build_equal(denominator_expr, _sym_build_integer(0, nbits*2));
    if (denominator_concrete == 0) {
        _sym_push_path_constraint(constraint_div_by_zero, true, site_id_div_by_zero);
        // printf("\texiting early with division by zero\n");
        return NULL;
    }
    _sym_push_path_constraint(constraint_div_by_zero, false, site_id_div_by_zero);

    bool overflow_concrete = false;

    #if nbits == 8 || nbits == 16 || nbits == 32
    {
        #if nbits == 8
        #define SIGNED_TYPE int8_t
        #elif nbits == 16
        #define SIGNED_TYPE int16_t
        #elif nbits == 32
        #define SIGNED_TYPE int32_t
        #else
        #error "how?"
        #endif
        assert(numerator_concrete_high == 0);
        if (is_signed) {
            target_ulong quotient = (target_long)numerator_concrete_low / (target_long)denominator_concrete;
            overflow_concrete = quotient != (SIGNED_TYPE)quotient;
        }
        else {
            target_ulong quotient = (target_ulong)numerator_concrete_low / (target_ulong)denominator_concrete;
            overflow_concrete = (quotient & mask_half) != quotient;
        }
        #undef SIGNED_TYPE
    }
    #else
    {
        // 64-bit division
        uint64_t r0 = numerator_concrete_low;
        uint64_t r1 = numerator_concrete_high;
        if (is_signed) {
            overflow_concrete = idiv64(&r0, &r1, (int64_t)denominator_concrete);
        }
        else {
            overflow_concrete = div64(&r0, &r1, (uint64_t)denominator_concrete);
        }
    }
    #endif
    void* quotient_expr = is_signed
        ? _sym_build_signed_div(numerator_expr, denominator_expr)
        : _sym_build_unsigned_div(numerator_expr, denominator_expr);

    void* constraint_overflow = _sym_build_not(
            _sym_build_equal(
                quotient_expr,
                is_signed
                    ? _sym_build_sext(_sym_extract_helper(quotient_expr, nbits - 1, 0), nbits)
                    : _sym_build_and(quotient_expr, _sym_build_integer(mask_half, nbits * 2))
            )
        );
    uintptr_t site_id_overflow = site_id_div_by_zero + 1;
    if (overflow_concrete) {
        _sym_push_path_constraint(constraint_overflow, true, site_id_overflow);
        // printf("\texiting early with division overflow\n");
        return NULL;
    }
    _sym_push_path_constraint(constraint_overflow, false, site_id_overflow);

    // q &= 0xff;
    quotient_expr = _sym_build_and(quotient_expr, _sym_build_integer(mask_half, nbits * 2));

    // r = (num % den) & 0xff;
    void* remainder_expr = is_signed
                                ? _sym_build_signed_rem(numerator_expr, denominator_expr)
                                : _sym_build_unsigned_rem(numerator_expr, denominator_expr);
    remainder_expr = _sym_build_and(
                        remainder_expr,
                        _sym_build_integer(mask_half, nbits * 2)
                    );

    assert(_sym_bits_helper(quotient_expr) == nbits * 2);
    assert(_sym_bits_helper(remainder_expr) == nbits * 2);

    // now update eax_expr and edx_expr if needed
    #if nbits == 8
    {
        void* combined = _sym_build_or(
                            _sym_build_shift_left(remainder_expr, _sym_build_integer(8, nbits * 2)),
                            quotient_expr
        );
        eax_expr = _sym_build_or(
            _sym_build_and(eax_expr, _sym_build_integer(~0xffff, TARGET_LONG_BITS)),
            _sym_build_zext(combined, TARGET_LONG_BITS - 16)
        );
        // edx_expr remains unchanged from entry
    }
    #elif nbits == 16
    {
        void* old_eax = _sym_build_and(eax_expr, _sym_build_integer(~0xffff, TARGET_LONG_BITS));
        void* old_edx = _sym_build_and(edx_expr, _sym_build_integer(~0xffff, TARGET_LONG_BITS));
        // void* new_eax =
        eax_expr = _sym_build_or(
            old_eax,
            _sym_build_zext(quotient_expr, TARGET_LONG_BITS - (nbits*2))
        );
        edx_expr = _sym_build_or(
            old_edx,
            _sym_build_zext(remainder_expr, TARGET_LONG_BITS - (nbits*2))
        );
    }
    #elif nbits == 32
    {
        // this gives us the zero-extension for a 64-bit target already, does nothing otherwise

        unsigned long long mask_full = (mask_half << nbits) | mask_half;
        eax_expr = _sym_build_and(quotient_expr, _sym_build_integer(mask_full, nbits * 2));
        edx_expr = _sym_build_and(remainder_expr, _sym_build_integer(mask_full, nbits * 2));
        // on 64-bit target, this does nothing, on 32-bit it truncates down to register width
        eax_expr = _sym_extract_helper(quotient_expr, TARGET_LONG_BITS - 1, 0);
        edx_expr = _sym_extract_helper(remainder_expr, TARGET_LONG_BITS - 1, 0);
    }
    #elif nbits == 64
    {
        eax_expr = _sym_extract_helper(quotient_expr, TARGET_LONG_BITS - 1, 0);
        edx_expr = _sym_extract_helper(remainder_expr, TARGET_LONG_BITS - 1, 0);
    }
    #else
    #error "unsupported nbits" nbits
    #endif

    // printf("\tresult: eax_expr: %p, edx_expr: %p\n", eax_expr, edx_expr);
    assert(_sym_bits_helper(eax_expr) == TARGET_LONG_BITS);
    assert(nbits == 8 || _sym_bits_helper(edx_expr) == TARGET_LONG_BITS);

    // now we return the combined expression back to the caller to split to the register expressions
    void* result = _sym_concat_helper(edx_expr, eax_expr);
    // printf("\tresult: %p\n", result);
    return result;
}

#undef nbits
#undef is_signed