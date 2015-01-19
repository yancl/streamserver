#ifndef STREAM_SERVER_FIFO_MAP_H
#define STREAM_SERVER_FIFO_MAP_H

#include <list>
#include <tr1/unordered_map>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include "slice.h"
#include "message.h"
#include "../gen-cpp/deep_score_service_types.h"

namespace deepscore {

class FIFOMap {
public:
  FIFOMap(int map_size);
  virtual ~FIFOMap();

  //add slice to the message slot according to slice.key
  void addSlice(const Slice* slice);

  //get next slice
  const Slice* nextSlice();

private:
  std::list<Message*>::iterator nextIter(std::list<Message*>::iterator itr);
  void waitForNewMessageInLock();
  void notifyNewMessageInLock();

private:
  bool _inited;
  bool _wait_for_next_message;
  int _map_size;
  std::tr1::unordered_map<std::string, std::list<Message*>::iterator>* _fifo_map;
  std::list<Message*> _messages;
  const Message* _to_be_delete_msg;
  //used to hold last finished message
  //std::list<Message*>::iterator _prev_message_itr;
  //used to hold the current reading message
  std::list<Message*>::iterator _current_message_itr;

  //protect _fifo_map,_messages,{_current,_prev}_message_itr
  pthread_mutex_t _fifo_mutex;
  pthread_cond_t _next_message_cond;

  //protect _wait_for_next_message
  //pthread_mutex_t _next_message_mutex;
  //pthread_cond_t _next_message_cond;

};

}

#endif
