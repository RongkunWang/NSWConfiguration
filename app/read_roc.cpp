#include <iostream>
#include <map>
#include <vector>
#include <numeric>
#include <bitset>


#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

int main()
{

    using boost::property_tree::ptree;

    using RegisterAndSize = std::pair<std::string, size_t>;

    // Following acts as ordered map of register names and sizes in bits
    using RegisterSizeVector = std::vector<RegisterAndSize>;
    using AddressRegisterMap = std::map<std::string, RegisterSizeVector>;
    using AddressSizeMap = std::map<std::string, size_t>;

    // These should be initialized by the base class
    RegisterSizeVector r;
    AddressRegisterMap ar;

    // Following should be done in a purely virtual function of the base class
    // setRegisterMapping
    r = {{"l1_first",1},{"even_parity",1},{"roc_id",6}};
    ar["0"] = std::move(r);
    r = {{"sroc3",2},{"sroc2",2},{"sroc1",2},{"sroc0",2}};
    ar["1"] = std::move(r);


    // Following should be done with a base class method
    // calculateTotalSizes
    AddressSizeMap as;
    for (auto e: ar) {
        auto name = e.first;
        auto register_sizes = e.second;

        // Total size of registers, by summing sizes of individual registers
        size_t total_size = std::accumulate(register_sizes.begin(), register_sizes.end(), 0,
                                            [](size_t sum, RegisterAndSize & p) {
                                            return sum + p.second;  });
        std::cout << name << " -> total size: " << total_size << std::endl;
        for (auto i : register_sizes) {
            std::cout << "register: " << i.first << " : " << i.second << "\n";
            /* code */
        }
        std::cout << "\n";
    }
    
    // Read ROC Config
    std::string file_path = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work/NSWConfiguration/data/roc_dummy_config.json";
    ptree config;
    try {
        boost::property_tree::read_json(file_path, config);
    } catch(std::exception & e) {
        std::cout << "Failed: " << e.what() <<  std::endl;
        throw;
    }

    auto rocconfig = config.get_child("ROC_L01_M01");
    write_json(std::cout, rocconfig);

    // Map of (address, bitstream for the register) TODO(cyildiz): This should be part of the base class
    std::map<std::string, std::string> bitstream;
    for (auto e: ar) {

        auto name = e.first;
        auto register_sizes = e.second;

        auto child = rocconfig.get_child(name);
        write_json(std::cout, child);
    
        std::string tempstr;
        for (auto rs: register_sizes) {
            auto register_name = rs.first;
            auto size = rs.second;

            auto u = child.get<unsigned>(register_name);
            // checkOverflow
            std::cout << register_name << " -> " << u << std::endl;
            
            // TODO(cyildiz): Large enough to take any register
            std::bitset<32> bs(u);
            auto stringbs = bs.to_string();
            stringbs = stringbs.substr(stringbs.size()-size, stringbs.size());
            //std::cout << stringbs << std::endl;
            tempstr += stringbs;
        }
        std::cout << tempstr << std::endl;
        // TODO(cyiliz): Put tempstr in a new map
        bitstream[name] = tempstr;


    }

    return 0;
}
