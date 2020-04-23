#!/bin/bash



PATH_MATRIX=/mnt/panzer/kanellok/SpMv-XComp/inputs/matrix/
PATH_SORT=/mnt/panzer/kanellok/SpMv-XComp/inputs/matrix_sort
for matrix in $PATH_MATRIX/*; 
do 
	filename="${matrix##*/}"
	python sort_edges.py $matrix $PATH_SORT/$filename
done 
