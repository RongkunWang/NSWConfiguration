#include "NSWConfiguration/hw/MMTP.h"
// pimpl
#include "NSWConfiguration/hw/MMTP_v1.h"
#include "NSWConfiguration/hw/MMTP_v2.h"
#include "NSWConfiguration/hw/OpcManager.h"

nsw::hw::MMTP::MMTP(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId"))
{
  m_version = config.get<int>("Version");
  if (m_version == 1) 
  {
    m_impl1 = std::make_shared<nsw::hw::MMTP_v1>(manager, config);
  } else if (m_version == 2) {
    m_impl2 = std::make_shared<nsw::hw::MMTP_v2>(manager, config);
  } else {
    ers::error(nsw::MMTPVersionMissing(ERS_HERE, "Missing `Version` identifier in the MMTP json!"));
  }
}

std::string nsw::hw::MMTP::getName() const 
{
}

std::map<std::string, std::uint32_t> nsw::hw::MMTP::readConfiguration() const
{
}

void nsw::hw::MMTP::writeConfiguration(bool doAlignArtGbtx) const
{
}

void nsw::hw::MMTP::writeRegister(std::string_view regAddress,
                                  const std::uint32_t value) const
{
}

std::uint32_t nsw::hw::MMTP::readRegister(std::string_view regAddress) const
{
}

void nsw::hw::MMTP::writeAndReadbackRegister(std::string_view regAddress,
                                             const std::uint32_t value) const
{
}

void nsw::hw::MMTP::toggleIdleStateHigh() const
{
}

boost::property_tree::ptree& nsw::hw::MMTP::getConfig() 
{
}

const boost::property_tree::ptree& nsw::hw::MMTP::getConfig() const 
{
}  

std::vector<std::uint32_t> nsw::hw::MMTP::readAlignment(const size_t n_reads) const
{
}

void nsw::hw::MMTP::setHorxEnvMonAddr(const bool tx, const std::uint8_t microPod, const bool temp, const bool loss, const std::uint8_t fiber) const 
{
}


void nsw::hw::MMTP::enableChannelRates(const bool enable) const 
{
}
