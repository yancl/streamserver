#ifndef SCRIBE_SLICE_H
#define SCRIBE_SLICE_H

#include <string>
#include "../gen-cpp/deep_score_service_types.h"

using namespace std;
/*
enum InnerSliceFlag {
  BEGIN = 0,
  MIDDLE = 1,
  END = 2,
  BROKEN = 10,
};*/

struct Slice {
  string _key;
  string _val;
  int _number;
  SliceFlag::type _flag;
  string _host;
  int _port;
  Slice(const string& key, const string& val, int number, SliceFlag::type flag, const string& host, int port):
  _key(key), _val(val), _number(number), _flag(flag), _host(host), _port(port) {
  }
};

#endif
