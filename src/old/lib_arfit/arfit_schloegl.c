#include	"arfit_schloegl.h"
#include	"gsl_matrix_wrapper.h"
#include	<gsl/gsl_sf_log.h>
#include	<gsl/gsl_complex_math.h>


/* mvar function
	estimates arvalues for the given time series Data stored in Y and maximum order pmax
	mode determines which algorithm to use
*/
int mvar( gsl_matrix *Y, int pmax, arfit_mode mode, gsl_matrix **ARF, gsl_matrix **RCF, gsl_matrix **PE, gsl_matrix **DC, int *varargout )
{
	int N, M, K;
	gsl_matrix *C = NULL, *PEF = NULL, *PEB = NULL, *ARB = NULL, *RCB = NULL;

	double dn;

	/* check parameters */
	if( !Y )
		return 1;
	
	N = Y->size1;
	M = Y->size2;

	/* check for incorrect model order value */
	if( pmax < 1 )
		pmax = ( N > M ) ? N - 1 : M - 1;

	/*MATLAB
	[C(:,1:M),n] = covm(Y,'M');
	PE(:,1:M)  = C(:,1:M)./n;
	*/
	gsl_matrix_cov( Y, Y, &C, &dn );
	gsl_matrix_part( C, PE, 0, 0, C->size1 - 1, M - 1, 0, 0 );
	gsl_matrix_scale( *PE, 1.0 / dn );


	/* choose mode of ar mode estimation */
	switch( mode )
	{
		case arfit_mode_ywbiased:
				/*
					Levinson-Wiggens-Robinson (LWR) algorithm using biased correlation function
					"Multichannel Yule-Walker"
				*/
		
		/* MATLAB
		C(:,1:M) = C(:,1:M)/N;
        PEF = C(:,1:M);  
        PEB = C(:,1:M);
		*/
		{
			gsl_matrix *cpyC = NULL;
			gsl_matrix_part( C, &cpyC, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_scale( cpyC, 1.0 / (double) N );
			gsl_matrix_part( cpyC, &C, 0, 0, cpyC->size1 - 1, cpyC->size2 - 1, 0, 0 );
			gsl_matrix_part( C, &PEF, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( C, &PEB, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_free( cpyC );
		}

		/*MATLAB
		for K=1:Pmax
		*/
		for( K = 1; K <= pmax; ++K )
		{
			int L;
			
			/*MATLAB
			[C(:,K*M+(1:M)),n] = covm(Y(K+1:N,:),Y(1:N-K,:),'M');
            C(:,K*M+(1:M)) = C(:,K*M+(1:M))/N;
			D = C(:,K*M+(1:M));
			*/
			gsl_matrix *cov1 		= gsl_matrix_part( Y, NULL, K, 0, N - 1, Y->size2 - 1, 0, 0 );
			gsl_matrix *cov2 		= gsl_matrix_part( Y, NULL, 0, 0, N - K - 1, Y->size2 - 1, 0, 0 );
			gsl_matrix *cov			= NULL;
			gsl_matrix *D 			= NULL;					
			gsl_matrix_cov( cov1, cov2, &cov, NULL );
			gsl_matrix_part( cov, &C, 0, 0, cov->size1 - 1, cov->size2 - 1, 0, K * M );
			gsl_matrix_part( C, &D, 0, K * M, C->size1 - 1, K * M + M - 1, 0, 0 );
			gsl_matrix_scale( D, 1.0 / (double)N );
			gsl_matrix_part( D, &C, 0, 0, D->size1 - 1, D->size2 - 1, 0, K * M );
			gsl_matrix_free( cov );
			gsl_matrix_free( cov2 );
			gsl_matrix_free( cov1 );

			/*MATLAB
			for L = 1:K-1,
            		D = D - ARF(:,L*M+(1-M:0))*C(:,(K-L)*M+(1:M));
            end;
			*/
			for( L = 1; L <= K - 1; ++L )
			{
				gsl_matrix *partARF = NULL;
				gsl_matrix *partC 	= NULL;
				gsl_matrix *prod 	= NULL;
				gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				gsl_matrix_part( C, &partC, 0, ( K - L ) * M, C->size1 - 1, ( K - L ) * M + M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partC );
				gsl_matrix_sub( D, prod );
				gsl_matrix_free( prod );
				gsl_matrix_free( partC );
				gsl_matrix_free( partARF );
			}

			/*MATLAB
			ARF(:,K*M+(1-M:0)) = D / PEB;	
           	ARB(:,K*M+(1-M:0)) = D'/ PEF;
			*/
			{
				gsl_matrix *invPEB 	= NULL;
				gsl_matrix *invPEF 	= NULL;
				gsl_matrix *prod1 	= NULL;
				gsl_matrix *prod2 	= NULL;
				invPEB 	= gsl_matrix_inverse( PEB );
				invPEF 	= gsl_matrix_inverse( PEF );		
				prod1 = gsl_matrix_multiply( D, invPEB );
				prod2 = gsl_matrix_multiply_trans_notrans( D, invPEF );
				gsl_matrix_part( prod1, ARF, 0, 0, prod1->size1 - 1, prod1->size2 - 1, 0, K * M - M );
				gsl_matrix_part( prod2, &ARB, 0, 0, prod2->size1 - 1, prod2->size2 - 1, 0, K * M - M );

				gsl_matrix_free( prod2 );
				gsl_matrix_free( prod1 );							
				gsl_matrix_free( invPEF );			
				gsl_matrix_free( invPEB );
			}

			/*MATLAB
			for L = 1:K-1,
            		tmp                    = ARF(:,L*M+(1-M:0)) - ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
            		ARB(:,(K-L)*M+(1-M:0)) = ARB(:,(K-L)*M+(1-M:0)) - ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
            		ARF(:,L*M+(1-M:0))     = tmp;
            end;
			*/
			for( L = 1; L <= K - 1; ++L )
			{
				gsl_matrix *tmp 	= NULL;
				gsl_matrix *partARF = NULL;
				gsl_matrix *partARB = NULL;
				gsl_matrix *prod 	= NULL;
				
				gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part(  ARB, &partARB,  0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partARB );
				gsl_matrix_sub( tmp, prod );

				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );
				partARF = partARB = prod = NULL;
				
				gsl_matrix_part( ARB,  &partARB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARB, partARF );
				gsl_matrix_free( partARB );
				partARB = NULL;
				gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
				gsl_matrix_sub( partARB, prod );
				gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
				gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, ( L * M ) - M );
				
				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );				
				gsl_matrix_free( tmp );
			}
			
			/*MATLAB
			RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
            RCB(:,K*M+(1-M:0)) = ARB(:,K*M+(1-M:0));
			*/
			gsl_matrix_part( *ARF,  RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
			gsl_matrix_part(  ARB, &RCB, 0, K * M - M, ARB->size1 - 1,    K * M - 1, 0, K * M - M );
			
			/*MATLAB
			PEF				= [eye(M) - ARF(:,K*M+(1-M:0))*ARB(:,K*M+(1-M:0))]*PEF;
            PEB				= [eye(M) - ARB(:,K*M+(1-M:0))*ARF(:,K*M+(1-M:0))]*PEB;
            PE(:,K*M+(1:M)) = PEF; 
			*/
			{
				gsl_matrix *identity 	= gsl_matrix_alloc( M, M );
				gsl_matrix *partARF		= NULL;
				gsl_matrix *partARB		= NULL;
				gsl_matrix *prod		= NULL;
				gsl_matrix *cpyPEF		= gsl_matrix_alloc( PEF->size1, PEF->size2 );
				gsl_matrix *cpyPEB		= gsl_matrix_alloc( PEB->size1, PEB->size2 );

				gsl_matrix_memcpy( cpyPEF, PEF );
				gsl_matrix_memcpy( cpyPEB, PEB );
				gsl_matrix_free( PEF ); gsl_matrix_free( PEB );
				gsl_matrix_set_identity( identity );
				gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part(  ARB, &partARB, 0, K * M - M, ARB->size1 - 1,    K * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partARB );
				gsl_matrix_sub( identity, prod );
				PEF = gsl_matrix_multiply( identity, cpyPEF );
				gsl_matrix_free( prod );
				prod = NULL;
				
				gsl_matrix_set_identity( identity );
				prod = gsl_matrix_multiply( partARB, partARF );
				gsl_matrix_sub( identity, prod );
				PEB = gsl_matrix_multiply( identity, cpyPEB );

				gsl_matrix_part( PEF, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );

				gsl_matrix_free( cpyPEB );
				gsl_matrix_free( cpyPEF );
				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );
				gsl_matrix_free( identity );
			}
			
			gsl_matrix_free( D );
		}

		break;

		case arfit_mode_ywunbiased: 
				/*	
					Levinson-Wiggens-Robinson (LWR) algorithm using unbiased correlation function
				*/
		
		/* MATLAB
		C(:,1:M) = C(:,1:M)/N;
        PEF = C(:,1:M);  
        PEB = C(:,1:M);
		*/
		{
			gsl_matrix *cpyC = NULL;
			gsl_matrix_part( C, &cpyC, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_scale( cpyC, 1.0 / (double) N );
			gsl_matrix_part( cpyC, &C, 0, 0, cpyC->size1 - 1, cpyC->size2 - 1, 0, 0 );
			gsl_matrix_part( C, &PEF, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( C, &PEB, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_free( cpyC );
		}
		
		/*MATLAB
		for K=1:Pmax
		*/
		for( K = 1; K <= pmax; ++K )
		{
			int L;
			
			/*MATLAB
			[C(:,K*M+(1:M)),n] = covm(Y(K+1:N,:),Y(1:N-K,:),'M');
            		C(:,K*M+(1:M)) = C(:,K*M+(1:M))/.n;
			D = C(:,K*M+(1:M));
			*/
			gsl_matrix *cov1 		= gsl_matrix_part( Y, NULL, K, 0, N - 1, Y->size2 - 1, 0, 0 );	
			gsl_matrix *cov2 		= gsl_matrix_part( Y, NULL, 0, 0, N - K - 1, Y->size2 - 1, 0, 0 );	
			gsl_matrix *cov			= NULL;
			gsl_matrix *D 			= NULL;					
			double		dn1;	
			gsl_matrix_cov( cov1, cov2, &cov, &dn1 );
			gsl_matrix_part( cov, &C, 0, 0, cov->size1 - 1, cov->size2 - 1, 0, K * M );	
			gsl_matrix_part( C, &D, 0, K * M, C->size1 - 1, K * M + M - 1, 0, 0 );
			gsl_matrix_scale( D, 1.0 / dn1 );
			gsl_matrix_part( D, &C, 0, 0, D->size1 - 1, D->size2 - 1, 0, K * M );	
			gsl_matrix_free( cov );	
			gsl_matrix_free( cov2 );
			gsl_matrix_free( cov1 );	
			
			/*MATLAB
			for L = 1:K-1,
            		D = D - ARF(:,L*M+(1-M:0))*C(:,(K-L)*M+(1:M));
            		end;
			*/
			for( L = 1; L <= K - 1; ++L )
			{
				gsl_matrix *partARF = NULL;
				gsl_matrix *partC 	= NULL;
				gsl_matrix *prod 	= NULL;
				gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				gsl_matrix_part( C, &partC, 0, ( K - L ) * M, C->size1 - 1, ( K - L ) * M + M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partC );
				gsl_matrix_sub( D, prod );
				gsl_matrix_free( prod );
				gsl_matrix_free( partC );
				gsl_matrix_free( partARF );
			}
		
			/*MATLAB
			ARF(:,K*M+(1-M:0)) = D / PEB;	
            		ARB(:,K*M+(1-M:0)) = D'/ PEF;
			*/
			{
				gsl_matrix *invPEB 	= NULL;
				gsl_matrix *invPEF 	= NULL;
				gsl_matrix *prod1 	= NULL;
				gsl_matrix *prod2 	= NULL;
				invPEB 	= gsl_matrix_inverse( PEB );
				invPEF 	= gsl_matrix_inverse( PEF );			
				prod1 = gsl_matrix_multiply( D, invPEB );
				prod2 = gsl_matrix_multiply_trans_notrans( D, invPEF );
				gsl_matrix_part( prod1, ARF, 0, 0, prod1->size1 - 1, prod1->size2 - 1, 0, K * M - M );
				gsl_matrix_part( prod2, &ARB, 0, 0, prod2->size1 - 1, prod2->size2 - 1, 0, K * M - M );

				gsl_matrix_free( prod2 );
				gsl_matrix_free( prod1 );							
				gsl_matrix_free( invPEF );			
				gsl_matrix_free( invPEB );
			}
			
			/*MATLAB
			for L = 1:K-1,
            		tmp                    = ARF(:,L*M+(1-M:0)) - ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
            		ARB(:,(K-L)*M+(1-M:0)) = ARB(:,(K-L)*M+(1-M:0)) - ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
            		ARF(:,L*M+(1-M:0))     = tmp;
            		end;
			*/
			for( L = 1; L <= K-1; ++L )
			{
				gsl_matrix *tmp 	= NULL;
				gsl_matrix *partARF = NULL;
				gsl_matrix *partARB = NULL;
				gsl_matrix *prod 	= NULL;
				
				gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part(  ARB, &partARB,  0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partARB );
				gsl_matrix_sub( tmp, prod );

				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );
				partARF = partARB = prod = NULL;
				
				gsl_matrix_part( ARB,  &partARB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARB, partARF );
				gsl_matrix_free( partARB );
				partARB = NULL;
				gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
				gsl_matrix_sub( partARB, prod );
				gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
				gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, ( L * M ) - M );
				
				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );				
				gsl_matrix_free( tmp );
			}
		
			/*MATLAB
			RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
			RCB(:,K*M+(1-M:0)) = ARB(:,K*M+(1-M:0));
			*/
			gsl_matrix_part( *ARF,  RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
			gsl_matrix_part(  ARB, &RCB, 0, K * M - M, ARB->size1 - 1,    K * M - 1, 0, K * M - M );
			
			/*MATLAB
			PEF = [eye(M) - ARF(:,K*M+(1-M:0))*ARB(:,K*M+(1-M:0))]*PEF;
           	PEB = [eye(M) - ARB(:,K*M+(1-M:0))*ARF(:,K*M+(1-M:0))]*PEB;
            PE(:,K*M+(1:M)) = PEF; 
			*/
			{
				gsl_matrix *identity 	= gsl_matrix_alloc( M, M );
				gsl_matrix *partARF		= NULL;
				gsl_matrix *partARB		= NULL;
				gsl_matrix *prod		= NULL;
				gsl_matrix *cpyPEF		= gsl_matrix_alloc( PEF->size1, PEF->size2 );
				gsl_matrix *cpyPEB		= gsl_matrix_alloc( PEB->size1, PEB->size2 );

				gsl_matrix_memcpy( cpyPEF, PEF );
				gsl_matrix_memcpy( cpyPEB, PEB );
				gsl_matrix_free( PEF );
				gsl_matrix_free( PEB );
				gsl_matrix_set_identity( identity );
				gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
				gsl_matrix_part(  ARB, &partARB, 0, K * M - M, ARB->size1 - 1,    K * M - 1, 0, 0 );
				prod = gsl_matrix_multiply( partARF, partARB );
				gsl_matrix_sub( identity, prod );
				PEF = gsl_matrix_multiply( identity, cpyPEF );
				gsl_matrix_free( prod );
				prod = NULL;
				
				gsl_matrix_set_identity( identity );
				prod = gsl_matrix_multiply( partARB, partARF );
				gsl_matrix_sub( identity, prod );
				PEB = gsl_matrix_multiply( identity, cpyPEB );

				gsl_matrix_part( PEB, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );

				gsl_matrix_free( cpyPEB );
				gsl_matrix_free( cpyPEF );
				gsl_matrix_free( prod );
				gsl_matrix_free( partARB );
				gsl_matrix_free( partARF );
				gsl_matrix_free( identity );
			}
		
			gsl_matrix_free( D );
		}

		break;

		case arfit_mode_vmbiased:	
				/*
					Partial Correlation Estimation: Vieira-Morf Method with biased covariance estimation
					("Nutall-Strand with biased covariance" in Schloegls Vergleich von AR Estimation Algorithmen)
				*/
		
		{
			gsl_matrix *F = NULL, *B = NULL;

			/*MATLAB
			F = Y;
        	B = Y;
        	PEF = C(:,1:M);
        	PEB = C(:,1:M);
			*/
			F = gsl_matrix_alloc( Y->size1, Y->size2 );
			B = gsl_matrix_alloc( Y->size1, Y->size2 );
			gsl_matrix_memcpy( F, Y );
			gsl_matrix_memcpy( B, Y );
			gsl_matrix_part( C, &PEF, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( C, &PEB, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			
			/*MATLAB
			for K=1:Pmax,
			*/
			for( K = 1; K <= pmax; ++K )
			{
				int L;
				
				/*MATLAB
				[D,n]	= covm(F(K+1:N,:),B(1:N-K,:),'M');
				*/
				gsl_matrix *D 		= NULL;
				gsl_matrix *cov1 	= NULL;
				gsl_matrix *cov2 	= NULL;
				gsl_matrix_part( F, &cov1, K, 0, N - 1, F->size2 - 1, 0, 0 );
				gsl_matrix_part( B, &cov2, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
				gsl_matrix_cov( cov1, cov2, &D, NULL );
				gsl_matrix_free( cov2 );
				gsl_matrix_free( cov1 );
				
				/*MATLAB
				ARF(:,K*M+(1-M:0)) = D / PEB;	
                ARB(:,K*M+(1-M:0)) = D'/ PEF;
				*/
				{
					gsl_matrix *invPEB 	= NULL;
					gsl_matrix *invPEF 	= NULL;
					gsl_matrix *prod	= NULL;
					invPEB 				= gsl_matrix_inverse( PEB );
					prod 				= gsl_matrix_multiply( D, invPEB );
					gsl_matrix_part( prod, ARF, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					gsl_matrix_free( prod );
					prod 				= NULL;
					invPEF 				= gsl_matrix_inverse( PEF );
					prod				= gsl_matrix_multiply_trans_notrans( D, invPEF );
					gsl_matrix_part( prod, &ARB, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( invPEF );
					gsl_matrix_free( invPEB );
				}
				
				/*MATLAB
				tmp        = F(K+1:N,:) - B(1:N-K,:)*ARF(:,K*M+(1-M:0)).';
                B(1:N-K,:) = B(1:N-K,:) - F(K+1:N,:)*ARB(:,K*M+(1-M:0)).';
                F(K+1:N,:) = tmp;
				*/
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partB	= NULL;
					gsl_matrix *partF	= NULL;
					gsl_matrix *part2	= NULL;
					gsl_matrix *tpart2 	= NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( B, &partB, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_part( F, &partF, K, 0, N - 1, F->size2 - 1, 0, 0 );
					tmp = gsl_matrix_alloc( partF->size1, partF->size2 );
					gsl_matrix_memcpy( tmp, partF );
					
					gsl_matrix_part( *ARF, &part2, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					tpart2 = gsl_matrix_alloc( part2->size2, part2->size1 );
					gsl_matrix_transpose_memcpy( tpart2, part2 );
					prod = gsl_matrix_multiply( partB, tpart2 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( part2 ); 	part2 	= NULL;
					gsl_matrix_free( prod ); 	prod 	= NULL;
					gsl_matrix_free( tpart2 );	tpart2	= NULL;
					gsl_matrix_part( ARB, &part2, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					tpart2 = gsl_matrix_alloc( part2->size2, part2->size1 );
					gsl_matrix_transpose_memcpy( tpart2, part2 );
					prod = gsl_matrix_multiply( partF, tpart2 );
					gsl_matrix_sub( partB, prod );
					gsl_matrix_part( partB, &B, 0, 0, partB->size1 - 1, partB->size2 - 1, 0, 0 );
					gsl_matrix_part( tmp, &F, 0, 0, tmp->size1 - 1, tmp->size2 - 1, K, 0 );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( tpart2 );
					gsl_matrix_free( part2 );
					gsl_matrix_free( partF );
					gsl_matrix_free( partB );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				for L = 1:K-1,
					tmp						= ARF(:,L*M+(1-M:0))		- ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
                    ARB(:,(K-L)*M+(1-M:0))	= ARB(:,(K-L)*M+(1-M:0))	- ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
                    ARF(:,L*M+(1-M:0))		= tmp;
                end;
				*/
				for( L = 1; L <= K - 1; ++L )
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partARF = NULL;
					gsl_matrix *partARB = NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARF, partARB );
					gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( prod );	prod	= NULL;
					gsl_matrix_free( partARF );	partARF	= NULL;
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARB, partARF );
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					gsl_matrix_sub( partARB, prod );
					gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
					gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, L * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( partARB );
					gsl_matrix_free( partARF );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
                RCB(:,K*M+(1-M:0)) = ARB(:,K*M+(1-M:0));
				*/
				gsl_matrix_part( *ARF, RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
				gsl_matrix_part( ARB, &RCB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, K * M - M );
				
				/*MATLAB
				[PEB,n] 		= covm(B(1:N-K,:),B(1:N-K,:),'M');
				[PEF,n] 		= covm(F(K+1:N,:),F(K+1:N,:),'M');
				PE(:,K*M+(1:M)) = PEF./n;
				*/
				{
					gsl_matrix	*cov 	= NULL;
					double		dn2;
					
					gsl_matrix_part( B, &cov, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEB, NULL );

					gsl_matrix_free( cov ); cov	= NULL;
					
					gsl_matrix_part( F, &cov, K, 0, N - 1, F->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEF, &dn2 );
					gsl_matrix_free( cov );
					cov = gsl_matrix_alloc( PEF->size1, PEF->size2 );
					gsl_matrix_memcpy( cov, PEF );
					gsl_matrix_scale( cov, 1.0 / dn2 );
					gsl_matrix_part( cov, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );
					gsl_matrix_free( cov );
					
				}
				
				gsl_matrix_free( D );
			}
			
			gsl_matrix_free( B );
			gsl_matrix_free( F );
		}
		
		break;

		case arfit_mode_nsbiased:	
				/*
					Partial Correlation Estimation: Nutall-Strand Method with biased covariance estimation
				*/
		
		{
			gsl_matrix *F = NULL, *B = NULL;

			/*MATLAB
			F = Y;
			B = Y;
        	PEF = C(:,1:M);
        	PEB = C(:,1:M);
			*/
			F = gsl_matrix_alloc( Y->size1, Y->size2 );
			B = gsl_matrix_alloc( Y->size1, Y->size2 );
			gsl_matrix_memcpy( F, Y );
			gsl_matrix_memcpy( B, Y );
			gsl_matrix_part( C, &PEF, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( C, &PEB, 0, 0, C->size1 - 1, M - 1, 0, 0 );
			
			/*MATLAB
			for K=1:Pmax,
			*/
			for( K = 1; K <= pmax; ++K )
			{
				int L;
				
				/*MATLAB
				[D,n]	= covm(F(K+1:N,:),B(1:N-K,:),'M');
				D = D./N;
				*/
				gsl_matrix *D 		= NULL;
				gsl_matrix *cov1 	= NULL;
				gsl_matrix *cov2 	= NULL;
				gsl_matrix_part( F, &cov1, K, 0, N - 1, F->size2 - 1, 0, 0 );
				gsl_matrix_part( B, &cov2, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
				gsl_matrix_cov( cov1, cov2, &D, NULL );
				gsl_matrix_scale( D, 1.0 / (double) N );
				gsl_matrix_free( cov2 );
				gsl_matrix_free( cov1 );
				
				/*MATLAB
				ARF(:,K*M+(1-M:0)) = 2*D / (PEB+PEF);	
                ARB(:,K*M+(1-M:0)) = 2*D'/ (PEF+PEB);
				*/
				{
					gsl_matrix *PEFB	= gsl_matrix_alloc( PEF->size1, PEF->size2 );
					gsl_matrix *invPEFB	= NULL;
					gsl_matrix *prod	= NULL;
					gsl_matrix_scale( D, 2.0 );
					gsl_matrix_memcpy( PEFB, PEF );
					gsl_matrix_add( PEFB, PEB );
					invPEFB 			= gsl_matrix_inverse( PEFB );
					prod 				= gsl_matrix_multiply( D, invPEFB );
					gsl_matrix_part( prod, ARF, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_free( prod ); prod = NULL;
					
					prod				= gsl_matrix_multiply_trans_notrans( D, invPEFB );
					gsl_matrix_part( prod, &ARB, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_scale( D, 0.5 );

					gsl_matrix_free( prod );
					gsl_matrix_free( invPEFB );
					gsl_matrix_free( PEFB );
				}
				
				/*MATLAB
				tmp        = F(K+1:N,:) - B(1:N-K,:)*ARF(:,K*M+(1-M:0)).';
                B(1:N-K,:) = B(1:N-K,:) - F(K+1:N,:)*ARB(:,K*M+(1-M:0)).';
                F(K+1:N,:) = tmp;
				*/
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partB	= NULL;
					gsl_matrix *partF	= NULL;
					gsl_matrix *part2	= NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( B, &partB, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_part( F, &partF, K, 0, N - 1, F->size2 - 1, 0, 0 );
					tmp = gsl_matrix_alloc( partF->size1, partF->size2 );
					gsl_matrix_memcpy( tmp, partF );
					
					gsl_matrix_part( *ARF, &part2, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					prod = gsl_matrix_multiply_notrans_trans( partB, part2 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( part2 ); 	part2 	= NULL;
					gsl_matrix_free( prod ); 	prod 	= NULL;
					gsl_matrix_part( ARB, &part2, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					prod = gsl_matrix_multiply_notrans_trans( partF, part2 );
					gsl_matrix_sub( partB, prod );
					gsl_matrix_part( partB, &B, 0, 0, partB->size1 - 1, partB->size2 - 1, 0, 0 );
					gsl_matrix_part( tmp, &F, 0, 0, tmp->size1 - 1, tmp->size2 - 1, K, 0 );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( part2 );
					gsl_matrix_free( partF );
					gsl_matrix_free( partB );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				for L = 1:K-1,
                   	tmp      				= ARF(:,L*M+(1-M:0))   		- ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
                   	ARB(:,(K-L)*M+(1-M:0)) 	= ARB(:,(K-L)*M+(1-M:0)) 	- ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
                   	ARF(:,L*M+(1-M:0))   	= tmp;
                end;
				*/
				for( L = 1; L <= K - 1; ++L )
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partARF = NULL;
					gsl_matrix *partARB = NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part(  ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARF, partARB );
					gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( prod );	prod	= NULL;
					gsl_matrix_free( partARF );	partARF	= NULL;
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part(  ARB, &partARB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARB, partARF );
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					gsl_matrix_sub( partARB, prod );
					gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
					gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, L * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( partARB );
					gsl_matrix_free( partARF );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
				*/
				gsl_matrix_part( *ARF, RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
				
				/*MATLAB
				[PEF,n] 			= covm(F(K+1:N,:),F(K+1:N,:),'M');
				PEF 				= PEF./N;
				
				[PEB,n] 			= covm(B(1:N-K,:),B(1:N-K,:),'M');
                PEB 				= PEB./N;
       
                PE(:,K*M+(1:M)) 	= PEF./n;
				*/
				{
					gsl_matrix	*cov 	= NULL;
					gsl_matrix	*part	= NULL;
					double		dn2;

					gsl_matrix_part( F, &cov, K, 0, N - 1, F->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEF, NULL );
					gsl_matrix_scale( PEF, 1.0 / (double) N );
					
					gsl_matrix_free( cov ); cov = NULL;
					
					gsl_matrix_part( B, &cov, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEB, &dn2 );
					gsl_matrix_scale( PEB, 1.0 / (double) N );
					
					gsl_matrix_part( PEF, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );
					gsl_matrix_part( *PE, &part, 0, K * M, (*PE)->size1 - 1, K * M + M - 1, 0, 0 );
					gsl_matrix_scale( part, 1.0 / dn2 );
					gsl_matrix_part( part, PE, 0, 0, part->size1 - 1, part->size2 - 1, 0, K * M );
					
					gsl_matrix_free( part );
					gsl_matrix_free( cov );
				}
				
				gsl_matrix_free( D );
			}
			
			gsl_matrix_free( B );
			gsl_matrix_free( F );
		}
		
		break;

		case arfit_mode_nsunbiased:	
				/*
					Partial Correlation Estimation: Nutall-Strand Method with unbiased covariance estimation
				*/
		
		{
			gsl_matrix *F = NULL, *B = NULL;

			/*MATLAB
			F = Y;
        	B = Y;
        	PEF = PE(:,1:M);
        	PEB = PE(:,1:M);
			*/
			F = gsl_matrix_alloc( Y->size1, Y->size2 );
			B = gsl_matrix_alloc( Y->size1, Y->size2 );
			gsl_matrix_memcpy( F, Y );
			gsl_matrix_memcpy( B, Y );
			gsl_matrix_part( *PE, &PEF, 0, 0, (*PE)->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( *PE, &PEB, 0, 0, (*PE)->size1 - 1, M - 1, 0, 0 );

			/*MATLAB
			for K=1:Pmax,
			*/
			for( K = 1; K <= pmax; ++K )
			{
				int L;
				
				/*MATLAB
				[D,n]	= covm(F(K+1:N,:),B(1:N-K,:),'M');
				D = D./N;
				*/
				double		dn;
				gsl_matrix *D 		= NULL;
				gsl_matrix *cov1 	= NULL;
				gsl_matrix *cov2 	= NULL;
				gsl_matrix_part( F, &cov1, K, 0, N - 1, F->size2 - 1, 0, 0 );
				gsl_matrix_part( B, &cov2, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
				gsl_matrix_cov( cov1, cov2, &D, &dn );
				gsl_matrix_scale( D, 1.0 / dn );
				gsl_matrix_free( cov2 );
				gsl_matrix_free( cov1 );

				/*MATLAB
				ARF(:,K*M+(1-M:0)) = 2*D / (PEB+PEF);	
                ARB(:,K*M+(1-M:0)) = 2*D'/ (PEF+PEB);
				*/
				{
					gsl_matrix *PEFB	= gsl_matrix_alloc( PEF->size1, PEF->size2 );
					gsl_matrix *invPEFB	= NULL;
					gsl_matrix *prod	= NULL;
					gsl_matrix_scale( D, 2.0 );
					gsl_matrix_memcpy( PEFB, PEF );
					gsl_matrix_add( PEFB, PEB );
					invPEFB 			= gsl_matrix_inverse( PEFB );
					prod 				= gsl_matrix_multiply( D, invPEFB );
					gsl_matrix_part( prod, ARF, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_free( prod ); prod = NULL;
					
					prod				= gsl_matrix_multiply_trans_notrans( D, invPEFB );
					gsl_matrix_part( prod, &ARB, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_scale( D, 0.5 );

					gsl_matrix_free( prod );
					gsl_matrix_free( invPEFB );
					gsl_matrix_free( PEFB );
				}

				/*MATLAB
				tmp        = F(K+1:N,:) - B(1:N-K,:)*ARF(:,K*M+(1-M:0)).';
                B(1:N-K,:) = B(1:N-K,:) - F(K+1:N,:)*ARB(:,K*M+(1-M:0)).';
                F(K+1:N,:) = tmp;
				*/
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partB	= NULL;
					gsl_matrix *partF	= NULL;
					gsl_matrix *part2	= NULL;
					gsl_matrix *tpart2 	= NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( B, &partB, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_part( F, &partF, K, 0, N - 1, F->size2 - 1, 0, 0 );
					tmp = gsl_matrix_alloc( partF->size1, partF->size2 );
					gsl_matrix_memcpy( tmp, partF );
					
					gsl_matrix_part( *ARF, &part2, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					tpart2 = gsl_matrix_alloc( part2->size2, part2->size1 );
					gsl_matrix_transpose_memcpy( tpart2, part2 );
					prod = gsl_matrix_multiply( partB, tpart2 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( part2 ); 	part2 	= NULL;
					gsl_matrix_free( prod ); 	prod 	= NULL;
					gsl_matrix_free( tpart2 );	tpart2	= NULL;
					gsl_matrix_part( ARB, &part2, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					tpart2 = gsl_matrix_alloc( part2->size2, part2->size1 );
					gsl_matrix_transpose_memcpy( tpart2, part2 );
					prod = gsl_matrix_multiply( partF, tpart2 );
					gsl_matrix_sub( partB, prod );
					gsl_matrix_part( partB, &B, 0, 0, partB->size1 - 1, partB->size2 - 1, 0, 0 );
					gsl_matrix_part( tmp, &F, 0, 0, tmp->size1 - 1, tmp->size2 - 1, K, 0 );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( tpart2 );
					gsl_matrix_free( part2 );
					gsl_matrix_free( partF );
					gsl_matrix_free( partB );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				for L = 1:K-1,
					tmp      = ARF(:,L*M+(1-M:0))   - ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
					ARB(:,(K-L)*M+(1-M:0)) = ARB(:,(K-L)*M+(1-M:0)) - ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
					ARF(:,L*M+(1-M:0))   = tmp;
				end;
				*/
				for( L = 1; L <= K - 1; ++L )
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partARF = NULL;
					gsl_matrix *partARB = NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARF, partARB );
					gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( prod );	prod	= NULL;
					gsl_matrix_free( partARF );	partARF	= NULL;
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARB, partARF );
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					gsl_matrix_sub( partARB, prod );
					gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
					gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, L * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( partARB );
					gsl_matrix_free( partARF );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
				*/
				gsl_matrix_part( *ARF, RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
				
				/*MATLAB
				[PEF,n] 			= covm(F(K+1:N,:),F(K+1:N,:),'M');
				PEF 				= PEF./n;
				
				[PEB,n] 			= covm(B(1:N-K,:),B(1:N-K,:),'M');
                PEB 				= PEB./n;
       
                PE(:,K*M+(1:M)) 	= PEF;
				*/
				{
					gsl_matrix	*cov 	= NULL;
					double		dn2;
					
					gsl_matrix_part( F, &cov, K, 0, N - 1, F->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEF, &dn2 );
					gsl_matrix_scale( PEF, 1.0 / dn2 );
					
					gsl_matrix_free( cov ); cov = NULL;
					
					gsl_matrix_part( B, &cov, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEB, &dn2 );
					gsl_matrix_scale( PEB, 1.0 / dn2 );
					
					gsl_matrix_part( PEF, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );					
					
					gsl_matrix_free( cov );
				}
				gsl_matrix_free( D );
			}
			gsl_matrix_free( B );
			gsl_matrix_free( F );
		}
		
		break;
		
		case arfit_mode_notset:
		case arfit_mode_vmunbiased:	
		default:
				/* 
					Partial Correlation Estimation: Vieira-Morf Method with unbiased covariance estimation
					("Nutall-Strand with unbiased covariance" in Schloegls Vergleich von AR Estimation Algorithmen )
				*/
		
		{
			gsl_matrix *F = NULL, *B = NULL;
			
			/*MATLAB
			F = Y;
        	B = Y;
        	PEF = PE(:,1:M);
       	 	PEB = PE(:,1:M);
			*/
			F = gsl_matrix_alloc( Y->size1, Y->size2 );
			B = gsl_matrix_alloc( Y->size1, Y->size2 );
			gsl_matrix_memcpy( F, Y );
			gsl_matrix_memcpy( B, Y );
			gsl_matrix_part( *PE, &PEF, 0, 0, (*PE)->size1 - 1, M - 1, 0, 0 );
			gsl_matrix_part( *PE, &PEB, 0, 0, (*PE)->size1 - 1, M - 1, 0, 0 );
			
			/*MATLAB
			for K=1:Pmax,
			*/
			for( K = 1; K <= pmax; ++K )
			{
				int L;
				
				/*MATLAB
				[D,n]	= covm(F(K+1:N,:),B(1:N-K,:),'M');
                D = D./n;
				*/
				gsl_matrix *D 		= NULL;
				gsl_matrix *cov1 	= NULL;
				gsl_matrix *cov2 	= NULL;
				double		dn1;
				gsl_matrix_part( F, &cov1, K, 0, N - 1, F->size2 - 1, 0, 0 );
				gsl_matrix_part( B, &cov2, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
				gsl_matrix_cov( cov1, cov2, &D, &dn1 );
				gsl_matrix_scale( D, 1.0 / dn1 );
				gsl_matrix_free( cov2 );
				gsl_matrix_free( cov1 );
				
				/*MATLAB
				ARF(:,K*M+(1-M:0)) = D / PEB;	
                ARB(:,K*M+(1-M:0)) = D'/ PEF;
				*/
				{
					gsl_matrix *invPEB 	= NULL;
					gsl_matrix *invPEF 	= NULL;
					gsl_matrix *prod	= NULL;
					invPEB 				= gsl_matrix_inverse( PEB );
					prod 				= gsl_matrix_multiply( D, invPEB );
					gsl_matrix_part( prod,  ARF, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					gsl_matrix_free( prod ); prod = NULL;
					invPEF 				= gsl_matrix_inverse( PEF );
					prod				= gsl_matrix_multiply_trans_notrans( D, invPEF );
					gsl_matrix_part( prod, &ARB, 0, 0, prod->size1 - 1, prod->size2 - 1, 0, K * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( invPEF );
					gsl_matrix_free( invPEB );
				}
				
				/*MATLAB
				tmp        = F(K+1:N,:) - B(1:N-K,:)*ARF(:,K*M+(1-M:0)).';
                B(1:N-K,:) = B(1:N-K,:) - F(K+1:N,:)*ARB(:,K*M+(1-M:0)).';
                F(K+1:N,:) = tmp;
				*/
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partB	= NULL;
					gsl_matrix *partF	= NULL;
					gsl_matrix *part2	= NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( B, &partB, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_part( F, &partF, K, 0, N - 1, F->size2 - 1, 0, 0 );
					tmp = gsl_matrix_alloc( partF->size1, partF->size2 );
					gsl_matrix_memcpy( tmp, partF );
					
					gsl_matrix_part( *ARF, &part2, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					prod = gsl_matrix_multiply_notrans_trans( partB, part2 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( part2 ); 	part2 	= NULL;
					gsl_matrix_free( prod ); 	prod 	= NULL;
					gsl_matrix_part( ARB, &part2, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, 0 );
					prod = gsl_matrix_multiply_notrans_trans( partF, part2 );
					gsl_matrix_sub( partB, prod );
					gsl_matrix_part( partB, &B, 0, 0, partB->size1 - 1, partB->size2 - 1, 0, 0 );
					gsl_matrix_part( tmp, &F, 0, 0, tmp->size1 - 1, tmp->size2 - 1, K, 0 );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( part2 );
					gsl_matrix_free( partF );
					gsl_matrix_free( partB );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				for L = 1:K-1,
					tmp						= ARF(:,L*M+(1-M:0))     - ARF(:,K*M+(1-M:0))*ARB(:,(K-L)*M+(1-M:0));
					ARB(:,(K-L)*M+(1-M:0))	= ARB(:,(K-L)*M+(1-M:0)) - ARB(:,K*M+(1-M:0))*ARF(:,L*M+(1-M:0));
					ARF(:,L*M+(1-M:0))		= tmp;
                end;
				*/
				for( L = 1; L <= K - 1; ++L )
				{
					gsl_matrix *tmp 	= NULL;
					gsl_matrix *partARF = NULL;
					gsl_matrix *partARB = NULL;
					gsl_matrix *prod	= NULL;
					
					gsl_matrix_part( *ARF, &partARF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, 0 );
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARF, partARB );
					gsl_matrix_part( *ARF, &tmp, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					gsl_matrix_sub( tmp, prod );
					gsl_matrix_free( prod );	prod	= NULL;
					gsl_matrix_free( partARF );	partARF	= NULL;
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB,  &partARB, 0, K * M - M, ARB->size1 - 1,    K * M - 1, 0, 0 );
					gsl_matrix_part( *ARF, &partARF, 0, L * M - M, (*ARF)->size1 - 1, L * M - 1, 0, 0 );
					prod = gsl_matrix_multiply( partARB, partARF );
					gsl_matrix_free( partARB ); partARB = NULL;
					gsl_matrix_part( ARB, &partARB, 0, ( K - L ) * M - M, ARB->size1 - 1, ( K - L ) * M - 1, 0, 0 );
					gsl_matrix_sub( partARB, prod );
					gsl_matrix_part( partARB, &ARB, 0, 0, partARB->size1 - 1, partARB->size2 - 1, 0, ( K - L ) * M - M );
					gsl_matrix_part( tmp, ARF, 0, 0, tmp->size1 - 1, tmp->size2 - 1, 0, L * M - M );
					
					gsl_matrix_free( prod );
					gsl_matrix_free( partARB );
					gsl_matrix_free( partARF );
					gsl_matrix_free( tmp );
				}
				
				/*MATLAB
				RCF(:,K*M+(1-M:0)) = ARF(:,K*M+(1-M:0));
                RCB(:,K*M+(1-M:0)) = ARB(:,K*M+(1-M:0));
				*/
				gsl_matrix_part( *ARF, RCF, 0, K * M - M, (*ARF)->size1 - 1, K * M - 1, 0, K * M - M );
				gsl_matrix_part( ARB, &RCB, 0, K * M - M, ARB->size1 - 1, K * M - 1, 0, K * M - M );
				
				/*MATLAB
				[PEF,n] = covm(F(K+1:N,:),F(K+1:N,:),'M');
                PEF = PEF./n;
				
				[PEB,n] = covm(B(1:N-K,:),B(1:N-K,:),'M');
                PEB = PEB./n;
				
				PE(:,K*M+(1:M)) = PEF;
				*/
				{
					gsl_matrix *cov 	= NULL;
					double		dn2;
					
					gsl_matrix_part( F, &cov, K, 0, N - 1, F->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEF, &dn2 );
					gsl_matrix_scale( PEF, 1.0 / dn2 );

					gsl_matrix_free( cov ); cov	= NULL;
					
					gsl_matrix_part( B, &cov, 0, 0, N - K - 1, B->size2 - 1, 0, 0 );
					gsl_matrix_cov( cov, cov, &PEB, &dn2 );
					gsl_matrix_scale( PEB, 1.0 / dn2 );

					gsl_matrix_free( cov );
					
					gsl_matrix_part( PEF, PE, 0, 0, PEF->size1 - 1, PEF->size2 - 1, 0, K * M );
				}
				
				gsl_matrix_free( D );
			}
			
			gsl_matrix_free( B );
			gsl_matrix_free( F );
		}
		
		break;
	}

	if( PEF ) gsl_matrix_free( PEF );
	if( PEB ) gsl_matrix_free( PEB );
	if( ARB ) gsl_matrix_free( ARB );
	if( RCB ) gsl_matrix_free( RCB );
	
	gsl_matrix_free( C );

	return 0;
}


/* arfit_schloegl function
	estimate ar model values
*/
arfit_output *arfit_schloegl( arfit_input *input, arfit_mode mode, arfit_output **output )
{
	int M, N, i, j, iopt, icomplex = 0;
	double value = 0.0, sum1 = 0.0, sum2 = 0.0;
	gsl_matrix *MAR = NULL, *RCF = NULL, *PE = NULL;
	gsl_vector *ne = NULL, *mean = NULL;
	gsl_vector_complex *logdp = NULL;

	/* check parameters */
	if( !input )
	{
		fprintf( stderr, "Invalid input argument" );
		return NULL;
	}
	if( input->pmax < input->pmin )
	{
		fprintf( stderr, "pmax must be greater than or equal to pmin. pmax set to pmin" );
		input->pmax = input->pmin;
	}
	if( input->zero > 1 )
	{
		fprintf( stderr, "zero is greater than 1. zero set to 1" );
		input->zero = 1;
	}
	else if( input->zero < 0 )
	{
		fprintf( stderr, "zero is less than 0. zero set to 0" );
		input->zero = 0;
	}

	/* check if output is already set so that it has to be released */
	if( *output )
		arfit_output_free( *output );

	/* allocate memory for output structure */
	arfit_output_alloc( *output );

	/* allocate output matrices and initiliaze them with 0 */
	(*output)->A = gsl_matrix_calloc( input->v->size2, input->pmax );
	(*output)->C = gsl_matrix_calloc( input->v->size2, input->v->size2 );

	/* vector holding mean value of every input channel */
	mean = gsl_vector_calloc( input->v->size2 );
	
	/* thresholding and normalization */
	for( i = 0; i < input->v->size2; ++i )
	{
		value 	= 0.0;
		sum1	= 0.0;
		sum2	= 0.0;
		for( j = 0; j < input->v->size1; ++j )
		{
			value 	 = gsl_matrix_get( input->v, j, i );
			sum1 	+= value;
			sum2  	+= value * value; 
		}
		sum1 	/= (double) input->v->size1;
		sum2	 = sqrt( ( sum2 - (double) input->v->size1 * sum1 * sum1 ) / (double)( input->v->size1 - 1 ) );
		gsl_vector_set( mean, i, sum1 );
		
		if( sum1 <= input->threshold || sum1 == 0.0 )
		{
			(*output)->sbc			= gsl_vector_calloc( 1 );
			(*output)->fpe 			= gsl_vector_calloc( 1 );
			(*output)->w			= gsl_vector_calloc( input->v->size2 );
			(*output)->iprocessed 	= 0;
			
			return *output;
		}
		else
		{
			for( j = 0; j < input->v->size1; ++j )
			{
				if( sum2 > 1 )
					value = ( gsl_matrix_get( input->v, j, i ) - sum1 ) / sum2;
				else
					value = 0.0;

				gsl_matrix_set( input->v, j, i, value );
			}
		}
	}

	/* transpose matrix to fit in common input scheme */

	N		= input->v->size1;
	M		= input->v->size2;

	/* estimate ar process for maximum order */
	mvar( input->v, input->pmax, mode, &MAR, &RCF, &PE, NULL, NULL );

	ne = gsl_vector_alloc( input->pmax - input->pmin + 1 );
	for( i = 0; i < ne->size; ++i )
		gsl_vector_set( ne, i, N - input->zero - ( input->pmin + i ) );

	logdp = gsl_vector_complex_alloc( input->pmax - input->pmin + 1 );
	for( i = input->pmin; i <= input->pmax; ++i )
	{   
		/* MATLAB
		logdp(p-pmin+1) = log(det(PE(:,p+(1:M))*(N-p-mcor)));
		*/
		double det;
		
		gsl_matrix *partPE 	= NULL;
		
		gsl_matrix_part( PE, &partPE, 0, i, PE->size1 - 1, i + M - 1, 0, 0 );
		gsl_matrix_scale( partPE, N - i - input->zero );
		
		det = gsl_matrix_det( partPE );
		if( det <= 0.0 )
		{
			gsl_complex zdet, zlog;
			GSL_SET_COMPLEX( &zdet, det, 0.0 );
			zlog = gsl_complex_log( zdet );
			gsl_vector_complex_set( logdp, i - input->pmin, zlog );
			icomplex = 1;
		}
		else
		{
			gsl_complex z;
			GSL_SET_COMPLEX( &z, gsl_sf_log( det ), 0.0 );
			gsl_vector_complex_set( logdp, i - input->pmin, z );
		}
		
		gsl_matrix_free( partPE );
	}

	/*MATLAB
	sbc = logdp/M - log(ne) .* (1-(M*(pmin:pmax)+mcor)./ne);
	fpe = logdp/M - log(ne.*(ne-M*(pmin:pmax)-mcor)./(ne+M*(pmin:pmax)+mcor));
	*/
	(*output)->sbc = gsl_vector_alloc( logdp->size );
	(*output)->fpe = gsl_vector_alloc( logdp->size );
	for( i = 0; i < logdp->size; ++i )
		gsl_vector_complex_set( logdp, i, gsl_complex_mul_real( gsl_vector_complex_get( logdp, i ), 1.0 / (double) M ) );
		
	for( i = 0; i < logdp->size; ++i )
	{
		gsl_complex z1, sz1;
		gsl_complex z2, sz2;
		GSL_SET_COMPLEX( &z1, gsl_sf_log( gsl_vector_get( ne, i ) ) * ( 1.0 - ( (double) M * ( input->pmin + i ) + input->zero ) / gsl_vector_get( ne, i ) ), 0.0 );
		GSL_SET_COMPLEX( &z2, gsl_sf_log( gsl_vector_get( ne, i ) * ( gsl_vector_get( ne, i ) - ( (double)M * ( input->pmin + i ) - input->zero ) ) / ( gsl_vector_get( ne, i ) + ( (double)M * ( input->pmin + i ) + input->zero ) ) ), 0.0 );
		sz1 = gsl_complex_sub( gsl_vector_complex_get( logdp, i ), z1 );
		sz2 = gsl_complex_sub( gsl_vector_complex_get( logdp, i ), z2 );
		if( icomplex == 0 )
		{
			gsl_vector_set( (*output)->sbc, i, GSL_REAL( sz1 ) );
			gsl_vector_set( (*output)->fpe, i, GSL_REAL( sz2 ) );
		}
		else
		{
			gsl_vector_set( (*output)->sbc, i, gsl_complex_abs( sz1 ) );
			gsl_vector_set( (*output)->fpe, i, gsl_complex_abs( sz2 ) );
		}
	}
	
	/*MATLAB
	% get index iopt of order that minimizes the order selection 
	% criterion specified by the variable selector
	if strcmpi(selector,'fpe'); 
		[val, iopt]  = min(fpe); 
	else %if strcmpi(selector,'sbc'); 
		[val, iopt]  = min(sbc); 
	end;

	% select order of model
	popt = pmin + iopt-1; % estimated optimum order
	*/
	if( input->selector == arfit_selector_sbc )
		iopt = gsl_vector_min_index( (*output)->sbc );
	else
		iopt = gsl_vector_min_index( (*output)->fpe );
	(*output)->popt = input->pmin + iopt;
  
	/* check if optimal order is other than the maximum order and recalculate model parameters */
	/*MATLAB
	if popt<pmax, 
        [MAR, RCF, PE] = mvar(Y, popt, 2);
	end;
	*/
	if( (*output)->popt < input->pmax )
	{
		gsl_matrix_free( MAR );
		gsl_matrix_free( RCF );
		gsl_matrix_free( PE );
		MAR = NULL;
		RCF = NULL;
		PE	= NULL;
		mvar( input->v, (*output)->popt, mode, &MAR, &RCF, &PE, NULL, NULL );
	}
	
	/* compute rest term of ar process */
	/*MATLAB
	C = PE(:,size(PE,2)+(1-M:0));
	*/
	gsl_matrix_part( PE, &(*output)->C, 0, PE->size2 - M, PE->size1 - 1, PE->size2 - 1, 0, 0 );

	/* check if mean value intercept is needed */
	if( input->zero )
	{
		/*MATLAB
		I = eye(M);
		*/
		gsl_matrix *indent = gsl_matrix_alloc(M,M);
		gsl_matrix_set_identity( indent );

		/*MATLAB
		for k = 1:popt,
                I = I - MAR(:,k*M+(1-M:0));
        end;
		*/
		for( i = 1; i <= (*output)->popt; ++i )
		{
			gsl_matrix *partMAR = gsl_matrix_part( MAR, NULL, 0, i * M - M, MAR->size1 - 1, i * M - 1, 0, 0 );
			gsl_matrix_sub( indent, partMAR );
			gsl_matrix_free( partMAR );
		}

		(*output)->w = gsl_vector_alloc( M );

		/*MATLAB
		w = -I*m';
		*/
		gsl_blas_dgemv( CblasNoTrans, -1.0, indent, mean, 1.0, (*output)->w );
		/*gsl_matrix_scale( indent, -1.0 );
		for( i = 0; i < M; ++i )
			gsl_vector_set( (*output)->w, i, gsl_matrix_get( indent, i, i ) * gsl_vector_get( mean, i ) );*/
		
		gsl_matrix_free( indent );
	}
	else
		/*MATLAB
		w = zeros(M,1);
		*/
		(*output)->w = gsl_vector_calloc( M );

	/* copy ar model parameters to output matrix */
	gsl_matrix_part( MAR, &(*output)->A, 0, 0, MAR->size1 - 1, MAR->size2 - 1, 0, 0 );
	for( i = 0; i < (*output)->A->size1; ++i ) for( j = 0; j < (*output)->A->size2; ++j ) if( isnan( gsl_matrix_get( (*output)->A, i, j ) ) )
		gsl_matrix_set( (*output)->A, i, j, 0.0 );

	gsl_vector_free( mean );
	gsl_vector_complex_free( logdp );
	gsl_vector_free( ne );
	gsl_matrix_free( PE );
	gsl_matrix_free( RCF );
	gsl_matrix_free( MAR );

	return *output;
}
