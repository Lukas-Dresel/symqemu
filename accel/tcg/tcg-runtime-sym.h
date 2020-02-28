#define SYM_HELPER_BINARY(name)                                                \
  DEF_HELPER_FLAGS_4(sym_##name##_i32, TCG_CALL_NO_RWG_SE, ptr,                \
                     i32, ptr, i32, ptr)                                       \
  DEF_HELPER_FLAGS_4(sym_##name##_i64, TCG_CALL_NO_RWG_SE, ptr,                \
                     i64, ptr, i64, ptr)

/* Arithmetic */
SYM_HELPER_BINARY(add)
SYM_HELPER_BINARY(sub)
SYM_HELPER_BINARY(mul)
SYM_HELPER_BINARY(div)
SYM_HELPER_BINARY(divu)
SYM_HELPER_BINARY(rem)
SYM_HELPER_BINARY(remu)

/* Shifts */
SYM_HELPER_BINARY(shift_right)
SYM_HELPER_BINARY(arithmetic_shift_right)
SYM_HELPER_BINARY(shift_left)

/* Logical operations */
SYM_HELPER_BINARY(and)
SYM_HELPER_BINARY(or)
SYM_HELPER_BINARY(xor)

#undef SYM_HELPER_BINARY

/* Guest memory */
DEF_HELPER_FLAGS_3(sym_load_guest, TCG_CALL_NO_RWG_SE, ptr, dh_alias_tl, ptr, i64)
DEF_HELPER_FLAGS_5(sym_store_guest_i32, TCG_CALL_NO_RWG, void, i32, ptr,
                   dh_alias_tl, ptr, i64)
DEF_HELPER_FLAGS_5(sym_store_guest_i64, TCG_CALL_NO_RWG, void, i64, ptr,
                   dh_alias_tl, ptr, i64)

/* Host memory */
DEF_HELPER_FLAGS_3(sym_load_host, TCG_CALL_NO_RWG_SE, ptr, ptr, i64, i64)
DEF_HELPER_FLAGS_5(sym_store_host_i32, TCG_CALL_NO_RWG, void, i32, ptr, ptr,
                   i64, i64)
DEF_HELPER_FLAGS_5(sym_store_host_i64, TCG_CALL_NO_RWG, void, i64, ptr, ptr,
                   i64, i64)

DEF_HELPER_FLAGS_4(sym_extract_i32, TCG_CALL_NO_RWG_SE, ptr, i32, ptr, i32, i32)
DEF_HELPER_FLAGS_4(sym_extract_i64, TCG_CALL_NO_RWG_SE, ptr, i64, ptr, i64, i64)

/* TODO In theory, host and guest address spaces could overlap. How do we
 * distinguish between the two? */

/* TODO call, brcond_i32/i64, neg */
