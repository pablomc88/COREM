#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Example that illustrates how to run COREM from the Python interpreter. It
# simulates the retina script "primate_parvo.py" and plots temporal evolution of
# membrane potentials of cells recorded by multimeters. It also computes a
# time-averaged population activity metric. It is also possible to simulate
# spiking output of ganglion cells using NEST (deactivated by default), but first
# the retina model 'ganglion cell' must be downloaded and built from the repository 
# https://github.com/pablomc88/Primate_Visual_System.

# Use NEST to simulate ganglion cells
use_NEST = False

if use_NEST:
    import nest
    import nest.raster_plot

import matplotlib.pyplot as plt
import numpy as np
import sys,os

# root path
root = "/home/pablo/Desktop/COREM_v2/COREM/"
# starting time of plots (the same value of the retina script!)
tstart = 100 # ms
# Parameters of the plot
rows = 3
cols = 4
# Array of time
time = np.arange(tstart,1200.0)
# Time intervals for the time-averaged population activity metric
time_intervals = [480.0,500.0,520.,540.0,560.0,580.0]
# Position of the cell situated in the center of the grid
center_cell = 210

# Simulation parameters
number_cells = 20*20
simTime = 1200.0

# IDs of modules to be analyzed
IDs = [
'Input',
'SNL_photoreceptor_L',
'SNL_activation_L_ON',
'SNL_activation_L_OFF',

'H1',
'MB_L_ON',
'MB_L_OFF',
'MB_M_ON',

'w_MB_L_ON_MG_L_ON',
'w_MB_L_OFF_MG_L_OFF',
'w_MB_M_ON_MG_M_ON',
'w_MB_M_OFF_MG_M_OFF'
]

# IDs for the population average
IDs_pop = [
'MB_L_ON'
]

# run COREM simulation
call = "./corem Retina_scripts/primate_parvo.py"
os.system(call)

# return simulated values
output_array = []
for ID in IDs:
    correct_value = True
    try:
	ispopavg = False
        for IDpop in IDs_pop:
            if IDpop == ID:
                ispopavg = True

        if ispopavg:
            output = np.float64(np.loadtxt(root+'results/'+ID+str(center_cell)))
        else:
            output = np.float64(np.loadtxt(root+'results/'+ID))

    except IOError:
        correct_value = False
    if correct_value:
        output_array.append(output)

# Plot membrane potentials

print "--- Membrane potentials ---"

fig = plt.figure()
fig.subplots_adjust(hspace=1.5)
fig.subplots_adjust(wspace=0.4)

current_row = 0
current_col = 0

for j in np.arange(len(IDs)):
    Vax = plt.subplot2grid((rows,cols), (current_row,current_col))
    Vax.plot(time,output_array[j])
    Vax.set_title(IDs[j])
    Vax.set_xlabel('Time (ms)')
    if j==0:
        Vax.set_ylabel('td')
    else:
        Vax.set_ylabel('Amplitude')

    if current_col < cols-1:
        current_col+=1
    else:
        current_col = 0
        current_row+=1

# Time-averaged population activity metric

avg_activity = np.zeros(len(time_intervals)-1)

print "--- Time-averaged population activity metric ---"

for ID in IDs_pop:
    print ID

    for cell in np.arange(number_cells):
        correct_value = True
        try:
            output = np.float64(np.loadtxt(root+'results/'+ID+str(cell)))
        except IOError:
            correct_value = False

        if correct_value:
            for interval in np.arange(len(time_intervals)-1):
                avg_activity[interval] += np.mean(output[time_intervals[interval]-tstart:
                    time_intervals[interval+1]-tstart])

for interval in np.arange(len(time_intervals)-1):
    print "Time interval [%s,%s] -> Averaged activity = %s mV " % (time_intervals[interval],
    time_intervals[interval+1],avg_activity[interval] / (number_cells * len(IDs_pop)))

# NEST simulation

if use_NEST:

    # Install modules just once
    model = nest.Models(mtype='nodes',sel='ganglion_cell')
    if not model:
        nest.Install("ganglion_cell_module")

    model = nest.Models(mtype='nodes',sel='corem')
    if not model:
        nest.Install("corem_module")

    # Reset kernel and network
    nest.ResetKernel()
    nest.ResetNetwork()

    # Number of threads (must be 1) and resolution (the same of the retina script)
    nest.SetKernelStatus({"local_num_threads": 1,'resolution': 1.0})

    print "--- NEST simulation ---"

    # Spike detector
    mult = nest.Create('spike_detector',1)

    # Midget ganglion cells
    retina_parvo_ganglion_cell_params = {
    "C_m": 100.0,
    "g_L": 10.0,
    "E_ex": 0.0,
    "E_in": -70.0,
    "E_L": -60.0,
    "V_th": -55.0,
    "V_reset": -60.0,
    "t_ref":  2.0,
    "rate": 0.0
    }

    GC_L_ON=nest.Create('ganglion_cell',number_cells,retina_parvo_ganglion_cell_params)
    GC_L_OFF=nest.Create('ganglion_cell',number_cells,retina_parvo_ganglion_cell_params)
    GC_M_ON=nest.Create('ganglion_cell',number_cells,retina_parvo_ganglion_cell_params)
    GC_M_OFF=nest.Create('ganglion_cell',number_cells,retina_parvo_ganglion_cell_params)

    # Connect to multimeter
    nest.Connect(GC_L_ON,mult)
    nest.Connect(GC_L_OFF,mult)
    nest.Connect(GC_M_ON,mult)
    nest.Connect(GC_M_OFF,mult)

    # Current noise added to ganglion cells
    retina_noise = nest.Create('noise_generator',1,{'mean': 0.0, 'std': 1.0 } ) # pA

    nest.Connect(retina_noise,GC_L_ON)
    nest.Connect(retina_noise,GC_L_OFF)
    nest.Connect(retina_noise,GC_M_ON)
    nest.Connect(retina_noise,GC_M_OFF)

    # Create and connect COREM nodes
    for i in range(0,number_cells):
            g=nest.Create('corem',1,{'port':float(i),'file':"Retina_scripts/primate_parvo.py"})
            nest.Connect(g,[GC_L_ON[i]])

    for i in range(number_cells,2*number_cells):
            g=nest.Create('corem',1,{'port':float(i),'file':"Retina_scripts/primate_parvo.py"})
            nest.Connect(g,[GC_L_OFF[i-number_cells]])

    for i in range(2*number_cells,3*number_cells):
            g=nest.Create('corem',1,{'port':float(i),'file':"Retina_scripts/primate_parvo.py"})
            nest.Connect(g,[GC_M_ON[i-2*number_cells]])

    for i in range(3*number_cells,4*number_cells):
            g=nest.Create('corem',1,{'port':float(i),'file':"Retina_scripts/primate_parvo.py"})
            nest.Connect(g,[GC_M_OFF[i-3*number_cells]])

    # Simulation
    nest.Simulate(simTime)

    # Raster plot
    nest.raster_plot.from_device(mult,hist=True)

plt.show()
