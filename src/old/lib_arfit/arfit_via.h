#ifndef     ARFIT_VISTA_HEADER_INCLUDED
#define     ARFIT_VISTA_HEADER_INCLUDED


/* interface header for arfit functions depending on Via fileformat*/

#include    "arfit.h"

#include    <viaio/Vlib.h>
#include    <viaio/VImage.h>
#include    <viaio/mu.h>
#include    <viaio/option.h>


/* information about functional time series data in via files */
typedef struct {
	VAttrList       vAttributes;
	VImage         *vFunctionalImages;
	int            *nFunctionalImageIndices;
	int             ntotalslices;
	int             nfunctionalslices;
	int             nsamples;
	int             nrows;
	int             ncolumns;
}
arfit_file_info;


/* functions used with arfit_input structure */
arfit_file_info *get_file_info( FILE *in_file, arfit_file_info *info );
void            free_file_info( arfit_file_info *info );
arfit_input     *add_sample_to_input( const arfit_file_info *info, arfit_input *input, int slice, int row, int column );
arfit_input     *add_samples_to_input( const arfit_file_info *info, arfit_input *input, int quantity, int *slices, int *rows, int *columns );
arfit_input     *reset_input_data( const arfit_file_info *info, arfit_input *input, int slice, int row, int column, int channel );
arfit_input     *detach_sample_from_input( arfit_input *input, int sample );
arfit_input     *clear_input( arfit_input *input );


#endif
