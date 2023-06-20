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
  is.reachable_sca  = dev.reachable();
  is.reachable_fpga = is.reachable_sca and dev.readFPGADone();
  if (not is.reachable_fpga) {
    return is;
  }
  const static auto statusToVector = [](const uint32_t status) {
    std::vector<std::uint32_t> vec{};
    for (std::size_t i{0}; i < nsw::padtrigger::NUM_PFEBS; ++i) {
      vec.emplace_back((status >> i) & 0b1);
    }
    return vec;
  };
  is.conf_bcid_offset   = dev.readSubRegister("000_control_reg", "conf_bcid_offset");
  is.conf_ro_bc_offset  = dev.readSubRegister("000_control_reg", "conf_ro_bc_offset");
  is.trigger_rate       = dev.readSubRegister("00B_trigger_rate_READONLY", "trigger_rate");
  is.xadc_temp          = dev.readFPGATemperature();
  is.pfeb_delays        = dev.readPFEBDelays();
  is.pfeb_bcids         = dev.readPFEBBCIDs();
  is.pfeb_status        = dev.readPFEBBcidErrorReadout();
  is.pfeb_statuses      = statusToVector(is.pfeb_status);
  is.pfeb_bcid_error    = dev.readPFEBBcidErrorTrigger();
  is.pad_bcid_error     = dev.readSubRegister("012_pad_bcid_error_READONLY", "pad_bcid_error");
  is.pad_bcid_error_dif = dev.readSubRegister("012_pad_bcid_error_READONLY", "pad_bcid_error_dif");
  is.pt_2_tp_lat        = dev.readSubRegister("013_pt_2_tp_lat_READONLY", "pt_2_tp_lat");
  is.tp_bcid_error      = dev.readSubRegister("014_tp_bcid_error_READONLY", "tp_bcid_error");
  is.tp_bcid_error_dif  = dev.readSubRegister("014_tp_bcid_error_READONLY", "tp_bcid_error_dif");
  is.trig_bcid_select   = dev.TrigBcidSelect();
  // is.trig_bcid_select   = dev.readFPGARegister(0xF0);

  is.trig_bcid_rate_m3  = dev.readBcidTriggerRate(is.trig_bcid_select - 3);
  is.trig_bcid_rate_m2  = dev.readBcidTriggerRate(is.trig_bcid_select - 2);
  is.trig_bcid_rate_m1  = dev.readBcidTriggerRate(is.trig_bcid_select - 1);
  is.trig_bcid_rate     = dev.readBcidTriggerRate(is.trig_bcid_select);
  is.trig_bcid_rate_p1  = dev.readBcidTriggerRate(is.trig_bcid_select + 1);
  is.trig_bcid_rate_p2  = dev.readBcidTriggerRate(is.trig_bcid_select + 2);
  is.trig_bcid_rate_p3  = dev.readBcidTriggerRate(is.trig_bcid_select + 3);
  ERS_INFO("is.trig_bcid_select 3sec sleep = " << is.trig_bcid_select);
  // ERS_INFO("is.trig_bcid_select 2sec sleep = " << is.trig_bcid_select);

  // ERS_INFO("Not scanning this time = " << is.trig_bcid_select);
  // is.trig_bcid_rate_m3  = 0;
  // is.trig_bcid_rate_m2  = 0;
  // is.trig_bcid_rate_m1  = 0;
  // is.trig_bcid_rate     = dev.readFPGARegister(0x0C);
  // is.trig_bcid_rate_p1  = 0;
  // is.trig_bcid_rate_p2  = 0;
  // is.trig_bcid_rate_p3  = 0;

  is.ttc_bcr_ocr_rate   = dev.readSubRegister("015_ttc_mon_0_READONLY", "ttc_bcr_ocr_rate");
  is.gt_rx_lol          = dev.readGtRxLol();
  return is;
}
