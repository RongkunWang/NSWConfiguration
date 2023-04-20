#include "NSWConfiguration/monitoring/PadTriggerRegisters.h"
#include <ers/ers.h>

nsw::mon::PadTriggerRegisters::PadTriggerRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager},
  m_helper{NUM_CONCURRENT}
{
}

void nsw::mon::PadTriggerRegisters::monitor(ISInfoDictionary* isDict,
                                            const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.getPadTriggers(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::PadTriggerRegisters::getData);
}

nsw::mon::is::PadTriggerRegisters
nsw::mon::PadTriggerRegisters::getData(const nsw::hw::PadTrigger& dev)
{
  auto is = nsw::mon::is::PadTriggerRegisters{};
  is.conf_bcid_offset   = dev.readSubRegister("000_control_reg", "conf_bcid_offset");
  is.conf_ro_bc_offset  = dev.readSubRegister("000_control_reg", "conf_ro_bc_offset");
  is.trigger_rate       = dev.readSubRegister("001_status_reg_READONLY", "trigger_rate");
  is.xadc_temp          = dev.readFPGATemperature();
  is.pfeb_bcids         = dev.readPFEBBCIDs();
  is.pfeb_status        = dev.readPFEBBcidErrorReadout();
  is.pfeb_bcid_error    = dev.readPFEBBcidErrorTrigger();
  is.pad_bcid_error     = dev.readSubRegister("012_pad_bcid_error_READONLY", "pad_bcid_error");
  is.pad_bcid_error_dif = dev.readSubRegister("012_pad_bcid_error_READONLY", "pad_bcid_error_dif");
  is.pt_2_tp_lat        = dev.readSubRegister("013_pt_2_tp_lat_READONLY", "pt_2_tp_lat");
  is.tp_bcid_error      = dev.readSubRegister("014_tp_bcid_error_READONLY", "tp_bcid_error");
  is.tp_bcid_error_dif  = dev.readSubRegister("014_tp_bcid_error_READONLY", "tp_bcid_error_dif");
  is.ttc_bcr_ocr_rate   = dev.readSubRegister("015_ttc_mon_0_READONLY", "ttc_bcr_ocr_rate");
  is.gt_rx_lol          = dev.readGtRxLol();
  return is;
}
