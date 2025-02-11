/* Scalar wrappers for double-precision Advanced SIMD vector math functions.

   Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <arm_neon.h>

#include "test-double-advsimd.h"

#define VEC_TYPE float64x2_t

VPCS_VECTOR_WRAPPER (acos_advsimd, _ZGVnN2v_acos)
VPCS_VECTOR_WRAPPER (asin_advsimd, _ZGVnN2v_asin)
VPCS_VECTOR_WRAPPER (atan_advsimd, _ZGVnN2v_atan)
VPCS_VECTOR_WRAPPER_ff (atan2_advsimd, _ZGVnN2vv_atan2)
VPCS_VECTOR_WRAPPER (cos_advsimd, _ZGVnN2v_cos)
VPCS_VECTOR_WRAPPER (exp_advsimd, _ZGVnN2v_exp)
VPCS_VECTOR_WRAPPER (exp10_advsimd, _ZGVnN2v_exp10)
VPCS_VECTOR_WRAPPER (exp2_advsimd, _ZGVnN2v_exp2)
VPCS_VECTOR_WRAPPER (log_advsimd, _ZGVnN2v_log)
VPCS_VECTOR_WRAPPER (log10_advsimd, _ZGVnN2v_log10)
VPCS_VECTOR_WRAPPER (log1p_advsimd, _ZGVnN2v_log1p)
VPCS_VECTOR_WRAPPER (log2_advsimd, _ZGVnN2v_log2)
VPCS_VECTOR_WRAPPER (sin_advsimd, _ZGVnN2v_sin)
VPCS_VECTOR_WRAPPER (tan_advsimd, _ZGVnN2v_tan)
