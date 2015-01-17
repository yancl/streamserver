#ifndef SCRIBE_STORE_H
#define SCRIBE_STORE_H

#include <list>
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
  //std::list<Slice>* _slices; 
  FIFOMap* _fifo_map;
};
#endif
