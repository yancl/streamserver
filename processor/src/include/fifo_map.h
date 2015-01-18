#ifndef SCRIBE_FIFO_MAP_H
#define SCRIBE_FIFO_MAP_H

#include <list>
#include <tr1/unordered_map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include "slice.h"
#include "message.h"
#include "../gen-cpp/deep_score_service_types.h"

class FIFOMap {
public:
  FIFOMap();
  virtual ~FIFOMap();

  //add slice to the message slot according to slice.key
  void addSlice(const Slice& slice);

  //get next slice
  Slice* nextSlice();

  //get consumed messages,log it or just delete it
  std::list<Message*>* getConsumedMessages(int count);

private:
  bool isNextIterEnd(std::list<Message*>::const_iterator citr);
  std::list<Message*>::iterator nextIter(std::list<Message*>::iterator itr);
  void waitForNewMessage();
  void notifyNewMessage();

private:
  bool _inited;
  bool _wait_for_next_message;
  std::tr1::unordered_map<std::string, std::list<Message*>::iterator> _fifo_map;
  std::list<Message*> _messages;
  //used to hold last finished message
  std::list<Message*>::iterator _prev_message_itr;
  //used to hold the current reading message
  std::list<Message*>::iterator _current_message_itr;

  //protect _fifo_map,_messages,{_current,_prev}_message_itr
  pthread_mutex_t _fifo_mutex;

  //protect _wait_for_next_message
  pthread_mutex_t _next_message_mutex;
  pthread_cond_t _next_message_cond;

};

#endif
