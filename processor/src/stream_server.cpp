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
#include "include/stream_server.h"

using namespace apache::thrift::concurrency;

using namespace std;

using boost::shared_ptr;

shared_ptr<StreamHandler> g_Handler;

#define DEFAULT_CHECK_PERIOD       5
#define DEFAULT_MAX_MSG_PER_SECOND 0
#define DEFAULT_MAX_QUEUE_SIZE     5000000LL
#define DEFAULT_SERVER_THREADS     3
#define DEFAULT_MAX_CONN           0

static string overall_category = "scribe_overall";
static string log_separator = ":";

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
  /*
  bool perfect_config = true;


  try {
    // Get the config data and parse it.
    // If a file has been explicitly specified we'll take the conf from there,
    // which is very handy for testing and one-off applications.
    // Otherwise we'll try to get it from the service management console and
    // fall back to a default file location. This is for production.
    string config_file;

    if (configFilename.empty()) {
      config_file = DEFAULT_CONF_FILE_LOCATION;
    } else {
      config_file = configFilename;
    }
    localconfig.parseConfig(config_file);
    // overwrite the current StoreConf
    config = localconfig;

    // load the global config
    config.getUnsigned("max_msg_per_second", maxMsgPerSecond);
    config.getUnsignedLongLong("max_queue_size", maxQueueSize);
    config.getUnsigned("check_interval", checkPeriod);
    if (checkPeriod == 0) {
      checkPeriod = 1;
    }
    config.getUnsigned("max_conn", maxConn);

    unsigned long int old_port = port;
    config.getUnsigned("port", port);
    if (old_port != 0 && port != old_port) {
      LOG_OPER("port %lu from conf file overriding old port %lu", port, old_port);
    }
    if (port <= 0) {
      throw runtime_error("No port number configured");
    }

    // check if config sets the size to use for the ThreadManager
    unsigned long int num_threads;
    if (config.getUnsigned("num_thrift_server_threads", num_threads)) {
      numThriftServerThreads = (size_t) num_threads;

      if (numThriftServerThreads <= 0) {
        LOG_OPER("invalid value for num_thrift_server_threads: %lu",
                 num_threads);
        throw runtime_error("invalid value for num_thrift_server_threads");
      }
    }

  } catch(const std::exception& e) {
    string errormsg("Bad config - exception: ");
    errormsg += e.what();
    perfect_config = false;
  }
  */
}

ResultCode::type StreamHandler::AddDataEntryStream(const std::vector<DataEntry, std::allocator<DataEntry> >&) {
  cout << "add data entry" << endl;
  return ResultCode::OK;
}
