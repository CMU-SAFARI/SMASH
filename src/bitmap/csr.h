#ifndef CSR_H
#define CSR_H
#include <stdlib.h>
#include <stdio.h>
#define true 1
#define false 0

struct csr{

	int *row_ptr;
	int *col_ptr;
	float *val;
	int size;
};

typedef struct csr csr;


csr read_csr(char *path)
{
	printf("Read matrix %s\n",path);
	FILE *fd;
	fd = fopen(path,"r");

	int rows, columns, nnz;

	fscanf(fd,"%d",&rows);
	fscanf(fd,"%d",&nnz);
	printf("Rows = %d Non-zero = %d \n",rows,nnz);

	csr csr_out;
	int total_nnz = 0 ;
	
	csr_out.row_ptr = (int *) malloc((rows+1)*sizeof(int));
	csr_out.col_ptr = (int *) malloc(nnz*sizeof(int));
	csr_out.val = (float*) malloc(nnz*sizeof(float));
	csr_out.size = rows;

	int row,col;
	float value;
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

	printf("Finished reading csr %s\n",path);
	return csr_out ;

}


csr csr_generator(int size,int nnz ){

	csr csr_out;
	int total_nnz = 0 ;
	srand(time(NULL));
	csr_out.row_ptr = (int *) malloc((size+1)*sizeof(int));
	csr_out.col_ptr = (int *) malloc(5*sizeof(int));
	csr_out.val = (int*) malloc(5*sizeof(int));


	printf("Create CSR with size %d and sparsity %d\n", size, nnz);
	csr_out.size = size;
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
		printf("Total_nnz = %d \n",total_nnz);
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
		    printf("Column = %d \n", index);
		}   
		csr_out.row_ptr[i+1] = csr_out.row_ptr[i]+nnz_row;
		free(visited);    
	}


	return csr_out;

}

#endif
