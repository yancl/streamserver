#include <pthread.h>
#include <sstream>
#include "include/compute_unit.h"

void* ComputeUnit::threadStatic(void *this_ptr) {
  ComputeUnit *compute_ptr = (ComputeUnit*)this_ptr;
  compute_ptr->run();
  return NULL;
}

ComputeUnit::ComputeUnit(BlockQueue<CallbackMsg>* callback_q):
  _callback_q(callback_q) {
  _store = new Store();
}

ComputeUnit::~ComputeUnit() {}

void ComputeUnit::addSlice(const Slice& slice) {
  _store->addSlice(slice);
}

void ComputeUnit::run() {
  LOG(INFO) << "[Enter] Compute Unit...";
  bool stop = false;
  while (!stop) {
    //slice constructs the message
    //one message contains (BEING,{MIDDLE,...},END) slices
    this->computeMessage();
  }
  LOG(INFO) << "[Exit] Compute Unit...";
}

void ComputeUnit::computeMessage() {
    //state machine start!
    bool meet_message_begin = false;
    while(true) {
      bool failed = false;
      std::string err;
      std::string warning;
      Slice* slice = _store->getSlice();
      if (slice == NULL) {
        LOG(FATAL) << "**NULL** slice happend!";
        return;
      }

      if(slice->_flag == SliceFlag::START) {
        //LOG(DEBUG) << "start to process message for key:" << slice._key;
        LOG(INFO) << "start to process message for key:" << slice->_key;

        //duplicated message?
        if (meet_message_begin) {
          err = "duplicated message header for key:" + slice->_key;
          failed = true;
          goto END;
        }
        meet_message_begin = true;

        //compute code

      } else if (slice->_flag == SliceFlag::MIDDLE) {
        if (!meet_message_begin) {
          err = "[MIDDLE]unorder message happend for key:" + slice->_key;
          failed = true;
          goto END;
        }

        //compute code

      } else if (slice->_flag == SliceFlag::FINISH) {
        if (!meet_message_begin) {
          err = "[END]unorder message happend for key:" + slice->_key;
          failed = true;
          goto END;
        }

        //compute code

        //LOG(DEBUG) << "finish to process message for key:" << slice._key;
        LOG(INFO) << "finish to process message for key:" << slice->_key;


        std::string message = "OK!";
        sendJsonResponse(slice->_host, slice->_port, slice->_key, message);
        ////delete slice;
        return;
      } else if (slice->_flag == SliceFlag::BROKEN) {
        //part of the message will not show,maybe sender restart or send timeout,etc
        //what we will do is to drop the compute?
        //
        warning = "broken message show for key:" + slice->_key;
        failed = true;
        goto END;
      } else {
        std::ostringstream oss;
        oss << "meet unexpected message flag for key:" << slice->_key << " flag:" << slice->_flag;
        err = oss.str();
        failed = true;
        goto END;
      }

END:
    if (failed) {
      if (!err.empty()) {
        LOG(ERROR) << err;
      }
      if (!warning.empty()) {
        LOG(WARNING) << warning;
      }
      ////delete slice;
      return;
    }
    //release resource
    ////delete slice;
  }
}


void ComputeUnit::sendJsonResponse(const std::string& host, int port, const std::string& key, const std::string& message) {
  _callback_q->push(CallbackMsg(host, port, key, message));
}
