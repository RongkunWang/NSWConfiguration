// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Uses bridge design pattern to implement multiple implementations

#ifndef NSWCONFIGURATION_CONFIGREADERAPI_H_
#define NSWCONFIGURATION_CONFIGREADERAPI_H_

#include <iostream>
#include <string>
#include <memory>
#include <set>

#include <boost/property_tree/ptree.hpp>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Types.h"

#include "ers/Issue.h"

ERS_DECLARE_ISSUE(nsw,
                  ConfigIssue,
                  "Problem while reading configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  ConfigBadNodeGeneral,
                  "No such node : " << node << " in common configuration tree",
                  ((std::string)node)
                  )
class ConfigReaderApi {

 protected:
  /// Merges 2 trees, overwrites elements in common tree, using the ones from specific
  /// \param common ptree that is fully populated
  /// \param specific ptree that is partially populated.
  virtual void mergeTree(const boost::property_tree::ptree& specific, boost::property_tree::ptree& common) const;

 public:
  /// Read configuration of a single front end element into a ptree
  boost::property_tree::ptree read(const std::string& element);

  /// Get names of all Front end elements in the configuration
  /// The base class method iterates through config ptree and finds all
  /// elements that start with MMFE8, PFEB, SFEB, ADDC, PadTriggerSCA, Router in the name.
  /// The results contain the full path of the element in the ptree
  virtual std::set<std::string> getAllElementNames() const = 0;

  /// Get names of Front end elements that match with regular expression
  /// \param regexp Regular expression to match. For instance to get path of
  ///     all MMFE8, one should use .*MMFE8.*
  /// \return Result is a subset of getAllElementNames()
  std::set<std::string> getElementNames(const std::string& regexp) const;

  /// Read configuration of front end, specifying number of vmm and tds in the FE
  virtual boost::property_tree::ptree readFEB(
      const std::string& element, size_t nvmm, size_t ntds,
      size_t vmm_start = 0, size_t tds_start = 0) const = 0;

  boost::property_tree::ptree readMMFE8(const std::string& element) const {
    return readFEB(element, nsw::NUM_VMM_PER_MMFE8, nsw::NUM_TDS_PER_MMFE8);
  }

  boost::property_tree::ptree readPFEB(const std::string& element) const {
    return readFEB(element, nsw::NUM_VMM_PER_PFEB, nsw::NUM_TDS_PER_PFEB);;
  }

  boost::property_tree::ptree readSFEB(const std::string& element, int nTDS) const {
    // return readFEB(element, 8, 4);
    return readFEB(element, nsw::NUM_VMM_PER_SFEB, nTDS);
  }

  boost::property_tree::ptree readSFEB6(const std::string& element) const {
    // return readFEB(element, 8, 4);
    return readFEB(element, nsw::NUM_VMM_PER_SFEB, nsw::NUM_TDS_PER_SFEB, nsw::SFEB6_FIRST_VMM, nsw::SFEB6_FIRST_TDS);;
  }

  virtual boost::property_tree::ptree readL1DDC(const std::string& element) const = 0;
  virtual boost::property_tree::ptree readADDC(const std::string& element, size_t nart) const = 0;
  virtual boost::property_tree::ptree readPadTriggerSCA(const std::string& element) const = 0;
  virtual boost::property_tree::ptree readRouter(const std::string& element) const = 0;
  virtual boost::property_tree::ptree readTP(const std::string& element) const = 0;
  virtual boost::property_tree::ptree readTPCarrier(const std::string& element) const = 0;

  virtual boost::property_tree::ptree& getConfig() = 0;
  virtual const boost::property_tree::ptree& getConfig() const = 0;
};

#endif  // NSWCONFIGURATION_CONFIGREADERAPI_H_

