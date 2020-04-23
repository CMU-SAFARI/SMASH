#include <stdio.h>
#include <stdlib.h>
#include <time.h>  
#include <math.h>

struct matrix{
	float *array;
	int rows;
    int columns;

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

	int i,j;
	for(int w = 0 ; w < nnz; w++)
	{
		fscanf(fd,"%d",&i);
		fscanf(fd,"%d",&j);

		if(weight){
		       fscanf(fd,"%f",&value);
		       mat.array[i*rows+j] = value; 
		}

	}  


	printf("Matrix Initialized using 2D array\n"); 

	return mat;


}