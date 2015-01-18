//  Copyright (c) 2007-2008 Facebook
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// See accompanying file LICENSE or visit the Scribe site at:
// http://developers.facebook.com/scribe/
//
// @author Bobby Johnson
// @author James Wang
// @author Jason Sobel
// @author Avinash Lakshman
// @author Anthony Giardullo

#include "include/callback.h"
#include "include/stream_handler.h"

#define DEFAULT_CHECK_PERIOD       5
#define DEFAULT_MAX_MSG_PER_SECOND 0
#define DEFAULT_MAX_QUEUE_SIZE     5000000LL
#define DEFAULT_SERVER_THREADS     3
#define DEFAULT_MAX_CONN           0

deepscore::StreamHandler::StreamHandler(unsigned long int server_port, const std::string& config_file)
  : port(server_port),
    numThriftServerThreads(DEFAULT_SERVER_THREADS),
    checkPeriod(DEFAULT_CHECK_PERIOD),
    configFilename(config_file),
    numMsgLastSecond(0),
    maxMsgPerSecond(DEFAULT_MAX_MSG_PER_SECOND),
    maxConn(DEFAULT_MAX_CONN),
    maxQueueSize(DEFAULT_MAX_QUEUE_SIZE) {
  time(&lastMsgTime);
  _callback_q_ptr = NULL;
}

deepscore::StreamHandler::~StreamHandler() {
  if(_callback_q_ptr != NULL) {
    delete _callback_q_ptr;
  }
}

void deepscore::StreamHandler::initialize() {
  //create callback queue
  _callback_q_ptr = new BlockQueue<CallbackMsg>();

  //init callback worker
  Callback* callback = new Callback(_callback_q_ptr);
  pthread_create(&(_callback_handler), NULL, Callback::threadStatic, (void*) callback);
  
  //init compute worker
  for (int i = 0; i < TOTAL_COMPUTE_THREAD_NUM; i++) {
    ComputeUnit* compute_unit = new ComputeUnit(_callback_q_ptr);
    this->_compute_units[i] = compute_unit;
    pthread_create(&(this->_thread_handlers[i]), NULL, ComputeUnit::threadStatic, (void*) compute_unit);
  }
}

deepscore::ResultCode::type deepscore::StreamHandler::AddDataSliceStream(const std::vector<DataSlice, std::allocator<DataSlice> >& slices) {
  std::vector<DataSlice>::const_iterator citr = slices.begin();
  for (;citr != slices.end(); citr++) {
    uint32_t num = deepscore::strhash::hash32(citr->key.c_str());
    Slice slice(citr->key, citr->val, citr->number, citr->flag, citr->host, citr->port);
    (_compute_units[num % TOTAL_COMPUTE_THREAD_NUM])->addSlice(slice);
  }
  return ResultCode::OK;
}