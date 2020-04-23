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

int *x; // dense vector
int *b; // output matrix
smash matrix_smash; //smash matrix format
char *path;
int comp0,comp1,comp2;
int K;


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
						matrix_smash.compression_ratio0 = atoi(optarg);	
						comp0 = atoi(optarg);
						printf("Compression ratio 0 %d \n",matrix_smash.compression_ratio0);
						break;
					case 's':
						matrix_smash.compression_ratio1 = atoi(optarg);
						comp1 = atoi(optarg);

						printf("Compression ratio 1 %d \n",matrix_smash.compression_ratio1);
						break;
					case 't':	
						matrix_smash.compression_ratio2 = atoi(optarg);
						comp2 = atoi(optarg);

						printf("Compression ratio 2 %d \n",matrix_smash.compression_ratio2);
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


void spmv(){

	int block;
	int cur_element = 0;
	int i,j;
	int bitmap0_contribution;
	int bitmap1_contribution;
	int bitmap2_contribution;
	

	printf("NZA blocks %d\n",matrix_smash.nza_blocks);


	for(block = 0; block < matrix_smash.nza_blocks; block++){

		index_bitmaps(&matrix_smash);

		#ifdef SIM
			index_bitmap_zsim(&i,&j); // Signal zsim/sniper to receive the indices
		#endif 

		#ifdef NATIVE
			bitmap0_contribution = (matrix_smash.counter0%matrix_smash.compression_ratio1)*matrix_smash.compression_ratio0; // Check the paper to see what are these computations
			bitmap1_contribution = (matrix_smash.counter1%matrix_smash.compression_ratio2)*matrix_smash.compression_ratio1*matrix_smash.compression_ratio0;
			bitmap2_contribution = matrix_smash.counter2*matrix_smash.compression_ratio2*matrix_smash.compression_ratio0*matrix_smash.compression_ratio1;

			i = bitmap0_contribution + bitmap1_contribution + bitmap2_contribution;
			j =  bitmap0_contribution + bitmap1_contribution + bitmap2_contribution;

			i = i / matrix_smash.columns;
			j = j % matrix_smash.columns;

			printf("Row A = %d \n", i);
			printf("Col B = %d \n", j);

		#endif


		for(int e=0; e < matrix_smash.compression_ratio0; e++){
			
			b[i] = x[j]*matrix_smash.nza[cur_element];
			cur_element++;
			j++;
			if(j > matrix_smash.columns){ i++; j=0;}
	
		}
	}
	

}

int main(int argc, char* argv[])
{


	parse_opt(argc,argv);

	csr matrix_csr = read_csr(path);

	construct_format(&matrix_smash,comp0,comp1,comp2);	
	construct_bitmap0_nza(&matrix_smash,&matrix_csr);
	construct_bitmap1(&matrix_smash);
	construct_bitmap2(&matrix_smash);

	matrix_smash.current_register0 = matrix_smash.bitmap0[0];
	matrix_smash.current_register1 = matrix_smash.bitmap1[0]; 
	matrix_smash.current_register2 = matrix_smash.bitmap2[0];
		
		
	print_bitmaps(&matrix_smash);

	
	b = (float*) malloc (sizeof(float)*matrix_smash.rows);
	x = (float*) malloc (sizeof(float)*matrix_smash.rows);


	#ifdef SIM
		int value;
		zsim_roi_begin();
		value = send_data_to_bmu(&matrix_smash);
		COMPILER_BARRIER(); // Compiler barrier to be sure that smash is stored in the simulator.
	#endif

	for(int i = 0; i < matrix_smash.rows ; i++)
	{
		x[i] = x[i]+1.0;
		b[i] = b[i]+1.0; // warmup the caches with the vector and the output to avoid weird caching effects
	}

	#ifdef NATIVE
		    clock_t t; 
			t = clock(); 
	#endif

	spmv();

	#ifdef SIM
		zsim_roi_end(); // run at least 10B instructions.
	#endif

	#ifdef NATIVE
    		double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 	
			printf("SpMV-Time:%f\n", time_taken); 
	#endif


	return 0;

}
