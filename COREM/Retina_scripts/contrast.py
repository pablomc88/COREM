###################################################################################
##                                                                               ##
## Retina model that reproduces contrast adaptation experiment by Ozuysal and    ## 
## Baccus [1]                                                                    ##
##                                                                               ##
## [1] Ozuysal, Yusuf, and Stephen A. Baccus. "Linking the computational         ##
## structure of variance adaptation to biophysical mechanisms." Neuron 73.5      ##
## (2012): 1002-1015.                                                            ##
##                                                                               ##
## Author: Pablo Martinez                                                        ##
## email: pablomc@ugr.es                                                         ##
##                                                                               ##
###################################################################################


### Simulation parameters ###

retina.TempStep('5') # simulation step (in ms)
retina.SimTime('40000') # simulation time (in ms)
retina.NumTrials('30') # number of trials
retina.PixelsPerDegree({'1'}) # pixels per degree of visual angle
retina.DisplayDelay('0') # display delay
retina.DisplayZoom({'10.0'}) # display zoom
retina.DisplayWindows('3') # Displays per row

### Visual input ###

retina.Input('whiteNoise',{'mean','0.5','contrast1','0.5','contrast2','0.1','period','1.0','switch','20000','sizeX','1','sizeY','1'})

### Creation of computational retinal microcircuits ###

# Temporal modules
retina.Create('LinearFilter','photoreceptors',{'type','Gamma','tau','75.68','n','9.74'})
retina.Create('LinearFilter','horizontal',{'type','Gamma','tau','45.49','n','6.37'})
retina.Create('SingleCompartment','bipolar',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.0','Cm','1200.0','E',{'0.0','0.0'}})
retina.Create('LinearFilter','conductance',{'type','Gamma','tau','31.0','n','5.0'})

# Nonlinearities
retina.Create('StaticNonLinearity','SNL_photoreceptors',{'slope','-1.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','SNL_horizontal',{'slope','1.007','offset','0.825','exponent','1.0'})
retina.Create('StaticNonLinearity','SNL_conductance',{'slope','70.77','offset','6.64','exponent','2.0'})
retina.Create('StaticNonLinearity','SNL_bipolar',{'slope','66.8','offset','4.21','exponent','1.0'})
retina.Create('ShortTermPlasticity','SNL_ganglion',{'slope','0.52','offset','-95.0','exponent','1.0','kf','0.5','kd','6.0','tau','12000.0'})
retina.Create('StaticNonLinearity','SNL_ganglion_static',{'slope','1.0','offset','-3.0','exponent','1.0','threshold','-81.0'})

### Connections ###

retina.Connect('L_cones','photoreceptors','Current')
retina.Connect('photoreceptors','SNL_photoreceptors','Current')
retina.Connect('SNL_photoreceptors','horizontal','Current')
retina.Connect('horizontal','SNL_horizontal','Current')
retina.Connect({'SNL_photoreceptors',-,'SNL_horizontal'},'bipolar','Current')

retina.Connect('bipolar','SNL_conductance','Current')
retina.Connect('SNL_conductance','conductance','Current')
retina.Connect('conductance','bipolar','Conductance')

retina.Connect('bipolar','SNL_bipolar','Current')
retina.Connect('SNL_bipolar','SNL_ganglion','Current')
retina.Connect('SNL_ganglion','SNL_ganglion_static','Current')

### Displays and data analysis  ###
retina.Show('Input','False','margin','0')

# LN multimeter
retina.multimeter('Linear-Nonlinear','Ganglion cell','SNL_ganglion_static',{'x','0','y','0','segment','1000.0','interval','10.0','start','21000','stop','30000.0','rangePlot','330','Show','True'}

