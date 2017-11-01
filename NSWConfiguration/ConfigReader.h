// Implementation of Config Db Reader
// We could have few implementations: json, oracle etc.
// Use bridge design pattern to implement multiple implementations?

#include <iostream>
#include <string>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

#ifndef INC_CONFIGREADER_H_
#define INC_CONFIGREADER_H_

class ConfigReaderApi {
 protected:
  ptree m_config;  // Ptree that holds all configuration

 public:
  /// Read the whole config db and dump it in the m_config tree, return a reference to the m_config
  virtual ptree & read() = 0;

  /// Read specific part of config db
  virtual ptree read(std::string element_name) = 0;
  virtual ~ConfigReaderApi() {}
};

class JsonApi: public ConfigReaderApi {
 private:
  std::string m_file_path;

 public:
  explicit JsonApi(std::string file_path): m_file_path(file_path) {}
  virtual ptree & read();
  virtual ptree read(std::string element_name);
};

class XmlApi: public ConfigReaderApi {
 private:
  std::string m_file_path;

 public:
  explicit XmlApi(std::string file_path): m_file_path(file_path) {}
  virtual ptree & read();
  virtual ptree read(std::string element_name);
};

class OracleApi: public ConfigReaderApi {
 private:
  std::string db_connection;

 public:
  explicit OracleApi(std::string db_connection) {}
  ~OracleApi() {}
  ptree & read();
  ptree read(std::string element_name);
};

class ConfigReader {
 private:
  std::string m_connection_string;
  std::unique_ptr<ConfigReaderApi> m_api;

 public:
  explicit ConfigReader(const std::string connection_string);
  ~ConfigReader();

  ptree read_config() {
    return m_api->read();
  }

  ptree read_config(std::string element_name) {
    return m_api->read(element_name);
  }
};
#endif  // INC_CONFIGREADER_H_

