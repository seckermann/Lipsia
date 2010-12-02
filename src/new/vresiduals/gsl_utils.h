
/* float */
extern gsl_vector_float *fmat_x_vector( gsl_matrix_float *, gsl_vector_float *, gsl_vector_float * );
extern gsl_vector_float *dmat_x_fvector( gsl_matrix *, gsl_vector_float *, gsl_vector_float * );
extern gsl_matrix_float *fmat_x_mat( gsl_matrix_float *, gsl_matrix_float *, gsl_matrix_float * );
extern gsl_matrix_float *fmat_x_matT( gsl_matrix_float *, gsl_matrix_float *, gsl_matrix_float * );
extern gsl_matrix_float *fmatT_x_mat( gsl_matrix_float *, gsl_matrix_float *, gsl_matrix_float * );
extern float fskalarproduct( gsl_vector_float *, gsl_vector_float * );
extern void fmatprint( FILE *, gsl_matrix_float *, const char * );

/* double */
extern gsl_vector *dmat_x_vector( gsl_matrix *, gsl_vector *, gsl_vector * );
extern gsl_matrix *dmat_x_mat( gsl_matrix *, gsl_matrix *, gsl_matrix * );
extern gsl_matrix *dmat_x_matT( gsl_matrix *, gsl_matrix *, gsl_matrix * );
extern double dskalarproduct( gsl_vector *, gsl_vector * );
extern gsl_matrix *dmat_PseudoInv( gsl_matrix *, gsl_matrix * );
extern gsl_matrix *dmat_x_matBand( gsl_matrix *, gsl_matrix *, gsl_matrix *, int );


#define dvset gsl_vector_set
#define dvget gsl_vector_get
#define dmset gsl_matrix_set
#define dmget gsl_matrix_get

#define fvset gsl_vector_float_set
#define fvget gsl_vector_float_get
#define fmset gsl_matrix_float_set
#define fmget gsl_matrix_float_get

