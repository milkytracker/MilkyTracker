dnl @synopsis AX_CHECK_ALIGNED_ACCESS_REQUIRED
dnl
dnl Copyright (C) 2006, 2009 Guido U. Draheim <guidod@gmx.de>
dnl Copyright (C) 2010 Peter Breitenlohner <tex-live@tug.org>
dnl
dnl This file is free software; the copyright holders
dnl give unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl
dnl While the x86 CPUs allow access to memory objects to be unaligned
dnl it happens that most of the modern designs require objects to be
dnl aligned - or they will fail with a buserror. That mode is quite known
dnl by big-endian machines (sparc, etc) however the alpha cpu is little-
dnl endian.
dnl
dnl The following function will test for aligned access to be required and
dnl set a config.h define HAVE_ALIGNED_ACCESS_REQUIRED (name derived by
dnl standard usage). Structures loaded from a file (or mmapped to memory)
dnl should be accessed per-byte in that case to avoid segfault type errors.
dnl
dnl @category C
dnl @author Guido U. Draheim <guidod@gmx.de>
dnl @author Peter Breitenlohner <tex-live@tug.org>
dnl @version 2010-02-01
dnl @license GPLWithACException
dnl @license BSD

AC_DEFUN([AX_CHECK_ALIGNED_ACCESS_REQUIRED],
[AC_CACHE_CHECK([if pointers to integers require aligned access],
  [ax_cv_have_aligned_access_required],
[if test "$cross_compiling" = yes; then
  case "$host_cpu" in
    alpha*|arm*|bfin*|hp*|mips*|sh*|sparc*|ia64|nv1)
      ax_cv_have_aligned_access_required=yes ;;
    *)
      ax_cv_have_aligned_access_required=no ;;
  esac
else
  AC_RUN_IFELSE([AC_LANG_PROGRAM([
#include <stdio.h>
#include <stdlib.h>
    ],[
  char* string = malloc(40);
  int i;
  for (i=0; i < 40; i++) string[[i]] = i;
  {
     void* s = string;
     int* p = (int*)(s+1);
     int* q = (int*)(s+2);

     if (*p == *q) { free(string); return 1; }
  }
  free(string);
  return 0;
    ])],
    [ax_cv_have_aligned_access_required=yes],
    [ax_cv_have_aligned_access_required=no],
    [ax_cv_have_aligned_access_required=no])
fi])
if test "$ax_cv_have_aligned_access_required" = yes ; then
  AC_DEFINE([HAVE_ALIGNED_ACCESS_REQUIRED], [1],
    [Define if pointers to integers require aligned access])
fi
])
