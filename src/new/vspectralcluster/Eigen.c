/*
** get eigenvectors/eigenvalues using lapack
*/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/os.h>
#include <viaio/VImage.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include "f2c.h"


/* lapack Subroutines */ 
extern int sstegr_(char *jobz, char *range, int *n, float *d__, 
		   float *e, float *vl, float *vu, int *il, int *iu, float *abstol, 
		   int *m, float *w, float *z__, int *ldz, int *isuppz, float *
		   work, int *lwork, int *iwork, int *liwork, int *info);

extern void ssytrd_(char *uplo, int *n, float *a, int *lda, float *d, float *e, 
	     float *tau, float *work, int *lwork, int *info);

extern int sormtr_(char *side, char *uplo, char *trans, int *m, 
		   int *n, float *a, int *lda, float *tau, float *c__, int *ldc, 	 
		   float *work, int *lwork, int *info);

void
LapackEigen(gsl_matrix *matrix,gsl_vector *eval,gsl_matrix *evec)
{
  int i,j,k,n=0,m=0;
  int il=0,iu=0,info,lwork,nb=32;
  int *iwork=NULL,*isuppz=NULL,liwork=0;
  float *A=NULL,*B=NULL,*D=NULL,*E=NULL,*TAU=NULL,*W=NULL,*work=NULL;
  float vl=0,vu=0,abstol=0.001;


  n = matrix->size1;
  if (matrix->size2 != n) VError(" LapackEigen: input matrix not symmetrical");

  fprintf(stderr," using lapack, n= %d\n",n);


  A = (float *) VMalloc(n*n*sizeof(float));

  k = 0;
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      A[k++] = (float)gsl_matrix_get(matrix,j,i);
    }
  }


  /*
  ** reduce to tridiagonal form
  */
  D     = (float *) VMalloc(n*sizeof(float));
  E     = (float *) VMalloc(n*sizeof(float));
  TAU   = (float *) VMalloc(n*sizeof(float));
  lwork = nb*n;
  work  = (float *) VMalloc(lwork*sizeof(float));

  fprintf(stderr," lapack: ssytrd...\n");
  ssytrd_("U",&n,A,&n,D,E,TAU,work,&lwork,&info);
  if (info != 0) VError(" ssytrd info= %d\n",info);
  

  /*
  ** get eigenvalues/eigenvectors of tridiagonal matrix
  */
  B = (float *) VMalloc(n*n*sizeof(float));
  W = (float *) VMalloc(n*sizeof(float));
  liwork = 10*n;
  iwork = (int *) VMalloc(liwork*sizeof(int));
  isuppz = (int *) VMalloc(2*n*sizeof(int));

  fprintf(stderr," lapack: sstegr...\n");
  sstegr_("V","A",&n,D,E,&vl,&vu,&il,&iu,&abstol,&m,W,B,
	  &n,isuppz,work,&lwork,iwork,&liwork,&info);
  if (info != 0) VError(" sstegr info= %d\n",info);



  /*
  ** multiply by householder matrices Q,
  ** eigenvectors are columns of Q*B
  */
  fprintf(stderr," lapack: sormtr...\n");
  sormtr_("L","U","N",&n,&n,A,&n,TAU,B,&n,work,&lwork,&info);
  if (info != 0) VError(" sormtr info= %d\n",info);
  fprintf(stderr," lapack: done\n");

  /*
  ** output 
  */
  for (i=0; i<n; i++) {
    gsl_vector_set(eval,i,(double)W[i]);
  }

  k = 0;
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      gsl_matrix_set(evec,j,i,(double)B[k++]);
    }
  }

  VFree(D);
  VFree(E);
  VFree(A);
  VFree(B);
  VFree(work);
}


