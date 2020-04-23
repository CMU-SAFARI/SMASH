#!/bin/bash



PATH_MATRIX=/mnt/panzer/kanellok/SpMv-XComp/inputs/matrix_sort/
PATH_CSR=/mnt/panzer/kanellok/SpMv-XComp/inputs/csr
for matrix in $PATH_MATRIX/*; 
do 
	filename="${matrix##*/}"
	python florida_to_csr.py $matrix $PATH_CSR/$filename
done 
