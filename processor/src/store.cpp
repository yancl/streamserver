#include "include/store.h"

#include <iostream>

deepscore::Store::Store() {
  _fifo_map = new FIFOMap();
}

deepscore::Store::~Store(){}

void deepscore::Store::addSlice(const Slice& slice) {
  _fifo_map->addSlice(slice);
}

deepscore::Slice* deepscore::Store::getSlice() {
  return _fifo_map->nextSlice();
}
