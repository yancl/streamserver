#include "include/fifo_map.h"
#include <iostream>
using namespace std;

deepscore::FIFOMap::FIFOMap(unsigned map_size):_inited(false),_wait_for_next_message(true),_map_size(map_size){
  if (!_inited) {
    pthread_mutex_init(&_fifo_mutex, NULL);
    //pthread_mutex_init(&_next_message_mutex, NULL);
    pthread_cond_init(&_next_message_cond, NULL);
    //set to invalid
    //_prev_message_itr = _current_message_itr = _messages.end();
    _current_message_itr = _messages.end();
    _to_be_delete_msg = NULL;
    _fifo_map = new std::tr1::unordered_map<std::string, std::list<Message*>::iterator>(_map_size);
    _inited = true;
  }
}

deepscore::FIFOMap::~FIFOMap() {
  if (_inited) {
    pthread_mutex_destroy(&_fifo_mutex);
    //pthread_mutex_destroy(&_next_message_mutex);
    pthread_cond_destroy(&_next_message_cond);
    if (_to_be_delete_msg != NULL) {
      delete _to_be_delete_msg;
    }
    delete _fifo_map;
  }
}

void deepscore::FIFOMap::addSlice(const Slice* slice) {
  pthread_mutex_lock(&_fifo_mutex); 

  std::list<Message*>::iterator message_iter = _messages.end();
  std::tr1::unordered_map<std::string, std::list<Message*>::iterator>::const_iterator map_citr = _fifo_map->find(slice->_key);
  if (map_citr == _fifo_map->end()) {
    _messages.push_back(new Message(slice->_key));
    message_iter = _messages.end();
    --message_iter;

    _fifo_map->insert(std::make_pair(slice->_key, message_iter));

    //new message,notify
    notifyNewMessageInLock();

  } else {
    message_iter = map_citr->second;
  }
  //add slice to the message bucket
  (*message_iter)->addSlice(slice);

  pthread_mutex_unlock(&_fifo_mutex);
}

const deepscore::Slice* deepscore::FIFOMap::nextSlice() {
  while (true) {
    if (_current_message_itr == _messages.end()) {
      pthread_mutex_lock(&_fifo_mutex);
      _current_message_itr = _messages.begin();
      //empty,wait
      if (_current_message_itr == _messages.end()) {
        waitForNewMessageInLock();
        _current_message_itr = _messages.begin();
      }
      pthread_mutex_unlock(&_fifo_mutex);
    }

    if (_current_message_itr != _messages.end()) {
      const Slice* slice = (*_current_message_itr)->nextSlice();
      //check should we move on to next message
      if ((slice->_flag == SliceFlag::FINISH) ||
          (slice->_flag == SliceFlag::BROKEN)) {
        Message* next_to_be_delete_msg = *_current_message_itr;

        pthread_mutex_lock(&_fifo_mutex);
        _fifo_map->erase(next_to_be_delete_msg->getKey());
        std::list<Message*>::iterator to_be_deleted_itr = _current_message_itr;
        ++_current_message_itr;
        _messages.erase(to_be_deleted_itr);
        pthread_mutex_unlock(&_fifo_mutex);

        if (_to_be_delete_msg != NULL) {
          delete _to_be_delete_msg;
        }
        //keep a reference to the return *slice*
        //it will be deleted next around because the slice will not be used then.
        _to_be_delete_msg = next_to_be_delete_msg;
      }
      return slice;
    }
  }
}

std::list<deepscore::Message*>::iterator deepscore::FIFOMap::nextIter(std::list<deepscore::Message*>::iterator itr) {
  std::list<deepscore::Message*>::iterator itr_copied = itr;
  if (itr == _messages.end()) {
    return itr;
  }
  return ++itr_copied;
}

void deepscore::FIFOMap::waitForNewMessageInLock() {
  struct timeval now;
  struct timespec abs_timeout;
  gettimeofday(&now, NULL);
  abs_timeout.tv_sec = now.tv_sec + 1;
  abs_timeout.tv_nsec = 0;

  //pthread_mutex_lock(&_next_message_mutex);
  _wait_for_next_message = true;
  pthread_cond_timedwait(&_next_message_cond, &_fifo_mutex, &abs_timeout);
  _wait_for_next_message = false;
  //pthread_mutex_unlock(&_next_message_mutex);
}

void deepscore::FIFOMap::notifyNewMessageInLock() {
  //pthread_mutex_lock(&_next_message_mutex);
  if (_wait_for_next_message) {
    _wait_for_next_message = false;
    pthread_cond_signal(&_next_message_cond);
  }
  //pthread_mutex_unlock(&_next_message_mutex);
}
