import sys

fp = open(sys.argv[1],'r')
lines = fp.readlines()



rows = (int)(lines[0].split(' ')[0])
columns = (int)(lines[0].split(' ')[1])
nnz = (int)(lines[0].split(' ')[2])

print(rows)
i=0
no_value = 0
row_ptr = [0 for x in range(rows+1)]
col_ptr = [[] for x in range(rows+1)]
values   = [[] for x in range(rows+1)]
for line in lines:
#    print (line)
    if i != 0:
        splits = line.split(' ')
        node1 =(int)(line.split(' ')[0])
        node2 =(int)(line.split(' ')[1])
        if(len(splits) == 3):
            value =(float)(line.split(' ')[2])
        else:
            value = 1
        row_ptr[node1] = row_ptr[node1]+1
        col_ptr[node1].append(node2)
        values[node1].append(value)
    i = i + 1

print (row_ptr[1])
print (col_ptr[1])

fp_csr = open(sys.argv[2],'w')



fp_csr.write((str)(rows))
fp_csr.write('\n')
fp_csr.write((str)(nnz))
fp_csr.write('\n')

acc = 0
for i in range(rows+1):
    fp_csr.write((str)(acc))
    fp_csr.write('\n')
    acc = acc + row_ptr[i]


col_ptr =  [inner for outer in col_ptr for inner in outer]
for i in range(nnz):
    fp_csr.write((str)(col_ptr[i]))
    fp_csr.write('\n')

values =  [inner for outer in values for inner in outer]
for i in range(nnz):
    fp_csr.write((str)(values[i]))
    fp_csr.write('\n')

fp.close()
fp_csr.close()

        

