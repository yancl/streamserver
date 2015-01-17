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
#include "include/compute_unit.h"
#include "include/deep_score_server.h"

using namespace apache::thrift::concurrency;

using namespace std;

using boost::shared_ptr;

shared_ptr<StreamHandler> g_Handler;

#define DEFAULT_CHECK_PERIOD       5
#define DEFAULT_MAX_MSG_PER_SECOND 0
#define DEFAULT_MAX_QUEUE_SIZE     5000000LL
#define DEFAULT_SERVER_THREADS     3
#define DEFAULT_MAX_CONN           0


void print_usage(const char* program_name) {
  cout << "Usage: " << program_name << " [-p port] [-c config_file]" << endl;
}

int main(int argc, char **argv) {

  try {
    /* Increase number of fds */
    struct rlimit r_fd = {65535,65535};
    if (-1 == setrlimit(RLIMIT_NOFILE, &r_fd)) {
      LOG_OPER("setrlimit error (setting max fd size)");
    }

    int next_option;
    const char* const short_options = "hp:c:";
    const struct option long_options[] = {
      { "help",   0, NULL, 'h' },
      { "port",   0, NULL, 'p' },
      { "config", 0, NULL, 'c' },
      { NULL,     0, NULL, 'o' },
    };

    unsigned long int port = 0;  // this can also be specified in the conf file, which overrides the command line
    std::string config_file;
    while (0 < (next_option = getopt_long(argc, argv, short_options, long_options, NULL))) {
      switch (next_option) {
      default:
      case 'h':
        print_usage(argv[0]);
        exit(0);
      case 'c':
        config_file = optarg;
        break;
      case 'p':
        port = strtoul(optarg, NULL, 0);
        break;
      }
    }

    // assume a non-option arg is a config file name
    if (optind < argc && config_file.empty()) {
      config_file = argv[optind];
    }

    // seed random number generation with something reasonably unique
    srand(time(NULL) ^ getpid());

    g_Handler = shared_ptr<StreamHandler>(new StreamHandler(port, config_file));
    g_Handler->initialize();

    scribe::startServer(); // never returns

  } catch(const std::exception& e) {
    LOG_OPER("Exception in main: %s", e.what());
  }

  LOG_OPER("scribe server exiting");
  return 0;
}

StreamHandler::StreamHandler(unsigned long int server_port, const std::string& config_file)
  : port(server_port),
    numThriftServerThreads(DEFAULT_SERVER_THREADS),
    checkPeriod(DEFAULT_CHECK_PERIOD),
    configFilename(config_file),
    numMsgLastSecond(0),
    maxMsgPerSecond(DEFAULT_MAX_MSG_PER_SECOND),
    maxConn(DEFAULT_MAX_CONN),
    maxQueueSize(DEFAULT_MAX_QUEUE_SIZE) {
  time(&lastMsgTime);
  StreamHandlerLock = scribe::concurrency::createReadWriteMutex();
}

StreamHandler::~StreamHandler() {
}

void StreamHandler::initialize() {
  for (int i = 0; i < TOTAL_COMPUTE_THREAD_NUM; i++) {
    ComputeUnit* compute_unit = new ComputeUnit();
    this->_compute_units[i] = compute_unit;
    pthread_create(&(this->_thread_handlers[i]), NULL, threadStatic, (void*) compute_unit);
  }
}

ResultCode::type StreamHandler::AddDataEntryStream(const std::vector<DataEntry, std::allocator<DataEntry> >& entries) {
  std::vector<DataEntry>::const_iterator citr = entries.begin();
  for (;citr != entries.end(); citr++) {
    uint32_t num = scribe::strhash::hash32(citr->key.c_str());
    Slice slice(citr->key, citr->val, citr->number, BROKEN, citr->host, citr->port);
    (_compute_units[num % TOTAL_COMPUTE_THREAD_NUM])->addSlice(slice);
  }
  cout << "add data entry" << endl;
  return ResultCode::OK;
}
