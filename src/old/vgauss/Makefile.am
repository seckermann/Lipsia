bin_PROGRAMS = vgauss

vgaussincludedir = @INC_VIA@

vgauss_SOURCES = vgauss.c

vgauss_LDADD = $(top_builddir)/src/lib_lipsia/liblipsia.la \
	       @LIBS_GSL@ @LIBS_M@ @LIBS_VIA@

INCLUDES = $(vgaussincludedir)
