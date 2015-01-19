#ifndef STREAM_SERVER_INI_PARSER_H
#define STREAM_SERVER_INI_PARSER_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace deepscore {

class IniParser {
public:
  IniParser():_inited(false){}
  virtual ~IniParser(){}
  bool init(const std::string& filename);
  std::string getStrValue(const std::string& key);
  int getIntValue(const std::string& key);

private:
  bool _inited;
  boost::property_tree::ptree _pt;
};

}

#endif
