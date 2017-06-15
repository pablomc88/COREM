###################################################################################
##                                                                               ##
## Model of the cat X pathway in the fovea region according to [1]. Most         ##
## parameters are taken from this reference, but undershoot filters were         ##
## reduced to be consistent with tonic responses of X cells. The input must be   ##
## normalized within the range [0,1]. The output is the ganglion's input current ##
## (expressed as a firing rate in Hz).                                           ##
##                                                                               ##
## [1] Masquelier, Timothee. "Relative spike time coding and STDP-based          ##
## orientation selectivity in the early visual system in natural continuous and  ##
## saccadic vision: a computational model." Journal of computational             ##
## neuroscience 32.3 (2012): 425-441.                                            ##
##                                                                               ##
## Author: Pablo Martinez                                                        ##
## email: pablomc@ugr.es                                                         ##
##                                                                               ##
###################################################################################


### Simulation parameters ###

retina.TempStep('1') # simulation step (in ms)
retina.SimTime('1200') # simulation time (in ms)
retina.NumTrials('1') # number of trials
retina.PixelsPerDegree({'4.5070'}) # pixels per degree of visual angle
retina.DisplayDelay('0') # display delay
retina.DisplayZoom({'10.0'}) # display zoom
retina.DisplayWindows('3') # Display windows per row

### Visual input ###

retina.Input('impulse',{'start','300.0','stop','900.0','amplitude','0.4','offset','0.0','sizeX','25','sizeY','25'})

### Creation of computational retinal microcircuits ###

# Temporal modules
retina.Create('LinearFilter','tmp_OPL_center',{'type','Gamma','tau','10.0','n','0.0'})
retina.Create('LinearFilter','tmp_OPL_surround',{'type','Gamma','tau','5.0','n','0.0'})
retina.Create('LinearFilter','tmp_OPL_undershoot',{'type','Gamma','tau','100.0','n','0.0'})
retina.Create('SingleCompartment','V_bipolar',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.0','Cm','1.0','E',{'0.0','0.0'}})
retina.Create('LinearFilter','tmp_Contrast_feedback',{'type','Gamma','tau','5.0','n','0.0'})
retina.Create('LinearFilter','tmp_IPL_undershoot',{'type','Gamma','tau','20.0','n','0.0'})

# Spatial filters
retina.Create('GaussFilter','Gauss_OPL_center',{'sigma','0.15','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_OPL_surround',{'sigma','0.8','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_Contrast_feedback',{'sigma','1.0','spaceVariantSigma','False'})

# Nonlinearities
# One gain of 10 Hz/Lum. unit
retina.Create('StaticNonLinearity','OPL_amplification',{'slope','10.0','offset','0.0','exponent','1.0'})
# OPL undershoot smaller than 0.8
retina.Create('StaticNonLinearity','OPL_undershoot',{'slope','0.5','offset','0.0','exponent','1.0'})
# An OPL relative weight of 1.0 seems a bit excessive
retina.Create('StaticNonLinearity','OPL_relative_weight',{'slope','0.9','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','Contrast_feedback_1',{'slope','1.0','offset','0.0','exponent','2.0'})
retina.Create('StaticNonLinearity','Contrast_feedback_2',{'slope','50.0','offset','5.0','exponent','1.0'})
# IPL undershoot smaller than 0.7
retina.Create('StaticNonLinearity','IPL_undershoot',{'slope','0.2','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','ON_Bip',{'slope','1.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','OFF_Bip',{'slope','-1.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','ON_Bip_rectification',{'slope','150.0','offset','37.0','exponent','1.0','threshold','0.0'})
retina.Create('StaticNonLinearity','OFF_Bip_rectification',{'slope','150.0','offset','37.0','exponent','1.0','threshold','0.0'})

### Connections ###

## OPL ##
# L * E_nc_tauc * T_wu_tauu * G_sigmac
retina.Connect('L_cones','tmp_OPL_center','Current')
retina.Connect('tmp_OPL_center','tmp_OPL_undershoot','Current')
retina.Connect('tmp_OPL_undershoot','OPL_undershoot','Current')
retina.Connect({'tmp_OPL_center',-,'OPL_undershoot'},'Gauss_OPL_center','Current')
# C * E_taus * G_sigmas
retina.Connect('Gauss_OPL_center','tmp_OPL_surround','Current')
retina.Connect('tmp_OPL_surround','Gauss_OPL_surround','Current')
retina.Connect('Gauss_OPL_surround','OPL_relative_weight','Current')
# I_OPL = lambda_OPL (C - S)
retina.Connect({'Gauss_OPL_center',-,'OPL_relative_weight'},'OPL_amplification','Current')

## Contrast gain control ##
# Q(Vbip) = ga0 + lambda_A (V_bip ^ 2)
retina.Connect('V_bipolar','Contrast_feedback_1','Current')
retina.Connect('Contrast_feedback_1','Contrast_feedback_2','Current')
# gA = Q(Vbip) * E_tauA * G_sigmaA
retina.Connect('Contrast_feedback_2','tmp_Contrast_feedback','Current')
retina.Connect('tmp_Contrast_feedback','Gauss_Contrast_feedback','Current')
# dVbip/dt = I_OPL - gA Vbip
retina.Connect('OPL_amplification','V_bipolar','Current')
retina.Connect('Gauss_Contrast_feedback','V_bipolar','Conductance')

## IPL: synaptic current upon ganglion cells ##
# Epsilon Vbip * T_wg_taug
retina.Connect('V_bipolar','tmp_IPL_undershoot','Current')
retina.Connect('tmp_IPL_undershoot','IPL_undershoot','Current')
retina.Connect({'V_bipolar',-,'IPL_undershoot'},'ON_Bip','Current')
retina.Connect({'V_bipolar',-,'IPL_undershoot'},'OFF_Bip','Current')
# I_gang = N (Epsilon Vbip * T_wg_taug)
retina.Connect('ON_Bip','ON_Bip_rectification','Current')
retina.Connect('OFF_Bip','OFF_Bip_rectification','Current')

### Displays and data analysis  ###
retina.Show('Input','False','margin','0')

# Temporal multimeters
retina.multimeter('temporal','Input','Input',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','OPL_center','Gauss_OPL_center',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','OPL_surround','Gauss_OPL_surround',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','OPL_output','OPL_amplification',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','V_bipolar','V_bipolar',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','ON_Bip','ON_Bip',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','OFF_Bip','OFF_Bip',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','ON_Bip_rectification','ON_Bip_rectification',{'x','12','y','12'},'Show','True','startTime','100')
retina.multimeter('temporal','OFF_Bip_rectification','OFF_Bip_rectification',{'x','12','y','12'},'Show','True','startTime','100')

