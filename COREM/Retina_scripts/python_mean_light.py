#!/usr/bin/env python
# -*- coding: utf-8 -*-

###################################################################################
##                                                                               ##
## Python script that executes in COREM the retina model for adaptation to the   ##
## mean light intensity.                                                         ##
##                                                                               ##
## Author: Pablo Martinez                                                        ##
## email: pablomc@ugr.es                                                         ##
##                                                                               ##
###################################################################################

import matplotlib.pyplot as plt
import numpy
import sys
import os

### root path ###
root = "/home/pablo/Desktop/COREM_v2/COREM/"

### folder for retina scripts ###
os.system("mkdir "+root+"Retina_scripts/scripts")

### Best set of parameters ###
# Parameter 1: tau_photo (ms)
# Parameter 2: n_photo
# Parameter 3: C_calcium (pF)
# Parameter 4: tau_calcium (ms)
# Parameter 5: n_calcium
# Parameter 6: C_inner (pF)
# Parameter 7: tau_inner (ms)
# Parameter 8: n_inner
# Parameter 9: tau_horizontal (ms)
# Parameter 10: n_horizontal
# Parameter 11: a_horizontal
# Parameter 12: c_horizontal
# Parameter 13: b_horizontal
# Parameter 14: a_additional_SNL
# Parameter 15: c_additional_SNL
# Parameter 16: a_inner
# Parameter 17: c_inner
# Parameter 18: threshold_inner
# Parameter 19: a_calcium
# Parameter 20: c_calcium
# Parameter 21: a_photo
# Parameter 22: c_photo

best_parameters = [ 20.0, 2.0,    100.0, 5.0, 2.0,      100.0, 10.0,3.0,       55.0,  1.0,
230.0, 4.0,20.0,      0.35, 0.0,     100.0, -2.0,-0.07,       0.15, -1.0,     -0.1, 0.0]

### Data to fit ###
# X Data
x0 = [0.0, 12.5, 25.0, 37.5, 50.0,56.25, 62.5,68.75, 75.0,81.25, 87.5,93.75, 100.0, 106.25, 112.5,118.75, 125.0,131.25, 137.5,143.75, 150.0, 162.5, 175.0, 187.5, 200.0, 212.5, 225.0, 237.5, 250.0, 262.5, 275.0, 287.5, 300.0]

x1 = [0.0, 12.5, 25.0, 37.5, 50.0, 62.5, 75.0, 87.5, 100.0, 112.5, 125.0, 137.5, 150.0, 162.5, 175.0, 187.5, 200.0, 212.5, 225.0, 237.5, 250.0, 262.5, 275.0, 287.5, 300.0]

## 100 td
# 10 ms
y0 = [0.0, 0.0, 0.0, 0.0, -3.0,-7.0, -7.0,-3.0, 0.0,-1.0, -2.0,-1.0, 0.8,0.8, 0.2,0.3, 0.8,0.8, 0.6,0.6, 0.5, 0.5, 0.4, 0.3, 0.2, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]
# 100 ms
y1 = [0.0, 0.0, 0.0, 0.0, -3.0, -11.0, -9.7, -9.5, -9.0, -8.9, -8.6, -8.4, -6.0, -0.7, 0.0, 1.6, 1.6, 1.8, 1.5, 1.2, 1.0, 0.8, 0.5, 0.4, 0.4]
# 160 ms
y2 = [0.0, 0.0, 0.0, 0.0, -3.0, -11.0, -10.0, -9.8, -9.1, -9.0, -8.8, -8.6, -8.5, -8.4, -8.4, -8.4, -8.2, -3.8, -0.2, 0.5, 1.9, 1.9, 2.0, 1.8, 1.2]

## 10 td
# 10 ms
y3 = [0.0,0.0 ,0.0 ,0.0 ,-0.5 ,-2.0 ,-1.2 ,-0.8 ,-0.8, -0.5, -0.25,-0.1,0.0,0.0 ,0.0,0.0,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 , 0.0]
# 100 ms
y4 = [0.0,0.0 ,0.0 ,0.0, -0.5,-3.0 , -4.2, -4.4, -4.7, -4.8, -4.7, -4.6, -4.0,-2.5 ,-1.5,-1.0,-0.5 ,-0.25 ,-0.1 ,-0.1 ,-0.1 ,-0.1 , -0.1,-0.1 ,-0.1 ]
# 160 ms
y5 = [0.0,0.0 ,0.0 ,0.0, -0.5, -3.0, -4.2,-4.5 ,-4.9,-4.9 ,-4.7 ,-4.6,-4.4, -4.4,-4.3,-4.3, -4.25, -3.5,-1.7 ,-1.2 ,-0.6 ,-0.4 ,-0.2 ,-0.1 ,-0.1 ]

## 1 td
# 10 ms
y6 = [0.0,0.0 ,0.0 ,0.0,-0.1 ,-0.25 ,-0.2 ,-0.16 ,-0.12, -0.1,-0.1 ,-0.08,-0.05,0.0 ,0.0,0.0,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ]
# 100 ms
y7 = [0.0,0.0 ,0.0 ,0.0,-0.1 ,-0.3 ,-0.6 ,-0.6 ,-0.85, -1.0, -1.1,-1.15,-1.2,-1.0 ,-0.75,-0.55,-0.4 ,-0.3 ,-0.2 ,-0.05 ,-0.05 ,-0.05 ,-0.02 ,0.0 , 0.0]
# 160 ms
y8 = [0.0,0.0 ,0.0 ,0.0, -0.1,-0.3 ,-0.6 ,-0.75 ,-0.85,-1.1 ,-1.2 ,-1.2,-1.25,-1.25 ,-1.25,-1.2,-1.2 ,-1.1 ,-0.85 ,-0.7 ,-0.55 ,-0.35 ,-0.25 ,-0.15 ,-0.1 ]


### Function to generate scripts ###
def generateScript(individual,simParam,ID,root):

    text_file = open(root+"Retina_scripts/scripts/retina"+str(ID)+".py", "w")

    text_file.write(""+

    "retina.TempStep('1')\n"+

    "retina.SimTime('800')\n"+

    "retina.NumTrials('1')\n"+

    "retina.PixelsPerDegree({'1'})\n"+

    "retina.DisplayDelay('0')\n"+

    "retina.DisplayZoom({'10.0'})\n"+

    "retina.DisplayWindows('3')\n"+

    "retina.Input('impulse',{'start','525.0','stop',"+str(525.0+simParam[0])+",'amplitude','"+str(8.0*simParam[1])+"','offset','"+str(simParam[1])+"','sizeX','1','sizeY','1'})\n"+

    "retina.Create('LinearFilter','phototransduction',{'type','Gamma','tau','"+str(individual[0])+"','n','"+str(individual[1])+"'})\n"+

    "retina.Create('SingleCompartment','conductanceCalciumFeedback',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.0','Cm','"+str(individual[2])+"','E',{'0.0','0.0'}})\n"+
    "retina.Create('LinearFilter','calciumFeedback',{'type','Gamma','tau','"+str(individual[3])+"','n','"+str(individual[4])+"'})\n"+

    "retina.Create('SingleCompartment','conductanceInnerSegment',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.0','Cm','"+str(individual[5])+"','E',{'0.0','0.0'}})\n"+
    "retina.Create('LinearFilter','innerSegmentFeedback',{'type','Gamma','tau','"+str(individual[6])+"','n','"+str(individual[7])+"'})\n"+

    "retina.Create('LinearFilter','horizontal',{'type','Gamma','tau','"+str(individual[8])+"','n','"+str(individual[9])+"'})\n"+

    "retina.Create('StaticNonLinearity','SNL_phototransduction',{'slope','"+str(individual[20])+"','offset','"+str(individual[21])+"','exponent','1.0'})\n"+
    "retina.Create('StaticNonLinearity','SNL_calciumFeedback',{'slope','"+str(individual[18])+"','offset','"+str(individual[19])+"','exponent','4.0'})\n"+
    "retina.Create('StaticNonLinearity','SNL_conductanceInnerSegment',{'slope','"+str(individual[15])+"','offset','"+str(individual[16])+"','exponent','2.0','threshold','"+str(individual[17])+"'})\n"+
    "retina.Create('StaticNonLinearity','SNL_horizontal',{'slope','"+str(individual[13])+"','offset','"+str(individual[14])+"','exponent','1.0'})\n"+
    "retina.Create('SymmetricSigmoidNonLinearity','SNL_photoreceptors',{'slope','"+str(individual[10])+"','offset','"+str(individual[11])+"','max','"+str(individual[12])+"'})\n"+

    "retina.Connect('L_cones','phototransduction','Current')\n"+
    "retina.Connect('phototransduction','SNL_phototransduction','Current')\n"+

    "retina.Connect('SNL_phototransduction','conductanceCalciumFeedback','Current')\n"+
    "retina.Connect('conductanceCalciumFeedback','SNL_calciumFeedback','Current')\n"+
    "retina.Connect('SNL_calciumFeedback','calciumFeedback','Current')\n"+
    "retina.Connect('calciumFeedback','conductanceCalciumFeedback','Conductance')\n"+

    "retina.Connect('conductanceCalciumFeedback','conductanceInnerSegment','Current')\n"+
    "retina.Connect('conductanceInnerSegment','SNL_conductanceInnerSegment','Current')\n"+
    "retina.Connect('SNL_conductanceInnerSegment','innerSegmentFeedback','Current')\n"+
    "retina.Connect('innerSegmentFeedback','conductanceInnerSegment','Conductance')\n"+

    "retina.Connect({'conductanceInnerSegment',-,'horizontal'},'SNL_horizontal','Current')\n"+
    "retina.Connect('SNL_horizontal','horizontal','Current')\n"+

    "retina.Connect('SNL_horizontal','SNL_photoreceptors','Current')\n"+

    "retina.Show('Input','False','margin','0')\n"+

    "retina.multimeter('temporal','SNL_photoreceptors','SNL_photoreceptors',{'x','0','y','0'},'Show','False')\n"


    )

    text_file.close()


### call the retina model in COREM ###
def retina_luminance(individual,ID,sel):

    # generate script
    if(sel==0):
        generateScript(individual,[10.0,100.0],ID,root)
    elif(sel==1):
        generateScript(individual,[100.0,100.0],ID,root)
    elif(sel==2):
        generateScript(individual,[160.0,100.0],ID,root)
    elif(sel==3):
        generateScript(individual,[10.0,10.0],ID,root)
    elif(sel==4):
        generateScript(individual,[100.0,10.0],ID,root)
    elif(sel==5):
        generateScript(individual,[160.0,10.0],ID,root)
    elif(sel==6):
        generateScript(individual,[10.0,1.0],ID,root)
    elif(sel==7):
        generateScript(individual,[100.0,1.0],ID,root)
    elif(sel==8):
        generateScript(individual,[160.0,1.0],ID,root)


    # run simulation
    call = "./corem Retina_scripts/scripts/retina"+str(ID)+".py "
    os.system(call)

    # return simulated values
    cnt = True

    try:
        tmp = numpy.float64(numpy.loadtxt(root+'results/_output_multimeter.txt'))
        cnt = False
    except IOError:
        cnt = True

    if(cnt==False):
        # V - V_0
        V_0 = tmp[499]
        for i in range(0,len(tmp)):
                tmp[i] = tmp[i] - V_0


        vector_to_return = tmp[499:800]
    else:
        vector_to_return = numpy.zeros(301)

    return vector_to_return


### Plotting ###

fig1 = plt.figure()
xdata = numpy.linspace(0, 300, 301)

data_y = retina_luminance(best_parameters,0,0)
plt.plot(xdata,data_y,'b')

data_y = retina_luminance(best_parameters,1,1)
plt.plot(xdata,data_y,'g')

data_y = retina_luminance(best_parameters,2,2)
plt.plot(xdata,data_y,'r')

plt.plot(x0,y0,'o')
plt.plot(x1,y1,'o')
plt.plot(x1,y2,'o')

fig2 = plt.figure()

data_y = retina_luminance(best_parameters,3,3)
plt.plot(xdata,data_y,'b')

data_y = retina_luminance(best_parameters,4,4)
plt.plot(xdata,data_y,'g')

data_y = retina_luminance(best_parameters,5,5)
plt.plot(xdata,data_y,'r')

plt.plot(x1,y3,'o')
plt.plot(x1,y4,'o')
plt.plot(x1,y5,'o')

fig3 = plt.figure()

data_y = retina_luminance(best_parameters,6,6)
plt.plot(xdata,data_y,'b')

data_y = retina_luminance(best_parameters,7,7)
plt.plot(xdata,data_y,'g')

data_y = retina_luminance(best_parameters,8,8)
plt.plot(xdata,data_y,'r')

plt.plot(x1,y6,'o')
plt.plot(x1,y7,'o')
plt.plot(x1,y8,'o')


plt.show()


