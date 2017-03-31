###################################################################################
##                                                                               ##
## Model of primate cones and horizontal cells adapted from the model by van     ##
## Hateren [1]. This script shows the response of the model to a a 100-ms step   ##
## of contrast 2 at a background illuminance of 100 td (see Fig. 6 in [1]).      ##
##                                                                               ##
## Parameters (default from Van Hateren's model):                                ##
## tauR = 0.49                                                                   ##
## tauE = 16.8                                                                   ##
## cb = 2.8 * 10**(-3)                                                           ##
## kb = 1.63 * 10**(-4)                                                          ##
## nX = 1.0                                                                      ##
## tauC = 2.89                                                                   ##
## ac = 9.08 * 10**(-2)                                                          ##
## nc = 4.0                                                                      ##
## taum = 4.0                                                                    ##
## ais = 7.09 * 10**(-2)                                                         ##
## gamma = 0.678                                                                 ##
## tauis = 56.9                                                                  ##
## gs = 0.5 (instead of 8.81)                                                    ##
## tau1 = 4.0                                                                    ##
## tau2 = 4.0                                                                    ##
## tauh = 20.0                                                                   ##
##                                                                               ##
## [1] van Hateren, Hans. "A cellular and molecular model of response kinetics   ##
## and adaptation in primate cones and horizontal cells." Journal of vision 5.4  ##
## (2005): 5-5.                                                                  ##
##                                                                               ##
## Author: Pablo Martinez                                                        ##
## email: pablomc@ugr.es                                                         ##
##                                                                               ##
###################################################################################

### Simulation parameters ###

retina.TempStep('1') # simulation step (in ms)
retina.SimTime('800') # simulation time (in ms)
retina.NumTrials('1') # number of trials
retina.PixelsPerDegree({'1.0'}) # pixels per degree of visual angle
retina.DisplayDelay('0') # display delay
retina.DisplayZoom({'10.0'}) # display zoom
retina.DisplayWindows('4') # Display windows per row

### Visual input ###

retina.Input('impulse',{'start','500.0','stop','600.0','amplitude','200.0','offset','100.0','sizeX','1','sizeY','1'})

### Creation of computational retinal microcircuits ###

# Temporal modules
retina.Create('LinearFilter','tmp_tauR',{'type','Gamma','tau','0.49','n','0.0'})
retina.Create('LinearFilter','tmp_tauE',{'type','Gamma','tau','16.8','n','0.0'})
retina.Create('SingleCompartment','calcium_feedback_SC',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.0','Cm','1.0','E',{'0.0','0.0'}})
retina.Create('LinearFilter','tmp_tauC',{'type','Gamma','tau','2.89','n','0.0'})
retina.Create('LinearFilter','tmp_taum',{'type','Gamma','tau','4.0','n','0.0'})
retina.Create('LinearFilter','tmp_tauis',{'type','Gamma','tau','56.9','n','0.0'})
retina.Create('LinearFilter','tmp_tau1',{'type','Gamma','tau','4.0','n','0.0'})
retina.Create('LinearFilter','tmp_tau2',{'type','Gamma','tau','4.0','n','0.0'})
retina.Create('LinearFilter','tmp_tauh',{'type','Gamma','tau','20.0','n','0.0'})

# Nonlinearities
retina.Create('StaticNonLinearity','beta',{'slope','0.000163','offset','0.0028','exponent','1.0'})
retina.Create('StaticNonLinearity','X',{'slope','1.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','1_div_alpha',{'slope','0.0908','offset','1.0','exponent','4.0'})
retina.Create('StaticNonLinearity','alpha',{'slope','1.0','offset','0.0001','exponent','-1.0'})
retina.Create('StaticNonLinearity','ais',{'slope','0.0709','offset','0.0','exponent','0.678'})
# Constant calculated by using a dark stimulus: (bkg_illuminance = pulse_amplitude = 0)
# Vis_dark = 13.9 mV
retina.Create('StaticNonLinearity','Vis',{'slope','1.0','offset','-13.9','exponent','1.0'})
retina.Create('StaticNonLinearity','gs',{'slope','0.5','offset','0.0','exponent','1.0'})

### Connections ###

# Phototransduction cascade
retina.Connect('L_cones','tmp_tauR','Current')
retina.Connect('tmp_tauR','tmp_tauE','Current')
retina.Connect('tmp_tauE','beta','Current')

# Calcium feedback
retina.Connect('alpha','calcium_feedback_SC','Current')
retina.Connect('beta','calcium_feedback_SC','Conductance')
retina.Connect('calcium_feedback_SC','X','Current')
retina.Connect('X','tmp_tauC','Current')
retina.Connect('tmp_tauC','1_div_alpha','Current')
retina.Connect('1_div_alpha','alpha','Current')

# Inner segment
retina.Connect({'X',/,'tmp_tauis'},'tmp_taum','Current')
retina.Connect('tmp_taum','ais','Current')
retina.Connect('ais','tmp_tauis','Current')

# Vis - Vis_dark
retina.Connect('tmp_taum','Vis','Current')

# Horizontal cell feedback
retina.Connect({'Vis',-,'tmp_tauh'},'gs','Current')
retina.Connect('gs','tmp_tau1','Current')
retina.Connect('tmp_tau1','tmp_tau2','Current')
retina.Connect('tmp_tau2','tmp_tauh','Current')

### Displays and data analysis  ###
retina.Show('X','False','margin','0')

# Temporal multimeters

#retina.multimeter('temporal','Input','Input',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','tmp_tauE','tmp_tauE',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','beta','beta',{'x','0','y','0'},'Show','True')

#retina.multimeter('temporal','tmp_tauC','tmp_tauC',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','1_div_alpha','1_div_alpha',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','alpha','alpha',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','X','X',{'x','0','y','0'},'Show','True')

#retina.multimeter('temporal','ais','ais',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','tmp_tauis','tmp_tauis',{'x','0','y','0'},'Show','True')
#retina.multimeter('temporal','tmp_taum','tmp_taum',{'x','0','y','0'},'Show','True')

retina.multimeter('temporal','Vis','Vis',{'x','0','y','0'},'Show','False')
