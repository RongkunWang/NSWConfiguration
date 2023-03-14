
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/hw/OpcManager.h"
#include "NSWConfiguration/hw/MMTP.h"
#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/hw/SCAInterface.h"

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fmt/core.h>

#define R__HAS_STD_SPAN
#include "TFile.h"
#include "TTree.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

std::atomic<bool> stop = false;

struct Input {
  std::string opc_slave;
  bool onlyTemp;
  int microPod;
  bool humanReadable;
  bool isSTGC;
  bool debug;
  int bus;

  // print table size
  int nsector;
  int cellWidth;

  // monitoring period
  int Niteration;
  int interval;

  bool printTable = false;
};

struct TreeData {
  std::string opc_ip;
  std::string slaveAddr;
  std::vector<std::string> envName;
  std::vector<float>       envData;
};

struct PrintData {
  //           column1,  column2,   column3..
  //  row1     val(1,1)  val(1, 2)
  //  row2
  //  row3
  std::vector<std::string>        row;
  std::vector<std::string>        column;
  std::vector<std::vector<float>> value;
};

template <class T> 
void printLine(const std::string rowName,
    std::vector<T> const v,
    const int cellWidth,
    const std::string delimiter) {
  std::cout << std::setw(cellWidth) << std::left << rowName << delimiter;
  for (const auto l : v) {
    std::cout << std::setw(cellWidth) << std::left << l << delimiter;
  }
  std::cout << std::endl;
}

void printTable(const PrintData ptab,
    const bool transpose,
    const int fold,
    const int cellWidth,
    const std::string delimiter) {
  if (transpose) {
    size_t iglob = 0;
    while(iglob < ptab.row.size()) {
      std::vector<std::string> tmp_row;
      auto iupper = std::min(ptab.row.size(), iglob + fold);

      for (size_t i = iglob; i < iupper; ++i) { 
        tmp_row.push_back(ptab.row[i]);
      }

      printLine("", tmp_row, cellWidth, delimiter);
      for (size_t j = 0; j < ptab.column.size(); ++j) {
        std::vector<float> v;
        for (size_t i = iglob; i < iupper; ++i) { 
          v.push_back(ptab.value[i][j]);
        }
        printLine(ptab.column[j], v, cellWidth, delimiter);
      }
      iglob = iupper;
      for (int i = 0; i < (fold + 1) * (cellWidth + 1); ++i) {
        std::cout << "-";
      }
      std::cout << std::endl;
    }
  } else {
    // folding not implemented yet
    printLine("", ptab.column, cellWidth, delimiter);
    for (size_t i = 0; i < ptab.row.size(); ++i) {
      printLine(ptab.row[i], ptab.value[i], cellWidth, delimiter);
    }
  }
}

void loop(const Input& input, TreeData& data, TTree& tree);

int main(int argc, const char *argv[]) {
  std::string description = "This program will scan over all Horx Environment Monitoring registers.";

  std::string fileName;

  Input input;
  TreeData data;

  po::options_description desc(description);
  desc.add_options()
    ("help,h", "produce help message")
    ("file,f", po::value<std::string>(&fileName)->default_value("out.root"),
      "output root file name")
    ("iteration,i", po::value<int>(&input.Niteration)->default_value(1),
      "number of times to run the scan(0 for running indefinitely")
    ("interval,I", po::value<int>(&input.interval)->default_value(1e3),
      "interval of scan between iterations in milisecond ")
    ("nsector,n", po::value<int>(&input.nsector)->default_value(8),
      "number of sector per row in print table")
    ("cellWidth,r", po::value<int>(&input.cellWidth)->default_value(18),
      "width of a cell in print table")
    ("debug,d",   po::bool_switch(&input.debug)->default_value(false),
      "Print debug info (Default: False)")
    ("print,p",   po::bool_switch(&input.printTable)->default_value(false),
      "Print table anyway (Default: False)")
    ("humanReadable,H",   po::bool_switch(&input.humanReadable)->default_value(false),
      "Human readable register (Default: False)")
    ("temp,t",   po::bool_switch(&input.onlyTemp)->default_value(false),
      "write only temperature registers(Default: False)")
    ("microPod,m", po::value<int>(&input.microPod)->default_value(0),
      "write only certain micropod registers(Default: False)")
    ("sTGC,s",   po::bool_switch(&input.isSTGC)->default_value(false),
      "do sTGC registers NOT SUPPORTED YET(Default: False)")
    // server related
    ("opc_slave,o", po::value<std::string>(&input.opc_slave)->default_value("pcatlnswfelix01.cern.ch:48020,NSW_TrigProc_MM"),
      "hostname for OPC server:port,<slave bus 1>,<slave bus 2>;hostname 2... ")
    ("bus,b", po::value<int>(&input.bus)->default_value(0),
      "bus name");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);


  // overwrite the printTable option
  if (input.onlyTemp || input.microPod != 0) input.printTable = true;

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  TFile f(fileName.c_str(), "RECREATE");
  TTree tree("data", fmt::format("Read back of Horx environment register{0}.",
    input.humanReadable ? "(Human Readable)" : "(Original)").c_str());

  tree.Branch("opcIP", &data.opc_ip);
  tree.Branch("slaveAddr", &data.slaveAddr);
  tree.Branch("envName", &data.envName);
  tree.Branch("envData", &data.envData);

  std::thread t(&loop, std::cref(input), std::ref(data), std::ref(tree));

  // hold it until pressing keyboard, only for indefinite reading
  if (input.Niteration == 0) {
    std::cin.get();

    stop = true;
  }

  t.join();

  // tree.Write();
  f.Close();

  std::cout  << "Reaching the end." << std::endl;
  return 0;
}

void loop(const Input& input, TreeData& data, TTree& tree) {
  int iteration = 0;
  auto opcManager = nsw::OpcManager{};
  // iteration
  while((input.Niteration == 0 || iteration < input.Niteration) && !stop ) {
    PrintData ptab;
    // loop over TPs
    for (const auto& host_group : nsw::tokenizeString(input.opc_slave, ";")) {
      const auto host_slave = nsw::tokenizeString(host_group, ",");
      data.opc_ip = host_slave[0];
      for (auto it = host_slave.begin() + 1; it != host_slave.end(); it++) {
        data.slaveAddr = fmt::format("{0}.I2C_{1}.bus{1}", *it, input.bus);
        if (input.debug) {
          std::cout << data.opc_ip << " " << data.slaveAddr << std::endl;
        }

        ptab.row.push_back(nsw::tokenizeString(data.slaveAddr, ".")[0]);
        ptab.value.push_back({});
        ptab.column.clear();

        
        // loop over all regisers
        for (std::uint8_t microPod = 1; microPod < 4; ++microPod) {
          if (input.microPod != 0 && input.microPod != microPod) continue;
          for (const auto itr: std::vector<std::pair<bool, std::string>>{
              {true, "tx"}, {false, "rx"}
              }) {


            std::string name;
            bool is_tx = itr.first;
            std::string TRX = itr.second;

            // dummy tree to use function
            pt::ptree ptree;
            ptree.put("OpcServerIp", data.opc_ip);
            ptree.put("OpcNodeId", *it);

            // MM TP
            nsw::hw::MMTP tp(opcManager, ptree);
            auto colName = fmt::format("mp{0}_{1}_temp", microPod, TRX);
            data.envName.push_back(colName);
            tp.setHorxEnvMonAddr(is_tx, microPod, true, false, 0);
            data.envData.push_back( tp.readRegister(nsw::mmtp::REG_HORX_ENV_MON_DATA) / (input.humanReadable?256.:1.));

            if (input.printTable) {
              ptab.column.push_back(colName);
              ptab.value.back().push_back(data.envData.back());
            }

            if (!input.onlyTemp) {

              colName = fmt::format("mp{0}_{1}_loss", microPod, TRX);
              data.envName.push_back(colName);
              tp.setHorxEnvMonAddr(is_tx, microPod, false, true, 0);
              data.envData.push_back( tp.readRegister(nsw::mmtp::REG_HORX_ENV_MON_DATA));

              if (input.printTable) {
                ptab.column.push_back(colName);
                ptab.value.back().push_back(data.envData.back());
              }

              for(int fiber = 0; fiber < 12; ++fiber) {
                colName = fmt::format("mp{0}_{1}_fb{2}", microPod, TRX, fiber);
                data.envName.push_back(colName);
                tp.setHorxEnvMonAddr(is_tx, microPod, false, false, fiber);
                data.envData.push_back( tp.readRegister(nsw::mmtp::REG_HORX_ENV_MON_DATA) / (input.humanReadable?10.:1.));
                if (input.printTable) {
                  ptab.column.push_back(colName);
                  ptab.value.back().push_back(data.envData.back());
                }
              }
            } 

          }
        } // loop over all regisers

        tree.Fill();
        data.envName.clear();
        data.envData.clear();



      }
    } // loop over TPs

    if (input.printTable) {
      std::cout << "\033[H\033[J" << std::flush;
    }

    std::cout <<  "******************************************" << std::endl;
    std::cout << "Done with iteration #" << iteration++ << ".";
    if (input.Niteration == 0) {
      std::cout << " End anytime by pressing <enter>.";
    }
    std::cout << std::endl;
    std::cout <<  "******************************************" << std::endl;

    if (input.printTable) {
      printTable(ptab, true, input.nsector, input.cellWidth, " ");
    }

    tree.AutoSave("SaveSelf");
    std::this_thread::sleep_for(std::chrono::milliseconds(input.interval));
  } // iteration
}

