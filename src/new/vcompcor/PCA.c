/*
** Principal components analysis using SVD
** 
** G.Lohmann, May 2011
*/
#include <viaio/Vlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>

/*
** centering, subtract mean
*/
gsl_matrix *
Centering(gsl_matrix *A,gsl_matrix *B)
{
  int i,j;
  double u,sum=0;
  gsl_vector *mean=NULL;

  mean = gsl_vector_alloc(A->size2);
  for (i=0; i<A->size2; i++) {
    sum = 0;
    for (j=0; j<A->size1; j++) sum += gsl_matrix_get(A,j,i);
    gsl_vector_set(mean,i,sum / (double)A->size1);
  }

  if (B == NULL) B = gsl_matrix_calloc(A->size1,A->size2);
  for (i=0; i<A->size2; i++) {
    for (j=0; j<A->size1; j++) {
      u = gsl_matrix_get(A,j,i) - gsl_vector_get(mean,i);
      gsl_matrix_set(B,j,i,u);
    }
  }
  gsl_vector_free(mean);
  return B;
}


gsl_matrix *
ReduceColumns(gsl_matrix *A,int ncomp,int first)
{
  int i,j;
  gsl_matrix *B=NULL;

  if ((first+ncomp) > (A->size2-1)) {
    fprintf(stderr," warning: reduce ncomp = %d\n",ncomp);
    ncomp = A->size2-first-1;
    if (ncomp < 1) VError(" comp: %d",ncomp);
  }
  fprintf(stderr,"# first: %d, ncomp= %d\n",first,ncomp);
  B = gsl_matrix_calloc(A->size1,ncomp);

  /* ignore 1st PC */
  for (i=first; i<first+ncomp; i++) {
    for (j=0; j<A->size1; j++) {
      gsl_matrix_set(B,j,i-first,gsl_matrix_get(A,j,i));
    }
  }
  return B;
}



/*
**  PCA via SVD
**  feature vectors are rows,
**  output: fewer feature vectors as rows
*/
gsl_matrix *
VPCA(gsl_matrix *A,int ncomp,int first)
{
  gsl_matrix *U=NULL,*UU=NULL,*V=NULL,*B=NULL;
  gsl_vector *s=NULL;
  gsl_vector *work=NULL;
  
  /* alloc */
  work = gsl_vector_calloc (A->size2);
  s = gsl_vector_calloc(A->size2);
  V = gsl_matrix_calloc(A->size2,A->size2);

  /* SVD */
  U = Centering(A,U);
  gsl_matrix_memcpy (A,U);
  gsl_linalg_SV_decomp(U,V,s,work);

  /* back project */
  UU = ReduceColumns(U,ncomp,first);
  gsl_matrix_free(U);
  B = gsl_matrix_calloc(ncomp,A->size2);
  gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,UU,A,0.0,B);
  gsl_matrix_free(UU);
  gsl_matrix_free(V);
  gsl_vector_free(s);
  gsl_vector_free(work);

  return B;
}
