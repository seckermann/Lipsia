bin_PROGRAMS = v2ndlevel

v2ndlevelincludedir = @INC_VIA@ -I../lib_lipsia

v2ndlevel_SOURCES = v2ndlevel.c Read2ndLevelDesign.c

v2ndlevel_LDADD = $(top_builddir)/src/lib_lipsia/liblipsia.la \
	          @LIBS_GSL@ @LIBS_M@ @LIBS_VIA@

AM_CFLAGS = -ansi -O2

INCLUDES = $(v2ndlevelincludedir)
