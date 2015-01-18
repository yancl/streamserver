#ifndef SCRIBE_MESSAGE_H
#define SCRIBE_MESSAGE_H

#include <list>
#include <sys/time.h>
#include "slice.h"

extern Slice* g_SlicePtr;

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
  void waitForNextSliceInLock();
  void notifyNextSliceInLock();
  std::list<Slice*>::iterator nextSliceIter(std::list<Slice*>::iterator itr);

private:
  time_t _create_time;
  std::list<Slice*> _slices;
  std::list<Slice*>::iterator _prev_slice_iter;
  std::list<Slice*>::iterator _next_slice_iter;
  int _next_slice_seq;
  bool _wait_for_next_slice;
  bool _broken;

   //protect each message
  pthread_mutex_t _next_slice_mutex;
  pthread_cond_t _next_slice_cond;
};
#endif
