#!/usr/bin/python3

#A small tool that creates the content of a byte array from the output of the following website for the used display: https://www.dcode.fr/binary-image
#create a black and white png with the dimensions: 128x32 Pixel
#The script searches the current folder for the input.txt file
#The array elements are output in the terminal.

import math

#read file and remove newlines
with open('input.txt', 'r') as file:
    data = file.read().replace('\n', '')

#calc length of the array
maxval = math.ceil(len(data)/8)


output = ""
page = 0
#loop for every array element
for index in range(maxval):

    #Read specification of the display to understand how to create the bytes. Isn't pretty, should only work.
    if index%128 == 0 and index != 0:
        page+=1
        
    index = index-128*page

    byte = data[128*7+index+page*128*8] + data[128*6+index+page*128*8] + data[128*5+index+page*128*8] + data[128*4+index+page*128*8] + data[128*3+index+page*128*8] + data[128*2+index+page*128*8] + data[128+index+page*128*8] + data[index+page*128*8]
    output += "0x{:02X}, ".format(int(byte,2))

print(output)
