#include "include/ini_parser.h"


bool deepscore::IniParser::init(const std::string& filename) {
  if (!_inited) {
    boost::property_tree::ini_parser::read_ini(filename, _pt);
    _inited = true;
    return true;
  }
  return false;
}

std::string deepscore::IniParser::getStrValue(const std::string& key) {
  return _pt.get<std::string>(key);
}

int deepscore::IniParser::getIntValue(const std::string& key) {
  return _pt.get<int>(key);
}
