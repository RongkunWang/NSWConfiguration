# NSW Configuration prototype

[![pipeline status](https://gitlab.cern.ch/atlas-muon-nsw-daq/NSWConfiguration/badges/dev/pipeline.svg)](https://gitlab.cern.ch/atlas-muon-nsw-daq/NSWConfiguration/commits/dev)

* [Configuration DB](#configuration-db)
* [Communication with Front End](#communication-with-front-end)
* [Installation](#installation)
  * [Just tell me how to compile!](#just-tell-me-how-to-compile)
* [Creating Config File](#creating-config-file)
* [Running](#Running)
* [Software Design](#software-design)
  * [ConfigReader](#configreader)
    * [How to implement a new ConfigReaderApi](#how-to-implement-a-new-configreaderapi)
  * [Configuration Classes](#configuration-classes)
  * [ConfigSender](#configsender)
  * [How to Implement a New FE](#how-to-implement-a-new-fe)
  * [Extra info for developers](#extra-info-for-developers)
    * [Compiler profile](#compiler-profile)
    * [git options](#git-options)
    * [CMake options](#cmake-options)
    * [External Software](#external-software)

## Configuration DB

* In production we'll use some kind of database.
* For prototyping json files will be used.

## Communication with Front End

* Communication is through OPC Client that talks to the server in the FELIX PC.
* The client implementation is open source: https://github.com/quasar-team/open62541-compat.git
* The ``OpcClient`` requires generated files from ``UaoClientForOpcUaSca``.

## Installation

These instructions are for ``centos7``, for ``SLC6``, check older versions of the README file.

### Just tell me how to compile!
* Go to any ``lxplus`` node:
```bash
ssh lxplus7.cern.ch
```

* Set up the environment (see [environment](#environment) for additional options)
```bash
export TDAQ_RELEASE=tdaq-09-03-00
source /cvmfs/atlas.cern.ch/repo/sw/tdaq/tools/cmake_tdaq/bin/cm_setup.sh ${TDAQ_RELEASE}
```

* Create a ``work`` directory and create a ``CMakeLists.txt``
```bash
mkdir -p work
cd work
export TDAQ_VERSION=$(echo $TDAQ_RELEASE | awk '{split($$0,a,"-"); printf("%d.%d.%d",a[2],a[3],a[4]);}')
cat <<'EOF'> CMakeLists.txt
cmake_minimum_required(VERSION 3.14.0)

set(TDAQ_VERSION 9.2.1 CACHE STRING "TDAQ version number")
set(NSWDAQ_VERSION 1.0.0 CACHE STRING "NSWDAQ version number")

message(STATUS "TDAQ_VERSION [${TDAQ_VERSION}]")
message(STATUS "NSWDAQ_VERSION [${NSWDAQ_VERSION}]")

project(nswdaq-ci-build VERSION ${NSWDAQ_VERSION})

find_package(TDAQ)
include(CTest)

tdaq_project(nswdaq-ci-build ${NSWDAQ_VERSION} USES tdaq ${TDAQ_VERSION})
EOF
```

* Clone the ``NSWConfiguration`` package using the ``--recursive`` option to get the submodules
```bash
git clone --recursive https://:@gitlab.cern.ch:8443/atlas-muon-nsw-daq/NSWConfiguration.git
```

* Build the package
```bash
cmake -B${CMTCONFIG} -S. -DTDAQ_VERSION=${TDAQ_VERSION} # Generate the build configuration
cmake --build ${CMTCONFIG}                              # Compile all programs and libraries
```

Now you can skip to the section on creating [confg files](#creating-config-file).

# Creating Config File

* Copy the [data/integration_config.json](data/integration_config.json)
  and modify it to create your configuration.
  

* The config file need to have 3 common configuration that will be applied to all
  front ends you create:
```
vmm_common_config,
roc_common_config,
tds_common_config
```

* Create the front end elements you want to configure. Each element must
  at least have the Opc server/address info such as:
```json
    "MMFE8-0001":{
        "OpcServerIp": "pcatlnswfelix01.cern.ch:4841",
        "OpcNodeId": "SCA on MMFE8 0319-1"
    },
```

* The front ends name must contain MMFE8, SFEB or PFEB, number of VMM/TDS
  will be determined from the name.

* You can override any part of common configuration for the frontends.
  For instance following configuration overrides ROC clock phase and 
  global thresholds of 2 VMMs for ``MMFE8-0001``

```json
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
```json
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
  to configure the element named ``MMFE8-0001``:

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

# Software Design

This section aims to help future developers about the design of NSWConfiguration. The software has few components and
the main idea is the following:

- ConfigReader reads configuration database, and returns ptree objects. It doesn't depend on anything
- Frontend specific configuration classes take the ptree objects, manipulate them if needed and create the bits that can
  be sent to frontends. They don't depend on anything.
- ConfigSender uses frontend specific configuration classes and write/read configuration to/from frontends using an
  OpcClient. It depends on Frontend specific configuration classes and OpcClient.

## ConfigReader

ConfigReader is the entry point to reading a configuration database and dumping it in ptree objects.  It doesn't depend
on any other component.

**ConfigReader**: ConfigReader class is the only class exposed to the user and it doesn't know the details of the
database. Depending on the connection string, it selects which API to use. It has only few methods, details of which
should be implemented in the ConfigReaderApi

**ConfigReaderApi**: This is the base class for any reader API.
- For instance if it's a json file, ``JsonApi`` is used and ``read()`` method creates a ptree from the json string.
- The more specific methods such as ``readFEB()`` then combine common configuration and frontend specific configurations
  to create the final ptree for the frontend.
- The `read(std::string element)` method figures out the front end type from the name, and choses the correct method to
  use.

### How to implement a new ConfigReaderApi
To implement reading configuration from another source(for instance OKS, oracle database etc.), one has to derive a new
class from ``ConfigReaderApi`` class, similar to ``JsonApi``. There are 2 ways to implement the new reader API:
1. Only implement the ``read()`` method to create a ptree identical to one from `JsonApi::read()` (It has to have
   ``common_config`` and subdetector  specific configuration elements in the same ptree). All the other methods in
   ConfigReaderApi has default implementations
1. If one doesn't the new API to read the whole configuration, but only read configuration of a certain elements with
   `read(std::string element` method, then one may need to implement several methods. Note that in this case
   ConfigReaderApi may require a reconsideration, and some methods may need to be converted to virtual.

## Configuration Classes
Configuration classes are simply container to for configuration registers. They are used to initialize and modify
configuration for a certain front end. Each configuration class is initialized by a ptree that is returned from
`ConfigReader::read(std::string element)` method. For instance the most commonly used ``FEBConfig`` is a generic class
that can represent MMFE8, sFEB and pFEB, depending on the number of ``vmm`` and ``tds`` in the input ptree.

## ConfigSender
ConfigSender is the main component that communicates with the frontends. The details of this communication is hidden in
``OpcClient`` class.

- ``UaoClientForOpcUaSca``: Classes to read/write from SCAs using OpcUa. It is generated by OpcUa team and  It is
  added to the software as a git submodule. Once in a while one may need to checkout the new version, for instance
  if a new front end element is added.
- ``OpcClient``: Uses the classes and methods from ``UaoClientForOpcUaSca`` to read/write frontends. Note that not all
  classes from ``UaoClientForOpcUaSca`` are used, but only the relevant ones.
- ``ConfigSender``: Uses both ``OpcClient`` and classes like ``FEBConfig``. It is the only class exposed to user and it
  hides the implementation details of OpcClient from user.

## How to Implement a New FE
If there is a new configuration component/front end element that need to be implemented in the NSWConfiguration, there
are several things to do:
- Create a new class for the front end element. Lets call this ``NewSCAConfig``. If the frontend contains a SCA, use
  ``SCAConfig`` class as base class. Ideally this class should contain some containers that have bitstreams that will be
  sent to the front end, and methods to modify the configuration. Take a look at the current configuration classes to
  have an idea.
- Implement the way to read the configuration from database/json. One needs to decide what kind of ptree should be
  accepted by the ``NewSCAConfig``. The constructor should take the ptree and fill the containers that hold configuration
  registers.
- Modify ``getElementType()`` if needed (See ``Utility.h``)
- Implement relevant reader method in ``ConfigReaderApi``. These method should create the ptree that will be used in
  constructor of ``NewSCAConfig``.
- If needed, implement a new OpcClient method using the relevant class from ``UaoClientForOpcUaSca``. (It's also possible
  that a new version of ``UaoClientForOpcUaSca`` need to be used, in this case checkout the new version)
- Implement the relevant ConfigSender method using the OpcClient method and ``NewSCAConfig`` class.
- If needed, modify programs ``NSWConfigRc``, ``configure_frontend`` and if needed create a new example program to read/send
  configuration of this front end.

## Issue Tracker:
* [ATLNSWDAQ: ATLAS NSW DAQ Software JIRA](https://its.cern.ch/jira/projects/ATLNSWDAQ)

## Extra info for developers
This information is designed for more seasoned developers, as a way to have more fine-grained control of the build process.

### Prerequisites

First you need to decide which tdaq release you will use.
Possible options are:
- ``nightly``: latest changes, not be ideal for long term stability
- ``tdaq-08-03-01``: August 2019 release with new implementation of swROD and ALTI
- ``tdaq-09-00-00``: March 2020 release
- ``tdaq-09-01-00``: September 2020 release
- ``tdaq-09-02-01``: August 2, 2021 release
- ``tdaq-09-02-01``: Current (August 2021) **default**
- other: Keep following Detector/DAQ meetings for newer releases.

#### Environment setup
Let's say you chose the ``tdaq`` release ``tdaq-09-03-00``.
Set the TDAQ environment, each ``tdaq`` release is compiled against a certain LCG release
(and with several compiler profile options).
For most uses, you will not need to specify the compiler profile and the following will be sufficient

```bash
export TDAQ_RELEASE=tdaq-09-03-00
source /cvmfs/atlas.cern.ch/repo/sw/tdaq/tools/cmake_tdaq/bin/cm_setup.sh ${TDAQ_RELEASE}
```

### Compiler profile
* To explicitly specify the compiler profile, you would:
```bash
export TDAQ_RELEASE=tdaq-09-03-00
export COMPILER_PROFILE=x86_64-centos7-gcc8-opt
source /cvmfs/atlas.cern.ch/repo/sw/tdaq/tools/cmake_tdaq/bin/cm_setup.sh ${TDAQ_RELEASE} ${COMPILER_PROFILE}
```

N.B., available profiles are:
* ``tdaq-09-03-00`` **default**
  * ``x86_64-centos7-gcc8-opt``
  * ``x86_64-centos7-gcc8-dbg``
  * ``x86_64-centos7-gcc10-opt``
  * ``x86_64-centos7-gcc10-dbg``
* ``tdaq-09-02-01``
  * ``x86_64-centos7-gcc8-opt``
  * ``x86_64-centos7-gcc8-dbg``
  * ``x86_64-centos7-gcc10-opt``
  * ``x86_64-centos7-gcc10-dbg``
* ``tdaq-08-03-01``, ``tdaq-09-00-00``, ``tdaq-09-01-00``
  * ``x86_64-centos7-gcc8-opt``
  * ``x86_64-centos7-gcc8-dbg``

### git options
* Clone a specific branch:
```bash
git clone --recursive -b my-special-branch https://:@gitlab.cern.ch:8443/atlas-muon-nsw-daq/NSWConfiguration.git
```

* If you did not clone with a specific branch above, check out the
  branch or tag you need. Latest developments are always in the
  ``dev`` branch.
```bash
cd NSWConfiguration
git checkout fancy-branch
git submodule update --recursive
cd ..
```


### CMake options
* You *can* use ``cmake_config`` (but it is not recommended for
  flexibility reasons) to generate the build files, and then the
  native buildsystem to run the build, e.g., for ``make``:
```bash
# cmake_config $CMTCONFIG -- -DTDAQ_VERSION=${TDAQ_VERSION} # Create build configuration
# N.B. TDAQ_VERSION in CMakeLists.txt **must** match the environment variable due to limitations of cmake_config
# cmake_config                                              # Create build configuration
cd $CMTCONFIG                                               # Go to the folder such as x86_64-centos7-gcc8-opt/
make -j                                                     # Build all the programs and libraries
```

* You can specify an alternative directory for the generated build files
```bash
cmake -Bfreedomfiles -S. -DTDAQ_VERSION=${TDAQ_VERSION}
cmake --build freedomfiles -- -j
```

* If available, you can change the buildsystem (you must do this in a
  new build directory, otherwise `cmake` will detect the build files
  from the other generator and will fail):
```bash
cmake -Bninja-build -S. -GNinja -DTDAQ_VERSION=${TDAQ_VERSION}
cmake --build ninja-build -- -j
```

* Compile only a specific target (e.g., list all targets)
```bash
cmake --build ninja-build --target help
```

* Pass options to the buildsystem after a `--`, e.g., number of parallel jobs
```bash
cmake --build ninja-build -- -j
```

### External Software

``NSWConfiguration`` requires an external Opc related software to
build and run.

N.B., support for using ``cmake`` to fetch and build external
dependencies has been added, and manually building them is no longer
recommended.

This can be controlled at the ``cmake`` generation with the following
flags, note that the default behaviour is ``ON``.


#### Manual dependencies (**deprecated**, probably not functional)
If you are using the fully ``cmake`` driven build mentioned above, you
can skip this section and proceed directly to the next step.

The old way of manually pointing to the external dependencies is not
guaranteed to work, but the instructions are left.

* Go to any lxplus node with ``centos7``:
```bash
ssh lxplus7.cern.ch
```

* Set the LCG environment with correct tag (``CMTCONFIG``) and LCG
  version from previous step
```bash
export LCG_VERSION=$TDAQ_LCG_RELEASE
export COMPILER_PROFILE=$CMTCONFIG
source /cvmfs/sft.cern.ch/lcg/views/$LCG_VERSION/$COMPILER_PROFILE/setup.sh
```

* Clone the ``open62541-compat`` package and select correct ``git`` tag
```bash
git clone --recursive -b v1.1.3-rc0 https://github.com/quasar-team/open62541-compat.git
cd open62541-compat
```

* Build the software
```bash
mkdir build
cd build
cmake .. -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_lcg.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -DSKIP_TESTS=ON
make -j
```

* After this, you should have following in your build directory:
```
libopen62541-compat.so
open62541/libopen62541.a
```

The path of ``open62541-compat`` will be set as the
```OPC_OPEN62541_PATH``` environment variable to compile
``NSWConfiguration``.

To use the OPC libraries in a partition, create an installed area
under ``open62541-compat`` directory, and copy libraries there:
```bash
mkdir -p installed/$COMPILER_PROFILE/lib/
cp build/libopen62541-compat.so  installed/$COMPILER_PROFILE/lib/
```

When you build ``NSWConfiguration``, you will need to invoke ``cmake``
with the following options:

```bash
cmake -B${CMTCONFIG} -S. -DBUILD_OPEN62541_COMPAT=OFF -DBUILD_UAOCLIENTFOROPCUASCA=OFF
```

but be warned that this will likely not work.

#### Prebuilt dependencies (**deprecated**, probably not functional)
If these dependencies have been built for the chosen TDAQ release, you
can skip the previous step and use the installation from `/afs`:

```bash
export OPC_OPEN62541_PATH=/afs/cern.ch/work/n/nswdaq/public/${TDAQ_RELEASE}/sw/external/open62541-compat
```
