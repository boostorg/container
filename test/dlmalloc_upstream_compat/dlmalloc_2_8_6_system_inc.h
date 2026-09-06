//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// The pristine dlmalloc_2_8_6.c, included with every warning turned off.
//
// It is the upstream release, kept here as the reference the class is held
// against, so not a line of it is ever edited. That means every warning it
// draws has to be turned off from outside - and naming them one by one is a
// list that only ever grows, because each compiler and each version finds
// different ones.
//
// No include guard on purpose: the driver reads this once per configuration,
// each time with a different set of DLC_ knobs already defined.
//
//////////////////////////////////////////////////////////////////////////////

//GCC and clang (which defines __GNUC__ too) say nothing about a system
//header, which turns off all of them at once, now and in future versions.
#if defined(__GNUC__)
#  pragma GCC system_header
#endif

#if defined(_MSC_VER)
   //MSVC has no such notion, so warning level 0 does the same job for
   //everything the front end reports.
   //
   //C4702 is not one of those: the optimizer emits it, by which time every
   //pragma region has been popped again, so it cannot be scoped to the
   //include at all and has to be off for the whole translation unit. The
   //disable therefore comes BEFORE the push, so that the pop below - which
   //restores the state as it was at the push - leaves it disabled.
#  pragma warning(disable : 4702)   //unreachable code, from the optimizer
#  pragma warning(push, 0)
#endif

#include "../dlmalloc_2_8_6.c"

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
