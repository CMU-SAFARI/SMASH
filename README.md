# SMASH

Source code of the sparse matrix kernels and utilities used to evaluate the schemes presented in
the MICRO'19 paper:

>Konstantinos Kanellopoulos, Nandita Vijaykumar, Christina Giannoula, Roknoddin Azizi, Skanda Koppula, Nika Mansouri Ghiasi, Taha Shahroodi, Juan Gomez-Luna, and Onur Mutlu,
[**SMASH: Co-designing Software Compression and Hardware-Accelerated Indexing for Efficient Sparse Matrix Operations**](https://people.inf.ethz.ch/omutlu/pub/SMASH-sparse-matrix-software-hardware-acceleration_micro19.pdf)

Proceedings of the 52nd International Symposium on Microarchitecture (MICRO), Columbus, OH, USA, October 2019. 

Please cite the above work if you make use of the tools provided in this repository.

The presentations of the paper are available on YouTube:
>[Lightning Talk Video](https://youtu.be/VN0PQ5zgLGg)\
>[Full Talk Video](https://youtu.be/LWYVQ3o_SdU)



## Using the Hierarchical Bitmap Format

We provide the source code to construct three-level hierarchical bitmaps under `/src/bitmap/bitmap.h`. 


## 1. Initialization

### To initialize the bitmaps perform the following steps: 

Read the matrix using the CSR Format.

```
 csr matrix_csr = read_csr(path);
```

Construct Bitmaps and and the Non-Zero Values Array.

```
construct_format(&matrix_smash,comp0,comp1,comp2);	
construct_bitmap0_nza(&matrix_smash,&matrix_csr);
construct_bitmap1(&matrix_smash);
construct_bitmap2(&matrix_smash);
```


## 2. Using the Hierarchical Bitmaps

The main function to manipulate the bitmaps is:
        $index_bitmap(smash *format) 

This function uses the following hardware intrinsic to quickly discover the set bits of the bitmaps:

```
uint64_t find_set_bit(smash* format, uint64_t a)
{
    ..
    uint64_t bit = __builtin_ctzl(a);  // Hardware primitive
    ..
}

```

In case your system does not support the aforementioned instruction we provide an alternative function that sequentially reads the bits of the bitmaps.

## 3. SpMV and SpMM

We provide one SpMV and one SpMM kernel which make use of the Hierarhical Bitmap format. You can execute these two applications using the following instructions:
```
./spmm_bitmap.e -f comp0 -s comp1 -t comp2 -i matrix_csr
./spmv_bitmap.e -f comp0 -s comp1 -t comp2 -i matrix_csr
```
You need to specify the compression ratios and the input matrix using the appropriate options. 

SpMM currently supports the square of a matrix. To avoid transposing, we make inverse the functionality of the row and column for matrix B. 


## 4. Locality Generator

We provide a python tool to produce matrices with various localities of sparsity. 

To produce a matrix execute the script in the following way:


```
python locality_generator.py #num_rows #num_cols #num_of_zero_elements #block_size #num_elements/block
```

Using this script you can produce matrices with different discrete localities base on the size of the block. 


## 5. Storage Calculation

To calculate the storage of the matrix execute the following script:

```
python storage.py #input_file #compression_ratio0 #compression_ratio1 #compression_ratio2
```

The storage calculation script can be used to explore the design space of different compression ratios and bitmap levels.

## 6. Matrix Utilities

We provide handy utilities to transform the matrices to the CSR format. We found out that multiple matrices from SNAP and UF contain mistakes (e.g., wrong number of non-zero elements). We provide the script  `sort_edges.py`to produce a correct matrix and sort the non-zero elements. The result matrix of this script is fed in the `florida_to_csr.py` script to produce the final matrix used by the `read_csr` function.



## Contact 

Konstantinos Kanellopoulos (konkanello@gmail.com)
 

