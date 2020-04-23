#ifndef BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>
#include <time.h>
#include "csr.h"


struct bitmap_3_level{

	unsigned long *bitmap0;
	unsigned long *bitmap1;
	unsigned long *bitmap2;

	int counter0;
	int counter1;
	int counter2;

	int current_index0;
	int current_index1;
	int current_index2;

	unsigned long current_register0;
	unsigned long current_register1;
	unsigned long current_register2;

	int current_block0;
	int current_block1;	
	int current_block2;
	
	int current_bitmap;	

	int bitmap0_bits;
	int bitmap1_bits;
	int bitmap2_bits;

	int bitmap0_blocks;
	int bitmap1_blocks;
	int bitmap2_blocks;

	float *nza;
	int nza_blocks;

	int compression_ratio0;
	int compression_ratio1;
	int compression_ratio2;	

	int rows;
	int columns;
};


typedef struct bitmap_3_level smash;

unsigned long* construct_bitmap(int num_bits){

	int byteArraySize = (num_bits+63)/64;
	unsigned long* byteArray;

	byteArray = (unsigned long*) malloc (byteArraySize*sizeof(unsigned long));
	
	if(byteArray == 0)
	{
	  fprintf(stderr,"Bitvector allocation failure \n");
	  exit(99);
	}
	for (int i=0; i < byteArraySize; i++)
		byteArray[i]=0x0000000000000000;

	return byteArray;
}

inline void set_bit(unsigned long* bitmap, int bitnumber){

	
	int index; 
	int mask;

	index = bitnumber >> 6;
	mask = bitnumber & 0x000000000000003f;
	mask = 0x0000000000000001 << mask;
	
	#ifdef DEBUG	

		printf("Mask = %#018" PRIx64 "\n",mask);
		printf("Index = %d\n", index);

	#endif



	bitmap[index] |= mask;


//	printf("%u\t",bitmap[index] |= mask);

}


inline void test_bit(int bitnumber, unsigned long* bitmap){
	
	uint64_t index; 
	uint64_t mask;


	index = bitnumber >> 6;
	mask = bitnumber & 0x000000000000003f;
	mask = 0x0000000000000001 << mask;

	//printf("%c", 0 != (bitmap[index]) & mask);

}

inline int read_bit(int bitnumber,unsigned long* bitmap)
{
		
	int index; 
	int mask;

	index = bitnumber >> 6;
	mask = bitnumber & 0x000000000000003f;
	mask = 0x0000000000000001 << mask;

	return  (0 != ((bitmap[index]) & mask));


}

void print_bitmaps(smash *format){
	
	for(int i=0; i< format->bitmap2_bits; i++)
	{
		printf("%u\t",read_bit(i,format->bitmap2));
	}
	printf("\n");
	for(int i=0; i< format->bitmap1_bits; i++)
	{
		printf("%u\t",read_bit(i,format->bitmap1));
	}
	printf("\n");
	for(int i=0; i< format->bitmap0_bits; i++)
	{
		printf("%u\t",read_bit(i,format->bitmap0));
	}
	printf("\n");

	return;

}


void construct_bitmap0_nza(smash* format, csr *matrix){
	int current_col=0;

	int previous_block = -1;
	int nza_blocks = 0;

	format->rows = matrix->size;
	format->columns = matrix->size;


	int blocks = ((matrix->size)*(matrix->size))/(format->compression_ratio0);
	format->bitmap0 = construct_bitmap(blocks);
	format->bitmap0_bits = blocks;

	#ifdef DEBUG
		printf("Initialized bitmap0 \n");
	#endif

	format->nza = (float*)malloc(sizeof(float));

	for(int i=0; i < matrix->size; i++){
		int row_elements = matrix->row_ptr[i+1]-matrix->row_ptr[i];

		#ifdef DEBUG
			printf("Row %d has %d elements \n",i,row_elements);
		#endif

		for (int j=0; j<row_elements; j++){
			int x = i;
			int y;
			y = matrix->col_ptr[current_col];
		        
			int block = (x*matrix->size+y)/(format->compression_ratio0);

			#ifdef DEBUG
				printf("Element in row %d and column %d and block %d \n",x,y,block);
			#endif

			if(block != previous_block){
				nza_blocks++;
				format->nza = (float*)realloc(format->nza,nza_blocks*format->compression_ratio0*sizeof(float)); // We do not care about the actual value
				previous_block = block;
			
			}
			
			int bitnumber = block;
			set_bit(format->bitmap0,bitnumber); 
			current_col+=1;
		}
	}

	format->nza_blocks = nza_blocks;	
	
	#ifdef DEBUG	
		printf("\n");
		printf("Constructed bitmap0 and nza \n");
	
		for(int i=0; i< format->bitmap0_bits; i++)
		{
			printf("%u\t",read_bit(i,format->bitmap0));
		}
		printf("\n");
	#endif 



}


void construct_bitmap1(smash *format){

	int bitmap1_size;
	bitmap1_size = (format->bitmap0_bits/format->compression_ratio1);
	format->bitmap1_bits = bitmap1_size;

	format->bitmap1 = construct_bitmap(bitmap1_size);

	unsigned long *new_bitmap0; 
	new_bitmap0 = (unsigned long*)malloc(sizeof(unsigned long));
	int new_bitmap0_blocks = 0;

	int index;
	int block;
	int previous_block=-1;

	for(int i = 0; i < format->bitmap0_bits; i++){

		if(read_bit(i,format->bitmap0)){
			block = i/(format->compression_ratio1);
			
			if(previous_block != block){
				new_bitmap0_blocks++;
				if(new_bitmap0_blocks % 64 ==0)
					new_bitmap0 = (unsigned long*)realloc(new_bitmap0,new_bitmap0_blocks*sizeof(unsigned long));
								
				#ifdef DEBUG
				
					printf("Set bit %d of bitmap0 \n",(new_bitmap0_blocks-1)*format->compression_ratio0+i%(format->compression_ratio0));

				#endif
				previous_block = block;
			}
				
			set_bit(new_bitmap0,(new_bitmap0_blocks-1)*format->compression_ratio0 + i%(format->compression_ratio0));
			set_bit(format->bitmap1,block);
		}
	}	

	format->bitmap0 = new_bitmap0;
	format->bitmap0_blocks = new_bitmap0_blocks;
	format->bitmap0_bits = new_bitmap0_blocks*format->compression_ratio1;

	#ifdef DEBUG	
		printf("Constructed Bitmap1 \n");
		for(int i=0; i< format->bitmap1_bits; i++)
		{
			printf("%u\t",read_bit(i,format->bitmap1));
		}
		printf("\n");
	#endif
	

	return;
}


void construct_bitmap2(smash *format)
{

	int bitmap2_size;
	bitmap2_size = (format->bitmap1_bits/format->compression_ratio2);
	format->bitmap2_bits = bitmap2_size;

	format->bitmap2 = construct_bitmap(bitmap2_size);

	unsigned long *new_bitmap1; 
	new_bitmap1 = (unsigned long*)malloc(sizeof(unsigned long));

	int new_bitmap1_blocks = 0;

	int index;
	int block;
	int previous_block=-1;

	for(int i = 0; i < format->bitmap1_bits; i++){

		if(read_bit(i,format->bitmap1)){
			block = i/(format->compression_ratio2);
			
			if(previous_block != block){
				new_bitmap1_blocks++;
				if(new_bitmap1_blocks % 64 ==0)
					new_bitmap1 = (unsigned long*)realloc(new_bitmap1,new_bitmap1_blocks*sizeof(unsigned long));	
				previous_block = block;
			}
	

			set_bit(new_bitmap1,(new_bitmap1_blocks-1)*format->compression_ratio1 + i%(format->compression_ratio1));
			set_bit(format->bitmap2,block);
		}
	}	

	format->bitmap1 = new_bitmap1;
	format->bitmap1_blocks = new_bitmap1_blocks;
	format->bitmap1_bits = new_bitmap1_blocks*format->compression_ratio2;


	#ifdef DEBUG	

		printf("Constructed Bitmap2 \n");

		for(int i=0; i< format->bitmap2_bits; i++)
		{
			printf("%u\t",read_bit(i,format->bitmap2));
		}
		printf("\n");

	#endif




	return;

}


void construct_format(smash  *format, int comp1, int comp2, int comp3)
{
	format->compression_ratio0 = comp1;
	format->compression_ratio1 = comp2;
	format->compression_ratio2 = comp3;

	format->current_block0 = 0;
	format->current_block1 = 0;

	format->counter0 = 0;	
	format->counter1 = 0;
	format->counter2 = 0;

	format->current_index0 = 0;	
	format->current_index1 = 0;	
	format->current_index2 = 0;	

	format->counter1 = 0;
	format->counter2 = 0;
	format->current_bitmap = 2;

	#ifdef DEBUG	
		printf("Initialized Compression Ratios \n");
	#endif
}

#ifdef NO_BCR
int index_bitmaps(smash *format)
{

	//@kanellok For potential software optimization: It would be ideal if someone writes the x86 assembly directly for this function

	for(;;){

		outerloop:

		if(format->current_bitmap==0){	
			if(format->counter0/format->compression_ratio1 != format->current_block0){
		
					format->current_bitmap=1; 
					format->current_block0 = format->counter0/format->compression_ratio1;
					goto outerloop;
			}

			if(format->counter0 > format->bitmap0_bits) return -1;
			
			while(!read_bit(format->counter0,format->bitmap0)){ 

				format->counter0++;	
				if(format->counter0/format->compression_ratio1 != format->current_block0){
		
					format->current_bitmap=1; 
					format->current_block0 = format->counter0/format->compression_ratio1;
					goto outerloop;
				}
				if(format->counter0 > format->bitmap0_bits) return -1;
			}	
			format->counter0++;
			
			printf("Bitmap 0, found set bit at %d\n",(format->counter0-1));			
	
			return (format->counter0-1);
		}
		else if(format->current_bitmap==1){
			
			if(format->counter1/format->compression_ratio1 != format->current_block1){
		
					format->current_bitmap=2; 
					format->current_block1 = format->counter1/format->compression_ratio2;
					goto outerloop;
			}
		
			if(format->counter1 > format->bitmap1_bits) return -1;
			
			while(!read_bit(format->counter1,format->bitmap1)){ 
				format->counter1++;
				if(format->counter1/format->compression_ratio2 != format->current_block1){
					format->current_bitmap=2;  
					format->current_block1 = format->counter1/format->compression_ratio1;
					goto outerloop;
				}
				if(format->counter1 > format->bitmap1_bits) return -1;
			
			}
		
			format->counter1++;
			printf("Bitmap 1, found set bit at %d\n",(format->counter1-1));
	
			format->current_bitmap=0;
	
		}	
		else if(format->current_bitmap==2){
			while(!read_bit(format->counter2,format->bitmap2)){ 
				format->counter2++;
				if(format->counter2 > format->bitmap2_bits) return -1;
			}
			format->counter2++;
			printf("Bitmap 2, found set bit at %d\n",(format->counter2-1));
			
			format->current_bitmap=1;
		}
		
	}
}
#endif 




uint64_t find_set_bit(smash* format, uint64_t a)
{

//@kanellok For potential software optimization: It would be ideal if someone writes the x86 assembly directly for this function

  static_assert( CHAR_BIT * sizeof(unsigned long) == 64, "__builtin_clzll isn't 64-bit operand size");
  uint64_t bit = __builtin_ctzl(a);      // BSR
  //printf("Found set bit at Bitmap[%d] = %d \n",(format->current_index0),bit);			

 // return a ? (1ULL << bit) : 0;              // ULL is guaranteed to be at least a 64-bit type
  return a ? bit : 0;              // ULL is guaranteed to be at least a 64-bit type

}



int index_bitmaps(smash *format)
{

	//@kanellok For future software optimizations: It would be ideal if someone writes the x86 assembly directly for this function
	int value;

	for(;;){
		outerloop:

		if(format->current_bitmap==0){	
					
			if((format->counter0+1) >= format->bitmap0_bits) return -1;
			
			if(format->current_register0 == 0){ 
				format->current_index0++;
				format->counter0 = format->current_index0*64;
				format->current_register0 = format->bitmap0[format->current_index0];
			}

			#ifdef DEBUG	
	
				printf("Register= %#018" PRIx64 "\n",format->current_register0);
			
			#endif

				value = find_set_bit(format,format->current_register0);



			#ifdef DEBUG	

				printf("Found set bit at Bitmap%d[%d] = %d current_block0 = %d \n",format->current_bitmap,(format->current_index0),value,format->current_block0);

			#endif	

			if((format->current_index0*64+(value))/format->compression_ratio1 != format->current_block0){
								format->current_bitmap=1; 
								format->current_block0 = (format->current_index0*64+(value))/format->compression_ratio1;
								goto outerloop;
			}		
			format->counter0 = format->current_index0*64+(value);
			format->current_register0 &= ~(1 << (value)) ;
			//if(format->current_register0 == 0){ goto outerloop;}
			#ifdef DEBUG	
				
				printf("Counter%d= %d\n",0,format->counter0);
			
			#endif



		
		//	 if(format->counter0 > format->bitmap0_bits) return -1;		
	
			return (format->counter0);
		}
		
		else if(format->current_bitmap==1){	

			if((format->counter1+1) >= format->bitmap1_bits) return -1;
			
			if(format->current_register1 == 0){ 
				format->current_index1++;
				format->counter1 = format->current_index1*64;
				format->current_register1 = format->bitmap1[format->current_index1];
			}
//			printf("Register= %#018" PRIx64 "\n",format->current_register1);
			value = find_set_bit(format,format->current_register1);
			
			#ifdef DEBUG	
				
				printf("Found set bit at Bitmap%d[%d] = %d \n",format->current_bitmap,format->current_index1,value);	
			
			#endif

			if((format->current_index1*64+(value))/format->compression_ratio2 != format->current_block1){
								format->current_bitmap=2; 
								format->current_block1 = (format->current_index1*64+(value))/format->compression_ratio2;
								goto outerloop;

			}
			format->counter1 = format->current_index1*64+(value);
			format->current_register1 &= ~(1 << (value)) ;

			#ifdef DEBUG	

				printf("Counter%d= %d\n",1,format->counter1);
			
			#endif
			
		 	 format->current_bitmap = 0;	 
		}	
		
		else if(format->current_bitmap==2){	
					
			if((format->counter2+1) >= format->bitmap2_bits) return -1;
			
			if(format->current_register2 == 0){ 
				format->current_index2++;
				format->counter2 = format->current_index2*64;
				format->current_register2 = format->bitmap2[format->current_index1];
			}
//			printf("Register= %#018" PRIx64 "\n",format->current_register2);
			value = find_set_bit(format,format->current_register2);
						
			#ifdef DEBUG	
				
				printf("Found set bit at Bitmap%d[%d] = %d,  current_index2 = %d \n",format->current_bitmap,(format->current_index2),value,format->current_index2);			
			
			#endif

			format->counter2 = format->current_index2*64+(value);
			format->current_register2 &= ~(1 << (value)) ;

			#ifdef DEBUG	

				printf("Counter%d= %d\n",2,format->counter2);
		
			#endif
			
		 	format->current_bitmap = 1;	 
		}	
			
	}

}




#endif
