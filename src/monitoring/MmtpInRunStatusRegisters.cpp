#include "NSWConfiguration/monitoring/MmtpInRunStatusRegisters.h"
#include "NSWConfiguration/TPConstants.h"

nsw::mon::MmtpInRunStatusRegisters::MmtpInRunStatusRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getMMTps()}, m_helper{NUM_CONCURRENT}
{}

void nsw::mon::MmtpInRunStatusRegisters::monitor(ISInfoDictionary* isDict,
                                                 const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.get(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::MmtpInRunStatusRegisters::getData);
}

nsw::mon::is::MmtpInRunStatusRegisters nsw::mon::MmtpInRunStatusRegisters::getData(
  const nsw::hw::MMTP& tp)
{
  auto is = nsw::mon::is::MmtpInRunStatusRegisters{};

  is.artFibersAlignment.clear();
  const auto& s = tp.readAlignment(1);
  std::transform(s.begin(),
                 s.end(),
                 std::back_inserter(is.artFibersAlignment),
                 [&](std::uint32_t c) { return static_cast<bool>(c); });

  is.nArtFibersAligned = std::accumulate(is.artFibersAlignment.begin(), is.artFibersAlignment.end(), 0U);

  is.artFibersBcidGood.clear();
  const auto word = tp.readRegister(nsw::mmtp::REG_GBT_BCID_OK);
  for (std::size_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS; fiber++) {
    is.artFibersBcidGood.push_back((word >> fiber) & 1);
  }

  is.nArtFibersBcidGood = std::accumulate(is.artFibersBcidGood.begin(), is.artFibersBcidGood.end(), 0U);

  is.idleState = (tp.readRegister(nsw::mmtp::REG_GLO_SYNC_IDLE_STATE) != 0U);
  is.bcidOffset = tp.readRegister(nsw::mmtp::REG_GLO_SYNC_BCID_OFFSET);
  is.suggestedBcidOffset = tp.readRegister(nsw::mmtp::REG_OFFSET_MODE_BCID);
  is.nGoodOffset = tp.readRegister(nsw::mmtp::REG_OFFSET_MODE_CNT);

  is.fiberBCIDs.clear();
  const auto numFibersPerReg = nsw::mmtp::NUM_FIBERS / nsw::mmtp::REG_FIBER_BCIDS.size();
  for (auto reg : nsw::mmtp::REG_FIBER_BCIDS) {
    const auto val = tp.readRegister(reg);
    for (std::size_t  i = 0; i < numFibersPerReg; ++i) {
      // FIXME: Proper name for magic number please
      constexpr static auto BYTE_MASK = unsigned{0xf};
      is.fiberBCIDs.push_back((val >> (i * 4)) & BYTE_MASK);
    }
  }

  return is;
}  
