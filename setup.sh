#!/bin/bash

# Setup for building sw with TDAQ

#source /cvmfs/sft.cern.ch/lcg/views/LCG_88/x86_64-slc6-gcc62-opt/setup.sh
#source /cvmfs/sft.cern.ch/lcg/views/LCG_83/x86_64-slc6-gcc49-opt/setup.sh # Use this for gcc49
cm_setup prod

# This variable should be same as $CMTCONFIG
export LCGPLAT

# This variable is used in the CMakeLists.txt
export FREEOPCUA_INSTALL_PATH=/afs/cern.ch/user/c/cyildiz/public/nsw-work/freeopcua/install/
#export LD_LIBRARY_PATH=$FREEOPCUA_INSTALL_PATH/${LCGPLAT}/usr/local/lib/:$LD_LIBRARY_PATH



