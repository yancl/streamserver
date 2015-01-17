//  Copyright (c) 2007-2009 Facebook
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

#ifndef SCRIBE_SERVER_H
#define SCRIBE_SERVER_H

#include "compute_unit.h"
#include "../gen-cpp/DeepScorerService.h"

const int TOTAL_COMPUTE_THREAD_NUM = 16;

class StreamHandler : virtual public DeepScorerServiceIf {

 public:
  StreamHandler(unsigned long int port, const std::string& conf_file);
  ~StreamHandler();

  void initialize();
  ResultCode::type AddDataEntryStream(const std::vector<DataEntry> & messages);

  unsigned long int port; // it's long because that's all I implemented in the conf class

  // number of threads processing new Thrift connections
  size_t numThriftServerThreads;


  inline unsigned long long getMaxQueueSize() {
    return maxQueueSize;
  }

  inline void setServer(
      boost::shared_ptr<apache::thrift::server::TNonblockingServer> & server) {
    this->server = server;
  }
  unsigned long getMaxConn() {
    return maxConn;
  }
 private:
  boost::shared_ptr<apache::thrift::server::TNonblockingServer> server;

  unsigned long checkPeriod; // periodic check interval for all contained stores


  std::string configFilename;
  time_t lastMsgTime;
  unsigned long numMsgLastSecond;
  unsigned long maxMsgPerSecond;
  unsigned long maxConn;
  unsigned long long maxQueueSize;

  /* mutex to syncronize access to StreamHandler.
   * A single mutex is fine since it only needs to be locked in write mode
   * during start/stop/reinitialize or when we need to create a new category.
   */
  boost::shared_ptr<apache::thrift::concurrency::ReadWriteMutex>
    StreamHandlerLock;

  // disallow empty construction, copy, and assignment
  StreamHandler();
  StreamHandler(const StreamHandler& rhs);
  const StreamHandler& operator=(const StreamHandler& rhs);

private:
  pthread_t _thread_handlers[TOTAL_COMPUTE_THREAD_NUM];
  ComputeUnit* _compute_units[TOTAL_COMPUTE_THREAD_NUM];
};
extern boost::shared_ptr<StreamHandler> g_Handler;
#endif // SCRIBE_SERVER_H
