#!/usr/bin/python
# coding: utf-8

import nest
import nest.raster_plot
import numpy as np
import matplotlib.pyplot as plt

# simulation parameters
ganglionCells = 25*25
simTime = 1200.0

# Kernel and Network reset
nest.ResetKernel()
nest.ResetNetwork()

# Number of threads (must be 1) and resolution
nest.SetKernelStatus({"local_num_threads": 1,'resolution': 1.0})

# Load COREM
nest.Install("COREM")

# Create spike detector and spiking nodes
mult = nest.Create('spike_detector',1)
spikingGanglion=nest.Create('iaf_neuron',ganglionCells,{'C_m':10.0,'tau_m':10.0})
nest.Connect(spikingGanglion,mult)

# COREM nodes
for i in range(0,ganglionCells):
	g=nest.Create('COREM',1,{'port':float(i),'file':'../Retina_scripts/example_1.py'})
	nest.Connect(g,[spikingGanglion[i]])

# Simulation
nest.Simulate(simTime)

# Raster plot
nest.raster_plot.from_device(mult,hist=False)
plt.show()
