#include "include/callback.h"
#include "include/stream_handler.h"
#include "include/ini_parser.h"

#define DEFAULT_CHECK_PERIOD       5
#define DEFAULT_MAX_MSG_PER_SECOND 0
#define DEFAULT_MAX_QUEUE_SIZE     5000000LL
#define DEFAULT_SERVER_THREADS     3
#define DEFAULT_MAX_CONN           0

deepscore::StreamHandler::StreamHandler(int server_port, const deepscore::IniParser& parser)
  : port(server_port),
    numThriftServerThreads(DEFAULT_SERVER_THREADS),
    checkPeriod(DEFAULT_CHECK_PERIOD),
    _ini_parser(parser),
    numMsgLastSecond(0),
    maxMsgPerSecond(DEFAULT_MAX_MSG_PER_SECOND),
    maxConn(DEFAULT_MAX_CONN),
    maxQueueSize(DEFAULT_MAX_QUEUE_SIZE) {
  time(&lastMsgTime);
  _callback_q_ptr = NULL;
  _compute_thread_num = 0;
  _thread_handlers = NULL;
  _compute_units = NULL;
}

deepscore::StreamHandler::~StreamHandler() {
  if(_callback_q_ptr != NULL) {
    delete _callback_q_ptr;
    delete _thread_handlers;
    for (int i=0; i<_compute_thread_num; i++) {
      delete _compute_units[i];
    }
    delete _compute_units;
  }
}

void deepscore::StreamHandler::initialize() {
  _compute_thread_num = _ini_parser.getIntValue("Server.compute_thread_num");
  int fifo_map_size = _ini_parser.getIntValue("Processor.fifo_map_size");

  printf(" compute t num :%d\n", _compute_thread_num);
  _thread_handlers = new pthread_t[_compute_thread_num];
  _compute_units =  new ComputeUnit*[_compute_thread_num];

  //create callback queue
  _callback_q_ptr = new BlockQueue<CallbackMsg>();

  //init callback worker
  Callback* callback = new Callback(_callback_q_ptr);
  pthread_create(&(_callback_handler), NULL, Callback::threadStatic, (void*) callback);
  
  //init compute worker
  for (int i = 0; i < _compute_thread_num; i++) {
    ComputeUnit* compute_unit = new ComputeUnit(_callback_q_ptr, fifo_map_size);
    this->_compute_units[i] = compute_unit;
    pthread_create(&(this->_thread_handlers[i]), NULL, ComputeUnit::threadStatic, (void*) compute_unit);
  }
}

deepscore::ResultCode::type deepscore::StreamHandler::AddDataSliceStream(const std::vector<DataSlice, std::allocator<DataSlice> >& slices) {
  std::vector<DataSlice>::const_iterator citr = slices.begin();
  for (;citr != slices.end(); citr++) {
    uint32_t num = deepscore::strhash::hash32(citr->key.c_str());
    const Slice* slice = new Slice(citr->key, citr->val, citr->number, citr->flag, citr->host, citr->port);
    (_compute_units[num % _compute_thread_num])->addSlice(slice);
  }
  return ResultCode::OK;
}
