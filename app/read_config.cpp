// Sample program to read configuration from db/json

#include <iostream>

#include "NSWConfiguration/ConfigReader.h"

int main(int argc, const char *argv[]) {
  ConfigReader reader("file:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.json");
  //ConfigReader reader("file:///afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/dummy_config.xml");
  //ConfigReader reader("file:///afs/cern.ch/atlas/project/CSCNRC/partition_tools/oks/tdaq-07-01-00/muons/segments/CSC/CSC-EndcapA.data.xml");
  auto config = reader.read_config();
  //write_xml(std::cout, config, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4)); // output ptree content
  write_json(std::cout, config);

  return 0;
}
