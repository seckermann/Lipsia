bin_PROGRAMS = vcacp

vcacpincludedir = @INC_VIA@

vcacp_SOURCES = vcacp.c Rotate.c Shear.c

# GaussConv.c Kernel.c ConvolveSep.c Convolve.c

vcacp_LDADD = $(top_builddir)/src/lib_lipsia/liblipsia.la \
	      @LIBS_GSL@ @LIBS_M@ @LIBS_VIA@

INCLUDES = $(vcacpincludedir)
