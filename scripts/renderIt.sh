#!/bin/bash

export PYTHONPATH=/Users/schmitt/paraview/paraview/trunk/plugins/vtkLFMReader/build:$PYTHONPATH
export PYTHONPATH=/Users/schmitt/paraview/opt-3.12.0/ParaView-3.12.0_qt-cocoa-4.6.4_OSX-10.6.8/bin/:$PYTHONPATH
export PYTHONPATH=/Users/schmitt/paraview/opt-3.12.0/ParaView-3.12.0_qt-cocoa-4.6.4_OSX-10.6.8/Utilities/VTKPythonWrapping/site-packages:$PYTHONPATH
export PATH=/Users/schmitt/paraview/opt-3.12.0/ParaView-3.12.0_qt-cocoa-4.6.4_OSX-10.6.8/bin:$PATH
pvpython LFM_mhd_ParaView.py
