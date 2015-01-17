#ifndef SCRIBE_SLICE_H
#define SCRIBE_SLICE_H

#include <string>
using namespace std;

enum SliceFlag {
  BEGIN = 0,
  MIDDLE = 1,
  END = 2,
  BROKEN = 10,
};

struct Slice {
  string _key;
  string _val;
  int _number;
  SliceFlag _flag;
  string _host;
  int _port;
  Slice(const string& key, const string& val, int number, SliceFlag flag, const string& host, int port):
  _key(key), _val(val), _number(number), _flag(flag), _host(host), _port(port) {
  }
};

#endif
