#include "include/fifo_map.h"
#include <iostream>
using namespace std;

deepscore::FIFOMap::FIFOMap(unsigned map_size):_inited(false),_wait_for_next_message(true),_map_size(map_size){
  if (!_inited) {
    pthread_mutex_init(&_fifo_mutex, NULL);
    pthread_mutex_init(&_next_message_mutex, NULL);
    pthread_cond_init(&_next_message_cond, NULL);
    //set to invalid
    _prev_message_itr = _current_message_itr = _messages.end();
    _fifo_map = new std::tr1::unordered_map<std::string, std::list<Message*>::iterator>(_map_size);
    _inited = true;
  }
}

deepscore::FIFOMap::~FIFOMap() {
  if (_inited) {
    pthread_mutex_destroy(&_fifo_mutex);
    pthread_mutex_destroy(&_next_message_mutex);
    pthread_cond_destroy(&_next_message_cond);
    delete _fifo_map;
  }
}

void deepscore::FIFOMap::addSlice(const Slice& slice) {
  pthread_mutex_lock(&_fifo_mutex); 

  std::list<Message*>::iterator message_iter = _messages.end();
  std::tr1::unordered_map<std::string, std::list<Message*>::iterator>::const_iterator map_citr = _fifo_map->find(slice._key);
  if (map_citr == _fifo_map->end()) {
    _messages.push_back(new Message());
    message_iter = _messages.end();
    --message_iter;

    _fifo_map->insert(std::make_pair(slice._key, message_iter));

    //new message,notify
    notifyNewMessage();

  } else {
    message_iter = map_citr->second;
  }
  //add slice the message bucket
  (*message_iter)->addSlice(slice);

  pthread_mutex_unlock(&_fifo_mutex);
}

deepscore::Slice* deepscore::FIFOMap::nextSlice() {
  while (true) {
    //get current message iterator
    pthread_mutex_lock(&_fifo_mutex);
    std::list<Message*>::iterator message_itr = _messages.end();
    std::list<Message*>::iterator message_itr_end = _messages.end();
    if (_current_message_itr == _messages.end()) {
      if( _prev_message_itr == _messages.end()) {
          _prev_message_itr = _current_message_itr = _messages.begin();
      } else {
          _current_message_itr = nextIter(_prev_message_itr);
      }
    }
    message_itr = _current_message_itr;
    pthread_mutex_unlock(&_fifo_mutex);

    //get slice from current message iterator
    if (message_itr != message_itr_end) {
      Slice* slice = (*message_itr)->nextSlice();
      //check should we move on to next message
      if ((slice->_flag == SliceFlag::FINISH) || 
          (slice->_flag == SliceFlag::BROKEN)) {
        pthread_mutex_lock(&_fifo_mutex);
        _prev_message_itr = _current_message_itr;
        ++_current_message_itr;
        pthread_mutex_unlock(&_fifo_mutex);
      }
      return slice;
    } else {
      //no more messages, wait for...
      waitForNewMessage();
    }
  }
}

std::list<deepscore::Message*>* deepscore::FIFOMap::getConsumedMessages(int count) {
  pthread_mutex_lock(&_fifo_mutex); 
  pthread_mutex_unlock(&_fifo_mutex);
  return NULL;
}

std::list<deepscore::Message*>::iterator deepscore::FIFOMap::nextIter(std::list<deepscore::Message*>::iterator itr) {
  std::list<deepscore::Message*>::iterator itr_copied = itr;
  if (itr == _messages.end()) {
    return itr;
  }
  return ++itr_copied;
}

void deepscore::FIFOMap::waitForNewMessage() {
  struct timeval now;
  struct timespec abs_timeout;
  gettimeofday(&now, NULL);
  abs_timeout.tv_sec = now.tv_sec + 1;
  abs_timeout.tv_nsec = 0;

  pthread_mutex_lock(&_next_message_mutex);
  _wait_for_next_message = true;
  pthread_cond_timedwait(&_next_message_cond, &_next_message_mutex, &abs_timeout);
  _wait_for_next_message = false;
  pthread_mutex_unlock(&_next_message_mutex);
}

void deepscore::FIFOMap::notifyNewMessage() {
  pthread_mutex_lock(&_next_message_mutex);
  if (_wait_for_next_message) {
    _wait_for_next_message = false;
    pthread_cond_signal(&_next_message_cond);
  }
  pthread_mutex_unlock(&_next_message_mutex);
}
