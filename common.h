#ifndef __COMMON_H__
#define __COMMON_H__

#include <proto/exec.h>
#include <inline/exec_protos.h>

/*
 * NEW_xx, where x equals to:
 * - A = array
 * - S = struct
 * - Z = zero'ed
 */
#define INTERNAL_NEW(_type_, _size_, _flags_) \
    (_type_)AllocVec(_size_, _flags_)

#define NEW_S(TYPE) \
    INTERNAL_NEW(TYPE *, sizeof(TYPE), MEMF_PUBLIC)
#define NEW_SZ(TYPE) \
    INTERNAL_NEW(TYPE *, sizeof(TYPE), MEMF_PUBLIC|MEMF_CLEAR)
#define NEW_A(TYPE, NUM) \
    INTERNAL_NEW(TYPE *, sizeof(TYPE) * NUM, MEMF_PUBLIC)
#define NEW_AZ(TYPE, NUM) \
    INTERNAL_NEW(TYPE *, sizeof(TYPE) * NUM, MEMF_PUBLIC|MEMF_CLEAR)

#define DELETE(PTR) FreeVec(PTR)

#endif
