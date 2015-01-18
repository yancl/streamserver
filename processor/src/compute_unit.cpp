#include "include/compute_unit.h"
#include "include/http_client.h"
#include <pthread.h>
#include <sstream>

void* threadStatic(void *this_ptr) {
  ComputeUnit *compute_ptr = (ComputeUnit*)this_ptr;
  compute_ptr->threadMember();
  return NULL;
}

ComputeUnit::ComputeUnit() {
  _store = new Store();
}

ComputeUnit::~ComputeUnit() {}

void ComputeUnit::addSlice(const Slice& slice) {
  _store->addSlice(slice);
}

void ComputeUnit::threadMember() {
  this->compute();
}


void ComputeUnit::compute() {
  cout<< "enter compute unit..." << endl;
  bool stop = false;
  while (!stop) {
    //one message contains (BEING,{MIDDLE,...},END) slices
    //slice compose the message stream
    this->computeMessage();
  }
  cout<< "exit compute unit..." << endl;
}

void ComputeUnit::computeMessage() {
    //state machine start!
    bool meet_message_begin = false;
    while(true) {
      sleep(1);
      bool failed = false;
      string err = "";
      Slice* slice = _store->getSlice();
      if (slice == NULL) {
        continue;
        //cerr << "found one NULL slice!" << endl;
        //return;
      }

      if(slice->_flag == SliceFlag::START) {
        //compute process start
        cout << pthread_self() <<"process message header part!" << endl;

        //duplicated message?
        if (meet_message_begin) {
          err = "duplicated message header.";
          failed = true;
          goto END;
        }
        meet_message_begin = true;

        //core code begin

        //core code end
      } else if (slice->_flag == SliceFlag::MIDDLE) {
        if (!meet_message_begin) {
          err = "unorder message happend!";
          failed = true;
          goto END;
        }
        cout << "process message middle part!" << endl;

        //core code begin

        //core code end
      } else if (slice->_flag == SliceFlag::FINISH) {
        if (!meet_message_begin) {
          err = "unorder message happend!";
          failed = true;
          goto END;
        }

        //last slice of the message
        cout << "process message end part!" << endl;
        //core code begin

        //core code end

        //compute over
        ostringstream oss;
        oss << "http://" << slice->_host << ":" << slice->_port << "/notify";
        //string url = "http://%s:%d/notify/";
        string url = oss.str();
        string msg = "OK";
        sendResponse(url, msg);
        ////delete slice;
        return;
      } else if (slice->_flag == SliceFlag::BROKEN) {
        //part of the message will not show,maybe sender restart or send timeout,etc
        //what we will do is to drop the compute?
        //
        err = "broken message show.";
        failed = true;
        goto END;
      } else {
        err = "meet unexpected message flag!";
        failed = true;
        goto END;
      }

END:
    if (failed) {
      cerr << err << endl;
      ////delete slice;
      return;
    }
    //release resource
    ////delete slice;
  }
}


void ComputeUnit::sendResponse(const string& url, const string& message) {
  CHttpClient http_client;
  string rsp;
  int rv = http_client.Get(url, rsp); 
  if (rv != 0) {
    cerr << "send rsp failed, rv:" << rv << " url:" << url << endl;
    return;
  }
  cout << "send rsp ok for url:" << url << endl;
}
