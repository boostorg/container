//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_WINAPI_HPP
#define BOOST_CONTAINER_DETAIL_WINAPI_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>

//////////////////////////////////////////////////////////////////////////////
//
// The Win32 API that the library uses, declared without <windows.h>
//
//////////////////////////////////////////////////////////////////////////////
//
// The declarations must be compatible with <windows.h>
//
// If BOOST_USE_WINDOWS_H is defined, <windows.h> is included instead and the
// types are the ones of <windows.h>.
//
// Cygwin is tested before _WIN32 because <windows.h> defines _WIN32 (and
// _WIN64) on Cygwin: the result must not depend on the include order.

#if defined(__CYGWIN__) || defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#if defined(BOOST_USE_WINDOWS_H)
#  include <windows.h>
#endif

//The struct tags of <windows.h>, so that the prototypes below are
//token-for-token compatible with it
struct _SECURITY_ATTRIBUTES;
struct HINSTANCE__;
#if !defined(__CYGWIN__)
struct _MEMORY_BASIC_INFORMATION;
struct _SYSTEM_INFO;
struct _RTL_CRITICAL_SECTION;
struct _RTL_CRITICAL_SECTION_DEBUG;
#endif

namespace boost {
namespace container {
namespace winapi {

#if defined(BOOST_USE_WINDOWS_H)

typedef ::DWORD        dword_type;
typedef ::ULONG_PTR    uintptr_type;   //SIZE_T, ULONG_PTR and DWORD_PTR
typedef ::BOOL         bool_type;
typedef ::HANDLE       handle_type;
typedef ::HMODULE      module_handle_type;

#else

//On Cygwin, the types of the w32api headers. They are not the ones of the
//Windows SDK, because Cygwin is LP64
#if defined(__CYGWIN__)
#  if defined(__x86_64__)
typedef unsigned long long uintptr_type;
#  else
typedef unsigned long      uintptr_type;
#  endif
#  if defined(__LP64__)
typedef unsigned int       dword_type;
#  else
typedef unsigned long      dword_type;
#  endif
#elif defined(_WIN64)
#  if defined(BOOST_HAS_MS_INT64)
typedef unsigned __int64   uintptr_type;
#  else
typedef unsigned long long uintptr_type;
#  endif
typedef unsigned long      dword_type;
#else
//ILP32 Windows: ULONG_PTR is "unsigned long"
typedef unsigned long      uintptr_type;
typedef unsigned long      dword_type;
#endif
typedef int                bool_type;
typedef void *             handle_type;
typedef ::HINSTANCE__ *    module_handle_type;

#endif   //BOOST_USE_WINDOWS_H

//CreateFileMappingA, MapViewOfFile, GetModuleHandleExA
static const dword_type page_readwrite_c        = 0x04;          //PAGE_READWRITE
static const dword_type file_map_read_write_c   = 0x02 | 0x04;   //FILE_MAP_WRITE|FILE_MAP_READ
static const dword_type module_handle_pin_c     = 0x01;          //GET_MODULE_HANDLE_EX_FLAG_PIN
static const dword_type module_from_address_c   = 0x04;          //GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS

BOOST_CONTAINER_FORCEINLINE void *invalid_handle()
{  return (void *)(~uintptr_type(0));  }   //INVALID_HANDLE_VALUE

#if !defined(__CYGWIN__)

//VirtualAlloc, VirtualFree
static const dword_type mem_commit_c     = 0x1000;     //MEM_COMMIT
static const dword_type mem_reserve_c    = 0x2000;     //MEM_RESERVE
static const dword_type mem_release_c    = 0x8000;     //MEM_RELEASE
static const dword_type mem_top_down_c   = 0x100000;   //MEM_TOP_DOWN

//Layout-compatible with MEMORY_BASIC_INFORMATION, SYSTEM_INFO and
//CRITICAL_SECTION, so the real definitions are never needed. The functions
//take pointers to them through a cast.
struct memory_basic_information
{
   void          *BaseAddress;
   void          *AllocationBase;
   dword_type     AllocationProtect;
#if defined(_WIN64)
   unsigned short PartitionId;
#endif
   uintptr_type   RegionSize;
   dword_type     State;
   dword_type     Protect;
   dword_type     Type;
};

struct system_info
{
   dword_type     dwOemId;
   dword_type     dwPageSize;
   void          *lpMinimumApplicationAddress;
   void          *lpMaximumApplicationAddress;
   uintptr_type   dwActiveProcessorMask;
   dword_type     dwNumberOfProcessors;
   dword_type     dwProcessorType;
   dword_type     dwAllocationGranularity;
   unsigned short wProcessorLevel;
   unsigned short wProcessorRevision;
};

struct critical_section
{
   ::_RTL_CRITICAL_SECTION_DEBUG *DebugInfo;
   long           LockCount;
   long           RecursionCount;
   void          *OwningThread;
   void          *LockSemaphore;
   uintptr_type   SpinCount;
};

#if defined(BOOST_USE_WINDOWS_H)
BOOST_CONTAINER_STATIC_ASSERT(sizeof(memory_basic_information) == sizeof(::MEMORY_BASIC_INFORMATION));
BOOST_CONTAINER_STATIC_ASSERT(sizeof(system_info) == sizeof(::SYSTEM_INFO));
BOOST_CONTAINER_STATIC_ASSERT(sizeof(critical_section) == sizeof(::CRITICAL_SECTION));
#endif

#endif   //!defined(__CYGWIN__)

}  //namespace winapi {
}  //namespace container {
}  //namespace boost {

#if !defined(BOOST_USE_WINDOWS_H)

extern "C" {

//Windows and Cygwin: the intermodule globals

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   CloseHandle(void *hObject);

__declspec(dllimport) void * __stdcall
   CreateFileMappingA
      ( void *hFile
      , ::_SECURITY_ATTRIBUTES *lpAttributes
      , boost::container::winapi::dword_type flProtect
      , boost::container::winapi::dword_type dwMaximumSizeHigh
      , boost::container::winapi::dword_type dwMaximumSizeLow
      , const char *lpName);

__declspec(dllimport) void * __stdcall
   MapViewOfFile
      ( void *hFileMappingObject
      , boost::container::winapi::dword_type dwDesiredAccess
      , boost::container::winapi::dword_type dwFileOffsetHigh
      , boost::container::winapi::dword_type dwFileOffsetLow
      , boost::container::winapi::uintptr_type dwNumberOfBytesToMap);

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   UnmapViewOfFile(const void *lpBaseAddress);

__declspec(dllimport) boost::container::winapi::dword_type __stdcall
   GetCurrentProcessId(void);

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   GetModuleHandleExA
      ( boost::container::winapi::dword_type dwFlags
      , const char *lpModuleName
      , ::HINSTANCE__ **phModule);

#if !defined(__CYGWIN__)

//Windows only: the dlmalloc heap

__declspec(dllimport) void * __stdcall
   VirtualAlloc
      ( void *lpAddress
      , boost::container::winapi::uintptr_type dwSize
      , boost::container::winapi::dword_type flAllocationType
      , boost::container::winapi::dword_type flProtect);

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   VirtualFree
      ( void *lpAddress
      , boost::container::winapi::uintptr_type dwSize
      , boost::container::winapi::dword_type dwFreeType);

__declspec(dllimport) boost::container::winapi::uintptr_type __stdcall
   VirtualQuery
      ( const void *lpAddress
      , ::_MEMORY_BASIC_INFORMATION *lpBuffer
      , boost::container::winapi::uintptr_type dwLength);

__declspec(dllimport) void __stdcall
   GetSystemInfo(::_SYSTEM_INFO *lpSystemInfo);

//Windows only: spin_mutex

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   SwitchToThread(void);

__declspec(dllimport) void __stdcall
   Sleep(boost::container::winapi::dword_type dwMilliseconds);

//Windows only: thread_mutex. InitializeCriticalSectionEx is the only one
//of the two that UWP has.

__declspec(dllimport) boost::container::winapi::bool_type __stdcall
   InitializeCriticalSectionEx
      ( ::_RTL_CRITICAL_SECTION *lpCriticalSection
      , boost::container::winapi::dword_type dwSpinCount
      , boost::container::winapi::dword_type Flags);

__declspec(dllimport) void __stdcall
   InitializeCriticalSection(::_RTL_CRITICAL_SECTION *lpCriticalSection);

__declspec(dllimport) void __stdcall
   EnterCriticalSection(::_RTL_CRITICAL_SECTION *lpCriticalSection);

__declspec(dllimport) void __stdcall
   LeaveCriticalSection(::_RTL_CRITICAL_SECTION *lpCriticalSection);

__declspec(dllimport) void __stdcall
   DeleteCriticalSection(::_RTL_CRITICAL_SECTION *lpCriticalSection);

#endif   //!defined(__CYGWIN__)

}  //extern "C"

#endif   //!defined(BOOST_USE_WINDOWS_H)

#endif   //Windows or Cygwin

#include <boost/container/detail/config_end.hpp>

#endif //BOOST_CONTAINER_DETAIL_WINAPI_HPP
