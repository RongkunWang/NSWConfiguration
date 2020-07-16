// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Uses bridge design pattern to implement multiple implementations

#ifndef NSWCONFIGURATION_CONFIGREADERAPI_H_
#define NSWCONFIGURATION_CONFIGREADERAPI_H_

#include <iostream>
#include <string>
#include <memory>
#include <set>

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
                  ConfigBadNode,
                  "No such node : " << node << " in common configuration of: " << fetype,
                  ((std::string)node)
                  ((std::string)fetype)
                  )

ERS_DECLARE_ISSUE(nsw,
                  ConfigIssue,
                  "Problem while reading configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  TDSConfigBadNode,
                  "No such node in tds common configuration: " << message,
                  ((const char *)message)
                  )

class ConfigReaderApi {
 private:
  /// Merges 2 trees, overwrites elements in common tree, using the ones from specific
  /// \param common ptree that is fully populated with all fields required by I2cMasterConfig
  /// \param specific ptree that is partially populated.
  virtual void mergeI2cMasterTree(ptree & specific, ptree & common);

  /// Merges 2 trees, overwrites elements in common tree, using the ones from specific
  /// \param common ptree that is fully populated with all fields required by VMMConfig
  /// \param specific ptree that is partially populated.
  virtual void mergeVMMTree(ptree & specific, ptree & common);

 protected:
  ptree m_config;  /// Ptree that holds all configuration

 public:
  /// Read the whole config db and dump it in the m_config tree
  virtual ptree & read() = 0;

  /// Read configuration of a single front end element into a ptree
  virtual ptree read(std::string element_name);

  /// Get names of all Front end elements in the configuration
  /// The base class method iterates through config ptree and finds all
  /// elements that start with MMFE8, PFEB, SFEB, ADDC in the name.
  /// The results contain the full path of the element in the ptree
  std::set<std::string> getAllElementNames();

  /// Get names of Front end elements that match with regular expression
  /// \param regexp Regular expression to match. For instance to get path of
  ///     all MMFE8, one should use .*MMFE8.*
  /// \return Result is a subset of getAllElementNames()
  std::set<std::string> getElementNames(std::string regexp);

  /// Read configuration of front end, specifying number of vmm and tds in the FE
  virtual ptree readFEB(std::string element_name, size_t nvmm, size_t ntds, size_t vmm_start = 0, size_t tds_start = 0);

  ptree readMMFE8(std::string element) {
    return readFEB(element, 8, 0);
  }

  ptree readPFEB(std::string element) {
    return readFEB(element, 3, 1);
  }
  
  ptree readSFEB(std::string element, int nTDS) {
    // return readFEB(element, 8, 4);
    return readFEB(element, 8, nTDS);
  }


  ptree readSFEB6(std::string element) {
    //return readFEB(element, 8, 4);
    return readFEB(element, 8, 4, 2, 1);
  }

  // TODO(cyildiz): Following read functions should be deprecated!
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

