#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "matrix_generator.h"
#include "zsim_hooks.h"




__attribute__ ((noinline)) int avoid(uintptr_t matrix, uintptr_t matrix2, uintptr_t vec, uintptr_t vec2, uintptr_t out, uintptr_t out2, uint32_t *bitmap, uint32_t *bitmap2, int bitmap_size, int bitmap2_size,int size)
{
	static int w;
	w = w + 10;
	w = w - 5;
	return w;
}




int main(int argc, char **argv) {




	char* path_A;
	char* path_B;
	
	if (argc == 3)
	{
		path_A = argv[1];
		path_B = argv[2];
	}
	int value;
	int flag; 
	struct matrix A,B,C;
	A = read_matrix(path_A);
	B = read_matrix(path_B);
	C.array = (float *)malloc(sizeof(float)*A.size*A.size);
	
	for(int i=0; i<A.size*A.size; i++) C.array[i]=0;	
	printf("Compute SGEMM using matrix\n");

	zsim_roi_begin();	


	#ifdef AVOID
	value = avoid((uintptr_t)(&A.array[0]),(uintptr_t)(&A.array[A.size*A.size-1]), (uintptr_t)(&B.array[0]), (uintptr_t)(&B.array[A.size-1]), (uintptr_t)(&C.array[0]), (uintptr_t)(&C.array[A.size-1]),A.bitmap,B.bitmap, A.bitmap_size, B.bitmap_size,A.size);
	#endif

	COMPILER_BARRIER();

	
	int m, n, k;
	int size;
	size= A.size;

	for(m=0; m<size; m++){
		if( m == size/128) break;
		for(n=0; n<size; n++){
			for(k=0; k<size; k++){
				C.array[m * size + n] += A.array[m * size + k] * B.array[k * size + n];
			}
		}
	}
	
	zsim_roi_end();

}
