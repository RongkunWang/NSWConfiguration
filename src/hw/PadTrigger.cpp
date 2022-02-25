#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/SCAInterface.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

#include <ers/ers.h>
#include <fmt/core.h>
#include <stdexcept>
#include <regex>

nsw::hw::PadTrigger::PadTrigger(const boost::property_tree::ptree& config):
  m_ptree{config},
  m_opcserverIp{config.get<std::string>("OpcServerIp")},
  m_scaAddress{config.get<std::string>("OpcNodeId")},
  m_padtriggerfpga{config.get_child(PADTRIGGER_NAME), PADTRIGGER_NAME, PADTRIGGER_REGISTERS}
{
  m_name = fmt::format("{}/{}", m_opcserverIp, m_scaAddress);
  m_scaAddressFPGA = fmt::format("{}.fpga.fpga", m_scaAddress);
  m_scaAddressJTAG = fmt::format("{}.jtag.fpga", m_scaAddress);
}

void nsw::hw::PadTrigger::writeConfiguration() const
{
  writeRepeatersConfiguration();
  writeVTTxConfiguration();
  writeFPGAConfiguration();
  writeJTAGBitfileConfiguration();
}

void nsw::hw::PadTrigger::writeRepeatersConfiguration() const
{
  if (not ConfigRepeaters()) {
    ERS_INFO(fmt::format("Skipping configuration of repeaters of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("Writing configuration of repeaters of {}", m_name));

  for (const auto& [rep, address, value]: m_repeaterSequenceOfCommands) {

    const auto gpio = fmt::format("gpio-repeaterChip{}", rep);

    // GPIO enable
    writeGPIO(gpio, true);

    // i2c write
    ERS_LOG(fmt::format("Repeater{} address {}: write {}", rep, address, value));
    writeRepeaterRegister(rep, address, value);

    // i2c readback
    const auto val = readRepeaterRegister(rep, address);
    ERS_LOG(fmt::format("Repeater{} address {}: readback {}", rep, address, val));

    // check and complain
    if (val != value) {
      const std::string msg = "Found mismatch in repeater readback";
      nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
      ers::error(issue);
    }

    // GPIO disable
    writeGPIO(gpio, false);
  }
}

void nsw::hw::PadTrigger::writeVTTxConfiguration() const
{
  if (not ConfigVTTx()) {
    ERS_LOG(fmt::format("Skipping configuration of VTTx of {}", m_name));
    return;
  }

  for (const auto& vttx: m_vttxs) {

    for (const auto& vttx_data: m_vttx_datas) {

      // write
      ERS_LOG(fmt::format("VTTx{} address {}: write {}", vttx, m_vttx_addr, vttx_data));
      writeVTTxRegister(vttx, m_vttx_addr, vttx_data);

      // readback
      const auto val = readVTTxRegister(vttx, m_vttx_addr);
      ERS_LOG(fmt::format("VTTx{} address {}: readback {}", vttx, m_vttx_addr, val));

      // check and complain
      if (vttx_data != val) {
        const std::string msg = "Found mismatch in VTTx readback";
        nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
        ers::error(issue);
      }

    }

  }

}

void nsw::hw::PadTrigger::writeJTAGBitfileConfiguration() const
{
  const std::string& fw = firmware();
  ERS_INFO(fmt::format("Firmware provided: {}", fw));
  if (fw.empty()) {
    ERS_INFO("Not uploading bitfile since firmware not provided");
    return;
  }
  ERS_INFO("Uploading bitfile via SCA JTAG, this will take a minute...");
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::writeXilinxFpga(opcConnection, m_scaAddressJTAG, fw);
  ERS_INFO("Upload finished");
}

void nsw::hw::PadTrigger::writeFPGAConfiguration() const
{
  if (not ConfigFPGA()) {
    ERS_INFO(fmt::format("Skipping configuration of FPGA of {}", m_name));
    return;
  }

  const auto& fpga = getFpga();

  for (const auto& [rname, value_str] : fpga.getBitstreamMap()) {

    // strings -> numerics
    const auto addr  = addressFromRegisterName(rname);
    const auto value = std::stoul(value_str, nullptr, nsw::BASE_BIN);

    // write
    ERS_INFO(fmt::format("{}: writing to {} ({:#02x}) with {:#08x}", m_name, rname, addr, value));
    try {
      writeFPGARegister(addr, value);
    } catch (const std::exception & ex) {
      if (addr == nsw::padtrigger::REG_CONTROL) {
        throw;
      } else {
        const auto msg = fmt::format(
          "{}: Failed to write to {}. This usually indicates an older firmware version. Skipping!",
          m_name, rname);
        nsw::PadTriggerConfigError issue(ERS_HERE, msg.c_str());
        ers::warning(issue);
        break;
      }
    }

    // readback
    const auto val = readFPGARegister(addr);
    ERS_INFO(fmt::format("{}: readback of {:#02x} gives {:#08x}", m_name, addr, val));

    // compare
    if (val != value) {
      const std::string msg = "Found mismatch in FPGA reg readback";
      nsw::PadTriggerReadbackMismatch issue(ERS_HERE, msg.c_str());
      ers::error(issue);
    }

  }

}

void nsw::hw::PadTrigger::writeControlSubRegister(const std::string& subreg, const std::uint32_t subval) const
{
  const auto rname = std::string{"000_control_reg"};
  const auto addr = addressFromRegisterName(rname);

  // overwrite the bits of interest
  const auto fpga = getFpga();
  const auto pos  = fpga.getAddressPositions().at(rname).at(subreg);
  const auto siz  = fpga.getAddressSizes()    .at(rname).at(subreg);
  const auto rpos = std::size_t{nsw::NUM_BITS_IN_WORD32 - pos - siz};
  const auto value =
    nsw::overwriteBits(readFPGARegister(addr), subval, rpos, siz);

  // write
  ERS_INFO(fmt::format("{}: writing to {:#02x} with {:#08x}", m_name, addr, value));
  writeFPGARegister(addr, value);
}

void nsw::hw::PadTrigger::toggleIdleState() const
{
  writeControlSubRegister("conf_startIdleState", std::uint32_t{false});
  writeControlSubRegister("conf_startIdleState", std::uint32_t{true});
  writeControlSubRegister("conf_startIdleState", std::uint32_t{false});
}

std::map<std::uint8_t, std::uint32_t> nsw::hw::PadTrigger::readConfiguration() const
{
  auto result = std::map<std::uint8_t, std::uint32_t>();
  for (const auto& [rname, ignored] : PADTRIGGER_REGISTERS) {
    const auto addr = addressFromRegisterName(rname);
    result.emplace(addr, readFPGARegister(addr));
  }
  return result;
}

void nsw::hw::PadTrigger::writeGPIO(const std::string& name, const bool value) const
{
  const auto addr = fmt::format("{}.{}.{}", m_scaAddress, "gpio", name);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendGPIO(opcConnection, addr, value);
}

void nsw::hw::PadTrigger::writeRepeaterRegister(const std::uint8_t repeater,
                                                const std::uint8_t regAddress,
                                                const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2c(opcConnection, addr, data);
}

void nsw::hw::PadTrigger::writeVTTxRegister(const std::uint8_t vttx,
                                            const std::uint8_t regAddress,
                                            const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "vttx", vttx,
                                "vttx", vttx);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2c(opcConnection, addr, data);
}

void nsw::hw::PadTrigger::writeFPGARegister(const std::uint8_t regAddress,
                                            const std::uint32_t value) const
{
  const std::vector<uint8_t> addr = { regAddress };
  const auto data = nsw::intToByteVector(value, nsw::NUM_BYTES_IN_WORD32, nsw::padtrigger::SCA_LITTLE_ENDIAN);

  std::vector<uint8_t> payload;
  payload.reserve(addr.size() + data.size());
  payload.insert(std::end(payload), std::begin(addr), std::end(addr));
  payload.insert(std::end(payload), std::begin(data), std::end(data));

  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  nsw::hw::SCA::sendI2cRaw(opcConnection, m_scaAddressFPGA, payload.data(), payload.size());
}

void nsw::hw::PadTrigger::writePFEBCommonDelay(const std::uint32_t value) const
{
  std::uint32_t word{0};
  constexpr std::uint32_t bits{nsw::padtrigger::NUM_BITS_PER_PFEB_BCID};
  for (auto it = nsw::NUM_BITS_IN_WORD32 / bits; it > 0; --it) {
    word += (value << it*bits);
  }
  ERS_INFO(fmt::format("Writing PFEB delay word {:#010x}", word));
  writeFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_23_16, word);
  writeFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_15_08, word);
  writeFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_07_00, word);
}

bool nsw::hw::PadTrigger::readGPIO(const std::string& name) const
{
  const auto addr = fmt::format("{}.{}.{}", m_scaAddress, "gpio", name);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  return nsw::hw::SCA::readGPIO(opcConnection, addr);
}

std::uint8_t nsw::hw::PadTrigger::readRepeaterRegister(const std::uint8_t repeater,
                                                       const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto val = nsw::hw::SCA::readI2cAtAddress(opcConnection, addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint8_t nsw::hw::PadTrigger::readVTTxRegister(const std::uint8_t vttx,
                                                   const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", m_scaAddress,
                                "vttx", vttx,
                                "vttx", vttx);
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto val = nsw::hw::SCA::readI2cAtAddress(opcConnection, addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint32_t nsw::hw::PadTrigger::readFPGARegister(const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = { regAddress };
  const auto& opcConnection = OpcManager::getConnection(m_opcserverIp);
  const auto value = nsw::hw::SCA::readI2cAtAddress(opcConnection, m_scaAddressFPGA,
                                                    data.data(), data.size(),
                                                    nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(value, nsw::padtrigger::SCA_LITTLE_ENDIAN);
}

std::vector<std::uint32_t> nsw::hw::PadTrigger::readPFEBBCIDs() const
{
  const auto bcids_23_16 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_23_16);
  const auto bcids_15_08 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_15_08);
  const auto bcids_07_00 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_07_00);
  return PFEBBCIDs(bcids_07_00, bcids_15_08, bcids_23_16);
}

std::vector< std::vector<std::uint32_t> > nsw::hw::PadTrigger::readPFEBBCIDs(std::size_t nread) const
{
  if (nread == std::size_t{0}) {
    const auto msg = "Why read the PFEB BCIDs zero times?";
    ers::warning(nsw::PadTriggerConfusion(ERS_HERE, msg));
  }
  auto collated_bcids = std::vector< std::vector<std::uint32_t> >(nsw::padtrigger::NUM_PFEBS);
  for (std::size_t iread = 0; iread < nread; iread++) {
    const auto bcids = readPFEBBCIDs();
    for (std::size_t it = 0; it < nsw::padtrigger::NUM_PFEBS; it++) {
      collated_bcids.at(it).push_back(bcids.at(it));
    }
  }
  return collated_bcids;
}

std::vector<std::uint32_t> nsw::hw::PadTrigger::readMedianPFEBBCIDs(std::size_t nread) const
{
  const auto bcids_per_pfeb = readPFEBBCIDs(nread);
  auto median_bcids = std::vector<std::uint32_t>();
  auto median = [](std::vector<std::uint32_t> v) {
    std::size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + n, v.end());
    return v[n];
  };
  for (const auto& bcids: bcids_per_pfeb) {
    median_bcids.emplace_back(median(bcids));
  }
  return median_bcids;
}

std::vector<std::uint32_t> nsw::hw::PadTrigger::rotatePFEBBCIDs(const std::vector<std::uint32_t>& bcids) const
{
  constexpr std::uint32_t NBCID{nsw::padtrigger::NUM_PFEB_BCIDS};
  auto rotated_bcids = std::vector<std::uint32_t>();
  for (const auto& bcid: bcids) {
    rotated_bcids.emplace_back((bcid < NBCID/2) ? bcid + NBCID : bcid);
  }
  return rotated_bcids;
}

bool nsw::hw::PadTrigger::checkPFEBBCIDs(const std::vector<std::uint32_t>& bcids) const
{
  auto is_connected = [](std::uint32_t bcid){ return bcid != nsw::padtrigger::PFEB_BCID_DISCONNECTED; };
  auto is_nonzero   = [](std::uint32_t bcid){ return bcid != 0; };
  const auto rotated_bcids = rotatePFEBBCIDs(bcids);

  bool any_connected    = std::any_of(bcids.cbegin(), bcids.cend(), is_connected);
  bool any_nonzero      = std::any_of(bcids.cbegin(), bcids.cend(), is_nonzero);
  bool all_incrementing = std::is_sorted(bcids.cbegin(), bcids.cend()) or
                          std::is_sorted(rotated_bcids.cbegin(), rotated_bcids.cend());

  return any_connected and any_nonzero and all_incrementing;
}

std::uint8_t nsw::hw::PadTrigger::addressFromRegisterName(const std::string& name) const
{
  std::uint8_t addr{0};
  try {
    const auto addr_str = name.substr(std::size_t{0}, name.find("_"));
    addr = std::stoul(addr_str, nullptr, nsw::BASE_HEX);
  } catch (std::exception & ex) {
    const auto msg = fmt::format("Cannot get address from: {}", name);
    nsw::PadTriggerConfigError issue(ERS_HERE, msg.c_str());
    ers::error(issue);
    throw issue;
  }
  return addr;
}

std::uint32_t nsw::hw::PadTrigger::firmware_dateword() const {

  std::uint32_t dateword = 0;
  const std::string& fw = firmware();
  if (fw.empty()) {
    return dateword;
  }

  // extract YYYY*MM*DD from firmware()
  //   [0-9]{N} means "N digits"
  //   .?       means "zero or one characters"
  const std::regex yyyy_mm_dd("[0-9]{4}.?[0-9]{2}.?[0-9]{2}");
  std::smatch matches;
  std::regex_search(fw, matches, yyyy_mm_dd);

  // check result
  if (matches.size() != 1) {
    ERS_INFO(fmt::format("Cannot extract YYYYMMDD from {}, returning {}", fw, dateword));
    return dateword;
  }

  // convert YYYY*MM*DD to YYYYMMDD
  for (const auto& match: matches) {
    const std::regex non_numeric("[^0-9]");
    const std::string datestr =
      std::regex_replace(std::string(match), non_numeric, "");
    dateword = static_cast<uint32_t>(std::stoul(datestr));
    break;
  }

  // return
  return dateword;
}

std::vector<std::uint32_t> nsw::hw::PadTrigger::PFEBBCIDs(std::uint32_t val_07_00,
                                                          std::uint32_t val_15_08,
                                                          std::uint32_t val_23_16
                                                          ) const {
  std::vector<std::uint32_t> bcids_07_00 = {};
  std::vector<std::uint32_t> bcids_15_08 = {};
  std::vector<std::uint32_t> bcids_23_16 = {};
  std::vector<std::uint32_t> bcids  = {};
  size_t bit_position = 0;
  while (bit_position < nsw::NUM_BITS_IN_WORD32) {
    bcids_07_00.push_back( (val_07_00 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bcids_15_08.push_back( (val_15_08 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bcids_23_16.push_back( (val_23_16 >> bit_position) & nsw::padtrigger::PFEB_BCID_BITMASK );
    bit_position += nsw::padtrigger::NUM_BITS_PER_PFEB_BCID;
  }
  for (const auto bcid: bcids_07_00) {
    bcids.push_back(bcid);
  }
  for (const auto bcid: bcids_15_08) {
    bcids.push_back(bcid);
  }
  for (const auto bcid: bcids_23_16) {
    bcids.push_back(bcid);
  }
  return bcids;
}
