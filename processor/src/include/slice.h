#ifndef STREAM_SERVER_SLICE_H
#define STREAM_SERVER_SLICE_H

#include <string>
#include "../gen-cpp/deep_score_service_types.h"

namespace deepscore {

struct Slice {
  std::string _key;
  std::string _val;
  int _number;
  SliceFlag::type _flag;
  std::string _host;
  int _port;
  Slice(const std::string& key, const std::string& val, int number, SliceFlag::type flag, const std::string& host, int port):
  _key(key), _val(val), _number(number), _flag(flag), _host(host), _port(port) {
  }
};

extern const Slice* g_SlicePtr;

}

#endif
