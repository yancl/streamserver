#ifndef SCRIBE_STORE_H
#define SCRIBE_STORE_H

#include <list>
#include "common.h"
#include "slice.h"
#include "fifo_map.h"

class Store {
public:
  Store();
  virtual ~Store();

public:
  void addSlice(const Slice& slice);
  Slice* getSlice();

private:
  FIFOMap* _fifo_map;
};
#endif
