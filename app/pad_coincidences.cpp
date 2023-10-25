//
// Program to read/write Pad Trigger configuration for testing coincidence logic
//
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ranges>

#include <boost/program_options.hpp>

#include <fmt/core.h>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/PadTrigger.h"
#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/Utility.h"

constexpr bool verbose{false};
constexpr bool simulation{false};

namespace po = boost::program_options;

void maskLayer(const nsw::hw::PadTrigger& dev,
               const std::uint32_t layer,
               const bool maskTo0,
               const bool maskTo1);

std::vector<bool> mask2vector(const std::uint32_t mask);

bool shouldTrigger(std::uint32_t majority_sel, std::vector<bool> layers);

std::uint32_t median(std::vector<std::uint32_t> vec);

std::string channel2letter(const std::uint32_t chan);

void activatePfebChannel(const nsw::hw::PadTrigger& dev,
                         const std::uint32_t pfeb,
                         const std::uint32_t chan);

void maskPfebChannel(const nsw::hw::PadTrigger& dev,
                     const std::uint32_t pfeb,
                     const std::uint32_t chan,
                     const bool maskTo0,
                     const bool maskTo1);

void maskPfebChannel(const nsw::hw::PadTrigger& dev,
                     const std::uint32_t pfeb,
                     const std::uint32_t chan,
                     const bool maskTo0);

int main(int argc, const char *argv[])
{

    // options
    std::string config_filename{""};
    po::options_description desc(std::string("Pad trigger coincidence script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)
         ->required(), "Config file path. REQUIRED")
        ("large", po::bool_switch()->
         default_value(false), "Use a large sector pattern")
        ("small", po::bool_switch()->
         default_value(false), "Use a small sector pattern")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.contains("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    po::notify(vm);
    const auto largesector = vm["large"].as<bool>();
    const auto smallsector = vm["small"].as<bool>();
    if (largesector == smallsector) {
      std::cout << "Please run with either --large or --small" << std::endl;
      return 1;
    }

    // parse json
    nsw::hw::DeviceManager deviceManager;
    nsw::ConfigReader reader{fmt::format("json://{}", config_filename)};
    for (const auto& name: reader.getAllElementNames()) {
      if (nsw::getElementType(name) == "PadTrigger") {
        std::cout << "Found " << name << std::endl;
        deviceManager.add(reader.readConfig(name));
      }
    }

    // constants for coincidences
    constexpr std::uint32_t nbits{2};
    constexpr std::uint32_t nmajoritys{1 << nbits};
    constexpr std::uint32_t zero{0};
    constexpr std::uint32_t nlayers{nsw::NUM_LAYERS_PER_TECH};
    constexpr std::uint32_t ncombos{1 << nlayers};
    constexpr std::uint32_t nreads{3};

    // pattern_t follows the same format as `pad_trigger_pattern_array` in firmware
    // ( (pfeb0, chan0), (pfeb1, chan1), ... )
    using pattern_t = std::array<std::pair<std::size_t, std::size_t>, nlayers>;
    constexpr pattern_t pattern_ls {{ {0,72}, {1,77}, {2,84}, {3,90}, {4,66}, {5,71}, {6,60}, {7,65} }};
    constexpr pattern_t pattern_ss {{ {0,51}, {1,48}, {2,55}, {3,52}, {4,51}, {5,48}, {6,35}, {7,33} }};
    const pattern_t pattern{largesector ? pattern_ls: pattern_ss};

    // test coincidences
    for (const auto& dev: deviceManager.getPadTriggers()) {

      if (not simulation) {
        dev.writeSubRegister("0F0_pfeb_mask_enable", "pfeb_mask_enable", 0x00ffffff);
      }

      for (const auto majority_sel: std::views::iota(zero, nmajoritys)) {

        if (not simulation) {
          dev.writeSubRegister("003_control_reg2", "conf_majority_sel", majority_sel);
        }

        for (const auto layermask: std::views::iota(zero, ncombos)) {

          const auto layersmask = mask2vector(layermask);

          // mask-to-0 everything
          for (const auto layer: std::views::iota(zero, nlayers)) {
            constexpr auto maskTo0{true};
            constexpr auto maskTo1{false};
            maskLayer(dev, layer, maskTo0, maskTo1);
          }

          // mask-to-1 our pattern
          for (const auto layer: std::views::iota(zero, nlayers)) {
            if (layersmask.at(layer)) {
              const auto& [pfeb, chan] = pattern.at(layer);
              activatePfebChannel(dev, pfeb, chan);
            }
          }
          nsw::snooze();
          nsw::snooze();
          std::string msgs{""};
          std::vector<std::uint32_t> rates{};
          for (const auto read: std::views::iota(zero, nreads)) {
            nsw::snooze();
            const auto rate = simulation ? 0 : dev.readSubRegister("00B_trigger_rate_READONLY", "trigger_rate");
            msgs = fmt::format("{} {}", msgs, rate);
            rates.emplace_back(rate);
          }
          const auto trigger_exp = shouldTrigger(majority_sel, layersmask);
          const auto trigger_obs = median(rates) > 40e6;
          const auto success = trigger_exp == trigger_obs ? "success" : "failure";
          std::cout << fmt::format("majority_sel {} layermask {:#010x} exp {} obs {} exp==obs {} (rates {})",
                                   majority_sel, layermask, trigger_exp, trigger_obs, success, msgs) << std::endl;
        }
      }
    }

    return 0;
}

bool shouldTrigger(std::uint32_t majority_sel, std::vector<bool> layers) {
  constexpr auto valid = [](const auto& val){return val;};
  constexpr std::uint32_t nlayersPerQuad{nsw::NUM_LAYERS_PER_TECH / 2};
  const auto nquad0 = std::count_if(layers.cbegin() + 0*nlayersPerQuad,
                                    layers.cbegin() + 1*nlayersPerQuad, valid);
  const auto nquad1 = std::count_if(layers.cbegin() + 1*nlayersPerQuad,
                                    layers.cbegin() + 2*nlayersPerQuad, valid);
  if (majority_sel == 0) {
    return nquad0 >= 3 and nquad1 >= 3;
  } else if (majority_sel == 1) {
    return nquad0 >= 3 or nquad1 >= 3;
  } else if (majority_sel == 2) {
    return (nquad0 >= 3 and nquad1 >= 2) or
           (nquad0 >= 2 and nquad1 >= 3);
  } else if (majority_sel == 3) {
    return (nquad0 >= 3 and nquad1 >= 1) or
           (nquad0 >= 1 and nquad1 >= 3);
  } else {
    throw std::runtime_error("shouldTrigger confused");
  }
  return false;
}

std::uint32_t median(std::vector<std::uint32_t> vec) {
  std::size_t n = vec.size() / 2;
  std::nth_element(vec.begin(), vec.begin() + n, vec.end());
  return vec.at(n);
}

std::vector<bool> mask2vector(const std::uint32_t layermask) {
  constexpr std::uint32_t zero{0};
  constexpr std::uint32_t nlayers{nsw::NUM_LAYERS_PER_TECH};
  std::vector<bool> out{};
  for (const auto layer: std::views::iota(zero, nlayers)) {
    out.emplace_back( static_cast<bool>((layermask >> layer) & 0b1) );
  }
  return out;
}

void maskLayer(const nsw::hw::PadTrigger& dev,
               const std::uint32_t layer,
               const bool maskTo0,
               const bool maskTo1) {
  if ((maskTo0 and maskTo1) or (not maskTo0 and not maskTo1)) {
    throw std::runtime_error("Cant maskTo0 and maskTo1");
  }
  constexpr std::uint32_t zeros{0};
  constexpr std::uint32_t ones{~zeros};
  constexpr std::uint32_t nlayers{nsw::NUM_LAYERS_PER_TECH};
  const auto valMaskTo0{maskTo0 ? ones : zeros};
  const auto valMaskTo1{maskTo1 ? ones : zeros};
  std::vector<std::string> strsMaskTo0{};
  std::vector<std::string> strsMaskTo1{};
  for (const auto radius: std::views::iota(zeros, nsw::NUM_RADII_STGC)) {
    strsMaskTo0.emplace_back(fmt::format("pfeb_{:02}_mask_to_0", radius*nlayers + layer));
    strsMaskTo1.emplace_back(fmt::format("pfeb_{:02}_mask_to_1", radius*nlayers + layer));
  }
  for (const auto& addr: dev.getFpga().getAddresses()) {
    const std::uint32_t def{0xff};
    std::uint32_t val{def};
    if (std::ranges::any_of(strsMaskTo0, [&](const auto& str){return nsw::contains(addr, str);})) {
      val = valMaskTo0;
    } else if (std::ranges::any_of(strsMaskTo1, [&](const auto& str){return nsw::contains(addr, str);})) {
      val = valMaskTo1;
    }
    if (val == def) {
      continue;
    }
    if (verbose) {
      std::cout << fmt::format(" L{} maskTo0={} -> {} = {:#010x}", layer, maskTo0, addr, val) << std::endl;
    }
    if (not simulation) {
      dev.writeAndReadbackFPGARegister(dev.addressFromRegisterName(addr), val);
    }
  }
}

std::string channel2letter(const std::uint32_t chan) {
  constexpr std::uint32_t channelsPerLetter{32};
  if (chan < channelsPerLetter * 1) {
    return "D";
  } else if (chan < channelsPerLetter * 2) {
    return "C";
  } else if (chan < channelsPerLetter * 3) {
    return "B";
  } else {
    return "A";
  }
}

void activatePfebChannel(const nsw::hw::PadTrigger& dev,
                         const std::uint32_t pfeb,
                         const std::uint32_t chan) {
  const bool maskTo0{false};
  const bool maskTo1{true};
  maskPfebChannel(dev, pfeb, chan, maskTo0, maskTo1);
}

void maskPfebChannel(const nsw::hw::PadTrigger& dev,
                     const std::uint32_t pfeb,
                     const std::uint32_t chan,
                     const bool maskTo0,
                     const bool maskTo1) {
  if ((maskTo0 and maskTo1) or (not maskTo0 and not maskTo1)) {
    throw std::runtime_error("Cant maskTo0 and maskTo1");
  }
  constexpr std::uint32_t channelsPerLetter{32};
  const auto letter{channel2letter(chan)};
  const auto value{std::uint32_t{1 << (chan % channelsPerLetter)}};
  const auto antivalue{~value};
  const auto valMaskTo0{maskTo0 ? value : antivalue};
  const auto valMaskTo1{maskTo1 ? value : antivalue};
  const auto strMaskTo0{fmt::format("pfeb_{:02}_mask_to_0_{}", pfeb, letter)};
  const auto strMaskTo1{fmt::format("pfeb_{:02}_mask_to_1_{}", pfeb, letter)};
  if (valMaskTo0 == 0 or valMaskTo1 == 0) {
    throw std::runtime_error("Value should never be zero");
  }
  for (const auto& addr: dev.getFpga().getAddresses()) {
    constexpr std::uint32_t def{0};
    std::uint32_t val{def};
    if (nsw::contains(addr, strMaskTo0)) {
      val = valMaskTo0;
    } else if (nsw::contains(addr, strMaskTo1)) {
      val = valMaskTo1;
    }
    if (val == def) {
      continue;
    }
    if (verbose) {
      std::cout << fmt::format(" PFEB{:02} chan{:03} -> {} = {:#010x}", pfeb, chan, addr, val) << std::endl;
    }
    if (not simulation) {
      dev.writeAndReadbackFPGARegister(dev.addressFromRegisterName(addr), val);
    }
  }
}
