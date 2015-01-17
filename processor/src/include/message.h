#ifndef SCRIBE_MESSAGE_H
#define SCRIBE_MESSAGE_H

#include <list>
#include "slice.h"

class Message {
public:
  Message();
  virtual ~Message();

public:
  //creator api
  void addSlice(const Slice& slice);

  //consumer api
  Slice* nextSlice();
  

private:
  time_t _create_time;
  std::list<Slice> _slices;
  std::list<Slice>::const_iterator current_slice_iter;
  int _next_slice_seq;
  
  
};
#endif
