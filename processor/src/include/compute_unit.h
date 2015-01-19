#ifndef STREAM_SERVER_COMPUTE_UNIT_H
#define STREAM_SERVER_COMPUTE_UNIT_H

#include "store.h"
#include "callback_msg.h"
#include "block_q.h"

namespace deepscore {

class ComputeUnit {
public:
  ComputeUnit(BlockQueue<CallbackMsg>* callback_q);
  virtual ~ComputeUnit();
  void addSlice(const Slice* slice);

public:
  static void* threadStatic(void *this_ptr);

private:
  void run();
  //one message contains serveral slices
  void computeMessage();

  //send response to caller
  void sendJsonResponse(const std::string& host, int port, const std::string& key, const std::string& json);

private:
  Store* _store;
  BlockQueue<CallbackMsg>* _callback_q;
};

}

#endif
