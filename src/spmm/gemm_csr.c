#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "matrix_generator.h"
#include "zsim_hooks.h"

static void sgemm_csr(int M, int N, int K, struct csr A, struct csr B, struct csr C){
	int n, k, m, nnz=0;
	int a_nnz_pr, b_nnz_pr;
	int nnz_C;
	int a_i, b_i;
	int c1,c2;
	float a, b, sum;
	int pointer_A, pointer_B;
	c1 = 0;
	c2 = 0;
	nnz_C = 0;
	printf("M = %d, N = %d \n", M, N);
	//getchar();
	C.row_ptr = (int*)malloc((M+1)*sizeof(int));
	C.row_ptr[0] = 0;
	C.col_ptr = (int*)malloc(M*M*sizeof(int));
	for(m=0; m<M; m++){

		if( m == M/128) return;
		a_nnz_pr = A.row_ptr[m+1] - A.row_ptr[m];
		//printf("A has %d elements in line %d \n", a_nnz_pr, m);
		if(a_nnz_pr == 0 )		
		{	
			
			C.row_ptr[m+1] = C.row_ptr[m];
			continue;
		}
		c1++;
		for(n=0; n<N; n++){

			b_nnz_pr = B.col_ptr[n+1] - B.col_ptr[n];
			//printf("B has %d elements in line %d \n", b_nnz_pr, n);
			if(b_nnz_pr == 0){
				sum = 0;
				c2++;
			}
			else{
				nnz_C++;	    	
			

			C.row_ptr[m+1] = C.row_ptr[m];
			pointer_A = A.row_ptr[m];
			pointer_B = B.col_ptr[n];

		//	printf("Pointer A = %d \n", pointer_A);
		//	printf("Pointer B = %d \n", pointer_B);
			sum = 0;
			int c=0;
			for(int i = 0; i < a_nnz_pr;i++){
		//		printf("A_col_ptr[%d] = %d \n",pointer_A, A.col_ptr[pointer_A]);
		//		printf("B_row_ptr[%d] = %d \n",pointer_B, B.row_ptr[pointer_B]);
				if( A.col_ptr[pointer_A]  == B.row_ptr[pointer_B] ){
					sum += A.val[pointer_A]*B.val[pointer_B]; 
					pointer_A++;					
	//				printf("A_val[%d] = %f \n",pointer_A, A.val[pointer_A]);
	//				printf("B_val[%d] = %f \n",pointer_B, B.val[pointer_B]);
	//				printf("Sum = %f \n", sum);
				}
				else if( A.col_ptr[pointer_A] >  B.row_ptr[pointer_B] && b_nnz_pr!=1 ){
					
				
					do {
						c++;
						pointer_B++;
					}
					while( (A.col_ptr[pointer_A] >  B.row_ptr[pointer_B]) && b_nnz_pr >= c );


				}
				else if( A.col_ptr[pointer_A] <  B.row_ptr[pointer_B] ){

					pointer_A++;

				 }
				}
			}

			if( sum != 0 ){
				C.row_ptr[m+1]++;
				C.col_ptr[nnz_C]=n;
			}      
		}
	}

	printf("A had %d non zero rows \n", c1 );
	printf("B had %d non zero rows \n", c2 );

}



int main(int argc, char **argv) {




	char* path_A;
	char* path_B;

	if (argc == 3)
	{
		path_A = argv[1];
		path_B = argv[2];
	}

	int flag; 
	printf("Compute SGEMM using csr\n");
	struct csr A,B,C;



	A = read_csr(path_A);

	printf("Matrix A is ready");
	printf("B's path: %s \n", path_B);
	B = read_csc(path_B);
	
	zsim_roi_begin();
	sgemm_csr(A.size,A.size,A.size,A,B,C);
	zsim_roi_end();

}
