#include "NSWConfiguration/monitoring/MmtpOutRunStatusRegisters.h"

#include "NSWConfiguration/Constants.h"

nsw::mon::MmtpOutRunStatusRegisters::MmtpOutRunStatusRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getMMTps()}, m_helper{NUM_CONCURRENT}
{}

void nsw::mon::MmtpOutRunStatusRegisters::monitor(ISInfoDictionary* isDict,
                                                  const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.get(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::MmtpOutRunStatusRegisters::getData);
}

nsw::mon::is::MmtpOutRunStatusRegisters nsw::mon::MmtpOutRunStatusRegisters::getData(
  const nsw::hw::MMTP& tp)
{
  auto is = nsw::mon::is::MmtpOutRunStatusRegisters{};
  // TP Horx env being monitored
  is.txMicropodTemp.resize(3, -1);
  is.rxMicropodTemp.resize(3, -1);
  is.txLoss.resize(36, 0);
  is.rxLoss.resize(36, 0);
  is.txFibersPower.resize(36, 0);
  is.rxArtFibersPower.resize(36, 0);
  for (std::uint8_t microPod = 1; microPod < 4; ++microPod) {
    uint8_t iMTP = microPod;
    if (microPod == 2) {
      iMTP = 3;
    } else if (microPod == 3) {
      iMTP = 2;
    }

    tp.setHorxEnvMonAddr(true, microPod, true, false, 0);
    is.txMicropodTemp[iMTP - 1] =
      static_cast<float>(readHorxEnvMonData(tp)) * nsw::mmtp::CONVERTER_HORX_TEMP_C;
    tp.setHorxEnvMonAddr(false, microPod, true, false, 0);
    is.rxMicropodTemp[iMTP - 1] =
      static_cast<float>(readHorxEnvMonData(tp)) * nsw::mmtp::CONVERTER_HORX_TEMP_C;

    tp.setHorxEnvMonAddr(true, microPod, false, true, 0);
    const auto valLossTx = readHorxEnvMonData(tp);
    tp.setHorxEnvMonAddr(false, microPod, false, true, 0);
    const auto valLossRx = readHorxEnvMonData(tp);

    for (std::uint8_t fiber = 0; fiber < nsw::mmtp::NUM_FIBERS_PER_MICROPOD; ++fiber) {
      auto iFib = (iMTP - 1) * nsw::mmtp::NUM_FIBERS_PER_MICROPOD + fiber;
      is.txLoss[iFib] = static_cast<bool>((valLossTx >> fiber) & 1U);
      is.rxLoss[iFib] = static_cast<bool>((valLossRx >> fiber) & 1U);
      tp.setHorxEnvMonAddr(true, microPod, false, false, fiber);
      is.txFibersPower[iFib] =
        static_cast<float>(readHorxEnvMonData(tp)) * nsw::mmtp::CONVERTER_HORX_FIBER_POWER_UW;

      tp.setHorxEnvMonAddr(false, microPod, false, false, fiber);
      is.rxArtFibersPower[iFib] =
        static_cast<float>(readHorxEnvMonData(tp)) * nsw::mmtp::CONVERTER_HORX_FIBER_POWER_UW;
    }
  }

  return is;
}

std::uint32_t nsw::mon::MmtpOutRunStatusRegisters::readHorxEnvMonData(const nsw::hw::MMTP& tp)
{
  return tp.readRegister(nsw::mmtp::REG_HORX_ENV_MON_DATA);
}