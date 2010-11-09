/*
** spectral clustering
**
** Lit:
**   Ulrike von Luxburg, Tech Report TR-149, MPI-KYB
**
**   Shi,J. and Malik,J. (2000), 
**   Normalized cuts and image segmentation,
**   IEEE-PAMI, 22(8), pp. 888-905
**
** G.Lohmann, May 2008
*/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_sort.h>

#define TINY 1.0e-6
extern void LapackEigen(gsl_matrix *,gsl_vector *,gsl_matrix *);

/*
** Shi, Malik, 2000: normalized cuts
*/
gsl_matrix *
L_rw(VImage src,gsl_matrix *cmat,int nvectors,int nclusters)
{
  int i,j;
  double u,w,d;
  gsl_matrix *laplacian_matrix=NULL;
  gsl_complex z;

  fprintf(stderr," L_rw\n");

  laplacian_matrix = gsl_matrix_calloc(nvectors,nvectors);
  for (i=0; i<nvectors; i++) {

    d = 0;
    for (j=0; j<nvectors; j++)      
      d += VGetPixel(src,0,i,j);
     
    if (d < TINY) {
      VWarning(" degree of node %d is zero",i);
      continue;
    }
    for (j=0; j<nvectors; j++) {
      w = VGetPixel(src,0,i,j);
      if (i == j)
	gsl_matrix_set(laplacian_matrix,i,j,1-w/d);
      else
	gsl_matrix_set(laplacian_matrix,i,j,-w/d);
    }
  }

  gsl_vector_complex *eval = gsl_vector_complex_alloc(nvectors);
  gsl_matrix_complex *evec = gsl_matrix_complex_alloc(nvectors,nvectors);

  gsl_eigen_nonsymmv_workspace *workspace = gsl_eigen_nonsymmv_alloc(nvectors);
  gsl_eigen_nonsymmv(laplacian_matrix,eval,evec,workspace);
  gsl_eigen_nonsymmv_sort(eval,evec,GSL_EIGEN_SORT_ABS_ASC);
  gsl_eigen_nonsymmv_free(workspace);
  

  j = nclusters + 5;
  if (j < 10) j = 10;
  if (j >= nvectors)j = nvectors;
  fprintf(stderr," eigenvalues, eigengap\n");

  for (i=0; i<j; i++) {
    z = gsl_vector_complex_get(eval,i);
    fprintf(stderr,"  %4d  %9.5f\n",i,GSL_REAL(z));
  }


  /*
  ** prepare k-means clustering matrix
  */
  cmat = gsl_matrix_calloc(nclusters,nvectors);
  for (i=0; i<evec->size1; i++) {
    for (j=0; j<nclusters; j++) {
      z = gsl_matrix_complex_get(evec,i,j);
      u = GSL_REAL(z);
      gsl_matrix_set(cmat,j,i,u);
    }
  }

  gsl_matrix_free(laplacian_matrix);
  gsl_matrix_complex_free(evec);

  return cmat;
}





/*
** Ng,Jordan, Weiss 2002
*/
gsl_matrix *
L_sym(VImage src,gsl_matrix *cmat,int nvectors,int nclusters,VBoolean lapack)
{
  int i,j;
  double u,w,d;
  gsl_matrix *laplacian_matrix=NULL;
  gsl_vector *dvec=NULL;

  fprintf(stderr," L_sym\n");
  laplacian_matrix = gsl_matrix_calloc(nvectors,nvectors);

  dvec = gsl_vector_calloc(nvectors);
    
  for (i=0; i<nvectors; i++) {
    d = 0;
    for (j=0; j<nvectors; j++)      
      d += VGetPixel(src,0,i,j);

    if (d < TINY)
      VWarning(" degree of node %d is zero",i);

    gsl_vector_set(dvec,i,sqrt(d));
  }

  for (i=0; i<nvectors; i++) {
    for (j=0; j<nvectors; j++) {
	
      d = gsl_vector_get(dvec,i) * gsl_vector_get(dvec,j);
      if (d < TINY) continue;
	
      w = VGetPixel(src,0,i,j);
      if (i == j)
	gsl_matrix_set(laplacian_matrix,i,j,1-w/d);
      else
	gsl_matrix_set(laplacian_matrix,i,j,-w/d);
    }
  }

  gsl_vector *eval = gsl_vector_alloc(nvectors);
  gsl_matrix *evec = gsl_matrix_alloc(nvectors,nvectors);

  if (lapack) {
    LapackEigen(laplacian_matrix,eval,evec);
  }
  else {
    gsl_eigen_symmv_workspace *workspace = gsl_eigen_symmv_alloc(nvectors);
    gsl_eigen_symmv(laplacian_matrix,eval,evec,workspace);
    gsl_eigen_symmv_sort(eval,evec,GSL_EIGEN_SORT_VAL_ASC);
    gsl_eigen_symmv_free(workspace);
  }

  /*
  ** determine number of clusters,
  ** use "eigengap" approach, Luxburg, p.18
  */
  j = nclusters + 10;
  if (j < 10) j = 10;
  if (j >= nvectors)j = nvectors;
  fprintf(stderr," eigenvalues, eigengap\n");

  for (i=0; i<j; i++) {
    if (gsl_vector_get(eval,i) > 0.999) break;
    fprintf(stderr,"  %4d  %9.5f\n",i,gsl_vector_get(eval,i));
  }



  /*
  ** prepare k-means clustering matrix
  */
  cmat = gsl_matrix_calloc(nclusters,nvectors);
  for (i=0; i<evec->size1; i++) {
    for (j=0; j<nclusters; j++) {
      u = gsl_matrix_get(evec,i,j);
      gsl_matrix_set(cmat,j,i,u);
    }
  }

  gsl_matrix_free(evec);
  gsl_matrix_free(laplacian_matrix);
  gsl_vector_free(dvec);

  return cmat;
}
