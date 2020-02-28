#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h"
#include "qemu/qemu-print.h"

/* This should make it easy to spot bogus expressions in the debugger. */
#define NOT_IMPLEMENTED ((void*)0xAAAAAAAAAAAAAAAA)

void *HELPER(sym_add_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_add_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_sub_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_sub_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_mul_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_mul_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_div_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_div_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_divu_i32)(uint32_t arg1, void *arg1_expr,
                           uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_divu_i64)(uint64_t arg1, void *arg1_expr,
                           uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_rem_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_rem_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_remu_i32)(uint32_t arg1, void *arg1_expr,
                           uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_remu_i64)(uint64_t arg1, void *arg1_expr,
                           uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_and_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_and_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_or_i64)(uint64_t arg1, void *arg1_expr,
                         uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_or_i32)(uint32_t arg1, void *arg1_expr,
                         uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_xor_i64)(uint64_t arg1, void *arg1_expr,
                          uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_xor_i32)(uint32_t arg1, void *arg1_expr,
                          uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_load_guest)(target_ulong addr, void *addr_expr, uint64_t length)
{
    /* TODO */
    /* TODO try an alternative address; cast the address to uint64_t */
    return NOT_IMPLEMENTED;
}

void HELPER(sym_store_guest_i32)(uint32_t value, void *value_expr,
                                 target_ulong addr, void *addr_expr,
                                 uint64_t length)
{
    /* TODO */
}

void HELPER(sym_store_guest_i64)(uint64_t value, void *value_expr,
                                 target_ulong addr, void *addr_expr,
                                 uint64_t length)
{
    /* TODO */
}

void *HELPER(sym_load_host)(void *addr, uint64_t offset, uint64_t length)
{
    /* TODO use addr + offset for the address and NULL for the address
     * expression */
    return NOT_IMPLEMENTED;
}

void HELPER(sym_store_host_i32)(uint32_t value, void *value_expr,
                                void *addr,
                                uint64_t offset, uint64_t length)
{
    /* TODO */
}

void HELPER(sym_store_host_i64)(uint64_t value, void *value_expr,
                                void *addr,
                                uint64_t offset, uint64_t length)
{
    /* TODO */
}

void *HELPER(sym_shift_right_i32)(uint32_t arg1, void *arg1_expr,
                                  uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_shift_right_i64)(uint64_t arg1, void *arg1_expr,
                                  uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_arithmetic_shift_right_i32)(uint32_t arg1, void *arg1_expr,
                                             uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_arithmetic_shift_right_i64)(uint64_t arg1, void *arg1_expr,
                                             uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_shift_left_i32)(uint32_t arg1, void *arg1_expr,
                                 uint32_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_shift_left_i64)(uint64_t arg1, void *arg1_expr,
                                 uint64_t arg2, void *arg2_expr)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_extract_i32)(uint32_t arg, void *arg_expr,
                              uint32_t ofs, uint32_t len)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}

void *HELPER(sym_extract_i64)(uint64_t arg, void *arg_expr,
                              uint64_t ofs, uint64_t len)
{
    /* TODO */
    return NOT_IMPLEMENTED;
}
