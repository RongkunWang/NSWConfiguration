#include <string>
#include <regex>
#include <sstream>
#include <fstream>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/program_options.hpp"

#include "NSWConfiguration/ConfigConverter.h"

using boost::property_tree::ptree;

namespace po = boost::program_options;


struct Args
{
    std::string m_inputName{""};
    std::string m_outputName{""};
    bool m_help{false};
};


[[nodiscard]] Args parseArgs(int argc, char* argv[])
{
    po::options_description desc("Calibrate the phases of the ROC clocks");
    desc.add_options()
        ("help,h", "show this help message")
        ("input,i", po::value<std::string>()->required(), "Input JSON")
        ("output,o", po::value<std::string>()->
            default_value("translated.json"), "Name of output JSON");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return Args{.m_help=true};
    }

    po::notify(vm);

    return Args{vm["input"].as<std::string>(),
                vm["output"].as<std::string>()
                };
}


ptree convert(const ptree& t_tree, ConfigConverter::RegisterAddressSpace t_space)
{
    try
    {
        const auto converter = ConfigConverter(t_tree, t_space, ConfigConverter::ConfigType::VALUE_BASED);
        return converter.getRegisterBasedConfig();
    }
    catch(const std::runtime_error& e1)
    {
        try
        {
            const auto converter = ConfigConverter(t_tree, t_space, ConfigConverter::ConfigType::REGISTER_BASED);
            return converter.getValueBasedConfig();
        }
        catch(const std::runtime_error& e2)
        {
            throw std::runtime_error("Ptree neither value nor register-based because\n" + std::string{e1.what()} + '\n' + std::string{e2.what()});
        }
    }
}


void iterate(ptree& t_tree)
{
    for (auto &child : t_tree)
    {
        if (child.first == "rocPllCoreAnalog")
        {
            const auto convertedTree = convert(child.second, ConfigConverter::RegisterAddressSpace::ROC_ANALOG);
            t_tree.get_child(child.first).clear();
            std::for_each(convertedTree.begin(), convertedTree.end(), [&t_tree] (const auto& t_el) { t_tree.put_child("rocPllCoreAnalog." + t_el.first, t_el.second); });
        }
        if (child.first == "rocCoreDigital")
        {
            const auto convertedTree = convert(child.second, ConfigConverter::RegisterAddressSpace::ROC_DIGITAL);
            t_tree.get_child(child.first).clear();
            std::for_each(convertedTree.begin(), convertedTree.end(), [&t_tree] (const auto& t_el) { t_tree.put_child("rocCoreDigital." + t_el.first, t_el.second); });
        }
        iterate(child.second);
    }
}


std::string fixTypes(const std::string& t_string)
{
    std::regex regex("\"(\\d+)\"");
    return std::regex_replace(t_string, regex, "$1");
}


void write(const std::string& t_text, const std::string& t_outputName)
{
    std::ofstream file;
    file.open(t_outputName);
    file << t_text;
    file.close();
}



int main(int argc, char* argv[])
{
    const auto args = parseArgs(argc, argv);
    if (args.m_help)
    {
        return 0;
    }

    ptree tree;
    boost::property_tree::json_parser::read_json(args.m_inputName, tree);
    try
    {    
        iterate(tree);
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    
    std::stringstream stream;
    boost::property_tree::json_parser::write_json(stream, tree);
    const auto fixedJson = fixTypes(stream.str());
    write(fixedJson, args.m_outputName);
}
