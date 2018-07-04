# NSW Configuration prototype

* [Configuration DB](#configuration-db)
* [Communication with Front End](#communication-with-front-end)
* [Installation](#Installation)
* [Running](#Running)

## Configuration DB

* In production we'll use some kind of database.
* For prototyping json files will be used.

## Communication with Front End

* Communication is through OPC Client that talks to the server in the FELIX PC.
* Client implementation is open source: https://github.com/quasar-team/open62541-compat.git
* The OpcClient requires generated files from UaoClientForOpcUaSca, which is added as submodule.

## Installation

* Go to any lxplus node: ```ssh lxplus.cern.ch```
* Create a work directory and create following ```CMakeLists.txt``` file:

```bash
mkdir work
cd work
printf "cmake_minimum_required(VERSION 3.4.3)\nfind_package(TDAQ)\ninclude(CTest)\ntdaq_project(NSWDAQ 1.0.0 USES tdaq 7.1.0)\n" > CMakeLists.txt
```

* Checkout this package using with `--recursive` option to get submodules

```bash
git clone --recursive https://gitlab.cern.ch/atlas-muon-nsw-daq/NSWConfiguration.git
```
* Setup tdaq and OpcUA environment for production release (For latter, you can use the build in eos area)

```bash
source /afs/cern.ch/atlas/project/tdaq/cmake/cmake_tdaq/bin/cm_setup.sh prod
export OPC_OPEN62541_PATH=/eos/atlas/atlascerngroupdisk/det-nsw/sw/OpcUa/open62541-compat/
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
cd $CMTCONFIG  # Go to the folder such as x86_64-slc6-gcc62-opt/
make -j        # Build all the programs and libraries
```

# Running

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

* To configure any of MMFE8, PFEB, SFEB, one can use the sample program
  to configure the element named ```MMFE8-0001```:

```bash
./configure_frontend -h                                       # Show help
./configure_frontend -c my_config.json -n MMFE8-0001 -r       # Configure ROC with config file my_config.json
./configure_frontend -c my_config.json -n MMFE8-0001 -r -v    # Configure ROC and all VMMs
./configure_frontend -c my_config.json -n MMFE8-0001 -r -v -t # Configure ROC, all VMMs and TDSs
```

## Issue Tracker:
* [ATLNSWDAQ: ATLAS NSW DAQ Software JIRA](https://its.cern.ch/jira/projects/ATLNSWDAQ)
