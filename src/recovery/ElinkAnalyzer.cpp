#include "NSWConfiguration/recovery/ElinkAnalyzer.h"

#include <algorithm>
#include <iterator>

#include <fmt/core.h>
#include <fmt/ranges.h>

nsw::ElinkAnalyzer::ElinkAnalyzer(const double thresholdPercentage,
                                  const std::uint64_t thresholdMinimum) :
  m_impl(thresholdPercentage, thresholdMinimum)
{}

std::unordered_set<std::uint64_t> nsw::ElinkAnalyzer::analyze(
  const std::vector<swrod::LinkStatistics>& data,
  const std::vector<std::uint64_t>& disabled)
{
  // Filter disabled links
  std::vector<swrod::LinkStatistics> filtered{};
  std::ranges::copy_if(data, std::back_inserter(filtered), [disabled](auto val) {
    return std::ranges::find(disabled, val.FID) != std::cend(disabled);
  });
  if (m_impl.skip(data)) {
    m_impl.setPrevious(data);
    return {};
  }

  std::unordered_set<std::uint64_t> result{};
  std::ranges::copy(internal::ElinkAnalyzerImpl::getFids(m_impl.analyzeNumReceived(data)),
                    std::inserter(result, std::end(result)));
  std::ranges::copy(internal::ElinkAnalyzerImpl::getFids(m_impl.analyzeNumBad(data)),
                    std::inserter(result, std::end(result)));

  m_impl.setPrevious(data);

  return result;
}

nsw::internal::ElinkAnalyzerImpl::ElinkAnalyzerImpl(const double thresholdPercentage,
                                                    const std::uint64_t thresholdMinimum) :
  m_thresholdPercentage(thresholdPercentage), m_thresholdMinimum(thresholdMinimum)
{}

std::vector<swrod::LinkStatistics> nsw::internal::ElinkAnalyzerImpl::diff(
  const std::vector<swrod::LinkStatistics>& data) const
{
  std::vector<swrod::LinkStatistics> result{};
  std::ranges::transform(data,
                         m_previousData,
                         std::back_inserter(result),
                         [](const swrod::LinkStatistics& current, const swrod::LinkStatistics& previous) {
                           swrod::LinkStatistics diff{};
                           diff.FID = current.FID;
                           diff.receivedPackets =
                             current.receivedPackets - previous.receivedPackets;
                           diff.corruptedPackets =
                             current.corruptedPackets - previous.corruptedPackets;
                           diff.droppedPackets = current.droppedPackets - previous.droppedPackets;
                           diff.missedPackets = current.missedPackets - previous.missedPackets;
                           return diff;
                         });
  ERS_LOG(fmt::format("{}", std::size(data)));
  ERS_LOG(fmt::format("{}", std::size(m_previousData)));
  ERS_LOG(fmt::format("{}", std::size(result)));
  return result;
}

std::vector<swrod::LinkStatistics> nsw::internal::ElinkAnalyzerImpl::analyzeNumReceived(
  const std::vector<swrod::LinkStatistics>& data) const
{
  if (data.empty()) {
    return {};
  }
  auto dataDiff = diff(data);
  const auto n = std::size(dataDiff) / 2;
  std::ranges::nth_element(dataDiff,
                           std::next(std::begin(dataDiff), static_cast<long>(n)),
                           [](swrod::LinkStatistics& first, swrod::LinkStatistics& second) {
                             return first.receivedPackets < second.receivedPackets;
                           });
  ERS_LOG(fmt::format("Size: {} {} {}", std::size(data), std::size(dataDiff), n));
  const auto medianReceived = dataDiff.at(n).receivedPackets;
  if (medianReceived < m_thresholdMinimum) {
    return {};
  }
  std::vector<swrod::LinkStatistics> result{};
  std::ranges::copy_if(dataDiff,
                       std::back_inserter(result),
                       [this, &medianReceived](const swrod::LinkStatistics& element) {
                         return std::fabs(element.receivedPackets - medianReceived) /
                                  static_cast<double>(medianReceived) >
                                m_thresholdPercentage;
                       });
  return result;
}

std::vector<swrod::LinkStatistics> nsw::internal::ElinkAnalyzerImpl::analyzeNumBad(
  const std::vector<swrod::LinkStatistics>& data) const
{
  const auto dataDiff = diff(data);
  std::vector<swrod::LinkStatistics> result{};
  std::ranges::copy_if(
    dataDiff, std::back_inserter(result), [this](const swrod::LinkStatistics& element) {
      const auto totalBad =
        element.corruptedPackets + element.droppedPackets + element.missedPackets;  // overflow?
      return totalBad > m_thresholdMinimum and
             static_cast<double>(totalBad) / static_cast<double>(element.receivedPackets) >
               m_thresholdPercentage;
    });
  return result;
}

std::vector<long> nsw::internal::ElinkAnalyzerImpl::getFids(
  const std::vector<swrod::LinkStatistics>& data)
{
  std::vector<long> result{};
  std::ranges::transform(data, std::back_inserter(result), [](const swrod::LinkStatistics& element) {
    return element.FID;
  });
  return result;
}
