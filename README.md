# NSW Configuration prototype

* [Configuration DB](#configuration-db)
* [Communication with Front End](#communication-with-front-end)
* [Installation](#Installation)

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
printf "cmake_minimum_required(VERSION 3.4.3)\nfind_package(TDAQ)\ninclude(CTest)\ntdaq_project(NSWTTC 1.0.0 USES tdaq 7.1.0)\n" > CMakeLists.txt
```

* Checkout this package using with `--recursive` option to get submodules

```bash
git checkout --recursive https://gitlab.cern.ch/atlas-muon-nsw-daq/NSWConfiguration.git
```
* Setup tdaq and OpcUA environment for production release (For latter, you can use the build in eos area)

```bash
source /afs/cern.ch/atlas/project/tdaq/cmake/cmake_tdaq/bin/cm_setup.sh prod
export OPC_OPEN62541_PATH=/eos/atlas/atlascerngroupdisk/det-nsw/sw/OpcUa/open62541-compat/
```
* Build the package

```bash
cmake_config
cd $CMTCONFIG
make
```

* To configure ROC, run configure_roc program:

```bash
./configure_roc -h # Show help
./configure_roc -c my_config.json # Configure ROC with config file my_config.json
./configure_roc -c my_config.json -v # Configure ROC and VMM
```

## Issue Tracker:
* [ATLNSWDAQ: ATLAS NSW DAQ Software JIRA](https://its.cern.ch/jira/projects/ATLNSWDAQ)
