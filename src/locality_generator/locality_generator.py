import os 
import sys




def produce_matrix(rows,cols,nnz,block_size,locality):

    blocks = (rows*cols)/block_size
    print blocks
    row_col_list = []
    
    locality_elements = locality    
    block = []
    row=0
    col=0

    for i in range(blocks):
        
        element = block_size*i
        row = element / cols
        col = element % cols
        
        for j in range(locality_elements):
            row_col_list.append((row,col))
            if(len(row_col_list) > nnz ):
                del row_col_list[-1]
                print row_col_list
                return row_col_list
            col = col +1 
            if(col == cols):
                col = 0
                row = row + 1
            if(row == rows and col == cols):
                print row_col_list
                return row_col_list
        


def print_matrix_to_file(adjacency_list,rows,cols):

    os.system("touch ./matrix.out")
    with open("matrix.out","w") as f:
        f.write(str(rows) +" "+str(cols)+'\n')
        print(adjacency_list)
        for element in adjacency_list:
            f.write (str(element[0])+" "+str(element[1])+'\n')
            


        



rows = (int)(sys.argv[1])
cols = (int)(sys.argv[2])
nnz  = (int)(sys.argv[3])
block_size =  (int)(sys.argv[4])
locality =  (int)(sys.argv[5]) # Specify number of elements per block

adj = produce_matrix(rows,cols,nnz,block_size,locality)
print_matrix_to_file(adj,rows,cols)
