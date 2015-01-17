#include "include/store.h"

#include <iostream>

Store::Store() {
  //_slices = new list<Slice>();
  _fifo_map = new FIFOMap();
}

Store::~Store(){}

void Store::addSlice(const Slice& slice) {
  //_slices->push_back(slice);
  _fifo_map->addSlice(slice);
}

Slice* Store::getSlice() {
  return _fifo_map->nextSlice();
  /*
  if (!_slices->empty()) {
    const Slice& t = _slices->front();
    Slice* slice = new Slice(t._key, t._val, t._number, t._flag, t._host, t._port);
    std::cout << "slice flag:" << slice->_flag << std::endl;
    _slices->pop_front();
    return slice;
  }
  return NULL;
  */
  /*
  string key = "key";
  string val = "val";
  int number = 0;
  InnerSliceFlag flag = BROKEN;
  string host = "127.0.0.1";
  int port = 1463;
  return new Slice(key, val, number, flag, host, port);
  */
}
