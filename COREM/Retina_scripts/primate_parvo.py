###################################################################################
##                                                                               ##
## Model of the primate Parvocellular pathway implemented with conductance-based ##
## neurons. Adapted from the model proposed in [1]. Some parameters of this      ##
## model are different from the parameters specified in Tables 1 and 2 in [1].   ##
## Amacrine cells have not been considered in this first version of the model.   ##
## Comment and uncomment the two different types of visual stimuli to see the    ##
## effects on the retinal cells' responses.                                      ##
##                                                                               ##
## [1] Martinez-Cañada, P., Morillas, C., Pelayo, F. (2017) A Conductance-Based  ##
## Neuronal Network Model for Color Coding in the Primate Foveal Retina. In      ##
## IWINAC 2017.                                                                  ##
##                                                                               ##
## Author: Pablo Martinez                                                        ##
## email: pablomc@ugr.es                                                         ##
##                                                                               ##
###################################################################################


### Simulation parameters ###

retina.TempStep('1') # simulation step (in ms)
retina.SimTime('1200') # simulation time (in ms)
retina.NumTrials('1') # number of trials
retina.PixelsPerDegree({'20.0'})    # pixels per degree of visual angle
                                    # (there are 20 x 20 cells within 1 x 1 deg
                                    # of visual angle)
retina.DisplayDelay('0') # display delay
retina.DisplayZoom({'10.0'}) # display zoom
retina.DisplayWindows('3') # Display windows per row

### Visual input ###

# The input is specified in trolands (td). 100 cd/m^2 corresponds approximately to a range of 500-1000 td 
# (depending on the species). Such a luminance corresponds roughly to the mean luminance outdoors on a 
# dull cloudy day, and is an order of magnitude higher than the typical mean luminance indoors. 

# Response to a light flash
retina.Input('impulse',{'start','500.0','stop','750.0','amplitude','1000.0','offset','100.0','sizeX','20','sizeY','20'})

# Input sequence
#retina.Input('sequence','input_sequences/Weberlaw/0_255/',{'InputFramePeriod','100'})

### Creation of computational retinal microcircuits ###

## Temporal modules
# Photoreceptors (Simplified linear version of the photoreceptors' model in [1])
retina.Create('LinearFilter','tmp_photoreceptor_L',{'type','Gamma','tau','15.0','n','0.0'})
retina.Create('LinearFilter','tmp_photoreceptor_L_overshoot',{'type','Gamma','tau','30.0','n','0.0'})
retina.Create('LinearFilter','tmp_photoreceptor_M',{'type','Gamma','tau','15.0','n','0.0'})
retina.Create('LinearFilter','tmp_photoreceptor_M_overshoot',{'type','Gamma','tau','30.0','n','0.0'})

# Midget bipolar cells
# R_L = 0.1 mOhm, Cm = 100 pF, E_ex = 0 mV, E_in = -70 mV, E_L = -60 mV (ON), E_L = -50 mV (OFF).
# ON bipolar cells receive excitatory synapses from photoreceptors and horizontal cells.
# OFF bipolar cells receive excitatory synapses from photoreceptors and inhibitory synapses
# from horizontal cells
retina.Create('SingleCompartment','MB_L_ON',{'number_current_ports','1.0','number_conductance_ports','3.0','Rm','0.1','Cm','100.0','E',{'0.0','0.0','-60.0'}})
retina.Create('SingleCompartment','MB_L_OFF',{'number_current_ports','1.0','number_conductance_ports','3.0','Rm','0.1','Cm','100.0','E',{'0.0','-70.0','-50.0'}})
retina.Create('SingleCompartment','MB_M_ON',{'number_current_ports','1.0','number_conductance_ports','3.0','Rm','0.1','Cm','100.0','E',{'0.0','0.0','-60.0'}})
retina.Create('SingleCompartment','MB_M_OFF',{'number_current_ports','1.0','number_conductance_ports','3.0','Rm','0.1','Cm','100.0','E',{'0.0','-70.0','-50.0'}})

# Horizontal cell H1
# R_L = 0.1 mOhm, Cm = 100 pF, E_ex = 0 mV, E_L = -60 mV
retina.Create('SingleCompartment','H1',{'number_current_ports','1.0','number_conductance_ports','2.0','Rm','0.1','Cm','100.0','E',{'0.0','-60.0'}})
# Delay of 5 ms in the synapse H1 - Bip. cells
retina.Create('LinearFilter','tmp_H1_delay',{'type','Gamma','tau','5.0','n','0.0'})

## Nonlinearities
# Conversion factor of the input stimulus to trolands (modify the slope to transform
# the input value to td)
retina.Create('StaticNonLinearity','input_to_trolands_L',{'slope','1.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','input_to_trolands_M',{'slope','1.0','offset','0.0','exponent','1.0'})
# Photoreceptors' overshoot
retina.Create('StaticNonLinearity','SNL_overshoot_L',{'slope','0.5','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','SNL_overshoot_M',{'slope','0.5','offset','0.0','exponent','1.0'})
# Photoreceptors' output. The offset is set to force a resting potential of -45.0 mV. The slope is calibrated
# to produce membrane potential amplitudes roughly similar to those of the model in [1].
retina.Create('StaticNonLinearity','SNL_photoreceptor_L',{'slope','0.015','offset','-45.0','exponent','1.0'})
retina.Create('StaticNonLinearity','SNL_photoreceptor_M',{'slope','0.015','offset','-45.0','exponent','1.0'})

# Activation functions of the synaptic channels of bipolar/horizontal cells to the release of glutamate
# from photoreceptors (k_syn = 4, theta_syn = -50.0 mV)
retina.Create('SigmoidNonLinearity','SNL_activation_L_ON',{'slope','-0.25','offset','12.5','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_L_OFF',{'slope','0.25','offset','-12.5','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_M_ON',{'slope','-0.25','offset','12.5','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_M_OFF',{'slope','0.25','offset','-12.5','max','1.0'})

# Synaptic connections from photoreceptors to bipolar cells (nS)
retina.Create('StaticNonLinearity','w_L_cone_MB_L_ON',{'slope','5.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_L_cone_MB_L_OFF',{'slope','6.5','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_M_cone_MB_M_ON',{'slope','5.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_M_cone_MB_M_OFF',{'slope','6.5','offset','0.0','exponent','1.0'})

# Synaptic connections from photoreceptors to horizontal cells (nS)
retina.Create('StaticNonLinearity','w_L_cone_H1',{'slope','2.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_M_cone_H1',{'slope','2.0','offset','0.0','exponent','1.0'})

# Activation function of the output of H1
retina.Create('SigmoidNonLinearity','SNL_activation_H1_output',{'slope','0.25','offset','-12.5','max','1.0'})

# Synaptic connections from horizontal cells to bipolar cells (nS)
retina.Create('StaticNonLinearity','w_H1_MB_L_ON',{'slope','3.5','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_H1_MB_M_ON',{'slope','3.5','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_H1_MB_L_OFF',{'slope','5.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_H1_MB_M_OFF',{'slope','5.0','offset','0.0','exponent','1.0'})

# Activation functions of the output of bipolar cells
retina.Create('SigmoidNonLinearity','SNL_activation_MB_L_ON_output',{'slope','0.33','offset','-11.67','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_MB_L_OFF_output',{'slope','0.33','offset','-11.67','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_MB_M_ON_output',{'slope','0.33','offset','-11.67','max','1.0'})
retina.Create('SigmoidNonLinearity','SNL_activation_MB_M_OFF_output',{'slope','0.33','offset','-11.67','max','1.0'})

# Synaptic connections from bipolar cells to ganglion cells (nS).
retina.Create('StaticNonLinearity','w_MB_L_ON_MG_L_ON',{'slope','20.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_MB_L_OFF_MG_L_OFF',{'slope','20.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_MB_M_ON_MG_M_ON',{'slope','20.0','offset','0.0','exponent','1.0'})
retina.Create('StaticNonLinearity','w_MB_M_OFF_MG_M_OFF',{'slope','20.0','offset','0.0','exponent','1.0'})

## Spatial filters
# Outer Plexiform Layer
retina.Create('GaussFilter','Gauss_OPL_center_L',{'sigma','0.03','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_OPL_center_M',{'sigma','0.03','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_L_cone_H1',{'sigma','0.1','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_M_cone_H1',{'sigma','0.1','spaceVariantSigma','False'})
retina.Create('GaussFilter','Gauss_OPL_surround',{'sigma','0.1','spaceVariantSigma','False'})

### Connections ###

## Photoreceptors ##
# L-cones
retina.Connect('L_cones','input_to_trolands_L','Current')
retina.Connect('input_to_trolands_L','tmp_photoreceptor_L','Current')
retina.Connect('tmp_photoreceptor_L','tmp_photoreceptor_L_overshoot','Current')
retina.Connect('tmp_photoreceptor_L_overshoot','SNL_overshoot_L','Current')
retina.Connect({'SNL_overshoot_L',-,'tmp_photoreceptor_L'},'Gauss_OPL_center_L','Current')
retina.Connect('Gauss_OPL_center_L','SNL_photoreceptor_L','Current')

# M-cones
retina.Connect('M_cones','input_to_trolands_M','Current')
retina.Connect('input_to_trolands_M','tmp_photoreceptor_M','Current')
retina.Connect('tmp_photoreceptor_M','tmp_photoreceptor_M_overshoot','Current')
retina.Connect('tmp_photoreceptor_M_overshoot','SNL_overshoot_M','Current')
retina.Connect({'SNL_overshoot_M',-,'tmp_photoreceptor_M'},'Gauss_OPL_center_M','Current')
retina.Connect('Gauss_OPL_center_M','SNL_photoreceptor_M','Current')

# Activation functions of the cation channel of bipolar cells (and horizontal cells)
retina.Connect('SNL_photoreceptor_L','SNL_activation_L_ON','Current')
retina.Connect('SNL_photoreceptor_L','SNL_activation_L_OFF','Current')
retina.Connect('SNL_photoreceptor_M','SNL_activation_M_ON','Current')
retina.Connect('SNL_photoreceptor_M','SNL_activation_M_OFF','Current')

# Synaptic connections from photoreceptors to bipolar cells
retina.Connect('SNL_activation_L_ON','w_L_cone_MB_L_ON','Current')
retina.Connect('SNL_activation_L_OFF','w_L_cone_MB_L_OFF','Current')
retina.Connect('SNL_activation_M_ON','w_M_cone_MB_M_ON','Current')
retina.Connect('SNL_activation_M_OFF','w_M_cone_MB_M_OFF','Current')

retina.Connect('w_L_cone_MB_L_ON','MB_L_ON','Conductance')
retina.Connect('w_L_cone_MB_L_OFF','MB_L_OFF','Conductance')
retina.Connect('w_M_cone_MB_M_ON','MB_M_ON','Conductance')
retina.Connect('w_M_cone_MB_M_OFF','MB_M_OFF','Conductance')

# Synaptic connections from photoreceptors to horizontal cells
# (taken from OFF activation functions)
retina.Connect('SNL_activation_L_OFF','Gauss_L_cone_H1','Current')
retina.Connect('SNL_activation_M_OFF','Gauss_M_cone_H1','Current')
retina.Connect('Gauss_L_cone_H1','w_L_cone_H1','Current')
retina.Connect('Gauss_M_cone_H1','w_M_cone_H1','Current')

retina.Connect({'w_L_cone_H1',+,'w_M_cone_H1'},'H1','Conductance')

# H1's output
retina.Connect('H1','Gauss_OPL_surround','Current')
retina.Connect('Gauss_OPL_surround','tmp_H1_delay','Current')
retina.Connect('tmp_H1_delay','SNL_activation_H1_output','Current')

# Synaptic connections from horizontal cells to bipolar cells
retina.Connect('SNL_activation_H1_output','w_H1_MB_L_ON','Current')
retina.Connect('w_H1_MB_L_ON','MB_L_ON','Conductance')

retina.Connect('SNL_activation_H1_output','w_H1_MB_M_ON','Current')
retina.Connect('w_H1_MB_M_ON','MB_M_ON','Conductance')

retina.Connect('SNL_activation_H1_output','w_H1_MB_L_OFF','Current')
retina.Connect('w_H1_MB_L_OFF','MB_L_OFF','Conductance')

retina.Connect('SNL_activation_H1_output','w_H1_MB_M_OFF','Current')
retina.Connect('w_H1_MB_M_OFF','MB_M_OFF','Conductance')

# Bipolar cells's output
retina.Connect('MB_L_ON','SNL_activation_MB_L_ON_output','Current')
retina.Connect('SNL_activation_MB_L_ON_output','w_MB_L_ON_MG_L_ON','Current')

retina.Connect('MB_L_OFF','SNL_activation_MB_L_OFF_output','Current')
retina.Connect('SNL_activation_MB_L_OFF_output','w_MB_L_OFF_MG_L_OFF','Current')

retina.Connect('MB_M_ON','SNL_activation_MB_M_ON_output','Current')
retina.Connect('SNL_activation_MB_M_ON_output','w_MB_M_ON_MG_M_ON','Current')

retina.Connect('MB_M_OFF','SNL_activation_MB_M_OFF_output','Current')
retina.Connect('SNL_activation_MB_M_OFF_output','w_MB_M_OFF_MG_M_OFF','Current')

# Interface with NEST
retina.Connect('w_MB_L_ON_MG_L_ON','Output','Current')
retina.Connect('w_MB_L_OFF_MG_L_OFF','Output','Current')
retina.Connect('w_MB_M_ON_MG_M_ON','Output','Current')
retina.Connect('w_MB_M_OFF_MG_M_OFF','Output','Current')

### Displays and data analysis  ###

retina.Show('Input','False','margin','0')
retina.Show('MB_L_ON','False','margin','0')
retina.Show('MB_L_OFF','False','margin','0')

# Temporal multimeters
retina.multimeter('temporal','Input','Input',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','SNL_photoreceptor_L','SNL_photoreceptor_L',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','SNL_activation_L_ON','SNL_activation_L_ON',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','SNL_activation_L_OFF','SNL_activation_L_OFF',{'x','10','y','10'},'Show','True','startTime','100')

retina.multimeter('temporal','H1','H1',{'x','10','y','10'},'Show','True','startTime','100')
# Record the temporal evolution of all cells in MB_L_ON layer
retina.multimeter('temporal_all','MB_L_ON','MB_L_ON',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','MB_L_OFF','MB_L_OFF',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','MB_M_ON','MB_M_ON',{'x','10','y','10'},'Show','True','startTime','100')

retina.multimeter('temporal','w_MB_L_ON_MG_L_ON','w_MB_L_ON_MG_L_ON',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','w_MB_L_OFF_MG_L_OFF','w_MB_L_OFF_MG_L_OFF',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','w_MB_M_ON_MG_M_ON','w_MB_M_ON_MG_M_ON',{'x','10','y','10'},'Show','True','startTime','100')
retina.multimeter('temporal','w_MB_M_OFF_MG_M_OFF','w_MB_M_OFF_MG_M_OFF',{'x','10','y','10'},'Show','True','startTime','100')

# Spatial multimeters (careful when using this multimeter because it overwrites the
# temporal multimeter with the same ID)
# retina.multimeter('spatial','MB_M_ON','MB_M_ON',{'timeStep','500','rowcol','True','value','10'},'Show','True')
