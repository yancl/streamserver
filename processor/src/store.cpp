#include "include/store.h"

#include <iostream>

deepscore::Store::Store(int fifo_map_size) {
  _fifo_map = new FIFOMap(fifo_map_size);
}

deepscore::Store::~Store(){}

void deepscore::Store::addSlice(const Slice* slice) {
  _fifo_map->addSlice(slice);
}

const deepscore::Slice* deepscore::Store::getSlice() {
  return _fifo_map->nextSlice();
}
