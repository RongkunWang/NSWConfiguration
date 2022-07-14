#ifndef NSWCONFIGURATION_RECOVERY_ELINKANALYZER_H
#define NSWCONFIGURATION_RECOVERY_ELINKANALYZER_H

#include <unordered_set>

#include "swrod/LinkStatistics.h"

namespace nsw {
  namespace internal {
    class ElinkAnalyzerImpl
    {
    public:
      ElinkAnalyzerImpl(double thresholdPercentage, std::uint64_t thresholdMinimum);
      [[nodiscard]] std::vector<swrod::LinkStatistics> diff(
        const std::vector<swrod::LinkStatistics>& data) const;
      [[nodiscard]] std::vector<swrod::LinkStatistics> analyzeNumReceived(
        const std::vector<swrod::LinkStatistics>& data) const;
      [[nodiscard]] std::vector<swrod::LinkStatistics> analyzeNumBad(
        const std::vector<swrod::LinkStatistics>& data) const;
      [[nodiscard]] static std::vector<long> getFids(const std::vector<swrod::LinkStatistics>& data);
      void setPrevious(const std::vector<swrod::LinkStatistics>& data) { m_previousData = data; }
      [[nodiscard]] bool skip(const std::vector<swrod::LinkStatistics>& data) const
      {
        return std::size(m_previousData) != std::size(data);
      }

    private:
      double m_thresholdPercentage{};
      std::uint64_t m_thresholdMinimum{};
      std::vector<swrod::LinkStatistics> m_previousData{};
    };
  }  // namespace internal

  class ElinkAnalyzer
  {
  public:
    ElinkAnalyzer(double thresholdPercentage, std::uint64_t thresholdMinimum);
    std::unordered_set<std::uint64_t> analyze(const std::vector<swrod::LinkStatistics>& data);

  private:
    internal::ElinkAnalyzerImpl m_impl;
  };
}  // namespace nsw

#endif
