// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Uses bridge design pattern to implement multiple implementations

#ifndef NSWCONFIGURATION_CONFIGREADERAPI_H_
#define NSWCONFIGURATION_CONFIGREADERAPI_H_

#include <iostream>
#include <string>
#include <memory>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include "ers/ers.h"

using boost::property_tree::ptree;

ERS_DECLARE_ISSUE(nsw,
                  ROCConfigBadNode,
                  "No such node in roc common configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  TDSConfigBadNode,
                  "No such node in tds common configuration: " << message,
                  ((const char *)message)
                  )

class ConfigReaderApi {
 protected:
  ptree m_config;  /// Ptree that holds all configuration

 public:
  /// Read the whole config db and dump it in the m_config tree
  virtual ptree & read() = 0;

  /// Read configuration of a single front end element
  virtual ptree read(std::string element_name);
  virtual ptree readVMM(std::string element_name);
  virtual ptree readROC(std::string element_name);
  virtual ptree readTDS(std::string element_name);
  virtual ~ConfigReaderApi() {}
};

class JsonApi: public ConfigReaderApi {
 private:
  std::string m_file_path;

 public:
  explicit JsonApi(std::string file_path): m_file_path(file_path) {}
  ptree & read();
};

class XmlApi: public ConfigReaderApi {
 private:
  std::string m_file_path;

 public:
  explicit XmlApi(std::string file_path): m_file_path(file_path) {}
  ptree & read();
};

class OracleApi: public ConfigReaderApi {
 private:
  std::string db_connection;

 public:
  explicit OracleApi(std::string db_connection) {}
  ~OracleApi() {}
  ptree & read();
};

class OksApi: public ConfigReaderApi {
 private:
  std::string m_file_path;

 public:
  explicit OksApi(std::string file_path): m_file_path(file_path) {}
  ptree & read();
};

#endif  // NSWCONFIGURATION_CONFIGREADERAPI_H_

