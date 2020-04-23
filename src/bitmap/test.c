#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
#include "csr.h"





int main(int argc, char *argv[])
{




	csr matrix_csr;

	//matrix_csr = read_csr(argv[1]);
	matrix_csr = csr_generator(atoi(argv[1]),atoi(argv[2]));
	smash matrix_smash;

	construct_format(&matrix_smash,2,2,2);	
	construct_bitmap0_nza(&matrix_smash,&matrix_csr);
	construct_bitmap1(&matrix_smash);
	construct_bitmap2(&matrix_smash);

	print_bitmaps(&matrix_smash);
	matrix_smash.current_register0 = matrix_smash.bitmap0[0];
	matrix_smash.current_register1 = matrix_smash.bitmap1[0];
	matrix_smash.current_register2 = matrix_smash.bitmap2[0];

	printf("Bitmap 0 bits = %d \n", matrix_smash.bitmap0_bits);
	for(int i = 0; i < matrix_smash.nza_blocks; i++)	
		index_bitmaps(&matrix_smash);
}
