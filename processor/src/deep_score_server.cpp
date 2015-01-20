#include "include/common.h"
#include "include/callback.h"
#include "include/compute_unit.h"
#include "include/stream_handler.h"

using boost::shared_ptr;
using namespace deepscore;

shared_ptr<StreamHandler> deepscore::g_Handler;
const deepscore::Slice* deepscore::g_SlicePtr;

void print_usage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [-f config_file]" << std::endl;
}

int main(int argc, char **argv) {

  try {
    /* Increase number of fds */
    /*
    struct rlimit r_fd = {65535,65535};
    if (-1 == setrlimit(RLIMIT_NOFILE, &r_fd)) {
      LOG(ERROR) << "setrlimit error (setting max fd size)";
    }*/

    int next_option;
    const char* const short_options = "hp:f:";
    const struct option long_options[] = {
      { "help",   0, NULL, 'h' },
      { "config", 0, NULL, 'f' },
      { NULL,     0, NULL, 'o' },
    };

    std::string config_file;
    while (0 < (next_option = getopt_long(argc, argv, short_options, long_options, NULL))) {
      switch (next_option) {
      default:
      case 'h':
        print_usage(argv[0]);
        exit(0);
      case 'f':
        config_file = optarg;
        break;
      }
    }

    if (config_file.empty()) {
      print_usage(argv[0]);
      exit(1);
    }


    // seed random number generation with something reasonably unique
    srand(time(NULL) ^ getpid());

    IniParser iniParser;
    iniParser.init(config_file);
    FLAGS_log_dir = iniParser.getStrValue("Logger.log_dir");
    int port = iniParser.getIntValue("Server.port");

    //init logger
    google::InitGoogleLogging(argv[0]);
    
    deepscore::g_SlicePtr = new Slice("X","X",-1,SliceFlag::BROKEN,"X",-1);

    deepscore::g_Handler = shared_ptr<StreamHandler>(new StreamHandler(port, iniParser));
    deepscore::g_Handler->initialize();

    deepscore::startServer(); // never returns

    delete deepscore::g_SlicePtr;

  } catch(const std::exception& e) {
    LOG(FATAL) << "exception in main:" << e.what();
  }

  LOG(FATAL) << "stream server exiting";
  return 0;
}
