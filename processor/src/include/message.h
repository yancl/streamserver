#ifndef STREAM_SERVER_MESSAGE_H
#define STREAM_SERVER_MESSAGE_H

#include <string>
#include <list>
#include "slice.h"

namespace deepscore {


class Message {
public:
  Message(const std::string& key);
  virtual ~Message();

public:
  //creator api
  void addSlice(const Slice* slice);

  //consumer api
  const Slice* nextSlice();

  //get key of the message
  std::string getKey();
  

private:
  void waitForNextSliceInLock();
  void notifyNextSliceInLock();
  std::list<const Slice*>::iterator nextSliceIter(std::list<const Slice*>::iterator itr);

private:
  time_t _create_time;
  std::string _key;
  std::list<const Slice*> _slices;
  std::list<const Slice*>::iterator _prev_slice_iter;
  std::list<const Slice*>::iterator _next_slice_iter;
  int _next_slice_seq;
  bool _wait_for_next_slice;
  bool _broken;

   //protect each message
  pthread_mutex_t _next_slice_mutex;
  pthread_cond_t _next_slice_cond;

};

}
#endif
