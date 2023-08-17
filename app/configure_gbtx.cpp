// Sample program to read configuration from json and send to any front end module

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>
#include <set>

#include <boost/program_options.hpp>

#include <fmt/ranges.h>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/hw/L1DDC.h"

namespace po = boost::program_options;
using namespace std::chrono_literals;

static constexpr std::size_t NUM_MAX_GBTX{3};

enum class Mode {
  TRAIN,
  READ,
  MONITOR,
  WRITE,
};

void validate(boost::any& v, const std::vector<std::string>& values, Mode* /*mode*/, int /*unused*/)
{
  using namespace boost::program_options;

  // Make sure no previous assignment to 'v' was made.
  validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  std::string const& s = validators::get_single_string(values);

  if (s == "train") {
    v = boost::any(Mode::TRAIN);
  } else if (s == "read") {
    v = boost::any(Mode::READ);
  } else if (s == "monitor") {
    v = boost::any(Mode::MONITOR);
  } else if (s == "write") {
    v = boost::any(Mode::WRITE);
  } else {
    throw validation_error(validation_error::invalid_option_value);
  }
}

void write(std::list<nsw::hw::L1DDC>& l1ddcs, const bool parallel)
{
  if (parallel) {
    constexpr static int NUM_THREADS = 14;
    IPCThreadPool threadPool{NUM_THREADS};

    fmt::print("Configuring {} L1DDCs in parallel\n", std::size(l1ddcs));
    for (auto& l1ddc : l1ddcs) {
      threadPool.addJob([&l1ddc]() { l1ddc.writeConfiguration(); });
    }
    threadPool.waitForCompletion();
  } else {
    auto counter{0};
    for (const auto& l1ddc : l1ddcs) {
      fmt::print("Starting L1DDC configuration {}/{}\n", ++counter, std::size(l1ddcs));
      l1ddc.writeConfiguration();
      fmt::print("Done with configure_board for {}\n", l1ddc.getConfig().getNodeName());
    }
  }
  fmt::print("Configuration done\n");
}

void train(std::list<nsw::hw::L1DDC>& l1ddcs, const bool parallel)
{
  for (auto& l1ddc : l1ddcs) {
    l1ddc.getConfig().trainGbtxsOn();
  }
  write(l1ddcs, parallel);
  nsw::snooze(std::chrono::microseconds{
    std::ranges::max(l1ddcs | std::views::transform([](const auto& l1ddc) {
                       return l1ddc.getConfig().trainGBTxPhaseWaitTime();
                     }))});
  for (auto& l1ddc : l1ddcs) {
    l1ddc.getConfig().trainGbtxsOff();
  }
  write(l1ddcs, parallel);
}

void read(std::list<nsw::hw::L1DDC>& l1ddcs)
{
  for (const auto& l1ddc : l1ddcs) {
    fmt::print("Reading configuration of L1DDC {}\n", l1ddc.getConfig().getNodeName());
    if (l1ddc.getConfig().getConfigureGBTx(0)) {
      fmt::print("GBTx0:\n");
      fmt::print("{}", nsw::getPrintableGbtxConfig(l1ddc.getGbtx0().readConfiguration()));
    }
    if (l1ddc.getConfig().getConfigureGBTx(1)) {
      fmt::print("GBTx1:\n");
      fmt::print("{}", nsw::getPrintableGbtxConfig(l1ddc.getGbtx1().readConfiguration()));
    }
    if (l1ddc.getConfig().getConfigureGBTx(2)) {
      fmt::print("GBTx2:\n");
      fmt::print("{}", nsw::getPrintableGbtxConfig(l1ddc.getGbtx2().readConfiguration()));
    }
    fmt::print("\n");
  }
}

std::vector<std::string> differences(const std::vector<std::uint8_t>& readvals,
                                     const std::vector<std::uint8_t>& checkvals,
                                     const std::vector<std::size_t>& monregs,
                                     const std::string& boardname)
{
  std::vector<std::string> diffs{};

  const std::set<std::size_t> monset(std::cbegin(monregs), std::cend(monregs));

  for (std::size_t reg{0}; reg < checkvals.size(); ++reg) {
    if (not monset.contains(reg)) {
      continue;
    }
    if (checkvals.at(reg) != readvals.at(reg)) {
      diffs.emplace_back(fmt::format("{} register {:03d} old={:02x} -> new={:02x}", boardname, reg, checkvals.at(reg), readvals.at(reg)));
    }
  }
  return diffs;
}

void monitor(std::list<nsw::hw::L1DDC>& l1ddcs, int wait_time, const std::vector<std::size_t>& monregs)
{
  std::map<std::string, std::map<std::uint8_t, std::vector<std::uint8_t>>> configs{};

  for (const auto& l1ddc : l1ddcs) {
    const auto l1ddc_name = l1ddc.getConfig().getNodeName();
    configs[l1ddc_name] = std::map<std::uint8_t, std::vector<std::uint8_t>>{};
    auto& config = configs[l1ddc_name];
    if (l1ddc.getConfig().getConfigureGBTx(0)) {
      config[0] = l1ddc.getGbtx0().readConfiguration();
    }
    if (l1ddc.getConfig().getConfigureGBTx(1)) {
      config[1] = l1ddc.getGbtx1().readConfiguration();
    }
    if (l1ddc.getConfig().getConfigureGBTx(2)) {
      config[2] = l1ddc.getGbtx2().readConfiguration();
    }
  }

  while (true) {
    for (const auto& l1ddc : l1ddcs) {
      const auto l1ddc_name = l1ddc.getConfig().getNodeName();
      auto& checkvals = configs[l1ddc_name];
      for (std::size_t idx{0}; idx < NUM_MAX_GBTX; ++idx){
        if (l1ddc.getConfig().getConfigureGBTx(idx)) {

          const auto config = [&l1ddc](const auto gbtIdx) {
            switch(gbtIdx) {
            case 0:
              return l1ddc.getGbtx0().readConfiguration();
            case 1:
              return l1ddc.getGbtx1().readConfiguration();
            case 2:
              return l1ddc.getGbtx2().readConfiguration();
            default:
              throw std::runtime_error(fmt::format("Invalid GBTx specified: {}", gbtIdx));
            }
          }(idx);

          const auto diffs = differences(config, checkvals[idx], monregs, l1ddc_name);

          if (not diffs.empty()) {
            fmt::print("{}::GBTx{}:\n{}\n", l1ddc_name, idx, fmt::join(diffs, "\n"));
            checkvals[idx] = config;
          }
        }
      }
    }
    nsw::snooze(std::chrono::milliseconds(wait_time));
  }
}

int main(int argc, char* argv[])
{
  std::string configFile{};
  std::string name{};
  std::vector<std::size_t> monregs{};
  int monsleep{};
  bool parallel{false};
  bool simulation{false};
  int trainGBTxPhaseWaitTime{1};
  bool dryRun{false};
  bool noRim{false};
  Mode mode{};

  po::options_description desc("This program interfaces L1DDCs");
  desc.add_options()
    ("help,h", "produce help message")
    ("mode",po::value<Mode>(&mode)->required(), "Mode (read, write, train)")
    ("config,c", po::value<std::string>(&configFile)->required(), "Configuration file path")
    ("name,n",po::value<std::string>(&name)->default_value(""),"Name of the L1DDC in the config file (all if empty)")
    ("monsleep",po::value<int>(&monsleep)->default_value(1000),"Time to wait between monitoring calls in ms (default 1000)")
    ("monregs",po::value<std::vector<std::size_t>>(&monregs)->multitoken(),"List of registers to monitor")
    ("parallel,p", po::bool_switch(&parallel)->default_value(false), "Configure boards in parallel")
    ("no-rim",po::bool_switch(&noRim)->default_value(false),"Do not configure RimL1DDCs (when no name provided)")
    ("simulation,s",po::bool_switch(&simulation)->default_value(false),"Do not run any configuration");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help") != 0U) {
    std::cout << desc << "\n";
    return 0;
  }

  po::notify(vm);

  std::vector<nsw::L1DDCConfig> l1ddcConfigs;

  const auto boards = [&noRim]() -> std::vector<std::string> {
    if (noRim) {
      return {"L1DDC"};
    }
    return {"L1DDC", "RimL1DDC"};
  }();

  for (const auto& board_type : boards) {
    const auto cfg = nsw::ConfigReader::makeObjects<nsw::L1DDCConfig>(
      fmt::format("json://{}", configFile), board_type, name);
    std::ranges::copy(cfg, std::back_inserter(l1ddcConfigs));
  }

  fmt::print("Configuring the following L1DDCs: {}",
             l1ddcConfigs |
               std::views::transform([](const auto& config) { return config.getNodeName(); }));

  if (simulation) {
    fmt::print(
      "Running in simulation mode. This is the point where the configuration would be sent.\n");
    return 0;
  }

  nsw::OpcManager manager{};
  std::list<nsw::hw::L1DDC> l1ddcs{};
  for (const auto& l1ddc : l1ddcConfigs) {
    l1ddcs.emplace_back(manager,l1ddc);
  }

  switch (mode) {
  case Mode::READ:
    read(l1ddcs);
    break;
  case Mode::WRITE:
    write(l1ddcs, parallel);
    break;
  case Mode::TRAIN:
    train(l1ddcs, parallel);
    break;
  default:
    break;
  }

  if (mode == Mode::MONITOR) {
    monitor(l1ddcs, monsleep, monregs);
  }
  return 0;
}
