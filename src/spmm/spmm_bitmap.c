#include <stdlib.h>
#include <stdio.h>
#include "zsim_hooks.h"
#include <stdint.h>
#include "../bitmap/bitmap.h"
#include "../bitmap/csr.h"
#include <getopt.h>
#include <time.h> 
#include "zsim_hooks.h"

// Global Variables

int *C; // output matrix
smash matrix_A_smash; //smash matrix format
smash matrix_B_smash; //smash matrix format
char *path;
int K;
int comp0,comp1,comp2;



__attribute__ ((noinline)) int send_data_to_bmu(smash *format){ // Include bitmap.h in the simulator and catch routine to save smash format in the simulator using PIN_SafeCopy() (In Sniper you can do it using builtin functions).
	static int i=1;
	i = i+10;
	i = i -5; 
	return i;
}

__attribute__ ((noinline)) int index_bitmap_zsim(int* i, int* j){   // Catch routine in zsim/sniper to perform the indexing in the simulator. Simulator needs to write back the values using PIN_SafeCopy() (In Sniper you can do it using builtin functions) the inverse way.
	static int w=5;
	w = w+10;
	w = w -5; 
	return w;
}



int parse_opt(int argc, char *argv[])
{

		fprintf(stderr,"Usage: ./spmv_bitmap -f compression_ratio0 -s compression_ratio1 -t compression_ratio2 -k #iterations\n");

		int opt;
		while((opt = getopt(argc, argv,"f:s:t:i:k:"))){
				if(opt == -1)
					break;
				switch(opt){
				
					case 'f': 
						matrix_A_smash.compression_ratio0 = atoi(optarg);	
						matrix_B_smash.compression_ratio0 = atoi(optarg);

						comp0 = atoi(optarg);

						printf("Compression ratio 0 of A %d \n",matrix_A_smash.compression_ratio0);
						printf("Compression ratio 0 of B %d \n",matrix_B_smash.compression_ratio0);

						break;
					case 's':
						matrix_A_smash.compression_ratio1 = atoi(optarg);	
						matrix_B_smash.compression_ratio1 = atoi(optarg);

						comp1 = atoi(optarg);


						printf("Compression ratio 1 of A %d \n",matrix_A_smash.compression_ratio1);
						printf("Compression ratio 1 of B %d \n",matrix_B_smash.compression_ratio1);
						break;
					case 't':	
						matrix_A_smash.compression_ratio2 = atoi(optarg);
                        matrix_B_smash.compression_ratio2 = atoi(optarg);

						comp2 = atoi(optarg);

						printf("Compression ratio 2 of A %d \n",matrix_A_smash.compression_ratio2);
						printf("Compression ratio 2 of B %d \n",matrix_B_smash.compression_ratio2);
						break;
					case 'i':
						path = optarg;	
						printf("Input path %s \n",path);	
						break;
					case 'k':
						K = atoi(optarg);	
						printf("K = %d \n",K);	
						break;		
					default:
						return 1;		
					
				
				}
		}
	
	

}


void calculate_indices(int *i, int *j, smash matrix_format){

        int bitmap0_A_contribution;
	    int bitmap1_A_contribution;
	    int bitmap2_A_contribution;
		int contrib;


        bitmap0_A_contribution = (matrix_format.counter0%matrix_format.compression_ratio1)*matrix_format.compression_ratio0; // Check the paper to see what are these computations
		bitmap1_A_contribution = (matrix_format.counter1%matrix_format.compression_ratio2)*matrix_format.compression_ratio1*matrix_format.compression_ratio0;
		bitmap2_A_contribution = matrix_format.counter2*matrix_format.compression_ratio2*matrix_format.compression_ratio0*matrix_format.compression_ratio1;
		contrib = bitmap0_A_contribution + bitmap1_A_contribution + bitmap2_A_contribution;
		(*i) = contrib / matrix_format.columns;
		(*j) = contrib % matrix_format.columns;

        return;
}


void spmm(){


	#ifdef DEBUG
		printf("NZA blocks A %d\n",matrix_A_smash.nza_blocks);
        printf("NZA blocks B %d\n",matrix_B_smash.nza_blocks);

	#endif


	int row_A,col_A,row_B,col_B;

	row_A = 0;
	col_A = 0;

	row_B = 0;
	col_B = 0;

	int curr_row_A;
	int prev_row_A; 	

	smash start_matrix_A;
	smash temp_B_smash;
	int nza_start_matrix_A;

	int current_nza_index_A=0;
	int current_nza_index_B=0;

	int temp_nza_index_matrix_B;
	int result;
	start_matrix_A = matrix_A_smash;

	int current_column_B=0;
	temp_B_smash = matrix_B_smash; // Init matrix B SMASH

	int B_visited = 0;
	int A_visited = 0;

				
	while(!B_visited){ // iterate over B's columns

		printf("Move Bitmap B\n");
		int col_changed = 0;

		while(!col_changed){ // Find next column of B


		#ifdef SIM
			index_bitmap_zsim(&row_A,&col_A); // Signal zsim/sniper to receive the indices
		#endif 

		#ifdef NATIVE
			result = index_bitmaps(&matrix_B_smash);	
			calculate_indices(&col_B,&row_B,matrix_B_smash);
		#endif

			if(result == -1) return;
			if(col_B != current_column_B) col_changed = 1;
		}

		temp_B_smash = matrix_B_smash;
		matrix_A_smash = start_matrix_A;


		#ifdef SIM
			index_bitmap_zsim(&row_B,&col_B); // Signal zsim/sniper to receive the indices
		#endif 

		#ifdef NATIVE
			index_bitmaps(&matrix_A_smash);
			calculate_indices(&row_A,&col_A,matrix_A_smash); //Progress Bitmap A 
		#endif 


		while(!A_visited){

	
		    if( col_A < row_B){ // progress A
							 

							#ifdef SIM
									index_bitmap_zsim(&row_A,&col_A); // Signal zsim/sniper to receive the indices
									if(row_A == -1 ) A_visited = 1; // We finished A
							#endif 

							#ifdef NATIVE
						    	result = index_bitmaps(&matrix_A_smash);
								if(result == -1 ) A_visited = 1; // We finished A
								calculate_indices(&row_A,&col_A,matrix_A_smash);
							#endif


							printf("Progress bitmap A \n");
							printf("Row A = %d \n",row_A);
							printf("Col A = %d \n",col_A);

							current_nza_index_A = matrix_A_smash.compression_ratio0*matrix_A_smash.current_block0;

							if(row_A != curr_row_A){  // if we change row in A, roll-back Bitmap B 
								matrix_B_smash = temp_B_smash;
								current_nza_index_B = matrix_B_smash.compression_ratio0*matrix_B_smash.current_block0; // change the nza_index also when rolling back BitmapB
								curr_row_A = row_A;
							}
				 }
				else if ( col_A > row_B){
							

							#ifdef SIM
									index_bitmap_zsim(&row_B,&col_B); // Signal zsim/sniper to receive the indices
									if(row_B == -1 ) return; // We finished A
							#endif 

							#ifdef NATIVE
								result = index_bitmaps(&matrix_B_smash);	
								if(result == -1 ) return;
								calculate_indices(&col_B,&row_B,matrix_B_smash);
							#endif

							

							printf("Progress bitmap B \n");
							printf("Row B = %d \n",row_B);
							printf("Col B = %d \n",col_B);
							
							if( col_B != current_column_B){  // If we change column, roll-back Bitmap A and store the beginning of B
									temp_B_smash = matrix_B_smash;
									matrix_A_smash = start_matrix_A;
									current_nza_index_A = 0;
							}

							current_nza_index_B = matrix_B_smash.compression_ratio0*matrix_B_smash.current_block0;
				}
				else if (col_A == row_B) {
							
							printf("Multiply blocks \n");

							for(int e=0; e < matrix_A_smash.compression_ratio0; e++){  // Multiply the blocks assuming that you have the same compression_ratio0
									C[row_A*matrix_A_smash.columns+col_B] = matrix_A_smash.nza[current_nza_index_A]*matrix_B_smash.nza[current_nza_index_B];
									current_nza_index_A +=1;
									current_nza_index_B +=1;
							}

							#ifdef SIM
									index_bitmap_zsim(&row_A,&col_A); // Signal zsim/sniper to receive the indices
									if(row_A == -1 ) A_visited = 1; // We finished A
							#endif 

							#ifdef NATIVE
								result = index_bitmaps(&matrix_A_smash);
								if(result == -1 ){
									printf("Completed A \n");
									A_visited = 1;
								} 
								calculate_indices(&row_A,&col_A,matrix_A_smash);
							#endif




							printf("Progress bitmap A after multiplication \n");
							printf("Row A = %d \n",row_A);
							printf("Col A = %d \n",col_A);

							current_nza_index_A = matrix_A_smash.compression_ratio0*matrix_A_smash.current_block0;

							if(row_A != curr_row_A){  // if we change row in A, roll-back Bitmap B 
								matrix_B_smash = temp_B_smash;
								current_nza_index_B = matrix_B_smash.compression_ratio0*matrix_B_smash.current_block0; // change the nza_index also when rolling back BitmapB
								curr_row_A = row_A;
							}
											
				}

		}


	

	}


	return;
	

}




int main(int argc, char* argv[])
{


	parse_opt(argc,argv);

	csr matrix_A_csr = read_csr(path);
    csr matrix_B_csr = read_csr(path);

	construct_format(&matrix_A_smash,comp0,comp1,comp2);	
	construct_bitmap0_nza(&matrix_A_smash,&matrix_A_csr);
	construct_bitmap1(&matrix_A_smash);
	construct_bitmap2(&matrix_A_smash);

    construct_format(&matrix_B_smash,comp0,comp1,comp2);	
	construct_bitmap0_nza(&matrix_B_smash,&matrix_B_csr);
	construct_bitmap1(&matrix_B_smash);
	construct_bitmap2(&matrix_B_smash);

	matrix_A_smash.current_register0 = matrix_A_smash.bitmap0[0];
	matrix_A_smash.current_register1 = matrix_A_smash.bitmap1[0]; 
	matrix_A_smash.current_register2 = matrix_A_smash.bitmap2[0];

    matrix_B_smash.current_register0 = matrix_B_smash.bitmap0[0];
	matrix_B_smash.current_register1 = matrix_B_smash.bitmap1[0]; 
	matrix_B_smash.current_register2 = matrix_B_smash.bitmap2[0];

	print_bitmaps(&matrix_A_smash);
	print_bitmaps(&matrix_B_smash);


	
	C = (float*) malloc (sizeof(float)*matrix_A_smash.rows*matrix_A_smash.columns);


	#ifdef SIM
		int value;
		zsim_roi_begin();
		value = send_data_to_bmu(&matrix_A_smash); //the matrices are the same, send only one
		COMPILER_BARRIER(); // Compiler barrier to be sure that smash is stored in the simulator.
	#endif

	for(int i = 0; i < matrix_A_smash.rows ; i++)
	{
		C[i] = C[i]+1.0; // warmup the caches with the vector and the output to avoid weird caching effects when running with less than 10B instructions
	}

	#ifdef NATIVE
		    clock_t t; 
			t = clock(); 
	#endif
	
	spmm();

	#ifdef SIM
		zsim_roi_end(); // run at least 10B instructions. If you dont, you will have weird issues with cache warmups
		                // Advice based on personal experience: ZSim's structure is a pain. They did not provide a way to catch routines, copy userspace memory etc. This way HW/SW mechanisms are not easy to design. 
						// Shoot for Sniper who is more modular. I am re-implementing SMASH there for a follow-up idea and I will open-source it soon.
	#endif

	#ifdef NATIVE
    		double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 	
			printf("SpMM-Time:%f\n", time_taken); 
	#endif


	return 0;

}
