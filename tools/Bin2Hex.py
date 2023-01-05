#!/usr/bin/python3

#nicht schön, soll nur funktionieren

import math

with open('input.txt', 'r') as file:
    data = file.read().replace('\n', '')

maxval = math.ceil(len(data)/8)


output = ""
page = 0
for index in range(maxval):
    if index%128 == 0 and index != 0:
        page+=1
        
    index = index-128*page

    byte = data[128*7+index+page*128*8] + data[128*6+index+page*128*8] + data[128*5+index+page*128*8] + data[128*4+index+page*128*8] + data[128*3+index+page*128*8] + data[128*2+index+page*128*8] + data[128+index+page*128*8] + data[index+page*128*8]
    output += "0x{:02X}".format(int(byte,2))

print(output)
