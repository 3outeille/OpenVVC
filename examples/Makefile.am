bin_PROGRAMS = dectest filmgraintest

dectest_SOURCES = dectest.c
dectest_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/../libovvc -I ../libovvc
dectest_LDADD = ../libovvc/libovvc.la

filmgraintest_SOURCES = filmgraintest.c
filmgraintest_CPPFLAGS = $(AM_CPPFLAGS) -O0 -I$(srcdir)/../libovvc -I ../libovvc
filmgraintest_LDADD = ../libovvc/libovvc.la

