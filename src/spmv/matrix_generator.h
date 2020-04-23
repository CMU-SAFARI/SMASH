#include <stdio.h>
#include <stdlib.h>
#include <time.h>  
#include <math.h>
#define true 0
#define false 1
#define CACHE_LINE_SIZE 64
#include <stdint.h>
struct matrix{
	float *array;
	uint32_t *bitmap;
	int bitmap_size;
	int size;

};
struct csr{

	int *row_ptr;
	int *col_ptr;
	float *val;
	int size;
};

struct bitmap_nnz{

	float *nnz_array;
	uint32_t *bitmap;
	int bitmap_size;
	int nnz_size;
	int matrix_size;

};


int rangeRandom (int min, int max){
	srand(time(NULL)); // Seed the time
	int n = max - min + 1;
	int remainder = RAND_MAX % n;
	int x;
	do{
		x = rand();
	}while (x >= RAND_MAX - remainder);
	return min + x % n;
}

struct csr csr_generator(int size,int nnz ){

	struct csr csr_out;
	int total_nnz = 0 ;
	srand(time(NULL));
	csr_out.row_ptr = (int *) malloc((size+1)*sizeof(int));
	csr_out.col_ptr = (int *) malloc(5*sizeof(int));
	csr_out.val = (int*) malloc(5*sizeof(int));


	printf("Create CSR with size %d and sparsity %d\n", size, nnz);
	int index; 
	int last_nnz = 0;
	int nnz_row = 0;
	int i = 0; 
	int j = 0;
	int *visited;
	int *temp;
	int max;
	int min;
	for( i = 0; i < size; i++)
	{   

		last_nnz += nnz_row;

		max = (int)((nnz + nnz/10.0)/100.0 * size);
		min = (int)((nnz - nnz/10.0)/100.0 * size);
		nnz_row = rand()%max + min ;
		total_nnz += nnz_row; 
		//printf("Total_nnz = %d \n",total_nnz);
		csr_out.col_ptr = (int *)realloc(csr_out.col_ptr, sizeof(int)*total_nnz);
		csr_out.val = (int *)realloc(csr_out.val, sizeof(int)*total_nnz);
		visited = (int *)malloc(sizeof(int)*size);

		for(int w =0 ; w < size; w++) visited[w] = 0;

		for ( j=0; j < nnz_row ; j++){
			csr_out.val[last_nnz+j] = 1;
			do{
				index = rand()%size;
			}while(visited[index]);

			visited[index] = true;
			csr_out.col_ptr[last_nnz+j] = index;
			//   printf("Column = %d \n", index);
		}   
		csr_out.row_ptr[i+1] = csr_out.row_ptr[i]+nnz_row;
		free(visited);    
	}


	return csr_out;

}



void print_binary(int n )
{

	while (n) {
		if (n & 1)
			printf("1");
		else
			printf("0");

		n >>= 1;
	}
	printf("\n");

	return;

}

struct bitmap_nnz read_matrix_bitmap_nnz(char *path)
{
	printf("Read matrix %s\n",path);
	FILE *fd;
	fd = fopen(path,"r");

	int rows, columns, nnz, weight;

	fscanf(fd,"%d",&rows);
	fscanf(fd,"%d",&columns);
	fscanf(fd,"%d",&nnz);    
	fscanf(fd,"%d",&weight);

	printf("Rows = %d  Columns = %d  Non-zero = %d \n",rows,columns,nnz);

	struct bitmap_nnz mat;
	mat.matrix_size = rows;

	int size_bitmap;
	int cache_lines = (rows*rows);
	cache_lines = cache_lines/16;

	//if(cache_lines % 32 == 0) size_bitmap = (int)(cache_lines/32);
	size_bitmap = cache_lines/32+1;
	mat.bitmap = (uint32_t*)malloc(sizeof(uint32_t)*size_bitmap);
	printf("Bitmap size = %d \n", size_bitmap);
	getchar();
	mat.bitmap_size = size_bitmap;
	mat.nnz_array = (float*)malloc(1);
	
	printf("Lets read the matrix\n");
	for(int i=0; i<size_bitmap; i++)
	{
		mat.bitmap[i]=0;
	}


	int i,j;
	float value;
	int nnz_cache_lines = 0;
	int previous_cache_line = -1;


	
	for(int w = 0 ; w < nnz; w++)
	{
		fscanf(fd,"%d",&i);
		fscanf(fd,"%d",&j);

		if(weight){
		       fscanf(fd,"%f",&value);
		}
		
				
		int cache_line = (i*sizeof(int)*rows+j*sizeof(int))/64;
		//printf("Cache line = %d \n", cache_line);
		if( cache_line != previous_cache_line){
	//		printf("Initialize new cache line: %d \n",nnz_cache_lines);
			nnz_cache_lines++;
			mat.nnz_array = (float*)realloc(mat.nnz_array,nnz_cache_lines*CACHE_LINE_SIZE);
			//memset(mat.nnz_array, 0, nnz_cache_lines*CACHE_LINE_SIZE); 
			previous_cache_line = cache_line;		
		}

		
		//printf("i = %d  j = %d \n",i,j);
		int bitmap_index = cache_line/32;
		//printf("Cache line =% d Bitmap index = %d \n",cache_line, bitmap_index);
		//uint32_t temp = mat.bitmap[cache_line/32];
		mat.bitmap[cache_line/32] = mat.bitmap[cache_line/32] | (uint32_t) ( 1 << ( cache_line%32));
	
	}  

	mat.nnz_size = nnz_cache_lines;

	printf("Everything Initialized\n"); 
	return mat;


}


struct matrix read_matrix(char *path)
{
	printf("Read matrix %s\n",path);
	FILE *fd;
	fd = fopen(path,"r");

	int rows, columns, nnz, weight;

	fscanf(fd,"%d",&rows);
	fscanf(fd,"%d",&columns);
	fscanf(fd,"%d",&nnz);    
	fscanf(fd,"%d",&weight);

	printf("Rows = %d  Columns = %d  Non-zero = %d \n",rows,columns,nnz);

	struct matrix mat;
	mat.array = (int *)malloc(sizeof(int)*(rows+1)*(rows+1));
	mat.size = rows;
	for (int i = 0 ; i < rows*rows; i++)
	{

		mat.array[i] = 0  ;
	}

	int cache_lines = ((rows)/16)*rows;
	int size_bitmap= cache_lines/32+1;
	if(cache_lines % 32 == 0) size_bitmap = cache_lines/32;
	else size_bitmap = cache_lines/32+1;
	mat.bitmap = (uint32_t*)malloc(sizeof(uint32_t)*size_bitmap);
	mat.bitmap_size = size_bitmap;


	for(int i=0; i<size_bitmap; i++)
	{
		mat.bitmap[i]=0;
	}

	int cache_line = 0;
	int previous_cache_line = 0;
	int i,j;
	float value;
	for(int w = 0 ; w < nnz; w++)
	{
		fscanf(fd,"%d",&i);
		fscanf(fd,"%d",&j);

		if(weight){
		       fscanf(fd,"%f",&value);
		       mat.array[i*rows+j] = value; 
		}


		/* Create Non nero data structure */
		/*                                */


		int bitmap_index = cache_line/32;
		uint32_t temp = mat.bitmap[cache_line/32];
		mat.bitmap[cache_line/32] = mat.bitmap[cache_line/32] | (uint32_t) ( 1 << ( cache_line%32));

	}  


	printf("Everything Initialized\n"); 

	return mat;


}


struct csr read_csr(char *path)
{
	printf("Read matrix %s\n",path);
	FILE *fd;
	fd = fopen(path,"r");

	int rows, columns, nnz;



	fscanf(fd,"%d",&rows);
	fscanf(fd,"%d",&nnz);
	printf("Rows = %d Non-zero = %d \n",rows,nnz);
	struct csr csr_out;
	int total_nnz = 0 ;
	srand(time(NULL));
	csr_out.row_ptr = (int *) malloc((rows+1)*sizeof(int));
	csr_out.col_ptr = (int *) malloc(nnz*sizeof(int));
	csr_out.val = (int*) malloc(nnz*sizeof(int));
	csr_out.size = rows;

	int row,col,value;
	for(int w = 0 ; w < rows + 1; w++)
	{
		fscanf(fd,"%d",&row);
		csr_out.row_ptr[w] = row;
	}
	for(int w= 0 ; w < nnz; w++)
	{	
		fscanf(fd,"%d",&col);	
		csr_out.col_ptr[w] = col;
	}
	for(int w= 0 ; w < nnz; w++)
	{
		fscanf(fd,"%f",&value);	
		csr_out.val[w] = value;
	}


	return csr_out ;


}


struct matrix matrix_generator(int size, int sparsity, int sparse_locality)
{
	printf("Create Matrix with size %d and sparsity %d\n", size, sparsity);

	struct matrix mat;
	mat.array = (int *) malloc(sizeof(int)*size*size);



	for (int i = 0 ; i < size*size; i++)
	{

		mat.array[i] = 0  ;
	}


	int total_nnz = 0 ;
	srand(time(NULL)); 
	int cache_lines = ((size)/16)*size;
	int size_bitmap= cache_lines/32+1;
	if(cache_lines % 32 == 0) size_bitmap = cache_lines/32;
	else size_bitmap = cache_lines/32+1;

	printf("Size as Int: %d\n", size_bitmap);
	mat.bitmap = (uint32_t*)malloc(sizeof(uint32_t)*size_bitmap);
	mat.bitmap_size = size_bitmap;

	for(int i=0; i<size_bitmap; i++)
	{
		mat.bitmap[i]=0;
	}


	int index; 
	int last_nnz = 0;
	int nnz_row = 0;
	int i = 0; 
	int j = 0;
	int *visited;
	int *temp;
	int max;
	int min;
	int cache_line;



	int nnz_elements = (int)(sparsity/100.0 * size * size);
	int nnz_rows = nnz_elements/size+1;
	int nnz_rows_loc = (sparse_locality*nnz_rows);
	if( nnz_rows_loc > size) nnz_rows_loc = size;
	int nnz_elements_per_row = ( nnz_elements / nnz_rows_loc);
	if(nnz_elements % nnz_rows_loc == 0 ) nnz_elements_per_row = nnz_elements / nnz_rows_loc;
	else nnz_elements_per_row = nnz_elements / nnz_rows_loc+1;

	printf("Non-zero elements: %d\n", nnz_elements);
	printf("Non-zero rows: %d\n", nnz_rows);
	printf("Non-zero rows locality: %d\n", nnz_rows_loc); 
	printf("Non-zero elements per row: %d\n", nnz_elements_per_row); 
	for( i = 0; i < nnz_rows_loc; i++)
	{   


		visited = (int *)malloc(sizeof(int)*size);

		for(int w =0 ; w < size; w++) visited[w] = 0;

		for ( j=0; j < nnz_elements_per_row ; j++){

			do{
				index = rand()%size;
			}while(visited[index]);

			visited[index] = true;
			mat.array[i*size+j] = 1 ;
			cache_line = (i*sizeof(int)*size+j*sizeof(int))/64;
			int bitmap_index = cache_line/32;
			uint32_t temp = mat.bitmap[cache_line/32];
			mat.bitmap[cache_line/32] = mat.bitmap[cache_line/32] | (uint32_t) ( 1 << ( cache_line%32));

			//printf("Input to shift = %u, Cache line = %d, Bitmap[%d] before = %d, Bitmap[%d] after = %d  \n", (uint32_t)(1 << (cache_line%32)), cache_line, cache_line/32, temp ,cache_line/32, mat.bitmap[cache_line/32]);

			//printf("Cache line of array[%d][%d] = %d  Bitmap = %d \n",i,index,(i*sizeof(int)*size+index*sizeof(int))/64, mat.bitmap[cache_line/32]);

		}   

		// free(visited);    
	}


	free(visited);

	return mat;


}




