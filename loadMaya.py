import os
import os.path
import sys
import maya.mel as mel
import maya.cmds as cmds

file1 = open(os.path.expanduser("~/Desktop/School/Senior Spring/CIS 497/qt-knots/qt-knots/controlPoints.txt"), 'r')

# get rid of line that seperates 
line = file1.readline()

# example: curve -d 3 -p 0 0 0 -p 1 1 1 -p 2 2 2 -p 3 3 3 -p 4 4 4 -k 0 -k 0 -k 0 -k 1 -k 2 -k 2 -k 2;
execution = 'curve -d 3'
numPoints = 0

firstPoint = ''
first = True
secondPoint = ''
second = True

while True:
    # Get next line from file
    line = file1.readline()
    if first:
    	first = False
    	firstPoint = line
    else:
    	if second:
    		second = False
    		secondPoint = line
 
    # if line is empty
    # end of file is reached
    if not line:
        break
    x = line.strip()
    execution += ' -p ' + x
    numPoints= numPoints + 1

for x in range(3):
	execution += ' -k 0'
for x in range(1, numPoints - 3):
	execution += ' -k ' + str(x)
for x in range(3):
	execution += ' -k ' + str(numPoints - 3)

mel.eval(execution + ';')
file1.close()

firstPointArray = [float(x) for x in firstPoint.split()] 
secondPointArray = [float(x) for x in secondPoint.split()] 
vector = ''

for index in range(len(firstPointArray)):
	pp = secondPointArray[index] - firstPointArray[index]
	vector += str(pp) 

print(firstPoint)
print(secondPoint)
print(vector)

angle = mel.eval('angleBetween -euler -v1 ' + vector + ' -v2 ' + '0.0 1.0 0.0;')
print(angle)

mel.eval('polyCylinder -r 1 -h 2 -sx 20 -sy 1 -sz 1 -ax 0 1 0 -rcp 0 -cuv 3 -ch 1;')
mel.eval('select -r pCylinder1;')
mel.eval('scale -r 0.203127 1 0.203127;')
mel.eval('scale -r 1 0.0997738 1;')
mel.eval('scale -r 0.454723 1 0.454723;')
mel.eval('scale -r 1 0.22715 1;')
mel.eval('move -a ' + firstPoint + ';')
mel.eval('rotate -r ' + str(angle[0]) + ' ' + str(angle[2]) + ' ' + str(angle[2]) + ';')
# extrude
mel.eval('polyExtrudeFacet -constructionHistory 1 -keepFacesTogether 1 -pvx 7.499999991 -pvy -3.499999986 -pvz 1.727336381 -divisions 30 -twist 0 -taper 1 -off 0 -thickness 0 -smoothingAngle 30 -inputCurve curve1  pCylinder1.f[20:39];')
mel.eval('setAttr "polyExtrudeFace1.divisions" 400;')




