import sys

fp = open(sys.argv[1],'r')
lines = fp.readlines()



rows = (int)(lines[0].split(' ')[0])
columns = (int)(lines[0].split(' ')[1])
nnz = (int)(lines[0].split(' ')[2])
weight  = (int)(lines[0].split(' ')[2])
print(rows)
print (columns)
print (nnz)
i=0
no_value = 0
nodes = [[] for x in range(rows+1)]
values   = [[] for x in range(rows+1)]
nnz_correct = 0
for line in lines:
    max = 0
    #print (line)
    if i != 0:
        splits = line.split(' ')
        node1 =(int)(line.split(' ')[0])
        #if(node1 > rows+1):
         #   print ("Error"+str(node1))
        if(node1 > max):
            max = node1
        node2 =(int)(line.split(' ')[1])
        if(len(splits) == 3):
            value =(float)(line.split(' ')[2])
        else:
            value = 1
        #nodes[node1].append((node2,value))
        nnz_correct = nnz_correct + 1
        
    i = i + 1

#print(nodes)
print(nnz_correct)
print(max)
fp_sort = open(sys.argv[2],'w')



fp_sort.write((str)(rows))
fp_sort.write(' ')

fp_sort.write((str)(rows))
fp_sort.write(' ')

nnz_sanity = 0
for i in range(rows):
    for j in range(len(nodes[i])):
            nnz_sanity = nnz_sanity + 1

fp_sort.write((str)(nnz_sanity))
fp_sort.write(' ')

fp_sort.write((str)(1))
fp_sort.write('\n')

nnz_sanity = 0
for i in range(rows):
    for j in range(len(nodes[i])):
        fp_sort.write((str)(i))
        fp_sort.write(' ')
        fp_sort.write((str)(nodes[i][j][0]))
        fp_sort.write(' ')
        fp_sort.write((str)(nodes[i][j][1]))
        fp_sort.write('\n')
        nnz_sanity = nnz_sanity + 1


print(nnz_sanity)
fp.close()
fp_sort.close()

        

