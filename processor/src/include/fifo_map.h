#ifndef SCRIBE_FIFO_MAP_H
#define SCRIBE_FIFO_MAP_H

#include <list>
#include <tr1/unordered_map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include "slice.h"
#include "message.h"

class FIFOMap {
public:
  FIFOMap();
  virtual ~FIFOMap();

  void init();

  //add slice to the message slot according to slice.key
  void addSlice(const Slice& slice);

  //get next slice
  Slice* nextSlice();

  //get consumed messages,log it or just delete it
  std::list<Message*>* getConsumedMessages(int count);

private:
  std::tr1::unordered_map<std::string, std::list<Message*>::iterator> _fifo_map;
  std::list<Message*> _messages;
  std::list<Message*>::iterator _current_message_itr;

  //protect when change the fifo map struct
  pthread_mutex_t _fifo_mutex;

  //protect when change the fifo message struct
  pthread_mutex_t _next_slice_mutex;

  //block on this cond when get the next slice
  pthread_cond_t _next_slice_cond;

};

#endif
