//===------------------------------------------------------------*- C++ -*-===//
//
// This file is distributed under BSD License. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Copyright (c) 2017 University of Kaiserslautern.
//
//===----------------------------------------------------------------------===//
// \file
// Portable definition of primitive types
//===----------------------------------------------------------------------===//


#if !defined __PRIMITIVE_TYPE_DEFS_HPP
#define __PRIMITIVE_TYPE_DEFS_HPP

#ifdef _MSC_VER
# define INLINE __forceinline
typedef  unsigned __int64 uint64;
typedef  unsigned __int32 uint32;
typedef  unsigned __int16 uint16;
typedef  unsigned __int8  uint8;
#else
# include <stdint.h>
# define INLINE inline
#endif

#if !(__APPLE__ & __MACH__)
typedef  uint64_t  uint64;
typedef  int64_t int64;
#else
typedef size_t uint64;
typedef size_t int64;
#endif

typedef uint32_t uint32;
typedef int32_t int32;
typedef uint16_t uint16;
typedef int16_t int16;
typedef int8_t int8;
typedef uint8_t uint8;

#endif // __PRIMITIVE_TYPE_DEFS_HPP
