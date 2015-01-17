#ifndef SCRIBE_STORE_H
#define SCRIBE_STORE_H

#include "slice.h"

class Store {
public:
  Store();
  virtual ~Store();

  void addSlice(const Slice& slice);
  Slice* getSlice();
};
#endif
