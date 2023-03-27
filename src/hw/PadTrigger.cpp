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

nsw::hw::PadTrigger::PadTrigger(OpcManager& manager, const boost::property_tree::ptree& config):
  ScaAddressBase(config.get<std::string>("OpcNodeId")),
  OpcConnectionBase(manager, config.get<std::string>("OpcServerIp"), config.get<std::string>("OpcNodeId")),
  m_ptree{config},
  m_padtriggerfpga{config.get_child(PADTRIGGER_NAME), PADTRIGGER_NAME, PADTRIGGER_REGISTERS}
{
  m_name = fmt::format("{}/{}", getOpcServerIp(), getScaAddress());
  m_scaAddressFPGA = fmt::format("{}.fpga.fpga", getScaAddress());
  m_scaAddressJTAG = fmt::format("{}.jtag.fpga", getScaAddress());
}

void nsw::hw::PadTrigger::writeConfiguration() const
{
  writeRepeatersConfiguration();
  writeVTTxConfiguration();
  writeJTAGBitfileConfiguration();
  writeFPGAConfiguration();
  toggleGtReset();
  deskewPFEBs();
  toggleIdleState();
  toggleOcrEnable();
  toggleBcidErrorReset();
  toggleGtRxLolReset();
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
    writeRepeaterRegister(rep, address, value);

    // i2c readback
    const auto val = readRepeaterRegister(rep, address);

    // check and complain
    if (val != value) {
      nsw::PadTriggerReadbackMismatch issue(ERS_HERE, "Repeater");
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
        nsw::PadTriggerReadbackMismatch issue(ERS_HERE, "VTTx");
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
  if (readGPIO(FPGA_DONE) and not ForceFirmwareUpload()) {
    ERS_INFO("Not uploading bitfile since firmware already uploaded");
    return;
  }
  ERS_LOG("Uploading bitfile via SCA JTAG, this will take a minute...");
  nsw::hw::SCA::writeXilinxFpga(getConnection(), m_scaAddressJTAG, fw);
  nsw::snooze();
  if (not readGPIO(FPGA_DONE)) {
    throw nsw::PadTriggerConfigError
      (ERS_HERE, fmt::format("Upload failed for {}", m_name));
  }
  ERS_LOG("Upload finished");
}

void nsw::hw::PadTrigger::writeFPGAConfiguration() const
{
  if (not ConfigFPGA()) {
    ERS_INFO(fmt::format("Skipping configuration of FPGA registers of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("Writing FPGA registers of {}", m_name));

  const auto& fpga = getFpga();

  for (const auto& [rname, value_str] : fpga.getBitstreamMap()) {

    // strings -> numerics
    const auto addr  = addressFromRegisterName(rname);
    const auto value = std::stoul(value_str, nullptr, nsw::BASE_BIN);

    // write
    ERS_LOG(fmt::format("{}: writing to {} ({:#04x}) with {:#010x}", m_name, rname, addr, value));
    try {
      writeAndReadbackFPGARegister(addr, value);
    } catch (const std::exception & ex) {
      if (addr == nsw::padtrigger::REG_CONTROL) {
        throw;
      } else {
        const auto msg = fmt::format(
          "{}: Failed to write to {}. This usually indicates an older firmware version. Skipping!",
          m_name, rname);
        ers::warning(nsw::PadTriggerConfigError(ERS_HERE, msg));
        break;
      }
    }
  }
}

void nsw::hw::PadTrigger::writeSubRegister(const std::string& rname,
                                           const std::string& subreg,
                                           const std::uint32_t subval,
                                           const bool quiet) const
{
  // get the register address
  const auto addr = addressFromRegisterName(rname);

  // overwrite the bits of interest
  const auto fpga = getFpga();
  const auto pos  = fpga.getAddressPositions().at(rname).at(subreg);
  const auto siz  = fpga.getAddressSizes()    .at(rname).at(subreg);
  const auto rpos = std::size_t{nsw::NUM_BITS_IN_WORD32 - pos - siz};
  const auto value =
    nsw::overwriteBits(readFPGARegister(addr), subval, rpos, siz);

  // write
  if (not quiet) {
    ERS_LOG(fmt::format("{}: writing to 0x{:02x} with 0x{:08x}", m_name, addr, value));
  }
  writeFPGARegister(addr, value);
}

void nsw::hw::PadTrigger::toggleOcrEnable() const
{
  if (not OcrEnable()) {
    ERS_INFO(fmt::format("Skipping toggleOcrEnable of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("toggleOcrEnable of {}", m_name));
  writeOcrEnDisable();
  writeOcrEnEnable();
  writeOcrEnDisable();
}

void nsw::hw::PadTrigger::toggleGtReset() const
{
  if (not GtReset()) {
    ERS_INFO(fmt::format("Skipping toggleGtReset of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("toggleGtReset of {}", m_name));
  writeGtSoftResetDisable();
  writeGtSoftResetEnable();
  writeGtSoftResetDisable();
  nsw::snooze();
  const auto pfeb_status = readFPGARegister(nsw::padtrigger::REG_PFEB_STATUS);
  ERS_INFO(fmt::format("{} PFEB status: 0x{:08x}", m_name, pfeb_status));
}

void nsw::hw::PadTrigger::toggleGtRxLolReset() const
{
  if (not GtRxLolReset()) {
    ERS_INFO(fmt::format("Skipping GtRxLolReset of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("toggleGtRxLolReset of {}", m_name));
  writeGtRxLolResetDisable();
  writeGtRxLolResetEnable();
  writeGtRxLolResetDisable();
  nsw::snooze();
}

void nsw::hw::PadTrigger::toggleIdleState() const
{
  if (not Toggle()) {
    ERS_INFO(fmt::format("Skipping toggleIdleState of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("toggleIdleState of {}", m_name));
  writeStartIdleStateDisable();
  writeStartIdleStateEnable();
  writeStartIdleStateDisable();
}

void nsw::hw::PadTrigger::toggleBcidErrorReset() const
{
  if (not BcidErrorReset()) {
    ERS_INFO(fmt::format("Skipping toggleBcidErrorReset of {}", m_name));
    return;
  }
  ERS_INFO(fmt::format("toggleBcidErrorReset of {}", m_name));
  writeBcidResetDisable();
  writeBcidResetEnable();
  writeBcidResetDisable();
  nsw::snooze();
  const auto pad_bcid_error = readFPGARegister(nsw::padtrigger::REG_BCID_ERROR);
  const auto tp_bcid_error  = readFPGARegister(nsw::padtrigger::REG_TP_BCID_ERROR);
  ERS_INFO(fmt::format("{} BCID error status: {:#010x} (pad) {:#010x} (tp)",
                       m_name, pad_bcid_error, tp_bcid_error));
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

std::map<std::string, std::uint32_t> nsw::hw::PadTrigger::readConfigurationSubRegisters() const
{
  auto result = std::map<std::string, std::uint32_t>();
  for (const auto& [rname, subregs] : PADTRIGGER_REGISTERS) {
    const auto value = readFPGARegister(addressFromRegisterName(rname));
    for (const auto& [subreg, bits] : subregs) {
      if (subreg == nsw::NOT_USED) {
        continue;
      }
      result.emplace(fmt::format("{}/{}", rname, subreg),
                     getSubRegisterFromRegister(rname, std::string{subreg}, value));
    }
  }
  return result;
}

void nsw::hw::PadTrigger::writeGPIO(const std::string& name, const bool value) const
{
  const auto addr = fmt::format("{}.{}.{}", getScaAddress(), "gpio", name);
  nsw::hw::SCA::sendGPIO(getConnection(), addr, value);
}

void nsw::hw::PadTrigger::writeRepeaterRegister(const std::uint8_t repeater,
                                                const std::uint8_t regAddress,
                                                const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", getScaAddress(),
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  nsw::hw::SCA::sendI2c(getConnection(), addr, data);
}

void nsw::hw::PadTrigger::writeVTTxRegister(const std::uint8_t vttx,
                                            const std::uint8_t regAddress,
                                            const std::uint8_t value) const
{
  const std::vector<uint8_t> data = {regAddress, value};
  const auto addr = fmt::format("{}.{}{}.{}{}", getScaAddress(),
                                "vttx", vttx,
                                "vttx", vttx);
  nsw::hw::SCA::sendI2c(getConnection(), addr, data);
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

  nsw::hw::SCA::sendI2cRaw(getConnection(), m_scaAddressFPGA, payload.data(), payload.size());
}

void nsw::hw::PadTrigger::writeAndReadbackFPGARegister(const std::uint8_t regAddress,
                                                       const std::uint32_t value) const
{
  writeFPGARegister(regAddress, value);
  const auto val = readFPGARegister(regAddress);
  if (val != value) {
    ers::error(nsw::PadTriggerReadbackMismatch(ERS_HERE, "FPGA"));
  }
}

void nsw::hw::PadTrigger::writePFEBDelay(const DelayVector& values) const
{
  std::uint32_t word_23_16{0};
  std::uint32_t word_15_08{0};
  std::uint32_t word_07_00{0};
  constexpr std::uint32_t bits{nsw::padtrigger::NUM_BITS_PER_PFEB_DELAY};
  constexpr std::uint32_t dels{nsw::NUM_BITS_IN_WORD32 / bits};
  if (values.size() != nsw::padtrigger::NUM_PFEBS) {
    const auto msg = fmt::format("Tried to write delays but N(delays) = {}", values.size());
    ers::warning(nsw::PadTriggerConfusion(ERS_HERE, msg));
  }
  for (auto it = std::uint32_t{0}; it < dels; it++) {
    word_23_16 += (values.at(it + 2*dels) << it*bits);
    word_15_08 += (values.at(it + 1*dels) << it*bits);
    word_07_00 += (values.at(it + 0*dels) << it*bits);
  }
  ERS_LOG(fmt::format("Writing PFEB delay word (23-16) {:#010x}", word_23_16));
  ERS_LOG(fmt::format("Writing PFEB delay word (15-08) {:#010x}", word_15_08));
  ERS_LOG(fmt::format("Writing PFEB delay word (07-00) {:#010x}", word_07_00));
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_23_16, word_23_16);
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_15_08, word_15_08);
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_07_00, word_07_00);
}

void nsw::hw::PadTrigger::writePFEBCommonDelay(const std::uint32_t value) const
{
  std::uint32_t word{0};
  constexpr std::uint32_t bits{nsw::padtrigger::NUM_BITS_PER_PFEB_DELAY};
  for (auto it = nsw::NUM_BITS_IN_WORD32 / bits; it > 0; --it) {
    word += (value << it*bits);
  }
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_23_16, word);
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_15_08, word);
  writeAndReadbackFPGARegister(nsw::padtrigger::REG_PFEB_DELAY_07_00, word);
}

bool nsw::hw::PadTrigger::readGPIO(const std::string& name) const
{
  const auto addr = fmt::format("{}.{}.{}", getScaAddress(), "gpio", name);
  return nsw::hw::SCA::readGPIO(getConnection(), addr);
}

std::uint8_t nsw::hw::PadTrigger::readRepeaterRegister(const std::uint8_t repeater,
                                                       const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", getScaAddress(),
                                "repeaterChip", repeater,
                                "repeaterChip", repeater);
  const auto val = nsw::hw::SCA::readI2cAtAddress(getConnection(), addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint8_t nsw::hw::PadTrigger::readVTTxRegister(const std::uint8_t vttx,
                                                   const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = {regAddress};
  const auto addr = fmt::format("{}.{}{}.{}{}", getScaAddress(),
                                "vttx", vttx,
                                "vttx", vttx);
  const auto val = nsw::hw::SCA::readI2cAtAddress(getConnection(), addr,
                                                  data.data(), data.size());
  return val.at(0);
}

std::uint32_t nsw::hw::PadTrigger::readFPGARegister(const std::uint8_t regAddress) const
{
  const std::vector<std::uint8_t> data = { regAddress };
  const auto value = nsw::hw::SCA::readI2cAtAddress(getConnection(), m_scaAddressFPGA,
                                                    data.data(), data.size(),
                                                    nsw::NUM_BYTES_IN_WORD32);
  return nsw::byteVectorToWord32(value, nsw::padtrigger::SCA_LITTLE_ENDIAN);
}

std::uint32_t nsw::hw::PadTrigger::readSubRegister(const std::string& rname,
                                                   const std::string& subreg) const
{
  return getSubRegisterFromRegister(rname, subreg, readFPGARegister(addressFromRegisterName(rname)));
}

std::uint32_t nsw::hw::PadTrigger::getSubRegisterFromRegister(const std::string& rname,
                                                              const std::string& subreg,
                                                              const std::uint32_t value) const
{
  constexpr std::uint64_t one{1};
  const auto fpga = getFpga();
  const auto pos  = fpga.getAddressPositions().at(rname).at(subreg);
  const auto siz  = fpga.getAddressSizes()    .at(rname).at(subreg);
  const auto rpos = nsw::NUM_BITS_IN_WORD32 - pos - siz;
  const auto mask = (one << (rpos + siz)) - (one << rpos);
  return (value & mask) >> rpos;
}

std::uint32_t nsw::hw::PadTrigger::readPFEBRate(const std::uint32_t pfeb, const bool quiet) const
{
  writeSubRegister("003_control_reg2", "pfeb_num", pfeb, quiet);
  return readSubRegister("00D_status_reg2_READONLY", "pfeb_hit_rate");
}

std::vector<std::uint32_t> nsw::hw::PadTrigger::readPFEBRates() const
{
  const bool quiet{true};
  auto rates = std::vector<std::uint32_t>();
  for (std::uint32_t pfeb = 0; pfeb < nsw::padtrigger::NUM_PFEBS; pfeb++) {
    rates.push_back(readPFEBRate(pfeb, quiet));
  }
  return rates;
}

BcidVector nsw::hw::PadTrigger::readPFEBBCIDs() const
{
  const auto bcids_23_16 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_23_16);
  const auto bcids_15_08 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_15_08);
  const auto bcids_07_00 = readFPGARegister(nsw::padtrigger::REG_PFEB_BCID_07_00);
  return PFEBBCIDs(bcids_07_00, bcids_15_08, bcids_23_16);
}

std::vector<BcidVector> nsw::hw::PadTrigger::readPFEBBCIDs(std::size_t nread) const
{
  if (nread == std::size_t{0}) {
    const auto msg = "Why read the PFEB BCIDs zero times?";
    ers::warning(nsw::PadTriggerConfusion(ERS_HERE, msg));
  }
  auto collated_bcids = std::vector<BcidVector>(nsw::padtrigger::NUM_PFEBS);
  for (std::size_t iread = 0; iread < nread; iread++) {
    pushBackColumn(collated_bcids, readPFEBBCIDs());
  }
  return collated_bcids;
}

BcidVector nsw::hw::PadTrigger::readMedianPFEBBCIDs(std::size_t nread) const
{
  const auto bcids_per_pfeb = readPFEBBCIDs(nread);
  auto median_bcids = BcidVector();
  auto median = [](BcidVector v) {
    std::size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + n, v.end());
    return v.at(n);
  };
  std::transform(std::cbegin(bcids_per_pfeb), std::cend(bcids_per_pfeb), std::back_inserter(median_bcids),
                 [&median](const auto& bcids){ return median(bcids); });
  return median_bcids;
}

std::vector<BcidVector> nsw::hw::PadTrigger::readMedianPFEBBCIDAtEachDelay(std::size_t nread) const
{
  auto collated_bcids = std::vector<BcidVector>(nsw::padtrigger::NUM_PFEBS);
  for (std::size_t delay = 0; delay < nsw::padtrigger::NUM_INPUT_DELAYS; delay++) {
    writePFEBCommonDelay(delay);
    pushBackColumn(collated_bcids, readMedianPFEBBCIDs(nread));
  }
  return collated_bcids;
}

BcidVector nsw::hw::PadTrigger::rotatePFEBBCIDs(BcidVector bcids) const
{
  constexpr std::uint32_t RANGE{nsw::padtrigger::PFEB_BCID_RANGE};
  std::transform(std::begin(bcids), std::end(bcids), std::begin(bcids), [](const auto& bcid) {
    return (bcid < RANGE / 2) ? bcid + RANGE : bcid;
  });
  return bcids;
}

bool nsw::hw::PadTrigger::checkPFEBBCIDs(const BcidVector& bcids) const
{
  if (bcids.empty()) {
    return false;
  }
  auto is_unique = [&bcids](std::uint32_t bcid){ return bcid != bcids.front(); };
  const auto rotated_bcids = rotatePFEBBCIDs(bcids);

  const bool any_unique       = std::any_of(bcids.cbegin(), bcids.cend(), is_unique);
  const bool all_decrementing = std::is_sorted(bcids.crbegin(), bcids.crend()) or
                                std::is_sorted(rotated_bcids.crbegin(), rotated_bcids.crend());

  return any_unique and all_decrementing;
}

BcidVector nsw::hw::PadTrigger::getViableBcids(const std::vector<BcidVector>& bcidPerPfebPerDelay) const
{
  // get viable BCIDs for each PFEB
  auto viablePerPfeb = std::vector<BcidVector>();
  for (const auto& bcidPerDelay: bcidPerPfebPerDelay) {
    viablePerPfeb.emplace_back(getViableBcids(bcidPerDelay));
  }

  // if no BCID are viable for any PFEB: bail
  if (std::all_of(viablePerPfeb.begin(), viablePerPfeb.end(),
                  [](const auto& viable){ return viable.empty(); })) {
    return BcidVector();
  }

  // get BCIDs which are viable for all PFEBs
  auto viableForAllPfeb = BcidVector();
  for (std::size_t bcid = 0; bcid < nsw::padtrigger::PFEB_BCID_RANGE; bcid++) {
    const auto ok = std::all_of(viablePerPfeb.begin(), viablePerPfeb.end(),
                                [bcid](const auto& viable){
                                  return viable.empty() or
                                  std::find(viable.begin(), viable.end(), bcid) != viable.end();
                                });
    if (ok) {
      viableForAllPfeb.push_back(bcid);
    }
  }
  return viableForAllPfeb;
}

BcidVector nsw::hw::PadTrigger::getViableBcids(const BcidVector& bcidPerDelay) const
{
  constexpr std::uint32_t RANGE{nsw::padtrigger::PFEB_BCID_RANGE};
  constexpr std::uint32_t MINCOUNTS{nsw::padtrigger::NUM_INPUT_DELAYS_PER_BC - nsw::padtrigger::NUM_INPUT_DELAYS_MARGIN};
  if (not checkPFEBBCIDs(bcidPerDelay)) {
    return BcidVector();
  }
  auto viable = BcidVector();
  for (std::uint32_t bcid = 0; bcid < RANGE; bcid++) {
    if (std::count(bcidPerDelay.begin(), bcidPerDelay.end(), bcid) >= MINCOUNTS) {
      viable.push_back(bcid);
    }
  }
  return viable;
}

std::uint32_t nsw::hw::PadTrigger::getTargetBcid(const std::vector<BcidVector>& bcidPerPfebPerDelay) const
{
  const auto viableBcids = getViableBcids(bcidPerPfebPerDelay);
  const auto nviable = viableBcids.size();
  if (nviable < 1 or nviable > 3) {
    const auto msg = fmt::format("N(Viable BCID) is strange: {}. Will not deskew", nviable);
    ers::warning(nsw::PadTriggerConfusion(ERS_HERE, msg));
    return 0xFFFFFFFF;
  }
  auto medianDelays = DelayVector();
  for (const auto& bcid: viableBcids) {
    medianDelays.push_back(getMedianDelay(bcid, bcidPerPfebPerDelay));
  }
  const auto idx = std::distance(medianDelays.begin(), std::min_element(medianDelays.begin(), medianDelays.end()));
  return viableBcids.at(idx);
}

std::uint32_t nsw::hw::PadTrigger::getMedianDelay(const std::uint32_t bcid,
                                                  const std::vector<BcidVector>& bcidPerPfebPerDelay) const
{
  auto delays = DelayVector();
  for (const auto& bcidPerDelay: bcidPerPfebPerDelay) {
    if (not checkPFEBBCIDs(bcidPerDelay)) {
      continue;
    }
    for (std::size_t delay = 0; delay < bcidPerDelay.size(); delay++) {
      if (bcidPerDelay.at(delay) == bcid) {
        delays.push_back(delay);
      }
    }
  }
  if (delays.empty()) {
    return 0;
  }
  auto median = [](DelayVector v) {
    std::size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + n, v.end());
    return v.at(n);
  };
  return median(delays);
}

DelayVector nsw::hw::PadTrigger::getTargetDelays(const std::uint32_t targetBcid,
                                                 const std::vector<BcidVector>& bcidPerPfebPerDelay) const
{
  auto delays = DelayVector();
  for (const auto& bcidPerDelay: bcidPerPfebPerDelay) {
    delays.push_back(getTargetDelay(targetBcid, bcidPerDelay));
  }
  return delays;
}

std::uint32_t nsw::hw::PadTrigger::getTargetDelay(const std::uint32_t targetBcid,
                                                  const BcidVector& bcidPerDelay) const
{
  const auto noDelay = std::uint32_t{0};
  if (not checkPFEBBCIDs(bcidPerDelay)) {
    return noDelay;
  }
  constexpr std::uint32_t TARGETPOS{nsw::padtrigger::NUM_INPUT_DELAYS_PER_BC / 2};
  constexpr std::uint32_t NUMDELSBC{nsw::padtrigger::NUM_INPUT_DELAYS_PER_BC};
  const std::size_t numBcids = std::count(bcidPerDelay.begin(), bcidPerDelay.end(), targetBcid);
  const std::size_t startPos = (bcidPerDelay.front() == targetBcid) ? NUMDELSBC - numBcids : 0;
  std::size_t pos{startPos};
  for (std::size_t delay = 0; delay < bcidPerDelay.size(); delay++) {
    if (pos == TARGETPOS) {
      return delay;
    }
    if (bcidPerDelay.at(delay) == targetBcid) {
      pos++;
    }
  }
  return noDelay;
}

void nsw::hw::PadTrigger::describeSkew(const std::vector<BcidVector>& bcidPerPfebPerDelay) const
{
  // describe the skew of each PFEB
  for (std::size_t pfeb = 0; pfeb < bcidPerPfebPerDelay.size(); pfeb++) {
    const auto& bcidPerDelay = bcidPerPfebPerDelay.at(pfeb);
    const auto msg = fmt::format("PFEB {:02}: {} ({})",
                                 pfeb, joinHexReversed(bcidPerDelay),
                                 joinHexReversed(getViableBcids(bcidPerDelay)));
    ERS_LOG(msg);
  }

  // describe which BCID are viable targets
  const auto viableBcids = getViableBcids(bcidPerPfebPerDelay);
  for (const auto vi: viableBcids) {
    const auto msg = fmt::format("Viable BCID {:x} with median delay {}",
                                 vi, getMedianDelay(vi, bcidPerPfebPerDelay));
    ERS_LOG(msg);
  }
}

void nsw::hw::PadTrigger::deskewPFEBs() const
{
  if (not Deskew()) {
    ERS_INFO(fmt::format("Skipping deskew of {}", m_name));
    return;
  }

  ERS_INFO("Start of PFEB deskew");
  const auto bcidPerPfebPerDelay = readMedianPFEBBCIDAtEachDelay(nsw::padtrigger::NUM_DESKEW_READS);
  describeSkew(bcidPerPfebPerDelay);
  const auto targetBcid = getTargetBcid(bcidPerPfebPerDelay);
  const auto targetDelays = getTargetDelays(targetBcid, bcidPerPfebPerDelay);
  writePFEBDelay(targetDelays);

  // summarize
  ERS_LOG(fmt::format("Target BCID: {:x}", targetBcid));
  ERS_INFO(fmt::format("Target delays: {}", joinHexReversed(targetDelays)));
  ERS_INFO(fmt::format("New BCIDs: {}", joinHexReversed(readPFEBBCIDs())));
}

std::string nsw::hw::PadTrigger::joinHexReversed(const std::vector<uint32_t>& vec) {
  if (vec.empty()) {
    return "";
  }
  std::string ret{"0x"};
  for (auto it = vec.crbegin(); it != vec.crend(); ++it) {
    ret += fmt::format("{:x}", *it);
  }
  return ret;
}

void nsw::hw::PadTrigger::pushBackColumn(std::vector< std::vector<std::uint32_t > >& matrix,
                                         const std::vector<std::uint32_t>& column) {
  for (std::size_t it = 0; it < column.size(); it++) {
    matrix.at(it).push_back(column.at(it));
  }
}

std::uint8_t nsw::hw::PadTrigger::addressFromRegisterName(const std::string& name) const
{
  std::uint8_t addr{0};
  try {
    const auto addr_str = name.substr(std::size_t{0}, name.find("_"));
    addr = std::stoul(addr_str, nullptr, nsw::BASE_HEX);
  } catch (std::exception & ex) {
    nsw::PadTriggerConfigError issue(ERS_HERE, fmt::format("Cannot get address from: {}", name));
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

BcidVector nsw::hw::PadTrigger::PFEBBCIDs(std::uint32_t val_07_00,
                                          std::uint32_t val_15_08,
                                          std::uint32_t val_23_16
                                          ) const {
  BcidVector bcids_07_00 = {};
  BcidVector bcids_15_08 = {};
  BcidVector bcids_23_16 = {};
  BcidVector bcids  = {};
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

std::uint32_t nsw::hw::PadTrigger::readFPGATemperature() const {
  return xadcToCelsius(readSubRegister("001_status_reg_READONLY", "xadc_temp_out"));
}

std::uint32_t nsw::hw::PadTrigger::xadcToCelsius(std::uint32_t temp) {
  // www.xilinx.com/support/documentation/user_guides/ug480_7Series_XADC.pdf
  return static_cast<std::uint32_t>((static_cast<double>(temp) * 503.975 / 4096) - 273.15);
}
