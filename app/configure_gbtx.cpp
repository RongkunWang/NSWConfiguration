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

std::map<std::uint8_t, std::string> differences(const std::vector<std::uint8_t>& readvals,
                                                const std::vector<std::uint8_t>& checkvals)
{
  std::map<std::uint8_t, std::string> diffs{};

  for (std::size_t reg{0}; reg < checkvals.size(); ++reg) {
    if (checkvals.at(reg) != readvals.at(reg)) {
      diffs[static_cast<std::uint8_t>(reg)] = fmt::format("Register {:03d} value changed to {:02x} from {:02x}", reg, readvals.at(reg), checkvals.at(reg));
    }
  }
  return diffs;
}

void monitor(std::list<nsw::hw::L1DDC>& l1ddcs, int wait_time)
{
  std::map<std::string, std::map<std::uint8_t, std::vector<std::uint8_t>>> configs{};

  for (const auto& l1ddc : l1ddcs) {
    configs[l1ddc.getConfig().getNodeName()] = std::map<std::uint8_t, std::vector<std::uint8_t>>{};
    auto& config = configs[l1ddc.getConfig().getNodeName()];
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
      fmt::print("Reading configuration of L1DDC {}\n", l1ddc.getConfig().getNodeName());
      auto& checkvals = configs[l1ddc.getConfig().getNodeName()];
      if (l1ddc.getConfig().getConfigureGBTx(0)) {
        const auto config = l1ddc.getGbtx0().readConfiguration();
        fmt::print("{}::GBTx0: {}\n", l1ddc.getConfig().getNodeName(), fmt::join(differences(config, checkvals[0]), "\n"));
        checkvals[0] = config;
      }
      if (l1ddc.getConfig().getConfigureGBTx(1)) {
        const auto config = l1ddc.getGbtx1().readConfiguration();
        fmt::print("{}::GBTx1: {}\n", l1ddc.getConfig().getNodeName(), fmt::join(differences(config, checkvals[1]), "\n"));
        checkvals[1] = config;
      }
      if (l1ddc.getConfig().getConfigureGBTx(2)) {
        const auto config = l1ddc.getGbtx2().readConfiguration();
        fmt::print("{}::GBTx2: {}\n", l1ddc.getConfig().getNodeName(), fmt::join(differences(config, checkvals[2]), "\n"));
        checkvals[2] = config;
      }
      fmt::print("\n");
    }
    nsw::snooze(std::chrono::milliseconds(wait_time));
  }
}

int main(int argc, char* argv[])
{
  std::string configFile{};
  std::string name{};
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
    monitor(l1ddcs, monsleep);
  }
  return 0;
}
