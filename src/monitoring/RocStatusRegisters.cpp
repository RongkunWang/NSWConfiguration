#include "NSWConfiguration/monitoring/RocStatusRegisters.h"

#include "NSWConfiguration/monitoring/Helper.h"

nsw::mon::RocStatusRegisters::RocStatusRegisters(const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getFebs()}, m_helper{NUM_CONCURRENT}
{}

void nsw::mon::RocStatusRegisters::monitor(ISInfoDictionary* isDict,
                                           const std::string_view serverName)
{
  m_helper.monitorAndPublish(
    m_devices.get(), isDict, m_threadPool, serverName, NAME, nsw::mon::RocStatusRegisters::getData);
}

nsw::mon::is::RocStatus nsw::mon::RocStatusRegisters::getData(const nsw::hw::FEB& feb)
{
  auto isObject = nsw::mon::is::RocStatus{};
  isObject.captureStatusVmm = std::vector{feb.getRoc().readRegister(32),
                                          feb.getRoc().readRegister(33),
                                          feb.getRoc().readRegister(34),
                                          feb.getRoc().readRegister(35),
                                          feb.getRoc().readRegister(36),
                                          feb.getRoc().readRegister(37),
                                          feb.getRoc().readRegister(38),
                                          feb.getRoc().readRegister(39)};
  isObject.parityCounterVmm = std::vector{feb.getRoc().readRegister(45),
                                          feb.getRoc().readRegister(46),
                                          feb.getRoc().readRegister(47),
                                          feb.getRoc().readRegister(48),
                                          feb.getRoc().readRegister(49),
                                          feb.getRoc().readRegister(50),
                                          feb.getRoc().readRegister(51),
                                          feb.getRoc().readRegister(52)};
  isObject.statusSroc = std::vector{feb.getRoc().readRegister(40),
                                    feb.getRoc().readRegister(41),
                                    feb.getRoc().readRegister(42),
                                    feb.getRoc().readRegister(43)};
  isObject.seu = feb.getRoc().readRegister(44);
  isObject.seuCounter = feb.getRoc().readRegister(53);
  isObject.ePllVmm0_ePllInstantLock = static_cast<bool>(feb.getRoc().readValue("ePllVmm0.ePllInstantLock"));
  isObject.ePllVmm1_ePllInstantLock = static_cast<bool>(feb.getRoc().readValue("ePllVmm1.ePllInstantLock"));
  isObject.ePllTdc_ePllInstantLock = static_cast<bool>(feb.getRoc().readValue("ePllTdc.ePllInstantLock"));
  isObject.ePllCore_ePllInstantLock = static_cast<bool>(feb.getRoc().readValue("ePllCore.ePllInstantLock"));
  return isObject;
}
