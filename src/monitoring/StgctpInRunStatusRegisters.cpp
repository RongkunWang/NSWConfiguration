#include "NSWConfiguration/monitoring/StgctpInRunStatusRegisters.h"

nsw::mon::StgctpInRunStatusRegisters::StgctpInRunStatusRegisters(
  const nsw::hw::DeviceManager& deviceManager) :
  m_devices{deviceManager.getSTGCTps()}, m_helper{NUM_CONCURRENT}
{}

void nsw::mon::StgctpInRunStatusRegisters::monitor(ISInfoDictionary* isDict,
                                                 const std::string_view serverName)
{
  m_helper.monitorAndPublish(m_devices.get(),
                             isDict,
                             m_threadPool,
                             serverName,
                             NAME,
                             nsw::mon::StgctpInRunStatusRegisters::getData);
}

nsw::mon::is::StgctpInRunStatusRegisters nsw::mon::StgctpInRunStatusRegisters::getData(
  const nsw::hw::STGCTP& tp)
{
  auto is = nsw::mon::is::StgctpInRunStatusRegisters{};

  // is.ErrBCIDmatch = (tp.readRegister(nsw::stgctp::REG_ERR_BCID_MATCH,    nsw::stgctp::MASK_ERR_BCID_MATCH) != 0U);
  // is.SLLatencyComp = tp.readRegister(nsw::stgctp::REG_SL_LATENCY_COMP,   nsw::stgctp::MASK_SL_LATENCY_COMP);
  // is.bcrRate =       tp.readRegister(nsw::stgctp::REG_BCR_RATE,          nsw::stgctp::MASK_BCR_RATE);
  // is.PadBCidSyncOk = (tp.readRegister(nsw::stgctp::REG_PAD_BXID_SYNC_OK, nsw::stgctp::MASK_PAD_BXID_SYNC_OK) != 0U);
  // is.PadHitRate = tp.readRegister(nsw::stgctp::REG_PAD_RATE,             nsw::stgctp::MASK_PAD_RATE);
  // is.MMDisable = (tp.readRegister(nsw::stgctp::REG_STGC_MM_DISABLE,      nsw::stgctp::MASK_STGC_MM_DISABLE) != 0U);
  // is.ToSLHitRate = tp.readRegister(nsw::stgctp::REG_TO_SL_RATE,          nsw::stgctp::MASK_TO_SL_RATE);
  // is.SectorID = tp.readRegister(nsw::stgctp::REG_SECTOR,                 nsw::stgctp::MASK_SECTOR);
  // is.MMBCidSyncOk = (tp.readRegister(nsw::stgctp::REG_MM_BXID_SYNC_OK,   nsw::stgctp::MASK_MM_BXID_SYNC_OK) != 0U);
  // is.PadIdleStatus = (tp.readRegister(nsw::stgctp::REG_PAD_IDLE_STATUS,  nsw::stgctp::MASK_PAD_IDLE_STATUS) != 0U);
  // is.MMIdleStatus = (tp.readRegister(nsw::stgctp::REG_MM_IDLE_STATUS,    nsw::stgctp::MASK_MM_IDLE_STATUS) != 0U);
  // is.PadArrivalBC = tp.readRegister(nsw::stgctp::REG_PAD_ARRIVAL_BC,     nsw::stgctp::MASK_PAD_ARRIVAL_BC);
  // is.MMArrivalBC = tp.readRegister(nsw::stgctp::REG_MM_ARRIVAL_BC,       nsw::stgctp::MASK_MM_ARRIVAL_BC);
  // is.IgnorePads = (tp.readRegister(nsw::stgctp::REG_IGNORE_PADS,         nsw::stgctp::MASK_IGNORE_PADS) != 0U);
  // is.IgnoreMM = (tp.readRegister(nsw::stgctp::REG_IGNORE_MM,             nsw::stgctp::MASK_IGNORE_MM) != 0U);
  // is.DisableNSWMon = (tp.readRegister(nsw::stgctp::REG_DISABLE_NSWMON,   nsw::stgctp::MASK_DISABLE_NSWMON) != 0U);
  // is.L1AOpeningOffset = tp.readRegister(nsw::stgctp::REG_L1A_OPENING_OFFSET, nsw::stgctp::MASK_L1A_OPENING_OFFSET);
  // is.L1ARequestOffset = tp.readRegister(nsw::stgctp::REG_L1A_REQUEST_OFFSET, nsw::stgctp::MASK_L1A_REQUEST_OFFSET);
  // is.L1AClosingOffset = tp.readRegister(nsw::stgctp::REG_L1A_CLOSING_OFFSET, nsw::stgctp::MASK_L1A_CLOSING_OFFSET);
  // is.L1ATimeoutWindow = tp.readRegister(nsw::stgctp::REG_L1A_TIMEOUT_WINDOW, nsw::stgctp::MASK_L1A_TIMEOUT_WINDOW);
  // is.L1APadEnabled = (tp.readRegister(nsw::stgctp::REG_L1A_PAD_EN,                nsw::stgctp::MASK_L1A_PAD_EN) != 0U);
  // is.L1AMergeEnabled = (tp.readRegister(nsw::stgctp::REG_L1A_MERGE_EN,            nsw::stgctp::MASK_L1A_MERGE_EN) != 0U);
  // is.StickyErrBCIDmatch = (tp.readRegister(nsw::stgctp::REG_STICKY_ERR_BCID_MATCH,      nsw::stgctp::MASK_STICKY_ERR_BCID_MATCH) != 0U);
  // is.Busy = (tp.readRegister(nsw::stgctp::REG_BUSY,                          nsw::stgctp::MASK_BUSY) != 0U);
  // is.MonDisabled = (tp.readRegister(nsw::stgctp::REG_MON_DISABLE,             nsw::stgctp::MASK_MON_DISABLE) != 0U);
  // is.NSWMonLimit = tp.readRegister(nsw::stgctp::REG_NSW_MON_LIMIT,           nsw::stgctp::MASK_NSW_MON_LIMIT);
  // is.MonLimit = tp.readRegister(nsw::stgctp::REG_MON_LIMIT,                  nsw::stgctp::MASK_MON_LIMIT);
  // is.MM_NSWMonEnabled = (tp.readRegister(nsw::stgctp::REG_MM_NSW_MON_EN,     nsw::stgctp::MASK_MM_NSW_MON_EN) != 0U);
  // is.SmallSector = (tp.readRegister(nsw::stgctp::REG_SMALL_SECTOR,           nsw::stgctp::MASK_SMALL_SECTOR) != 0U);
  // is.NoStretch   = (tp.readRegister(nsw::stgctp::REG_NO_STRETCH,             nsw::stgctp::MASK_NO_STRETCH)   != 0U);
  // is.SyncFiFoEmpty = (tp.readRegister(nsw::stgctp::REG_SYNC_FIFO_EMPTY,      nsw::stgctp::MASK_SYNC_FIFO_EMPTY) != 0U);

  return is;
}
