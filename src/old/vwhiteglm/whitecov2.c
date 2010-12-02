#include "mathtools.h"
#include <viaio/VImage.h>
#include <math.h>
#include <string.h>
#include <gsl/gsl_linalg.h>

void whitecov2( VImage **effect_image,
				VImage **sdeffect_image,
				VImage rho_vol,
				gsl_matrix_float *Y,
				gsl_matrix_float *X,
				gsl_matrix_float *con,
				VFloat *Dfs,
				int *dim,
				int slice
			  )
{

	int numlags = dim[2];
	int type = dim[3];
	int numcon = dim[4];
	int n = Y->size1;
	VFloat Df = Dfs[numcon];

	int i, j, k;

	/* some checks and warnings */
	if( X->size2 != n )
		VError( "whitecov2: Warning! sizes of design and data matrix do not match." );

	if( X->size1 != con->size2 )
		VError( "whitecov2: Warning! contrast and design do not match." );

	/* calculate irho vector */
	/* :  if numlags==1
	   % bin rho to intervals of length drho, avoiding -1 and 1:
	       irho=round(rho_vol(:,slice,1)/drho)*drho;
	       irho=min(irho,1-drho);
	       irho=max(irho,-1+drho);
	else
	    % use dummy unique values so every pixel is analysed seperately:
	       irho=(1:numpix)';
	end*/
	gsl_vector_float *irho = gsl_vector_float_alloc( VImageNBands( rho_vol ) );

	if( numlags == 1 ) {
		float *p = irho->data;
		const float drho = 0.01;

		for( i = 0; i < irho->size; i++ ) {
			float tmp = ROUND( ( VPixel( rho_vol, i, slice, 0, VFloat ) / drho ) ) * drho;
			tmp = MIN( tmp, ( 1 - drho ) );
			tmp = MAX( tmp, ( -1 + drho ) );
			*p++ = tmp;
		}
	} else {
		float *p = irho->data;

		for( i = 1; i <= irho->size; i++ )
			*p++ = i;
	}

	/* : X=X';  */
	gsl_matrix_float *transX = gsl_matrix_float_alloc( X->size2, X->size1 );
	gsl_matrix_float_transpose_memcpy( transX, X );

	/* : Xstar=X; */
	gsl_matrix_float *XStar = gsl_matrix_float_alloc( transX->size1, transX->size2 );
	gsl_matrix_float_memcpy( XStar, transX );

	/* END allocate memory buffer */

	/****************************** order == 1 *****************************/
	if( numlags == 1 ) {

		/* allocate memory buffers */
		gsl_vector_float *pixBig = gsl_vector_float_alloc( irho->size );
		gsl_matrix_float *pinvXStar = gsl_matrix_float_alloc( XStar->size2, XStar->size1 );
		gsl_matrix_float *V = gsl_matrix_float_alloc( pinvXStar->size1, pinvXStar->size1 );
		gsl_matrix_float *buffer_01 = gsl_matrix_float_alloc( con->size1, V->size2 );
		gsl_matrix_float *buffer_02 = gsl_matrix_float_alloc( con->size1, con->size1 );
		gsl_matrix_float *cVcinv = gsl_matrix_float_alloc( con->size1, con->size1 );

		gsl_vector_float *u = funique( irho );
		float *rho = u->data;

		for( i = 0; i < u->size; i++ ) {

			/* :  pix=int16(find(irho==rho)); */
			float *p = irho->data;
			float *ppB = pixBig->data;
			int counter = 0;

			for( j = 0; j < irho->size; j++ ) {
				if( *p++ == *rho ) {
					*ppB++ = ( float )j;
					counter++;
				}
			}

			gsl_vector_float *pix = gsl_vector_float_alloc( counter );
			ppB = pixBig->data;
			float *pp = pix->data;

			for( j = 0; j < counter; j++ ) {
				*pp++ = *ppB++;
			}

			/* END  */

			/* : Ystar=Y(:,pix); */
			gsl_matrix_float *YStar = fmat_subcols( Y, pix );

			/* : factor=1./sqrt(1-rho^2); */
			float factor = pow( 1 - *rho * *rho, -0.5 );

			/* : Ystar(k1,:)=(Y(k1,pix)-rho*Y(k1-1,pix))*factor; */
			/* point to last element */
			float *pStar = YStar->data + ( YStar->size1 * YStar->size2 ) - 1;

			/* traverse matrix in reverse order */
			for ( j = 0; j < ( YStar->size1 - 1 )*YStar->size2; ++j ) {
				*pStar = ( *pStar - *rho * *( pStar - YStar->size2 ) ) * factor;
				pStar--;
			}

			/* END  */

			/* : Xstar(k1,:)=(X(k1,:)-rho*X(k1-1,:))*factor; */
			pStar = XStar->data + XStar->size2;
			float *pX = transX->data + transX->size2;

			for ( j = 0; j < ( XStar->size1 - 1 )*XStar->size2; ++j ) {
				*pStar++ = ( *pX - *rho * *( pX - transX->size2 ) ) * factor;
				pX++;
			}

			/* END  */

			/* : pinvXstar=pinv(Xstar); */
			fmat_PseudoInv( XStar, pinvXStar );

			/* : betahat=pinvXstar*Ystar; */
			gsl_matrix_float *betahat = fmat_x_mat( pinvXStar, YStar, NULL );

			/* : resid=Ystar-Xstar*betahat; */
			gsl_matrix_float *buffer = fmat_x_mat( XStar, betahat, NULL );
			gsl_matrix_float_sub( YStar, buffer );
			gsl_matrix_float *resid =  YStar;
			gsl_matrix_float_free( buffer );

			/* : SSE=sum(resid.^2,1); */
			buffer = gsl_matrix_float_alloc( resid->size1, resid->size2 );
			gsl_matrix_float_memcpy ( buffer, resid );
			gsl_matrix_float_mul_elements ( buffer, buffer );
			gsl_vector_float *sse = fsum( buffer, 1, NULL );
			gsl_matrix_float_free( buffer );

			/* : sd=sqrt(SSE/Df); */
			gsl_vector_float *sd = gsl_vector_float_alloc( sse->size );
			float *pSSE = sse->data;
			float *pSD = sd->data;

			for ( j = 0; j < sse->size; ++j ) {
				*pSD++ = ( float )sqrt( *pSSE++ / Df );
			}

			/* :   V=pinvXstar*pinvXstar'; */
			fmat_x_matT( pinvXStar, pinvXStar, V );

			/* : mag_ef=contrast*betahat; */
			gsl_matrix_float *mag_ef = fmat_x_mat( con, betahat, NULL );

			/* : mag_sd=sqrt(diag(contrast*V*contrast'))*sd; */
			fmat_x_mat( con, V, buffer_01 );
			fmat_x_matT( buffer_01, con, buffer_02 );

			gsl_vector_float_view diag = gsl_matrix_float_diagonal( buffer_02 );
			gsl_matrix_float *mag_sd =
				gsl_matrix_float_alloc( diag.vector.size, sd->size );
			float *pMag = mag_sd->data;

			for ( j = 0; j < diag.vector.size; ++j ) {
				float s = sqrt( gsl_vector_float_get( &diag.vector, j ) );
				pSD = sd->data;

				for ( k = 0; k < sd->size; ++k ) {
					*pMag++ = s * *pSD++;
				}
			}

			if( type == 0 ) {
				/* : effect_slice(pix,:)=mag_ef';  */
				/* : sdeffect_slice(pix,:)=mag_sd'; */
				for ( k = 0; k < numcon; k++ ) {
					VFloat *peff = &VPixel( ( *effect_image )[k], slice, 0, 0, VFloat );
					VFloat *psd = &VPixel( ( *sdeffect_image )[k], slice, 0, 0, VFloat );
					float *pmeff = mag_ef->data + k * mag_ef->size2;
					float *pmsd = mag_sd->data + k * mag_sd->size2;
					float *ppix = pix->data;

					for( j = 0; j < pix->size; j++ ) {
						*( peff + ( int )*ppix ) = ( VFloat ) * pmeff++;
						*( psd + ( int )*ppix++ ) = ( VFloat ) * pmsd++;
					}
				}
			}

			else {
				if( type == 3 ) {
					/* : cVcinv=pinv(contrast*V*contrast'); */
					/* buffer_02 contains the result from con * V * con */
					fmat_PseudoInv( buffer_02, cVcinv );

					/* : SST=sum((cVcinv*mag_ef).*mag_ef,1); */
					buffer = fmat_x_mat( cVcinv, mag_ef, NULL );
					gsl_matrix_float_mul_elements ( buffer, mag_ef );
					gsl_vector_float *sst = fsum( buffer, 1, NULL );
					gsl_matrix_float_free( buffer );

					/* :
					 *      effect_slice(pix,1)=(SST./(SSE+(SSE<=0)).*(SSE>0)/Df)'; */
					VFloat *peff =  &VPixel( ( *effect_image )[0], slice, 0, 0, VFloat );
					float *psst = sst->data;
					float *psse = sse->data;
					float *ppix = pix->data;

					for ( j = 0; j < pix->size; ++j ) {
						*( peff + ( int )*ppix++ ) =
							*psst / ( *psse + ( *psse <= 0 ) ) * ( ( *psse>0 ) * Df );
						psst++;
						psse++;
					}

					gsl_vector_float_free( sst );

					for ( k = 1; k < numcon; k++ )
						VCopyImage( ( *effect_image )[0], ( *effect_image )[k], VAllBands );
				} else {
					/* :
					 * effect_slice(pix,1)= (mag_ef./(mag_sd+(mag_sd<=0)).*(mag_sd>0))'; */
					for ( k = 0; k < numcon; k++ ) {
						VFloat *peff =  &VPixel( ( *effect_image )[k], slice, 0, 0, VFloat );
						float *pmeff = mag_ef->data + k * mag_ef->size2;
						float *pmsd = mag_sd->data + k * mag_sd->size2;
						float *ppix = pix->data;

						for ( j = 0; j < pix->size; ++j ) {
							*( peff + ( int )*ppix++ ) =
								*pmeff / ( *pmsd + ( *pmsd <= 0 ) ) * ( *pmsd > 0 );
							pmeff++;
							pmsd++;
						}
					}
				}
			}

			/* free some memory */
			gsl_matrix_float_free( betahat );
			gsl_matrix_float_free( YStar );
			gsl_matrix_float_free( mag_ef );
			gsl_matrix_float_free( mag_sd );
			gsl_vector_float_free( pix );
			gsl_vector_float_free( sse );
			gsl_vector_float_free( sd );

			rho++;
		} /* END for(i=0;i<u->size;i++)  */

		/* free some memory */
		gsl_vector_float_free( pixBig );
		gsl_matrix_float_free( pinvXStar );
		gsl_matrix_float_free( V );
		gsl_matrix_float_free( buffer_01 );
		gsl_matrix_float_free( buffer_02 );
		gsl_matrix_float_free( cVcinv );

	} /******************* END order == 1 **********************************/

	/*************************** order > 1 *********************************/
	else {

		/* allocate memory buffer */
		gsl_vector_float *Coradj_pix = gsl_vector_float_alloc( VImageNColumns( rho_vol ) + 1 );
		gsl_matrix_float *Ainv = gsl_matrix_float_alloc( Coradj_pix->size, Coradj_pix->size );
		gsl_matrix *dbuff = gsl_matrix_alloc( Ainv->size1, Ainv->size2 );
		gsl_matrix_float *A = gsl_matrix_float_alloc( Ainv->size1, Ainv->size2 );
		int nl = Ainv->size2;
		gsl_matrix_float *buffer_01 = gsl_matrix_float_alloc( n - nl, 1 );
		gsl_matrix_float *buffer_02 = gsl_matrix_float_alloc( A->size1, 1 );
		gsl_matrix_float *buffer_03 = gsl_matrix_float_alloc( A->size1, transX->size2 );
		gsl_matrix_float *B = gsl_matrix_float_alloc( buffer_01->size1, A->size2 );
		gsl_matrix_float *Vmhalf = gsl_matrix_float_alloc( n - nl, n );
		gsl_matrix_float *buffer_04 = gsl_matrix_float_alloc( Vmhalf->size1, transX->size2 );
		gsl_matrix_float *YStar = gsl_matrix_float_alloc( n, 1 );
		gsl_matrix_float *pinvXStar = gsl_matrix_float_alloc( XStar->size2, XStar->size1 );
		gsl_matrix_float *betahat = gsl_matrix_float_alloc( pinvXStar->size1, YStar->size2 );
		gsl_matrix_float *buffer_05 = gsl_matrix_float_alloc( XStar->size1, betahat->size2 );
		gsl_vector_float *sse = gsl_vector_float_alloc( 1 );
		gsl_vector_float *sd = gsl_vector_float_alloc( YStar->size2 );
		gsl_matrix_float *V = gsl_matrix_float_alloc( pinvXStar->size1, pinvXStar->size1 );
		gsl_matrix_float *mag_ef = gsl_matrix_float_alloc( con->size1, betahat->size2 );
		gsl_matrix_float *buffer_06 = gsl_matrix_float_alloc( con->size1, V->size2 );
		gsl_matrix_float *buffer_07 = gsl_matrix_float_alloc( con->size1, con->size1 );
		gsl_matrix_float *mag_sd = gsl_matrix_float_alloc( con->size1, sd->size );
		gsl_matrix_float *cVcinv = gsl_matrix_float_alloc( buffer_07->size2, buffer_07->size1 );
		gsl_matrix_float *buffer_08 = gsl_matrix_float_alloc( mag_ef->size1, mag_ef->size2 );
		gsl_vector_float *sst = gsl_vector_float_alloc( 1 );
		/* END allocate memory buffer */


		/* we know that irho only contains a list of indices for every voxel so we can skip
		   the unique call and iterate over irho itself */
		float *rho = irho->data;

		for( i = 0; i < irho->size; i++ ) {

			/* : Coradj_pix=squeeze(rho_vol(pix,slice,:)); */

			float *pCoradj = Coradj_pix->data + 1;

			for( k = 0; k < Coradj_pix->size - 1; k++ ) {
				*( pCoradj++ ) = VPixel( rho_vol,
										 i,
										 slice,
										 k,
										 VFloat );
			}

			/* END  */

			/* : [Ainvt posdef]=chol(toeplitz([1 Coradj_pix'])); */

			/* at first the toeplitz matrix */
			Coradj_pix->data[0] = 1;
			fmat_toeplitz( Coradj_pix, Ainv );

			/* the cholesky decomposition */
			/* double buffer */
			for( j = 0; j < Ainv->size1; j++ ) {
				for( k = 0; k < Ainv->size2; k++ ) {
					gsl_matrix_set( dbuff, j, k, ( double )gsl_matrix_float_get( Ainv, j, k ) );
				}
			}

			if( gsl_linalg_cholesky_decomp( dbuff ) == GSL_EDOM )
				VError( "Calculation error, cholesky decomposition failed for pix=%d", i );

			/* note: the gsl cholesky factorization returns the inverse
			 * of Ainvt in the lower triangular part of it's output matrix.
			 * Since we will use the inverse matrix of Ainvt in the next steps we
			 * will work with the lower triangular part in contrast to the matlab algorithm */

			/* convert back to float and remove upper triangular matrix */
			for( j = 0; j < Ainv->size1; j++ ) {
				for( k = 0; k < Ainv->size2; k++ ) {
					if( j < k ) {
						gsl_matrix_float_set( Ainv, j, k, 0 );
					} else {
						gsl_matrix_float_set( Ainv, j, k, ( float )gsl_matrix_get( dbuff, j, k ) );
					}
				}
			}

			/* END  */

			/* :  A=inv(Ainvt'); */
			fInv( Ainv, A );

			/* : B=ones(n-nl,1)*A(nl,:); */
			gsl_matrix_float_set_all( buffer_01, 1 );
			gsl_matrix_float_view subm = gsl_matrix_float_submatrix( A, nl - 1, 0, 1, A->size2 );
			fmat_x_mat( buffer_01, &subm.matrix, B );
			/* END  */

			/* : Vmhalf=spdiags(double(B),double(1:nl),double(n-nl),double(n)); */
			gsl_matrix_float_set_zero( Vmhalf );

			for( j = 0; j < B->size1; j++ ) {
				for( k = 0; k < B->size2; k++ ) {
					gsl_matrix_float_set( Vmhalf, j, k + 1 + j, gsl_matrix_float_get( B, j, k ) );
				}
			}

			/* END  */


			/* : Ystar=single(zeros(n,1)); */
			gsl_matrix_float_set_zero( YStar );

			/* : Ystar(1:nl)=A*Y(1:nl,pix); */
			subm = gsl_matrix_float_submatrix( Y, 0, i, nl, 1 );
			fmat_x_mat( A, &subm.matrix, buffer_02 );

			for( j = 0; j < nl; j++ ) {
				gsl_matrix_float_set( YStar, j, 0, gsl_matrix_float_get( buffer_02, j, 0 ) );
			}

			/* END  */

			/* : Ystar((nl+1):n)=single(Vmhalf*double(Y(:,pix))); */
			subm = gsl_matrix_float_submatrix( Y, 0, i, Y->size1, 1 );
			fmat_x_mat( Vmhalf, &subm.matrix, buffer_01 );

			for( j = nl; j < n; j++ ) {
				gsl_matrix_float_set( YStar, j, 0, gsl_matrix_float_get( buffer_01, j - nl, 0 ) );
			}

			/* END  */

			/* : Xstar(1:nl,:)=A*X(1:nl,:); */
			subm = gsl_matrix_float_submatrix( transX, 0, 0, nl, transX->size2 );
			fmat_x_mat( A, &subm.matrix, buffer_03 );

			for( j = 0; j < buffer_03->size1; j++ ) {

				for( k = 0; k < buffer_03->size2; k++ ) {
					gsl_matrix_float_set( XStar, j, k, gsl_matrix_float_get( buffer_03, j, k ) );
				}
			}

			/* END  */

			/* : Xstar((nl+1):n,:)=single(Vmhalf*double(X)); */
			fmat_x_mat( Vmhalf, transX, buffer_04 );

			for( j = nl; j < n; j++ ) {
				for( k = 0; k < transX->size2; k++ ) {
					gsl_matrix_float_set( XStar, j, k, gsl_matrix_float_get( buffer_04, j - nl, k ) );
				}
			}

			/* END  */

			/* : pinvXstar=pinv(Xstar); */
			fmat_PseudoInv( XStar, pinvXStar );

			/* : betahat=pinvXstar*Ystar; */
			fmat_x_mat( pinvXStar, YStar, betahat );


			/* : resid=Ystar-Xstar*betahat; */
			fmat_x_mat( XStar, betahat, buffer_05 );
			gsl_matrix_float_sub( YStar, buffer_05 );
			gsl_matrix_float *resid =  YStar;

			/* : SSE=sum(resid.^2,1); */
			gsl_matrix_float_mul_elements ( resid, resid );
			fsum( resid, 1, sse );

			/* : sd=sqrt(SSE/Df); */
			float *pSSE = sse->data;
			float *pSD = sd->data;

			for ( j = 0; j < sse->size; ++j ) {
				*pSD++ = ( float )sqrt( *pSSE++ / Df );
			}

			/* :   V=pinvXstar*pinvXstar'; */
			fmat_x_matT( pinvXStar, pinvXStar, V );

			/* : mag_ef=contrast*betahat; */
			fmat_x_mat( con, betahat, mag_ef );

			/* : mag_sd=sqrt(diag(contrast*V*contrast'))*sd; */
			fmat_x_mat( con, V, buffer_06 );
			fmat_x_matT( buffer_06, con, buffer_07 );

			gsl_vector_float_view diag = gsl_matrix_float_diagonal( buffer_07 );
			float *pMag = mag_sd->data;

			for ( j = 0; j < diag.vector.size; ++j ) {
				float s = sqrt( gsl_vector_float_get( &diag.vector, j ) );
				pSD = sd->data;

				for ( k = 0; k < sd->size; ++k ) {
					*pMag++ = s * *pSD++;
				}
			}

			/* END  */

			if( type == 0 ) {
				/* : effect_slice(pix,:)=mag_ef';  */
				/* : sdeffect_slice(pix,:)=mag_sd'; */
				for ( k = 0; k < numcon; k++ ) {
					VFloat *peff = &VPixel( ( *effect_image )[k], slice, 0, 0, VFloat );
					VFloat *psd = &VPixel( ( *sdeffect_image )[k], slice, 0, 0, VFloat );
					float *pmeff = mag_ef->data + k * mag_ef->size2;
					float *pmsd = mag_sd->data + k * mag_sd->size2;
					*( peff + i ) = ( VFloat ) * pmeff;
					*( psd + i ) = ( VFloat ) * pmsd;
				}
			} else {
				if( type == 3 ) {
					/* : cVcinv=pinv(contrast*V*contrast'); */
					/* buffer_07 contains the result from con * V * con */
					fmat_PseudoInv( buffer_07, cVcinv );

					/* : SST=sum((cVcinv*mag_ef).*mag_ef,1); */
					fmat_x_mat( cVcinv, mag_ef, buffer_08 );
					gsl_matrix_float_mul_elements ( buffer_08, mag_ef );
					fsum( buffer_08, 1, sst );

					/* :
					 *      effect_slice(pix,1)=(SST./(SSE+(SSE<=0)).*(SSE>0)/Df)'; */
					VFloat *peff =  &VPixel( ( *effect_image )[0], slice, 0, 0, VFloat );
					float *psst = sst->data;
					float *psse = sse->data;
					*( peff + i ) = *psst / ( *psse + ( *psse <= 0 ) ) * ( ( *psse>0 ) * Df );

					for ( k = 1; k < numcon; k++ )
						VCopyImage( ( *effect_image )[0], ( *effect_image )[k], VAllBands );
				} else {

					/* :
					 * effect_slice(pix,1)= (mag_ef./(mag_sd+(mag_sd<=0)).*(mag_sd>0))'; */
					for ( k = 0; k < numcon; k++ ) {
						VFloat *peff =  &VPixel( ( *effect_image )[k], slice, 0, 0, VFloat );
						float *pmeff = mag_ef->data + k * mag_ef->size2;
						float *pmsd = mag_sd->data + k * mag_sd->size2;
						*( peff + i ) = *pmeff / ( *pmsd + ( *pmsd <= 0 ) ) * ( *pmsd > 0 );
					}
				}
			}

			/* next value */
			rho++;
		} /* END for(i=0;i<irho->size;i++) */

		/* free some memory */
		gsl_vector_float_free( Coradj_pix );
		gsl_matrix_float_free( Ainv );
		gsl_matrix_free( dbuff );
		gsl_matrix_float_free( A );
		gsl_matrix_float_free( buffer_01 );
		gsl_matrix_float_free( buffer_02 );
		gsl_matrix_float_free( buffer_03 );
		gsl_matrix_float_free( buffer_04 );
		gsl_matrix_float_free( B );
		gsl_matrix_float_free( Vmhalf );
		gsl_matrix_float_free( YStar );
		gsl_matrix_float_free( pinvXStar );
		gsl_matrix_float_free( buffer_05 );
		gsl_vector_float_free( sd );
		gsl_matrix_float_free( V );
		gsl_matrix_float_free( mag_ef );
		gsl_matrix_float_free( buffer_06 );
		gsl_matrix_float_free( buffer_07 );
		gsl_matrix_float_free( mag_sd );
		gsl_matrix_float_free( buffer_08 );
		gsl_vector_float_free( sse );
		gsl_vector_float_free( sst );

	}/************************************** END order > 1 ***************************************/

	if( type == 2 ) {
		/* :
		 * minus = ones(size(effect_slice),'single');
		 * minus(find(effect_slice<0))=single(-1.0);
		 * effect_slice=single(sqrt(Df .* log(1 + effect_slice.^2 ./ Df) .* (1 - 0.5 ./ Df)));
		 * effect_slice=effect_slice .* minus; */
		for ( k = 0; k < numcon; k++ ) {
			VFloat *peff = &VPixel( ( *effect_image )[k], slice, 0, 0, VFloat );

			for ( i = 0; i < VImageNColumns( ( *effect_image )[k] ) * VImageNRows( ( *effect_image )[k] ); ++i ) {
				VFloat oldVal = *peff;
				Df = Dfs[k];
				*peff = sqrt( Df * log( 1 + pow( *peff, 2 ) / Df ) * ( 1 - 0.5 / Df ) );
				*peff = ( oldVal < 0 ) ? *peff * -1.0 : *peff;
				peff++;
			}
		}
	}

	/* free allocated memory */
	gsl_matrix_float_free( XStar );
	gsl_matrix_float_free( transX );
	/* END free allocated memory */

}
