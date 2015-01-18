#include <iostream>
#include <sys/time.h>
#include "include/message.h"
using namespace std;

deepscore::Message::Message():_next_slice_seq(0), _wait_for_next_slice(true), _broken(false){
  pthread_mutex_init(&_next_slice_mutex, NULL);
  pthread_cond_init(&_next_slice_cond, NULL);
  _prev_slice_iter = _next_slice_iter = _slices.end();
}

deepscore::Message::~Message(){
  pthread_mutex_destroy(&_next_slice_mutex);
  pthread_cond_destroy(&_next_slice_cond);
}

void deepscore::Message::addSlice(const Slice& slice) {
  pthread_mutex_lock(&_next_slice_mutex);
  _slices.push_back(new Slice(slice._key, slice._val, slice._number, slice._flag, slice._host, slice._port));

  notifyNextSliceInLock();
  pthread_mutex_unlock(&_next_slice_mutex);
}

deepscore::Slice* deepscore::Message::nextSlice() {
  while (true) {
    bool need_wait = false;
    pthread_mutex_lock(&_next_slice_mutex);
    if (_next_slice_iter == _slices.end()) {
      if (_prev_slice_iter == _slices.end()) {
        _next_slice_iter = _slices.begin();
      } else {
        _next_slice_iter = nextSliceIter(_prev_slice_iter);
      }
    }

    if (_next_slice_iter == _slices.end()) {
      need_wait = true;
    } else {
      Slice* slice_ptr = NULL;
      if ((*_next_slice_iter)->_number == _next_slice_seq) {
        slice_ptr = (*_next_slice_iter);
        _next_slice_seq++;
        _prev_slice_iter = _next_slice_iter;
        _next_slice_iter++;
      } else {
        //broken?unorder? seq num continuous
        _broken = true;
        //incase that we don't need to alloc&dealloc memory
        slice_ptr = g_SlicePtr;
      }
      pthread_mutex_unlock(&_next_slice_mutex);
      return slice_ptr;
    }
    if (need_wait) {
      waitForNextSliceInLock();
    }
    pthread_mutex_unlock(&_next_slice_mutex);
  }
}

void deepscore::Message::waitForNextSliceInLock() {
  cout << "begin to cond for next slice ..." << endl;
  struct timeval now;
  struct timespec abs_timeout;
  gettimeofday(&now, NULL);
  abs_timeout.tv_sec = now.tv_sec + 1;
  abs_timeout.tv_nsec = 0;

  _wait_for_next_slice = true;
  pthread_cond_timedwait(&_next_slice_cond, &_next_slice_mutex, &abs_timeout);
  _wait_for_next_slice = false;
  cout << "cond for next slice finished!" << endl;
}


void deepscore::Message::notifyNextSliceInLock() {
  if (_wait_for_next_slice) {
    _wait_for_next_slice = false;
    pthread_cond_signal(&_next_slice_cond);
  }
}

std::list<deepscore::Slice*>::iterator deepscore::Message::nextSliceIter(std::list<deepscore::Slice*>::iterator itr) {
  if (itr == _slices.end()) {
    return itr;
  }
  std::list<deepscore::Slice*>::iterator copied_iter = itr;
  return ++copied_iter;
}
