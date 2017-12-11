# NSW Configuration prototype

* [Configuration DB](#configuration-db)
* [Communication with Front End](#communication-with-front-end)
* [Installation](#Installation)

## Configuration DB

* In production we'll use some kind of database.
* For prototyping we'll use json files.

## Communication with Front End

* Communication will be through OPC Client that talks to the server in the FELIX PC.
* Client implementation is open source: https://github.com/quasar-team/open62541-compat.git
* The OpcClient requires generated files from UaoClientForOpcUaSca, which is added as submodule

## Installation

* Checkout this package using with `--recursive` option to get submodules
* In a seperate location, checkout and build https://github.com/quasar-team/open62541-compat.git
* Set `OPC_OPEN62541_PATH` environment variable as the directory of open62541-compat
* Build this package against the tdaq release in the usual way.

## Issue Tracker:
* [ATLNSWDAQ: ATLAS NSW DAQ Software JIRA](https://its.cern.ch/jira/projects/ATLNSWDAQ)
