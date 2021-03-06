#ifndef STREAM_SERVER_STORE_H
#define STREAM_SERVER_STORE_H

#include <list>
#include "common.h"
#include "slice.h"
#include "fifo_map.h"

namespace deepscore {

class Store {
public:
  Store(int fifo_map_size);
  virtual ~Store();

public:
  void addSlice(const Slice* slice);
  const Slice* getSlice();

private:
  FIFOMap* _fifo_map;
};

}
#endif
