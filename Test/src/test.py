#!/opt/local/bin/python
import cv

obj=cv.LoadImage("GT-00000363.PNG")
#obj[575,719]
#iplimage(nChannels=3 width=720 height=576 widthStep=2160 )>

for i in range(obj.width):
    for j in range(obj.height):
        if obj[j,i][0] != 0:
            print obj[j,i]

