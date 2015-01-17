#include "include/store.h"

Store::Store(){}
Store::~Store(){}

void Store::addSlice(const Slice& slice) {

}

Slice* Store::getSlice() {
  string key = "key";
  string val = "val";
  int number = 0;
  SliceFlag flag = BROKEN;
  string host = "127.0.0.1";
  int port = 1463;
  return new Slice(key, val, number, flag, host, port);
}
