#include "NSWConfiguration/monitoring/StgctpOutRunStatusRegisters.h"

#include "NSWConfiguration/TPConstants.h"

nsw::mon::StgctpOutRunStatusRegisters::StgctpOutRunStatusRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getSTGCTps()}, m_helper{NUM_CONCURRENT}
{}

void nsw::mon::StgctpOutRunStatusRegisters::monitor(ISInfoDictionary* isDict,
                                                  const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.get(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::StgctpOutRunStatusRegisters::getData);
}

nsw::mon::is::StgctpOutRunStatusRegisters nsw::mon::StgctpOutRunStatusRegisters::getData(
  const nsw::hw::STGCTP& tp)
{
  auto is = nsw::mon::is::StgctpOutRunStatusRegisters{};

  // is.bcrRate =       tp.readRegister(nsw::stgctp::REG_BCR_RATE,          nsw::stgctp::MASK_BCR_RATE);
  // is.SectorID = tp.readRegister(nsw::stgctp::REG_SECTOR,                 nsw::stgctp::MASK_SECTOR);
  // is.PadIdleStatus = (tp.readRegister(nsw::stgctp::REG_PAD_IDLE_STATUS,  nsw::stgctp::MASK_PAD_IDLE_STATUS) != 0U);
  // is.MMIdleStatus = (tp.readRegister(nsw::stgctp::REG_MM_IDLE_STATUS,    nsw::stgctp::MASK_MM_IDLE_STATUS) != 0U);

  return is;
}

std::uint32_t nsw::mon::StgctpOutRunStatusRegisters::readHorxEnvMonData(const nsw::hw::STGCTP& tp)
{
  // return tp.readRegister(nsw::stgctp::REG_SL_LATENCY_COMP, 0x2);
}
