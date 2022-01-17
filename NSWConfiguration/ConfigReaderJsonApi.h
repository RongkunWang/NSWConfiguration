#ifndef NSWCONFIGURATION_CONFIGREADERJSONAPI_H
#define NSWCONFIGURATION_CONFIGREADERJSONAPI_H

#include <iostream>
#include <string>
#include <memory>
#include <set>

#include <boost/property_tree/ptree.hpp>
#include <utility>

#include "NSWConfiguration/ConfigReaderApi.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Types.h"

#include "ers/Issue.h"

ERS_DECLARE_ISSUE(nsw,
                  ROCConfigBadNode,
                  "No such node in roc common configuration: " << message,
                  ((const char *)message)
                  )

ERS_DECLARE_ISSUE(nsw,
                  ConfigBadNode,
                  "No such node: " << node << " in common configuration of: " << fetype,
                  ((std::string)node)
                  ((std::string)fetype)
                  )

ERS_DECLARE_ISSUE(nsw,
                  TDSConfigBadNode,
                  "No such node in tds common configuration: " << message,
                  ((const char *)message)
                  )

class JsonApi: public ConfigReaderApi {
 private:
  /// Merges 2 trees, overwrites elements in common tree, using the ones from specific
  /// \param common ptree that is fully populated with all fields required by I2cMasterConfig
  /// \param specific ptree that is partially populated.
  void mergeI2cMasterTree(boost::property_tree::ptree & specific, boost::property_tree::ptree & common) const;

  /// Merges 2 trees, overwrites elements in common tree, using the ones from specific
  /// \param common ptree that is fully populated with all fields required by VMMConfig
  /// \param specific ptree that is partially populated.
  void mergeVMMTree(boost::property_tree::ptree & specific, boost::property_tree::ptree & common) const;

  boost::property_tree::ptree read();

  /**
   * @brief Validate that all devices in the map are contained in the JSON
   *
   * \throws std::runtime_error Not all devices are in JSON
   */
  void validateDeviceMap() const;

  /**
   * @brief Get the all device names from the JSON
   * 
   * @return std::set<std::string> all names in JSON
   */
  std::set<std::string> getAllElementNamesFromJson() const;


 public:
  explicit JsonApi(std::string file_path, nsw::DeviceMap devices={});
  explicit JsonApi(const boost::property_tree::ptree& tree);

  boost::property_tree::ptree readL1DDC(const std::string& element) const override;
  boost::property_tree::ptree readADDC(const std::string& element, size_t nart) const override;
  boost::property_tree::ptree readPadTriggerSCA(const std::string& element) const override;
  boost::property_tree::ptree readRouter(const std::string& element) const override;
  boost::property_tree::ptree readSTGCTP(const std::string& element) const override;
  boost::property_tree::ptree readTP(const std::string& element) const override;
  boost::property_tree::ptree readTPCarrier(const std::string& element) const override;

  /// Read configuration of front end, specifying number of vmm and tds in the FE
  boost::property_tree::ptree readFEB(
      const std::string& element, size_t nvmm, size_t ntds,
      size_t vmm_start = 0, size_t tds_start = 0) const override;

  std::set<std::string> getAllElementNames() const override;

  boost::property_tree::ptree& getConfig() override;
  const boost::property_tree::ptree& getConfig() const override;


 private:
  std::string m_file_path;
  nsw::DeviceMap m_devices{};
  boost::property_tree::ptree m_config;  /// Ptree that holds all configuration
};

#endif  // NSWCONFIGURATION_CONFIGREADERJSONAPI_H
