#include "NSWConfiguration/monitoring/CarriertpInRunStatusRegisters.h"

nsw::mon::CarriertpInRunStatusRegisters::CarriertpInRunStatusRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getTpCarriers()}, m_helper{NUM_CONCURRENT}
{ }

void nsw::mon::CarriertpInRunStatusRegisters::monitor(ISInfoDictionary* isDict,
                                                 const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.get(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::CarriertpInRunStatusRegisters::getData);
}

nsw::mon::is::CarriertpInRunStatusRegisters nsw::mon::CarriertpInRunStatusRegisters::getData(
  const nsw::hw::TPCarrier& tp)
{
  auto is = nsw::mon::is::CarriertpInRunStatusRegisters{};

  std::uint32_t regval = tp.readRegister(nsw::carrier::REG_GBT_PHASE_SKEW);
   is.txFramePhase = (regval & 0xF);
   is.rxFramePhase = ((regval >> 4 ) & 0xF) ;
   is.rxGbtPhase   = ((regval >> 8 ) & 0xF);
   is.bcrPhAlgn   =  ((regval >> 12) & 0x7);
 
  return is;
}
