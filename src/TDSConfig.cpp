#include <memory>
#include <utility>

#include "boost/property_tree/ptree.hpp"

#include "NSWConfiguration/TDSConfig.h"

using boost::property_tree::ptree;

nsw::TDSCodec::TDSCodec() {
    setRegisterMapping();
    calculateTotalSizes();
}

// This function has to be specific for each FE
void nsw::TDSCodec::setRegisterMapping() {
    i2c::RegisterSizeVector reg_size;

     reg_size = {{"", }, {"", }, {"", }, {"", }, {"", }, {"", }, };

     reg_size = {{"BCID_Offset", 12}, {"BCID_Rollover_Value", 12}, {"CKBC_Clock_Phase", 4},
                 {"Strip_Match_Window", 4}};
     m_addr_reg["0"] = std::move(reg_size);

     reg_size = {{"Ck160_1_Phase", 5}, {"Ck160_0_Phase", 5}, {"SER_PLL_I", 4}, {"SER_PLL_R", 2}};
     m_addr_reg["1"] = std::move(reg_size);

     reg_size = {{"Chan127", 1}, {"Chan126", 1}, {"Chan125", 1}, {"Chan124", 1},
                 {"Chan123", 1}, {"Chan122", 1}, {"Chan121", 1}, {"Chan120", 1},
                 {"Chan119", 1}, {"Chan118", 1}, {"Chan117", 1}, {"Chan116", 1},
                 {"Chan115", 1}, {"Chan114", 1}, {"Chan113", 1}, {"Chan112", 1},
                 {"Chan111", 1}, {"Chan110", 1}, {"Chan109", 1}, {"Chan108", 1},
                 {"Chan107", 1}, {"Chan106", 1}, {"Chan105", 1}, {"Chan104", 1},
                 {"Chan103", 1}, {"Chan102", 1}, {"Chan101", 1}, {"Chan100", 1},
                 {"Chan099", 1}, {"Chan098", 1}, {"Chan097", 1}, {"Chan096", 1},
                 {"Chan095", 1}, {"Chan094", 1}, {"Chan093", 1}, {"Chan092", 1},
                 {"Chan091", 1}, {"Chan090", 1}, {"Chan089", 1}, {"Chan088", 1},
                 {"Chan087", 1}, {"Chan086", 1}, {"Chan085", 1}, {"Chan084", 1},
                 {"Chan083", 1}, {"Chan082", 1}, {"Chan081", 1}, {"Chan080", 1},
                 {"Chan079", 1}, {"Chan078", 1}, {"Chan077", 1}, {"Chan076", 1},
                 {"Chan075", 1}, {"Chan074", 1}, {"Chan073", 1}, {"Chan072", 1},
                 {"Chan071", 1}, {"Chan070", 1}, {"Chan069", 1}, {"Chan068", 1},
                 {"Chan067", 1}, {"Chan066", 1}, {"Chan065", 1}, {"Chan064", 1},
                 {"Chan063", 1}, {"Chan062", 1}, {"Chan061", 1}, {"Chan060", 1},
                 {"Chan059", 1}, {"Chan058", 1}, {"Chan057", 1}, {"Chan056", 1},
                 {"Chan055", 1}, {"Chan054", 1}, {"Chan053", 1}, {"Chan052", 1},
                 {"Chan051", 1}, {"Chan050", 1}, {"Chan049", 1}, {"Chan048", 1},
                 {"Chan047", 1}, {"Chan046", 1}, {"Chan045", 1}, {"Chan044", 1},
                 {"Chan043", 1}, {"Chan042", 1}, {"Chan041", 1}, {"Chan040", 1},
                 {"Chan039", 1}, {"Chan038", 1}, {"Chan037", 1}, {"Chan036", 1},
                 {"Chan035", 1}, {"Chan034", 1}, {"Chan033", 1}, {"Chan032", 1},
                 {"Chan031", 1}, {"Chan030", 1}, {"Chan029", 1}, {"Chan028", 1},
                 {"Chan027", 1}, {"Chan026", 1}, {"Chan025", 1}, {"Chan024", 1},
                 {"Chan023", 1}, {"Chan022", 1}, {"Chan021", 1}, {"Chan020", 1},
                 {"Chan019", 1}, {"Chan018", 1}, {"Chan017", 1}, {"Chan016", 1},
                 {"Chan015", 1}, {"Chan014", 1}, {"Chan013", 1}, {"Chan012", 1},
                 {"Chan011", 1}, {"Chan010", 1}, {"Chan009", 1}, {"Chan008", 1},
                 {"Chan007", 1}, {"Chan006", 1}, {"Chan005", 1}, {"Chan004", 1},
                 {"Chan003", 1}, {"Chan002", 1}, {"Chan001", 1}, {"Chan000", 1}};
     m_addr_reg["2"] = std::move(reg_size);

     reg_size = {{"trig_lut7", 15}, {"NU", 1}, {"trig_lut6", 15}, {"NU", 1}, {"trig_lut5", 15}, {"NU", 1}, {"trig_lut4", 15}, {"NU", 1},
                 {"trig_lut3", 15}, {"NU", 1}, {"trig_lut2", 15}, {"NU", 1}, {"trig_lut1", 15}, {"NU", 1}, {"trig_lut0", 15}, {"NU", 1}};
     m_addr_reg["3"] = std::move(reg_size);

     reg_size = {{"trig_lutf", 15}, {"NU", 1}, {"trig_lute", 15}, {"NU", 1}, {"trig_lutd", 15}, {"NU", 1}, {"trig_lutc", 15}, {"NU", 1},
                 {"trig_lutb", 15}, {"NU", 1}, {"trig_luta", 15}, {"NU", 1}, {"trig_lut9", 15}, {"NU", 1}, {"trig_lut8", 15}, {"NU", 1}};
     m_addr_reg["4"] = std::move(reg_size);

     reg_size = {{"Chan000", 1}, {"Chan001", 1}, {"Chan002", 1}, {"Chan003", 1},
                 {"Chan004", 1}, {"Chan005", 1}, {"Chan006", 1}, {"Chan007", 1},
                 {"Chan008", 1}, {"Chan009", 1}, {"Chan010", 1}, {"Chan011", 1},
                 {"Chan012", 1}, {"Chan013", 1}, {"Chan014", 1}, {"Chan015", 1}};
     m_addr_reg["5"] = std::move(reg_size);

     reg_size = {{"Chan016", 1}, {"Chan017", 1}, {"Chan018", 1}, {"Chan019", 1},
                 {"Chan020", 1}, {"Chan021", 1}, {"Chan022", 1}, {"Chan023", 1},
                 {"Chan024", 1}, {"Chan025", 1}, {"Chan026", 1}, {"Chan027", 1},
                 {"Chan028", 1}, {"Chan029", 1}, {"Chan030", 1}, {"Chan031", 1}};
     m_addr_reg["6"] = std::move(reg_size);

     reg_size = {{"Chan032", 1}, {"Chan033", 1}, {"Chan034", 1}, {"Chan035", 1},
                 {"Chan036", 1}, {"Chan037", 1}, {"Chan038", 1}, {"Chan039", 1},
                 {"Chan040", 1}, {"Chan041", 1}, {"Chan042", 1}, {"Chan043", 1},
                 {"Chan044", 1}, {"Chan045", 1}, {"Chan046", 1}, {"Chan047", 1}};
     m_addr_reg["7"] = std::move(reg_size);

     reg_size = {{"Chan048", 1}, {"Chan049", 1}, {"Chan050", 1}, {"Chan051", 1},
                 {"Chan052", 1}, {"Chan053", 1}, {"Chan054", 1}, {"Chan055", 1},
                 {"Chan056", 1}, {"Chan057", 1}, {"Chan058", 1}, {"Chan059", 1},
                 {"Chan060", 1}, {"Chan061", 1}, {"Chan062", 1}, {"Chan063", 1}};
     m_addr_reg["8"] = std::move(reg_size);

     reg_size = {{"Chan064", 1}, {"Chan065", 1}, {"Chan066", 1}, {"Chan067", 1},
                 {"Chan068", 1}, {"Chan069", 1}, {"Chan070", 1}, {"Chan071", 1},
                 {"Chan072", 1}, {"Chan073", 1}, {"Chan074", 1}, {"Chan075", 1},
                 {"Chan076", 1}, {"Chan077", 1}, {"Chan078", 1}, {"Chan079", 1}};
     m_addr_reg["9"] = std::move(reg_size);

     reg_size = {{"Chan080", 1}, {"Chan081", 1}, {"Chan082", 1}, {"Chan083", 1},
                 {"Chan084", 1}, {"Chan085", 1}, {"Chan086", 1}, {"Chan087", 1},
                 {"Chan088", 1}, {"Chan089", 1}, {"Chan090", 1}, {"Chan091", 1},
                 {"Chan092", 1}, {"Chan093", 1}, {"Chan094", 1}, {"Chan095", 1}};
     m_addr_reg["10"] = std::move(reg_size);

     reg_size = {{"Chan096", 1}, {"Chan097", 1}, {"Chan098", 1}, {"Chan099", 1},
                 {"Chan100", 1}, {"Chan101", 1}, {"Chan102", 1}, {"Chan103", 1}};
     m_addr_reg["11"] = std::move(reg_size);


}

nsw::TDSConfig::TDSConfig(ptree config): I2cFEConfig(config) {
    codec = std::make_unique<TDSCodec>();
    m_address_bitstream = codec->buildConfig(config);
}
