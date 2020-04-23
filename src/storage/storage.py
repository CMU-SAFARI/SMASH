import sys
import os




def storage_calculation(file,b2,b1,b0,output):
    bitmap2 = {}
    bitmap1 = {}
    bitmap0 = {}
    row_ptr = []
    col_ind = []
    values = []
    with open(file) as f:
        lines = f.readlines()
        rows = (int)(lines[0])
        print("Rows = "+(str)(rows))
        bitmap2_ratio = (int)(b2)
        bitmap1_ratio = (int)(b1)
        bitmap0_ratio = (int)(b0)

        bitmap0_size = (rows*rows)/bitmap0_ratio
        bitmap1_size = (bitmap0_size)/bitmap1_ratio
        bitmap2_size = (bitmap1_size)/bitmap2_ratio
        nnz = (int)(lines[1])
        index = 2
        for i in range(rows+1):
            row_ptr.append(int(lines[index]))
            index  = index + 1
        for i in range(nnz):
            col_ind.append(int(lines[index]))
            index = index + 1
        for i in range(nnz):
            values.append(float(lines[index]))
            index = index + 1

        for i in range(rows):
            elements = row_ptr[i+1] - row_ptr[i]
            #print elements
            for j in range(elements):
                row_index = i
                column_index = col_ind[row_ptr[i]+j]
                #print ("Row = " + str(row_index))
                #print ("Column = " + str(column_index))
                bitmap0[(row_index*rows+column_index)/bitmap0_ratio] = 1


        for i in range(bitmap0_size):
            if(bitmap0.get(i,-1)!=-1):
                bitmap1[i/bitmap1_ratio] = 1

        for i in range(bitmap1_size):
            if(bitmap1.get(i,-1)!=-1):
                bitmap2[i/bitmap2_ratio] = 1

    storage = len(bitmap0.keys())*bitmap0_ratio*4*8 + len(bitmap1.keys())*bitmap1_ratio + len(bitmap2.keys())*bitmap2_ratio + bitmap2_size
    storage = (storage*1.0)/(8.0)
    #print (row_ptr)
    #print (col_ind)
    #print (values)
    f  = open(output,"a+")
    f.write(str(b2)+'.'+str(b1)+'.'+str(b0)+'\n')
    f.write("Storage:"+str(storage)+'\n')
    f.write("Rows:"+str(rows))
    f.write("Bitmap2:"+str(bitmap2_size)+'\n')
    f.write("Bitmap1:"+str(len(bitmap2.keys())*bitmap2_ratio)+'\n')
    f.write("Bitmap0:"+str(len(bitmap1.keys())*bitmap1_ratio)+'\n')
    f.write("CSR Storage:"+str(len(row_ptr)*4+len(col_ind)*4+len(values)*4)+'\n')
    f.close()


b2 = sys.argv[4]
b1 = sys.argv[3]
b0 = sys.argv[2]


storage_calculation('./'+sys.argv[1],b2,b1,b0,sys.argv[1].split('/')[-1])















