bin_PROGRAMS = vaddcovariates

vaddcovariatesincludedir = @INC_VIA@

vaddcovariates_SOURCES = vaddcovariates.c

vaddcovariates_LDADD = $(top_builddir)/src/lib_lipsia/liblipsia.la \
	       @LIBS_M@  @LIBS_GSL@ @LIBS_VIA@

AM_CFLAGS = -ansi -O2

INCLUDES = $(vaddcovariatesincludedir)
