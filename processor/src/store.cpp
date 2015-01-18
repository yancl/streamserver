#include "include/store.h"

#include <iostream>

Store::Store() {
  _fifo_map = new FIFOMap();
}

Store::~Store(){}

void Store::addSlice(const Slice& slice) {
  _fifo_map->addSlice(slice);
}

Slice* Store::getSlice() {
  return _fifo_map->nextSlice();
}
