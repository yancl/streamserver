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

#include "include/common.h"
#include "include/stream_handler.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using boost::shared_ptr;


/*
 * Time functions
 */

unsigned long deepscore::clock::nowInMsec() {
  // There is a minor race condition between the 2 calls below,
  // but the chance is really small.

  // Get current time in timeval
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // Get current time in sec
  time_t sec = time(NULL);

  return ((unsigned long)sec) * 1000 + (tv.tv_usec / 1000);
}

/*
 * Hash functions
 */

uint32_t deepscore::integerhash::hash32(uint32_t key) {
  return key;
}

uint32_t deepscore::strhash::hash32(const char *s) {
  // Use the djb2 hash (http://www.cse.yorku.ca/~oz/hash.html)
  if (s == NULL) {
    return 0;
  }
  uint32_t hash = 5381;
  int c;
  while ((c = *s++)) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }
  return hash;
}

/*
 * Starting a deepscore server.
 */
void deepscore::startServer() {
  boost::shared_ptr<TProcessor> processor(new DeepScorerServiceProcessor(deepscore::g_Handler));
  /* This factory is for binary compatibility. */
  boost::shared_ptr<TProtocolFactory> protocol_factory(
    new TBinaryProtocolFactory(0, 0, false, false)
  );
  boost::shared_ptr<ThreadManager> thread_manager;

  if (deepscore::g_Handler->numThriftServerThreads > 1) {
    // create a ThreadManager to process incoming calls
    thread_manager = ThreadManager::newSimpleThreadManager(
      deepscore::g_Handler->numThriftServerThreads
    );

    shared_ptr<PosixThreadFactory> thread_factory(new PosixThreadFactory());
    thread_manager->threadFactory(thread_factory);
    thread_manager->start();
  }

  shared_ptr<TNonblockingServer> server(new TNonblockingServer(
                                          processor,
                                          protocol_factory,
                                          deepscore::g_Handler->port,
                                          thread_manager
                                        ));
  deepscore::g_Handler->setServer(server);

  LOG_OPER("Starting stream server on port %lu", deepscore::g_Handler->port);
  fflush(stderr);

  // throttle concurrent connections
  unsigned long mconn = deepscore::g_Handler->getMaxConn();
  if (mconn > 0) {
    LOG_OPER("Throttle max_conn to %lu", mconn);
    server->setMaxConnections(mconn);
    server->setOverloadAction(T_OVERLOAD_CLOSE_ON_ACCEPT);
  }

  server->serve();
  // this function never returns
}


/*
 * Stopping a deepscore server.
 */
void deepscore::stopServer() {
  exit(0);
}
