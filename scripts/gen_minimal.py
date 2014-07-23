#!/bin/python

f = open("input_data_minimal_for_test.txt", "w")
input_data = [12, 10, 9, 8, 6, 5, 4, 3, 3]

f.write("%d\n" % sum(input_data))
f.write("\n1\n")
f.write("%s\n" % " ".join(str(x+1) for x in range(sum(input_data))))
f.write("\n1\n")

i = 0
while sum(input_data)>0:
    if input_data[i] > 0:
        f.write("%d " % i)
        input_data[i] -= 1
    i += 1
    if i==len(input_data):
        i = 0

f.close()
