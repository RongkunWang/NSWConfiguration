#include <memory>
#include <vector>
#include <exception>
#include <string>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigReaderApi.h"

nsw::ConfigReader::ConfigReader(const std::string connection_string, const std::vector<std::string> components):
    m_connection_string(connection_string),
    m_components(components) {
    // Open db, json file or oracle db
    if (m_connection_string.find("json://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("json://").length());
        m_api = std::make_unique<JsonApi> (file_path);
    } else  if (m_connection_string.find("xml://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("xml://").length());
        m_api = std::make_unique<XmlApi> (file_path);
    } else if (m_connection_string.find("oksconfig:") == 0) {
        m_api = std::make_unique<OksApi> (m_connection_string);
    } else if (m_connection_string.find("oracle:") == 0) {
        m_api = std::make_unique<OracleApi> (m_connection_string);
    } else {
        nsw::ConfigIssue issue(ERS_HERE, "Problem accessing the configuration in any of the supported formats. The string has to be preceed by file type (e.g. json://).");
        throw issue;
    }
}

nsw::ConfigReader::ConfigReader(const std::string connection_string):
    m_connection_string(connection_string) {
    // Open db, json file or oracle db
    if (m_connection_string.find("json://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("json://").length());
        m_api = std::make_unique<JsonApi> (file_path);
    } else  if (m_connection_string.find("xml://") == 0) {
        std::string file_path = m_connection_string.substr(std::string("xml://").length());
        m_api = std::make_unique<XmlApi> (file_path);
    } else if (m_connection_string.find("oksconfig:") == 0) {
        m_api = std::make_unique<OksApi> (m_connection_string);
    } else if (m_connection_string.find("oracle:") == 0) {
        m_api = std::make_unique<OracleApi> (m_connection_string);
    } else {
        nsw::ConfigIssue issue(ERS_HERE, "Problem accessing the configuration in any of the supported formats. The string has to be preceed by file type (e.g. json://).");
        throw issue;
    }
}

nsw::ConfigReader::~ConfigReader() {
  // Cleanup, disconnect from db...
}
