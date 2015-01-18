#ifndef SCRIBE_COMPUTE_UNIT_H
#define SCRIBE_COMPUTE_UNIT_H

#include "store.h"

#include <iostream>
using namespace std;

void* threadStatic(void *this_ptr);

class ComputeUnit {
public:
  ComputeUnit();
  virtual ~ComputeUnit();
  void addSlice(const Slice& slice);
  void threadMember();

private:
  void compute();
  //one message contains serveral slices
  void computeMessage();

  //send response to caller
  void sendResponse(const string& url, const string& message);

private:
  Store* _store;
};


#endif
