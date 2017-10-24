// Sample program to read configuration from db/json

#include <iostream>

#include "ConfigReader.h"

int main(int argc, const char *argv[]) {
  ConfigReader reader("file:///afs/cern.ch/user/c/cyildiz/public/nsw-work/nsw-configuration/data/dummy_config.json");
  auto config = reader.read_config();
  write_json(std::cout, config);

  return 0;
}
