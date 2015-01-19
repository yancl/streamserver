#include <pthread.h>
#include <sstream>
#include <iostream>
#include "include/compute_unit.h"


void* deepscore::ComputeUnit::threadStatic(void *this_ptr) {
  ComputeUnit *compute_ptr = (ComputeUnit*)this_ptr;
  compute_ptr->run();
  return NULL;
}

deepscore::ComputeUnit::ComputeUnit(BlockQueue<CallbackMsg>* callback_q):
  _callback_q(callback_q) {
  _store = new Store();
}

deepscore::ComputeUnit::~ComputeUnit() {}

void deepscore::ComputeUnit::addSlice(const Slice* slice) {
  _store->addSlice(slice);
}

void deepscore::ComputeUnit::run() {
  LOG(INFO) << "[Enter] Compute Unit...";
  bool stop = false;
  while (!stop) {
    //slice constructs the message
    //one message contains (BEING,{MIDDLE,...},END) slices
    this->computeMessage();
  }
  LOG(INFO) << "[Exit] Compute Unit...";
}

void deepscore::ComputeUnit::computeMessage() {
    struct timeval start, end;
    //state machine start!
    bool meet_message_begin = false;
    while(true) {
      bool failed = false;
      std::string err;
      std::string warning;
      const Slice* slice = _store->getSlice();

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
        gettimeofday(&start, NULL);

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

        LOG(INFO) << "finish to process message for key:" << slice->_key;

        gettimeofday(&end, NULL);
        unsigned long timeuse = 1000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec)/1000;
        std::cout << "cost:(" << timeuse << ")ms" << std::endl;

        std::string message = "OK!";
        sendJsonResponse(slice->_host, slice->_port, slice->_key, message);
        break;
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
        break;
      }
    }//end while
  //release resource
}


void deepscore::ComputeUnit::sendJsonResponse(const std::string& host, int port, const std::string& key, const std::string& message) {
  LOG(INFO) << "send json message for key:[" << key << "] msg:" << message;
  _callback_q->push(CallbackMsg(host, port, key, message));
}
