LT_CONFIG_LTDL_DIR([libltdl])

AC_INIT([libmbus-wm-gb], [0.0.1], [huarun.lin@163.com], [libmbus-wm-gb], [https://github.com/huarunlin/libmbus-wm-gb/])
AC_CONFIG_AUX_DIR([libltdl/config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_PROG_LIBTOOL
# fix for automake 1.11 & 1.12
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LDFLAGS="$LDFLAGS -version-info 0:0:1"

dnl ----------------------
dnl
AC_PROG_CC

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile mbus/Makefile libmbus-wm-gb.pc])
AC_OUTPUT


echo \
"----------------------------------------------------------
Configuration:

	Source location:	${srcdir}
	Compile:		${CC}
	Compiler flags:		${CFLAGS}
	Linker flags:		${LDFLAGS}
	Host system type:	${host}
	Install path:		${prefix}

	See config.h for further configuration.
----------------------------------------------------------"
