# NSW Configuration prototype

* [Configuration DB](#configuration-db)
* [Communication with Front End](#communication-with-front-end)
* [Installation](#Installation)
  * [External Software](#external-software)
  * [NSWConfiguration](#nswconfiguration)
* [Creating Config File](#creating-config-file)
* [Running](#Running)

## Configuration DB

* In production we'll use some kind of database.
* For prototyping json files will be used.

## Communication with Front End

* Communication is through OPC Client that talks to the server in the FELIX PC.
* Client implementation is open source: https://github.com/quasar-team/open62541-compat.git
* The OpcClient requires generated files from UaoClientForOpcUaSca, which is added as submodule.

## Installation

These instructions are for centos7, for SLC6, check older versions of the README file.

### Prerequisites

First you need to decide which tdaq release you will use. Possible options:
- ```nightly```: latest changes, not be ideal for long term stability
- ```tdaq-08-03-01```: August 2019 release with new implementation of Software ROD and ALTI
- other: Keep following Detector/DAQ meetings for newer releases.

Lets say you chose tdaq release tdaq-08-03-01. Set the environment and find out which LCG
release you are using. (Each tdaq release is compiled against a certain LCG release)

```bash
ssh lxplus7.cern.ch
source /afs/cern.ch/atlas/project/tdaq/cmake/cmake_tdaq/bin/cm_setup.sh tdaq-08-03-01 x86_64-centos7-gcc8-opt
echo $TDAQ_LCG_RELEASE   # LCG release you should use
echo $CMTCONFIG   # Tag that defines OS and gcc version. For this example, it's: ```x86_64-centos7-gcc8-opt```
```

### External Software

NSW Configuration requires external Opc related software to build and run.
Note that you can skip this step and use installation from prebuilt /eos or /afs areas (see below)

* Go to any lxplus node with centos7: ```ssh lxplus7.cern.ch```

* Set lcg environment with correct tag(CMTCONFIG) and LCG version from previous step
```bash
export LCG_VERSION=LCG_96
export HW_TAG=x86_64-centos7-gcc8-opt
source /cvmfs/sft.cern.ch/lcg/views/$LCG_VERSION/$HW_TAG/setup.sh
```

* Clone open62541-compat package and select correct git tag
```bash
git clone https://github.com/quasar-team/open62541-compat.git
cd open62541-compat
git checkout v1.1.1
```

Build the software.
```bash
mkdir build
cd build
cmake .. -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_lcg.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -DSKIP_TESTS=ON -DBoost_NO_BOOST_CMAKE=ON
make -j
```

After this, you should have following in your build directory:
```
libopen62541-compat.so
open62541/libopen62541.a
```

The path of open62541-compat will be set as ```OPC_OPEN62541_PATH``` environment variable to compile NSWConfiguration

To use the OPC libraries in a partition, create a installed area under open62541-compat directory, and copy libraries there:
```bash
mkdir -p installed/$HW_TAG/lib/
cp build/libopen62541-compat.so  installed/$HW_TAG/lib/
```

### NSWConfiguration

* Go to any lxplus node: ```ssh lxplus7.cern.ch```
* Create a work directory and create following ```CMakeLists.txt```

```bash
mkdir work
cd work
printf "cmake_minimum_required(VERSION 3.4.3)\nfind_package(TDAQ)\ninclude(CTest)\ntdaq_project(NSWDAQ 1.0.0 USES tdaq 8.3.1)\n" > CMakeLists.txt
```

If you will use another tdaq release, replace 8.3.1 with the release number (for nightly use 99.0.0)

* Clone this package using with `--recursive` option to get submodules

```bash
git clone --recursive https://:@gitlab.cern.ch:8443/atlas-muon-nsw-daq/NSWConfiguration.git
```
* Setup tdaq and OpcUA environment for production release.
  For latter, you can your build from previous step, or a build from eos area

```bash
source /afs/cern.ch/atlas/project/tdaq/cmake/cmake_tdaq/bin/cm_setup.sh tdaq-08-03-01 # replace nightly with any other release
export OPC_OPEN62541_PATH=/eos/atlas/atlascerngroupdisk/det-nsw/sw/OpcUa/open62541-compat-v1.1.1
```

* Checkout the branch or tag you need. Latest developments are in `dev` branch.
```bash
cd NSWConfiguration
git checkout dev
git submodule update
cd ..
```

* Build the package

```bash
cmake_config   # Create build configuration
cd $CMTCONFIG  # Go to the folder such as x86_64-centos7-gcc8-opt/
make -j        # Build all the programs and libraries
```

# Creating Config File

* Copy the [data/integration_config.json](data/integration_config.json)
  and modify it to create your configuration.
  

* The config file need to have 3 common configuration that will be applied to all
  front ends you create: ```vmm_common_config, roc_common_config, tds_common_config```

* Create the front end elements you want to configure. Each element must
  at least have the Opc server/address info such as:
```
    "MMFE8-0001":{
        "OpcServerIp": "pcatlnswfelix01.cern.ch:4841",
        "OpcNodeId": "SCA on MMFE8 0319-1"
    },
```

* The front ends name must contain MMFE8, SFEB or PFEB, number of VMM/TDS
  will be determined from the name.

* You can override any part of common configuration for the frontends.
  For instance following configuration overrides ROC clock phase and 
  global thresholds of 2 VMMs for ```MMFE8-0001```

```
    "MMFE8-0001":{
        "OpcServerIp": "pcatlnswfelix01.cern.ch:4841",
        "OpcNodeId": "SCA on MMFE8 0319-1",
        "rocPllCoreAnalog": {
            "reg115": {
                "ePllPhase160MHz_0[4]": 0,
                "ePllPhase40MHz_0": 102
            },
            "reg116": {
                "ePllPhase160MHz_0[4]": 0,
                "ePllPhase40MHz_0": 102
            },
            "reg117": {
                "ePllPhase160MHz_0[4]": 0,
                "ePllPhase40MHz_0": 102
            },
            "reg118": {
                "ePllPhase160MHz_0[0:3]": 6,
                "ePllPhase160MHz_1[0:3]": 6
            },
            "reg119": {
                "ePllPhase160MHz_0[3:0]": 6
            }
        },
        "vmm2":{
            "sdt_dac": 170
        },
        "vmm4":{
            "sdt_dac": 250
        }
    }
```

* VMM channel registers are special, in a sense that you can either enter a single value
  for all channels, or an array of 64. Following will mask the first 10 channels of vmm2.
```
    "MMFE8-0001":{
        "OpcServerIp": "pcatlnswfelix01.cern.ch:4841",
        "OpcNodeId": "SCA on MMFE8 0319-1",
        "vmm2": {
            "channel_sm" : [1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        }
    }
```

# Running
* To configure any of MMFE8, PFEB, SFEB, one can use the sample program
  to configure the element named ```MMFE8-0001```:

```bash
# Assuming you are at the build directory
cd NSWConfiguration
./configure_frontend -h                                       # Show help
./configure_frontend -c my_config.json -n MMFE8-0001 -r       # Configure ROC with config file my_config.json
./configure_frontend -c my_config.json -n MMFE8-0001 -r -v    # Configure ROC and all VMMs
./configure_frontend -c my_config.json -n MMFE8-0001 -r -v -t # Configure ROC, all VMMs and TDSs
```

* To configure all front ends in the config file, leave out the -n option

```bash
./configure_frontend -c my_config.json -r -v -t # Configure ROC, all VMMs and TDSs on all front ends in config file
```

## Issue Tracker:
* [ATLNSWDAQ: ATLAS NSW DAQ Software JIRA](https://its.cern.ch/jira/projects/ATLNSWDAQ)
