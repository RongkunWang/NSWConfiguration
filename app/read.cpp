#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <glob.h>

namespace pt = boost::property_tree;

std::vector<std::string> glob_nice(std::string glob_path) {
  auto result = std::vector<std::string>();
  glob_t glob_result;
  glob(glob_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for(unsigned int i = 0; i < glob_result.gl_pathc; ++i)
    result.push_back( glob_result.gl_pathv[i] );
  globfree(&glob_result);
  return result;
}

int read() {

  unsigned int ipatt = 0;
  std::cout << std::endl;
  std::string json_files = "/afs/cern.ch/user/r/rbrener/public/Trigger_pad_patterns_json_files_old_format/*.json";
  auto filenames = glob_nice(json_files);
  for (auto filename: filenames) {

    if (ipatt > 0)
      break;
    std::cout << filename << std::endl;
    pt::ptree patts;
    pt::read_json(filename, patts);

    for (auto febkv: patts) {
      std::cout << febkv.first << std::endl;
      for (auto vmmkv: febkv.second) {
        std::cout << " VMM " << vmmkv.first << std::endl;
        for (auto chkv: vmmkv.second) {
          unsigned int ch = chkv.second.get<unsigned>("");
          std::cout << "  CH " << ch << std::endl;
        }
      }
    }

    ipatt++;

  }

  std::cout << std::endl;
  return 0;
}

