/* Generated alltypes.h for x86_64 - musl libc */
#ifndef _BITS_ALLTYPES_H
#define _BITS_ALLTYPES_H

#define _Addr long
#define _Int64 long
#define _Reg long

#define __BYTE_ORDER 1234
#define __LONG_MAX 0x7fffffffffffffffL

/* wchar_t - use Clang's builtin type if available */
#ifndef __cplusplus
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#ifdef __WCHAR_TYPE__
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef int wchar_t;
#endif
#endif
#endif


#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2
typedef long double float_t;
typedef long double double_t;
#else
typedef float float_t;
typedef double double_t;
#endif

#ifndef __max_align_t_defined
#define __max_align_t_defined
typedef struct { long long __ll; long double __ld; } max_align_t;
#endif

/* Common types from alltypes.h.in */
#ifndef __size_t_defined
#define __size_t_defined
typedef unsigned _Addr size_t;
#endif

#ifndef __ptrdiff_t_defined
#define __ptrdiff_t_defined
typedef _Addr ptrdiff_t;
#endif

typedef unsigned _Addr uintptr_t;
typedef _Addr ssize_t;
typedef _Addr intptr_t;
typedef _Addr regoff_t;
typedef _Reg register_t;

#ifndef __time_t_defined
#define __time_t_defined
typedef _Int64 time_t;
#endif

typedef _Int64 suseconds_t;

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;
typedef signed _Int64   int64_t;
typedef signed _Int64   intmax_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned _Int64 uint64_t;
typedef unsigned _Int64 u_int64_t;
typedef unsigned _Int64 uintmax_t;

typedef unsigned mode_t;
typedef unsigned _Reg nlink_t;
typedef _Int64 off_t;
typedef unsigned _Int64 ino_t;
typedef unsigned _Int64 dev_t;
typedef long blksize_t;
typedef _Int64 blkcnt_t;
typedef unsigned _Int64 fsblkcnt_t;
typedef unsigned _Int64 fsfilcnt_t;

typedef unsigned wint_t;
typedef unsigned long wctype_t;

typedef void * timer_t;
typedef int clockid_t;
typedef long clock_t;

#ifndef __timeval_defined
#define __timeval_defined
struct timeval { time_t tv_sec; suseconds_t tv_usec; };
#endif

#ifndef __timespec_defined
#define __timespec_defined
struct timespec { time_t tv_sec; long tv_nsec; };
#endif

typedef int pid_t;
typedef unsigned id_t;
typedef unsigned uid_t;
typedef unsigned gid_t;
typedef int key_t;
typedef unsigned useconds_t;

typedef struct __mbstate_t { unsigned __opaque1, __opaque2; } mbstate_t;

typedef struct __locale_struct * locale_t;

typedef struct __sigset_t { unsigned long __bits[128/sizeof(long)]; } sigset_t;

struct iovec { void *iov_base; size_t iov_len; };

typedef unsigned socklen_t;
typedef unsigned short sa_family_t;

typedef long long loff_t;

/* FILE type - opaque */
typedef struct _IO_FILE FILE;

/* va_list - use builtin */
#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

#ifndef __va_list_defined
#define __va_list_defined
typedef __builtin_va_list va_list;
#endif

/* NULL */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* EOF */
#define EOF (-1)

#endif /* _BITS_ALLTYPES_H */
