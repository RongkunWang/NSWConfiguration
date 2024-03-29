#include "NSWConfiguration/hw/FEB.h"

nsw::hw::FEB::FEB(OpcManager& manager, const nsw::FEBConfig& config) :
  ScaAddressBase(config.getAddress()),
  m_roc(manager, config),
  m_vmms([&config, &manager]() {
    std::vector<VMM> vmms;
    vmms.reserve(config.getVmms().size());
    for (std::size_t iVmm = 0; iVmm < config.getVmms().size(); iVmm++) {
      vmms.emplace_back(manager, config, iVmm);
    }
    return vmms;
  }()),
  m_tdss([&config, &manager]() {
    std::vector<TDS> tdss;
    tdss.reserve(config.getTdss().size());
    for (std::size_t iTds = 0; iTds < config.getTdss().size(); iTds++) {
      tdss.emplace_back(manager, config, iTds);
    }
    return tdss;
  }()),
  m_firstVmm(config.getFirstVmmIndex()),
  m_firstTds(config.getFirstTdsIndex())
{}

void nsw::hw::FEB::writeConfiguration(const bool resetVmm,
                                      const bool resetTds,
                                      const bool disableVmmCaptureInputs) const
{
  m_roc.writeConfiguration();
  if (disableVmmCaptureInputs) {
    m_roc.disableVmmCaptureInputs();
  }
  for (const auto& vmm : m_vmms) {
    vmm.writeConfiguration(resetVmm);
  }
  for (const auto& tds : m_tdss) {
    tds.writeConfiguration(resetTds);
  }
}